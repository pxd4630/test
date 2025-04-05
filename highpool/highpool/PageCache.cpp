#include"PageCache.h"
#include"Common.h"
//实例化对象
PageCache PageCache::_sInst;
Span* PageCache::NewSpan(size_t k)
{
	assert(k > 0);
	//一次申请大于128页直接向堆申请

	if (k > NPAGES-1)
	{
		void* ptr = SystemAlloc(k);
	//	Span* span = new Span;
		Span* span = _spanpool.New();
		span->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;//获得起始页号
		span->_n = k;
		//_idSpanMap[span->_pageId] = span;
		_idSpanMap.set(span->_pageId, span);
		return span;
	}


	//先检查对映的第k个桶里面有没有span
	if (!_spanLists[k].Empty())
	{
		Span* kSpan = _spanLists[k].PopFront();

		for (int i = 0; i < kSpan->_n; ++i)
		{
			//_idSpanMap[kSpan->_pageId + i] = kSpan;
			_idSpanMap.set(kSpan->_pageId + i, kSpan);
		}

		return kSpan; //是写错了  看到了吗  然后这里映射还没有添加
	}

	//检查后面的桶里面有没有span
	//有---->进行切分
	//int ret = 0;
	for (size_t i = k + 1; i < NPAGES; ++i)
	{
		//ret++;
		//cout << &_spanLists[i] << endl;
		//cout << ret << endl;
		if (!_spanLists[i].Empty())
		{
			Span* nSpan = _spanLists[i].PopFront();
		//	Span* kSpan = new Span;
			Span* kSpan = _spanpool.New();
		//再nspan头部切下K页
			//K页span返回
			//nspan挂到对应的映射位置
			//页号的起始位置,页的数量的变化
			kSpan->_pageId = nSpan->_pageId;
			kSpan->_n = k;
			nSpan->_pageId += k;
			nSpan->_n -= k;
			_spanLists[nSpan->_n].PushFront(nSpan);
			//建立id和span的映射,方便central cache回收小内存,查找对应的span
			for (int i = 0; i < kSpan->_n; ++i)
			{
				//_idSpanMap[kSpan->_pageId + i] = kSpan;
				_idSpanMap.set(kSpan->_pageId + i, kSpan);
			}

			return kSpan;
		}
	}
	//cout << "888888888" << endl;
	// 走到这个位置就说明后面没有大页的span了
	// 这时就去找堆要一个128页的span
	//Span* bigSpan = new Span;
	Span* bigSpan = _spanpool.New();
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	//============================================
	//cout <<"bigspan申请页号:    " << bigSpan->_pageId << endl;
	bigSpan->_n = NPAGES - 1;

	_spanLists[bigSpan->_n].PushFront(bigSpan);

	return NewSpan(k);
}


Span* PageCache::MapObjectToSpan(void* obj)
{
	PAGE_ID id = (PAGE_ID)obj >> PAGE_SHIFT;//找到对应的地址页号
	//std::unique_lock<mutex> lock(_pageMtx);
	//auto ret = _idSpanMap.find(id);
	//if (ret != _idSpanMap.end())//就是找到对应的span了
	//{
	//	return ret->second;
	//}
	//else
	//{
	//	assert(false);
	//	return nullptr;
	//}
	auto ret = (Span*)_idSpanMap.get(id);
	assert(ret!= nullptr);
	return ret;
}

void PageCache::ReleaseSpanToPageCache(Span* span)
{
	//如果一次性还来的内存大于128页,直接还给堆
	if (span->_n > NPAGES - 1)
	{
		//首先要找到要换还回去内存的地址--->找到它的页地址
		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		SystemFree(ptr);
		//delete span;
		_spanpool.Delete(span);
		return;
	}

	//对span前后页进行合并
	while (1)
	{
		PAGE_ID prevId = span->_pageId - 1;
		//auto ret = _idSpanMap.find(prevId);
		////前面的页号没有,不合并了
		//if (ret == _idSpanMap.end())
		//{
		//	break;
		//}
		auto ret = (Span*)_idSpanMap.get(prevId);
		if (ret == nullptr)
		{
			break;
		}

		//前面相邻页的span再使用,不能进行合并
		Span* prevSpan = ret;
		if (prevSpan->_isuse == true)
		{
			break;
		}

		//合并超过128页的span没办法管理,合并不了
		if (prevSpan->_n + span->_n > NPAGES - 1)
		{
			break;
		}
		//pageId决定位置,n决定大小
		span->_pageId = prevSpan->_pageId;
		span->_n += prevSpan->_n;
		//将span合并后,删除之前位置的span防止在被申请
		_spanLists[prevSpan->_n].Erase(prevSpan);
		//delete prevSpan;
		_spanpool.Delete(prevSpan);
	}

	//向后合并
	while (1)
	{
		PAGE_ID nextId = span->_pageId+span->_n;
		/*auto ret = _idSpanMap.find(nextId);
		if (ret == _idSpanMap.end())
		{
			break;
		}*/
		auto ret = (Span*)_idSpanMap.get(nextId);
		if (ret == nullptr)
		{
			break;
		}
		Span* nextSpan = ret;
		if (nextSpan->_isuse == true)
		{
			break;
		}
		if (nextSpan->_n + span->_n > NPAGES-1)
		{
			break;
		}
		span->_n += nextSpan->_n;
		_spanLists[nextSpan->_n].Erase(nextSpan);
		//delete nextSpan;
		_spanpool.Delete(nextSpan);
	}
	_spanLists[span->_n].PushFront(span);
	span->_isuse = false;
	//重新建立映射
	/*_idSpanMap[span->_pageId] = span;
	_idSpanMap[span->_pageId + span->_n - 1] = span;*///存首尾
	_idSpanMap.set(span->_pageId, span);
	_idSpanMap.set(span->_pageId + span->_n - 1, span);
}