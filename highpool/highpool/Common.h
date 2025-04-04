#pragma once
#include <iostream>
#include <list>
#include <time.h> 
#include <vector>
#include <cassert>
#include <thread>
#include <algorithm>
#include <mutex>
#include<unordered_map>
//#include"ObjectPool.h"
using namespace std;
static const size_t MAX_BYTES = 1024 * 256;//256KB 32页
static const size_t NFREELIST = 208;//桶的数量 这里应该是208个桶
static const size_t NPAGES = 129;//pan桶数量
static const size_t PAGE_SHIFT = 13;
#ifdef _WIN64  //WIN64兼容WIN32
typedef unsigned long long	PAGE_ID;
#elif _WIN32
typedef size_t PAGE_ID;
#else
//linux
#endif
//考虑平台移植性
#ifdef _WIN32
#include<windows.h>
#else
// 
#endif

// 定长内存池
//template<size_t N>
//class ObjectPool
//{};

// 直接去堆上按页申请空间
inline static void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	//cout << "向堆申请" << endl;
	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); //8k 
#else
	// linux下brk mmap等
#endif

	if (ptr == nullptr)
		throw std::bad_alloc();

	return ptr;
}


inline static void SystemFree(void* ptr)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	// sbrk unmmap等
#endif
}



//在 Nextobj 函数中不使用引用 (&) 而直接返回值时，函数返回的是一个拷贝而不是原始指针的引用。
// 这意味着你修改的是这个拷贝，而不是实际存储在对象中的指针。因此，任何对返回值的修改都不会影响到原始数据。
static void*& Nextobj(void* obj)
{
	return *(void**)obj;
}

//管理好切分好的小对象的自由链表		
class FreeList {
public:
	//肯定得有申请内存块和归还内存块

	//归还内存  头插---->知道下一个节点的地址
	void Push(void* obj)
	{
		Nextobj(obj) = _freeList;
		_freeList = obj;
		++_size;
	}

	void PushRange(void* start, void* end,size_t n)
	{
		Nextobj(end) = _freeList;
		_freeList = start;
		_size += n;
	}
	void PopRange(void*& start, void*& end, size_t n)
	{
		assert(n <= _size);
		start = _freeList;
		end = start;
		for (size_t i = 0; i < n-1; i++)
		{
			end = Nextobj(end);
		}
		_freeList = Nextobj(end);
		Nextobj(end) = nullptr;
		_size -= n;
	}

	//申请使用内存 头删
	void* Pop()
	{
		assert(_freeList); //申请内存--->这里有内存吗?
		void* obj = _freeList;
		_freeList = Nextobj(obj);
		--_size;
		return obj;
	}

	

	bool Empty()
	{
		return _freeList == nullptr;
	}

	size_t& MaxSize()
	{
		return _MaxSize;
	}

	size_t Size()
	{
		return _size;
	}



private:
	void* _freeList=nullptr;
	size_t _MaxSize = 1;
	size_t	_size = 0;
};

//计算对象大小的对齐映射规则
class SizeClass {

public:
	// 整体控制在最多10%左右的内碎片浪费
	// [1,128]					8byte对齐	    freelist[0,16)
	// [128+1,1024]				16byte对齐	    freelist[16,72)
	// [1024+1,8*1024]			128byte对齐	    freelist[72,128)
	// [8*1024+1,64*1024]		1024byte对齐     freelist[128,184)
	// [64*1024+1,256*1024]		8*1024byte对齐   freelist[184,208)

	//static inline size_t _RoundUp(size_t bytes, size_t alignNum)//返回最终申请的字节大小
	//{
	//	if (bytes % alignNum != 0)
	//	{
	//		bytes = (bytes / alignNum + 1) * alignNum;
	//		return bytes;
	//	}
	//	else
	//	{
	//		return bytes;
	//	}
	//}

	static inline size_t _RoundUp(size_t bytes, size_t alignNum)
	{
		int ret = ((bytes + alignNum - 1) & ~(alignNum - 1));
		return ((bytes + alignNum - 1) & ~(alignNum - 1));
		//((bytes + alignNum - 1) & ~(alignNum - 1)
	}

