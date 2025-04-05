#pragma once
#include"Common.h"

class ThreadCache
{
public:
	//申请和释放内存
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);//释放那块空间,归还到那个桶下面

	void* FetchFromCentralCache(size_t index,size_t size);//threadcache没有内存满足,向中心内存申请  告诉位置

	// 释放对象时，链表过长时，回收内存回到中心缓存
	void ListTooLong(FreeList& list, size_t size);

private:
	FreeList _freeLists[NFREELIST];
};
// TLS thread local storage 使线程无需加锁,每个线程只能访问自己的资源
//TLS 是 "Thread Local Storage" 的缩写，中文通常称为“线程局部存储”。
// 它是一种计算机编程技术，用于在多线程环境中为每个线程提供独立的、私有的变量副本。
// 这意味着即使多个线程正在同时运行并且它们都在访问所谓的“全局”或静态变量，
// 每个线程看到的都是该变量的唯一副本，从而避免了数据竞争和同步问题。
//static 保证资源只在当前文件课件
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;

