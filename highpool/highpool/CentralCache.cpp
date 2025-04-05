#include "CentralCache.h"
#include "PageCache.h"
CentralCache CentralCache::_sInst;

Span* CentralCache::GetOneSpan(SpanList& list, size_t byte_size)
{
	//查看当前的spanlist是否还有未分配的对象
	//
	Span* it = list.Begin();
	//检查是否存在
	while (it != list.End())
	{
		if (it->_freeList != nullptr)
		{
			return it;
		 }
		else
		{
			it = it->_next;
		}
	}

	//先把centralcache的桶锁释放,为了回收其他线程对span的申请的内存,不会堵塞
	list._mtx.unlock();
	//span不足向pagecache申请
	//加锁防止线程冲突
	PageCache::GetInstance()->_pageMtx.lock();
	//申请span
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(byte_size));
	span->_objSize = byte_size;
	//cout << span->_pageId << endl;
	span->_isuse = true;
	PageCache::GetInstance()->_pageMtx.unlock();

	//对获取的span进行切分,不用加锁,其他线程访问不到

	//计算span大块内存的起始地址和大块内存的的大小
	char* start = (char*)(span->_pageId << PAGE_SHIFT);
	size_t bytes = span->_n << PAGE_SHIFT;
	//printf("bytes: %d\n", bytes);
	char* end = start + bytes;
	span->_freeList = start;  
	start += byte_size;
	void* tail = span->_freeList;
	int i = 0;
	while (start < end)
	{
		Nextobj(tail) = start;
		tail = Nextobj(tail); //tail=start
		start += byte_size;
	}
	Nextobj(tail) = nullptr;
	//切好span,挂到桶的时候加锁
	list._mtx.lock();
	list.PushFront(span);

	return span;

}

//从中心缓存获取一定数量的缓存给threadcache
	//中心缓存是否足够申请了
size_t CentralCache:: FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	size_t index = SizeClass::Index(size); //找桶的位置
	//中心资源,共享资源,上锁
	_spanLists[index]._mtx.lock();
	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span);
	assert(span->_freeList);
	//从span中获得batchNum个对象
	//不够,有多少拿多少
	start = span->_freeList;
	end = start;
	size_t i = 0;
	size_t actualNum = 1;
	while (i < batchNum - 1 &&  Nextobj(end) != nullptr)
	{
		end = Nextobj(end);
		++i;
		++actualNum;
	}
	span->_freeList = Nextobj(end);
	Nextobj(end) = nullptr;
	span->_useCount += actualNum;

	_spanLists[index]._mtx.unlock();

	return actualNum;

}
//centralpage与threadcache的映射规则相同
//回收到span
void CentralCache::ReleaseseListToSpans(void* start, size_t byte_size)
{
	size_t index = SizeClass::Index(byte_size);
	//上锁防止一边回收资源一边申请资源
	_spanLists[index]._mtx.lock();
	while (start)
	{
		void* next = Nextobj(start);
		//内存块返回span,通过映射查找小内存块对应的span
		Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
		Nextobj(start) = span->_freeList;
		span->_freeList = start;
		span->_useCount--;


		//当usecount--==0,说明被申请出的小内存碎块全部换回来了
		if (span->_useCount == 0)
		{
			_spanLists[index].Erase(span);
			span->_freeList = nullptr;
			span->_next = nullptr;
			span->_prev = nullptr;

			// 释放span给page cache时，使用page cache的锁就可以了
			// 这时把桶锁解掉
			_spanLists[index]._mtx.unlock();
			PageCache::GetInstance()->_pageMtx.lock();
			PageCache::GetInstance()->ReleaseSpanToPageCache(span);
			PageCache::GetInstance()->_pageMtx.unlock();
			_spanLists[index]._mtx.lock();
		}
	
		start = next;
	}
	_spanLists[index]._mtx.unlock();
}