	static inline size_t RoundUp(size_t size)//接受需要申请的字节大小   ----->划分区域
	{
		if (size <= 128)
		{
			return _RoundUp(size, 8);
		}
		else if (size <= 1024)
		{
			return _RoundUp(size, 16);
		}
		else if (size <= 8 * 1024)
		{
			return _RoundUp(size, 128);
		}
		else if (size <= 64 * 1024)
		{
			return _RoundUp(size, 1024);
		}
		else if (size <= 256 * 1024)
		{
			return _RoundUp(size, 8 * 1024);
		}
		else
		{
			//自行调控
			return _RoundUp(size, 1 << PAGE_SHIFT);
		}
	}
	/*size_t _Index(size_t bytes, size_t alignNum)
	{
	if (bytes % alignNum == 0)
	{
	return bytes / alignNum - 1;
	}
	else
	{
	return bytes / alignNum;
	}
	}*/

	static inline size_t _Index(size_t bytes, size_t align_shift)
	{
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	// 计算映射的哪一个自由链表桶  /相对位置关系
	static inline size_t Index(size_t bytes)
	{
		assert(bytes <= MAX_BYTES);

		// 每个区间有多少个链
		//使用第一个方法return _index((bytes-minbytes),alignNum)
		//下标从0开始
		static int group_array[4] = { 16, 56, 56, 56 };
		if (bytes <= 128) {
			return _Index(bytes, 3);
		}
		else if (bytes <= 1024) {
			return _Index(bytes - 128, 4) + group_array[0];
		}
		else if (bytes <= 8 * 1024) {
			return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (bytes <= 64 * 1024) {
			return _Index(bytes - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
		}
		else if (bytes <= 256 * 1024) {
			return _Index(bytes - 64 * 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
		}
		else {
			assert(false);
		}

		return -1;
	}
	//确定上下限
	static size_t NumMoveSize(size_t size)
	{
		assert(size > 0);
		int num = MAX_BYTES / size;
		if (num < 2)
			num = 2;
		if (num > 512)
			num = 512;

		return num;
	}
	//优化
	// 计算一次向系统获取几个页
	// 单个对象 8byte
	// ...
	// 单个对象 256KB
	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);
		size_t npage = num * size;

		npage >>= PAGE_SHIFT;
		if (npage == 0)
			npage = 1;

		return npage;
	}
};



//管理多个连续页大块内存跨度
struct Span {
	PAGE_ID		_pageId = 0;//大块内存的起始页号
	size_t		_n=0;//页的数量
	Span*		_next = nullptr;//双向链表,方便管理,后续pagecache进行回收
	Span*		_prev = nullptr;
	size_t		_objSize = 0;//
	size_t		_useCount=0;//切好的小块内存,分配给threadcache的计数      一个页分配的数量
	void*		_freeList=nullptr;//切好的小块内存的自由列表,就是管理小块内存
	bool		_isuse = false;//当前的span是否正在被使用
};

//带头双向循环链表
class SpanList {

public:
	SpanList()
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	bool Empty()
	{
		return _head->_next == _head;
	}

	void Insert(Span* pos, Span* newSpan)
	{
		assert(pos);
		assert(newSpan);
		Span* prev = pos->_prev;
		prev->_next = newSpan;
		newSpan->_prev = prev;
		newSpan->_next = pos;
		pos->_prev = newSpan;
	}

	void Erase(Span* pos)
	{
		assert(pos);
		if (pos == _head)
		{
			int a = 10;
		}
		Span* prev = pos->_prev;
		Span* next = pos->_next;
		prev->_next = next;
		next->_prev = prev;
	}

	Span* Begin()
	{
		return _head->_next;
	}

	Span* End()
	{
		return _head;
	}

	void PushFront(Span* span)
	{
		Insert(Begin(), span);
	}

	Span* PopFront()
	{
		Span* front = _head->_next;
		Erase(front);
		return front;
	}

private:
	Span* _head;
public:
	std::mutex _mtx;//桶锁

};