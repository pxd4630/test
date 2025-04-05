#pragma once
#include"Common.h"
//单例模式  饿汉模式
class CentralCache
{
public:
	static CentralCache* GetInstance()
	{
		return &_sInst;//加引用可以对对象进行写操作
	}

	//获得一个非空的span
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	//从中心缓存获取一定数量的缓存给threadcache
	//中心缓存是否足够申请了
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);

	void ReleaseseListToSpans(void* start, size_t byte_size);
private:
	SpanList	_spanLists[NFREELIST];
private:
	CentralCache()
	{

	}
	CentralCache(const CentralCache&) = delete;//避免进行拷贝构造

	static CentralCache _sInst;
};
