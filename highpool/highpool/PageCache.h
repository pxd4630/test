#pragma once
#include"Common.h"
#include"ObjectPool.h"
#include"PageMap.h"
//单例模式
class PageCache {
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}

	Span* NewSpan(size_t k);

	//获取对象(就是小块内存)到span的映射  threadcaceh---->centralcache
	Span* MapObjectToSpan(void* obj);//就是小块内存

	//释放空闲的span到pagecache,合并相邻的span
	void ReleaseSpanToPageCache(Span* span);
 
	std::mutex _pageMtx;
private:
	PageCache()
	{ }
	PageCache(const PageCache&) = delete;
private:
	ObjectPool<Span> _spanpool;
	//std::unordered_map<PAGE_ID, Span*> _idSpanMap;
	//std::unordered_map<PAGE_ID, size_t> _isSizeMap;//做标记
	TCMalloc_PageMap1<32-PAGE_SHIFT> _idSpanMap;
	SpanList	_spanLists[NPAGES];

	static PageCache _sInst;
};