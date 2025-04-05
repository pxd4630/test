#include "CentralCache.h"
#include "PageCache.h"
CentralCache CentralCache::_sInst;

Span* CentralCache::GetOneSpan(SpanList& list, size_t byte_size)
{
	//�鿴��ǰ��spanlist�Ƿ���δ����Ķ���
	//
	Span* it = list.Begin();
	//����Ƿ����
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

	//�Ȱ�centralcache��Ͱ���ͷ�,Ϊ�˻��������̶߳�span��������ڴ�,�������
	list._mtx.unlock();
	//span������pagecache����
	//������ֹ�̳߳�ͻ
	PageCache::GetInstance()->_pageMtx.lock();
	//����span
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(byte_size));
	span->_objSize = byte_size;
	//cout << span->_pageId << endl;
	span->_isuse = true;
	PageCache::GetInstance()->_pageMtx.unlock();

	//�Ի�ȡ��span�����з�,���ü���,�����̷߳��ʲ���

	//����span����ڴ����ʼ��ַ�ʹ���ڴ�ĵĴ�С
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
	//�к�span,�ҵ�Ͱ��ʱ�����
	list._mtx.lock();
	list.PushFront(span);

	return span;

}

//�����Ļ����ȡһ�������Ļ����threadcache
	//���Ļ����Ƿ��㹻������
size_t CentralCache:: FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	size_t index = SizeClass::Index(size); //��Ͱ��λ��
	//������Դ,������Դ,����
	_spanLists[index]._mtx.lock();
	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span);
	assert(span->_freeList);
	//��span�л��batchNum������
	//����,�ж����ö���
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
//centralpage��threadcache��ӳ�������ͬ
//���յ�span
void CentralCache::ReleaseseListToSpans(void* start, size_t byte_size)
{
	size_t index = SizeClass::Index(byte_size);
	//������ֹһ�߻�����Դһ��������Դ
	_spanLists[index]._mtx.lock();
	while (start)
	{
		void* next = Nextobj(start);
		//�ڴ�鷵��span,ͨ��ӳ�����С�ڴ���Ӧ��span
		Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
		Nextobj(start) = span->_freeList;
		span->_freeList = start;
		span->_useCount--;


		//��usecount--==0,˵�����������С�ڴ����ȫ����������
		if (span->_useCount == 0)
		{
			_spanLists[index].Erase(span);
			span->_freeList = nullptr;
			span->_next = nullptr;
			span->_prev = nullptr;

			// �ͷ�span��page cacheʱ��ʹ��page cache�����Ϳ�����
			// ��ʱ��Ͱ�����
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