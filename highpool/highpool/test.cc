#pragma once
#include"Common.h"
#include"ConcurrentAlloc.h"
#include"ObjectPool.h"
void Alloc1()
{
	for (size_t i = 0; i < 5; ++i)
	{
		void* ptr = ConcurrentAlloc(6);
	}
}

void Alloc2()
{
	for (size_t i = 0; i < 5; ++i)
	{
		void* ptr = ConcurrentAlloc(7);
	}
}



void TLSTest()
{
	std::thread t1(Alloc1);
	t1.join();

	std::thread t2(Alloc2);
	t2.join();	
}


void TestConcurrentAlloc1()
{
	void* p1 = ConcurrentAlloc(6);
	void* p2 = ConcurrentAlloc(8);
	void* p3 = ConcurrentAlloc(1);
	void* p4 = ConcurrentAlloc(7);
	void* p5 = ConcurrentAlloc(8);
	void* p6 = ConcurrentAlloc(8);
	void* p7 = ConcurrentAlloc(8);


	cout << p1 << endl;
	cout << p2 << endl;
	cout << p3 << endl;
	cout << p4 << endl;
	cout << p5 << endl;
	cout << p6 << endl;
	cout << p7 << endl;


	ConcurrentFree(p1);
	ConcurrentFree(p2);
	ConcurrentFree(p3);
	ConcurrentFree(p4);
	ConcurrentFree(p5);
	ConcurrentFree(p6);
	ConcurrentFree(p7);
}

void TestConcurrentAlloc2()
{
	for (size_t i = 0; i < 1024; ++i)
	{
		void* p1 = ConcurrentAlloc(6);
		cout << p1 << endl;
	}

	void* p2 = ConcurrentAlloc(8);
	cout << p2 << endl;
}
void MultiThreadAlloc1()
{
	std::vector<void*> v;
	for (size_t i = 0; i < 7; ++i)
	{
		void* ptr = ConcurrentAlloc(6);
		v.push_back(ptr);
	}

	int ret = 0;
	for (auto e : v)
	{
		cout << ret<<"ret" << endl;
		ret++;
		ConcurrentFree(e);
	}
}

void MultiThreadAlloc2()
{
	std::vector<void*> v;
	for (size_t i = 0; i < 7; ++i)
	{
		void* ptr = ConcurrentAlloc(16);
		v.push_back(ptr);
	}

	for (auto e : v)
	{
		ConcurrentFree(e);
	}
}


void BigAlloc()
{
	void* p1 = ConcurrentAlloc(257 * 1024);
	ConcurrentFree(p1);
	void* p2 = ConcurrentAlloc(1024 * 8 * 129);
	ConcurrentFree(p2);
}

void TestMultiThread()
{
	std::thread t1(MultiThreadAlloc1);
	//std::thread t2(MultiThreadAlloc2);

	t1.join();
	//t2.join();
}
//优化1
//一次申请超过threadcache的最大内存
//优化2
//不使用new,从定长内存池中申请,析构
//优化回收小块内存


//int main()
//{
////	TestObjectPool();
//	//TLSTest();
//	//TestConcurrentAlloc1();
//	TestMultiThread();
//	//TestConcurrentAlloc2();
//
////	BigAlloc();
//	return 0;
//}