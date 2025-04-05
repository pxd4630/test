//��Ź�����Դ,�����ļ���Ҫ���ļ�
#include"ThreadCache.h"
#include"CentralCache.h"
void* ThreadCache:: FetchFromCentralCache(size_t index, size_t size)
{
	//��������
	// ����ʼ���������㷨
	// 1���ʼ����һ����central cacheһ������Ҫ̫�࣬��ΪҪ̫���˿����ò���
	// 2������㲻Ҫ���size��С�ڴ�������ôbatchNum�ͻ᲻��������ֱ������
	// 3��sizeԽ��һ����central cacheҪ��batchNum��ԽС
	// 4��sizeԽС��һ����central cacheҪ��batchNum��Խ��  1
	size_t batchNum = min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));//һ�������������
	if (batchNum == _freeLists[index].MaxSize())
	{
		_freeLists[index].MaxSize() += 1;
	}
	//�и�span			
	void* start = nullptr;
	void* end = nullptr;
	size_t actualNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	if (actualNum == 1)
	{
		assert(start == end);
		return start;
	}
	else
	{
		_freeLists[index].PushRange(Nextobj(start), end,actualNum-1);
		return start;
	}

}

void* ThreadCache::Allocate(size_t size)//----->���϶���  --->ӳ�����
{
	assert(size < MAX_BYTES);
	size_t alignSize = SizeClass::RoundUp(size);//���������ڴ���С
	//size_t index = SizeClass::Index(alignSize);//�ҳ���Ӧ���±귽����н�����ɾ����
	size_t index = SizeClass::Index(size);//�ҳ���Ӧ���±귽����н�����ɾ����


	if (!_freeLists[index].Empty())
	{
		return _freeLists[index].Pop();//
	}
	else
	{
		return FetchFromCentralCache(index, alignSize); //���벻��  ��һ������Ŷ
	}
}

//���ڴ�---->�ҵ���Ӧ�Ĺ�ϣͰ����
void ThreadCache::Deallocate(void* ptr, size_t size)
{
	//cout << "��ʼ�����ڴ�" << endl;
	assert(ptr);
	assert(size <= MAX_BYTES);
	
	size_t index = SizeClass::Index(size);
	_freeLists[index].Push(ptr);

	//������ĳ��ȴ���һ������������ڴ�ʱ�Ϳ�ʼ������һ��list��central cache
	if (_freeLists[index].Size() >= _freeLists[index].MaxSize())
	{
		ListTooLong(_freeLists[index], size);
	}

}

void ThreadCache:: ListTooLong(FreeList& list, size_t size)
{
	void* start = nullptr; 
	void* end = nullptr;
	//��ȡһ���ڴ��ڴ�,start,end������β����ط����䶯
	//printf("list.maxsize: %d\n", list.MaxSize());
	//printf("size: %d\n", list.Size());
	list.PopRange(start, end, list.MaxSize());


	CentralCache::GetInstance()->ReleaseseListToSpans(start, size);
}