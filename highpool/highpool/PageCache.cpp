#include"PageCache.h"
#include"Common.h"
//ʵ��������
PageCache PageCache::_sInst;
Span* PageCache::NewSpan(size_t k)
{
	assert(k > 0);
	//һ���������128ҳֱ���������

	if (k > NPAGES-1)
	{
		void* ptr = SystemAlloc(k);
	//	Span* span = new Span;
		Span* span = _spanpool.New();
		span->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;//�����ʼҳ��
		span->_n = k;
		//_idSpanMap[span->_pageId] = span;
		_idSpanMap.set(span->_pageId, span);
		return span;
	}


	//�ȼ���ӳ�ĵ�k��Ͱ������û��span
	if (!_spanLists[k].Empty())
	{
		Span* kSpan = _spanLists[k].PopFront();

		for (int i = 0; i < kSpan->_n; ++i)
		{
			//_idSpanMap[kSpan->_pageId + i] = kSpan;
			_idSpanMap.set(kSpan->_pageId + i, kSpan);
		}

		return kSpan; //��д����  ��������  Ȼ������ӳ�仹û�����
	}

	//�������Ͱ������û��span
	//��---->�����з�
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
		//��nspanͷ������Kҳ
			//Kҳspan����
			//nspan�ҵ���Ӧ��ӳ��λ��
			//ҳ�ŵ���ʼλ��,ҳ�������ı仯
			kSpan->_pageId = nSpan->_pageId;
			kSpan->_n = k;
			nSpan->_pageId += k;
			nSpan->_n -= k;
			_spanLists[nSpan->_n].PushFront(nSpan);
			//����id��span��ӳ��,����central cache����С�ڴ�,���Ҷ�Ӧ��span
			for (int i = 0; i < kSpan->_n; ++i)
			{
				//_idSpanMap[kSpan->_pageId + i] = kSpan;
				_idSpanMap.set(kSpan->_pageId + i, kSpan);
			}

			return kSpan;
		}
	}
	//cout << "888888888" << endl;
	// �ߵ����λ�þ�˵������û�д�ҳ��span��
	// ��ʱ��ȥ�Ҷ�Ҫһ��128ҳ��span
	//Span* bigSpan = new Span;
	Span* bigSpan = _spanpool.New();
	void* ptr = SystemAlloc(NPAGES - 1);
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT;
	//============================================
	//cout <<"bigspan����ҳ��:    " << bigSpan->_pageId << endl;
	bigSpan->_n = NPAGES - 1;

	_spanLists[bigSpan->_n].PushFront(bigSpan);

	return NewSpan(k);
}


Span* PageCache::MapObjectToSpan(void* obj)
{
	PAGE_ID id = (PAGE_ID)obj >> PAGE_SHIFT;//�ҵ���Ӧ�ĵ�ַҳ��
	//std::unique_lock<mutex> lock(_pageMtx);
	//auto ret = _idSpanMap.find(id);
	//if (ret != _idSpanMap.end())//�����ҵ���Ӧ��span��
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
	//���һ���Ի������ڴ����128ҳ,ֱ�ӻ�����
	if (span->_n > NPAGES - 1)
	{
		//����Ҫ�ҵ�Ҫ������ȥ�ڴ�ĵ�ַ--->�ҵ�����ҳ��ַ
		void* ptr = (void*)(span->_pageId << PAGE_SHIFT);
		SystemFree(ptr);
		//delete span;
		_spanpool.Delete(span);
		return;
	}

	//��spanǰ��ҳ���кϲ�
	while (1)
	{
		PAGE_ID prevId = span->_pageId - 1;
		//auto ret = _idSpanMap.find(prevId);
		////ǰ���ҳ��û��,���ϲ���
		//if (ret == _idSpanMap.end())
		//{
		//	break;
		//}
		auto ret = (Span*)_idSpanMap.get(prevId);
		if (ret == nullptr)
		{
			break;
		}

		//ǰ������ҳ��span��ʹ��,���ܽ��кϲ�
		Span* prevSpan = ret;
		if (prevSpan->_isuse == true)
		{
			break;
		}

		//�ϲ�����128ҳ��spanû�취����,�ϲ�����
		if (prevSpan->_n + span->_n > NPAGES - 1)
		{
			break;
		}
		//pageId����λ��,n������С
		span->_pageId = prevSpan->_pageId;
		span->_n += prevSpan->_n;
		//��span�ϲ���,ɾ��֮ǰλ�õ�span��ֹ�ڱ�����
		_spanLists[prevSpan->_n].Erase(prevSpan);
		//delete prevSpan;
		_spanpool.Delete(prevSpan);
	}

	//���ϲ�
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
	//���½���ӳ��
	/*_idSpanMap[span->_pageId] = span;
	_idSpanMap[span->_pageId + span->_n - 1] = span;*///����β
	_idSpanMap.set(span->_pageId, span);
	_idSpanMap.set(span->_pageId + span->_n - 1, span);
}