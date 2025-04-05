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
static const size_t MAX_BYTES = 1024 * 256;//256KB 32ҳ
static const size_t NFREELIST = 208;//Ͱ������ ����Ӧ����208��Ͱ
static const size_t NPAGES = 129;//panͰ����
static const size_t PAGE_SHIFT = 13;
#ifdef _WIN64  //WIN64����WIN32
typedef unsigned long long	PAGE_ID;
#elif _WIN32
typedef size_t PAGE_ID;
#else
//linux
#endif
//����ƽ̨��ֲ��
#ifdef _WIN32
#include<windows.h>
#else
// 
#endif

// �����ڴ��
//template<size_t N>
//class ObjectPool
//{};

// ֱ��ȥ���ϰ�ҳ����ռ�
inline static void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	//cout << "�������" << endl;
	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); //8k 
#else
	// linux��brk mmap��
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
	// sbrk unmmap��
#endif
}



//�� Nextobj �����в�ʹ������ (&) ��ֱ�ӷ���ֵʱ���������ص���һ������������ԭʼָ������á�
// ����ζ�����޸ĵ������������������ʵ�ʴ洢�ڶ����е�ָ�롣��ˣ��κζԷ���ֵ���޸Ķ�����Ӱ�쵽ԭʼ���ݡ�
static void*& Nextobj(void* obj)
{
	return *(void**)obj;
}

//������зֺõ�С�������������		
class FreeList {
public:
	//�϶����������ڴ��͹黹�ڴ��

	//�黹�ڴ�  ͷ��---->֪����һ���ڵ�ĵ�ַ
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

	//����ʹ���ڴ� ͷɾ
	void* Pop()
	{
		assert(_freeList); //�����ڴ�--->�������ڴ���?
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

//��������С�Ķ���ӳ�����
class SizeClass {

public:
	// ������������10%���ҵ�����Ƭ�˷�
	// [1,128]					8byte����	    freelist[0,16)
	// [128+1,1024]				16byte����	    freelist[16,72)
	// [1024+1,8*1024]			128byte����	    freelist[72,128)
	// [8*1024+1,64*1024]		1024byte����     freelist[128,184)
	// [64*1024+1,256*1024]		8*1024byte����   freelist[184,208)

	//static inline size_t _RoundUp(size_t bytes, size_t alignNum)//��������������ֽڴ�С
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

	static inline size_t RoundUp(size_t size)//������Ҫ������ֽڴ�С   ----->��������
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
			//���е���
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

	// ����ӳ�����һ����������Ͱ  /���λ�ù�ϵ
	static inline size_t Index(size_t bytes)
	{
		assert(bytes <= MAX_BYTES);

		// ÿ�������ж��ٸ���
		//ʹ�õ�һ������return _index((bytes-minbytes),alignNum)
		//�±��0��ʼ
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
	//ȷ��������
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
	//�Ż�
	// ����һ����ϵͳ��ȡ����ҳ
	// �������� 8byte
	// ...
	// �������� 256KB
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



//����������ҳ����ڴ���
struct Span {
	PAGE_ID		_pageId = 0;//����ڴ����ʼҳ��
	size_t		_n=0;//ҳ������
	Span*		_next = nullptr;//˫������,�������,����pagecache���л���
	Span*		_prev = nullptr;
	size_t		_objSize = 0;//
	size_t		_useCount=0;//�кõ�С���ڴ�,�����threadcache�ļ���      һ��ҳ���������
	void*		_freeList=nullptr;//�кõ�С���ڴ�������б�,���ǹ���С���ڴ�
	bool		_isuse = false;//��ǰ��span�Ƿ����ڱ�ʹ��
};

//��ͷ˫��ѭ������
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
	std::mutex _mtx;//Ͱ��

};