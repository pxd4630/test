//存放公共资源,所有文件需要的文件
#include"ThreadCache.h"
#include"CentralCache.h"
void* ThreadCache:: FetchFromCentralCache(size_t index, size_t size)
{
	//反馈调节
	// 慢开始反馈调节算法
	// 1、最开始不会一次向central cache一次批量要太多，因为要太多了可能用不完
	// 2、如果你不要这个size大小内存需求，那么batchNum就会不断增长，直到上限
	// 3、size越大，一次向central cache要的batchNum就越小
	// 4、size越小，一次向central cache要的batchNum就越大  1
	size_t batchNum = min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));//一批次申请的数量
	if (batchNum == _freeLists[index].MaxSize())
	{
		_freeLists[index].MaxSize() += 1;
	}
	//切割span			
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

void* ThreadCache::Allocate(size_t size)//----->向上对齐  --->映射规则
{
	assert(size < MAX_BYTES);
	size_t alignSize = SizeClass::RoundUp(size);//求出申请的内存块大小
	//size_t index = SizeClass::Index(alignSize);//找出对应的下标方便进行进行增删操作
	size_t index = SizeClass::Index(size);//找出对应的下标方便进行进行增删操作


	if (!_freeLists[index].Empty())
	{
		return _freeLists[index].Pop();//
	}
	else
	{
		return FetchFromCentralCache(index, alignSize); //申请不到  第一个问题哦
	}
}

//还内存---->找到对应的哈希桶插入
void ThreadCache::Deallocate(void* ptr, size_t size)
{
	//cout << "开始回收内存" << endl;
	assert(ptr);
	assert(size <= MAX_BYTES);
	
	size_t index = SizeClass::Index(size);
	_freeLists[index].Push(ptr);

	//当链表的长度大于一次批量申请的内存时就开始还回来一段list给central cache
	if (_freeLists[index].Size() >= _freeLists[index].MaxSize())
	{
		ListTooLong(_freeLists[index], size);
	}

}

void ThreadCache:: ListTooLong(FreeList& list, size_t size)
{
	void* start = nullptr; 
	void* end = nullptr;
	//截取一段内存内存,start,end是输出形参数回发生变动
	//printf("list.maxsize: %d\n", list.MaxSize());
	//printf("size: %d\n", list.Size());
	list.PopRange(start, end, list.MaxSize());


	CentralCache::GetInstance()->ReleaseseListToSpans(start, size);
}