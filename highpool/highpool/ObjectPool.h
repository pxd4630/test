#pragma once
#include"Common.h"
using namespace std;
#ifdef _WIN32
#include<windows.h>
#else
// 
#endif
template<class T>
class ObjectPool {

public:
	//申请内存空间 rnmdeex
	T* New()
	{
		T* obj = nullptr;
		if (_freeList != nullptr)//优先从自链表中去内存使用
		{
			void* next = *((void**)_freeList);
			obj =(T*) _freeList;//指针赋给谁,就是让谁指向谁
			_freeList = next;
		}
		else
		{
			//if (_memory == nullptr)//还未开辟一块内存
			if (_remainBytes < sizeof(T))
			{
				_remainBytes = 1024 * 128;//16span
				_memory = (char*)SystemAlloc(_remainBytes >> 13);//申请一块内存,给这个指针,
				//char* _memory = (char*)malloc(_remainBytes);
				if (_memory == nullptr)	//申请空间失败
				{
					throw bad_alloc();
				}
			}
				obj = (T*)_memory;
				//防止申请下来的内存无法存放下一个字节的地址
				size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
				_remainBytes -= objSize;
				_memory += objSize;
			

		}
		//使用定位new调用T的构造函数
		new(obj)T;

		return obj;
	}
	//将使用完的空间换回来,通过一自由链表管理	就是头插
	void Delete(T* obj)
	{
		obj->~T();
		*(void**)obj = _freeList;//存放指针的指向问题
		_freeList = obj;
	}

private:
	char* _memory = nullptr;//指向大内存的指针
	size_t		_remainBytes = 0;//大块内存切分后剩余的字节数
	void* _freeList = nullptr;//存储还换回来的内存

};

//定长内存池与malloc性能测试
//struct TreeNode
//{
//	int _val;
//	TreeNode* _left;
//	TreeNode* _right;
//																                                                    
//	TreeNode() :_val(0),_left(nullptr),_right(nullptr)
//	{
//
//	}
//};
//

//void TestObjectPool()
//{
//	//申请轮询次数
//	const size_t Rounds = 5;
//	//每次轮询释放多少次(申请,销毁的次数)
//	const size_t N = 10000;
//
//	vector<TreeNode*>   v1;
//	v1.reserve(N);
//
//	size_t begin1 = clock();
//	for  (int i = 0; i < Rounds; i++)
//	{
//		for (int k = 0; k < N; k++)
//		{
//			v1.push_back(new TreeNode);//申请内存
//		}
//		for (int k = 0; k < N; k++)
//		{
//			delete v1[k];//释放内存
//		}
//		v1.clear();
//	}
//	size_t end1 = clock(); 
//
//	vector<TreeNode*> v2;
//	v2.reserve(10000);
//	ObjectPool<TreeNode> TNPool;
//	size_t begin2 = clock();
//	for (int i = 0; i < Rounds; i++)   
//	{
//		for (int k = 0; k < N; k++)
//		{
//			v2.push_back(TNPool.New());//申请内存
//		}
//		for (int k = 0; k < N; k++)
//		{
//			TNPool.Delete(v2[k]);//释放内存
//		}
//		v2.clear();
//	}
//	size_t end2 = clock();
//	cout << "new cost time:" << end1 - begin1 << endl;
//	cout << "object pool cost time:" << end2 - begin2 << endl;
//
//
//
//}
