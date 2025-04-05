#pragma once
#include"Common.h"

class ThreadCache
{
public:
	//������ͷ��ڴ�
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);//�ͷ��ǿ�ռ�,�黹���Ǹ�Ͱ����

	void* FetchFromCentralCache(size_t index,size_t size);//threadcacheû���ڴ�����,�������ڴ�����  ����λ��

	// �ͷŶ���ʱ���������ʱ�������ڴ�ص����Ļ���
	void ListTooLong(FreeList& list, size_t size);

private:
	FreeList _freeLists[NFREELIST];
};
// TLS thread local storage ʹ�߳��������,ÿ���߳�ֻ�ܷ����Լ�����Դ
//TLS �� "Thread Local Storage" ����д������ͨ����Ϊ���ֲ߳̾��洢����
// ����һ�ּ������̼����������ڶ��̻߳�����Ϊÿ���߳��ṩ�����ġ�˽�еı���������
// ����ζ�ż�ʹ����߳�����ͬʱ���в������Ƕ��ڷ�����ν�ġ�ȫ�֡���̬������
// ÿ���߳̿����Ķ��Ǹñ�����Ψһ�������Ӷ����������ݾ�����ͬ�����⡣
//static ��֤��Դֻ�ڵ�ǰ�ļ��μ�
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;

