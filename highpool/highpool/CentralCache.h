#pragma once
#include"Common.h"
//����ģʽ  ����ģʽ
class CentralCache
{
public:
	static CentralCache* GetInstance()
	{
		return &_sInst;//�����ÿ��ԶԶ������д����
	}

	//���һ���ǿյ�span
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	//�����Ļ����ȡһ�������Ļ����threadcache
	//���Ļ����Ƿ��㹻������
	size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size);

	void ReleaseseListToSpans(void* start, size_t byte_size);
private:
	SpanList	_spanLists[NFREELIST];
private:
	CentralCache()
	{

	}
	CentralCache(const CentralCache&) = delete;//������п�������

	static CentralCache _sInst;
};
