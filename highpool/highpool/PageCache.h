#pragma once
#include"Common.h"
#include"ObjectPool.h"
#include"PageMap.h"
//����ģʽ
class PageCache {
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}

	Span* NewSpan(size_t k);

	//��ȡ����(����С���ڴ�)��span��ӳ��  threadcaceh---->centralcache
	Span* MapObjectToSpan(void* obj);//����С���ڴ�

	//�ͷſ��е�span��pagecache,�ϲ����ڵ�span
	void ReleaseSpanToPageCache(Span* span);
 
	std::mutex _pageMtx;
private:
	PageCache()
	{ }
	PageCache(const PageCache&) = delete;
private:
	ObjectPool<Span> _spanpool;
	//std::unordered_map<PAGE_ID, Span*> _idSpanMap;
	//std::unordered_map<PAGE_ID, size_t> _isSizeMap;//�����
	TCMalloc_PageMap1<32-PAGE_SHIFT> _idSpanMap;
	SpanList	_spanLists[NPAGES];

	static PageCache _sInst;
};