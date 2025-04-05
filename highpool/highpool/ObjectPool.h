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
	//�����ڴ�ռ� rnmdeex
	T* New()
	{
		T* obj = nullptr;
		if (_freeList != nullptr)//���ȴ���������ȥ�ڴ�ʹ��
		{
			void* next = *((void**)_freeList);
			obj =(T*) _freeList;//ָ�븳��˭,������˭ָ��˭
			_freeList = next;
		}
		else
		{
			//if (_memory == nullptr)//��δ����һ���ڴ�
			if (_remainBytes < sizeof(T))
			{
				_remainBytes = 1024 * 128;//16span
				_memory = (char*)SystemAlloc(_remainBytes >> 13);//����һ���ڴ�,�����ָ��,
				//char* _memory = (char*)malloc(_remainBytes);
				if (_memory == nullptr)	//����ռ�ʧ��
				{
					throw bad_alloc();
				}
			}
				obj = (T*)_memory;
				//��ֹ�����������ڴ��޷������һ���ֽڵĵ�ַ
				size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
				_remainBytes -= objSize;
				_memory += objSize;
			

		}
		//ʹ�ö�λnew����T�Ĺ��캯��
		new(obj)T;

		return obj;
	}
	//��ʹ����Ŀռ任����,ͨ��һ�����������	����ͷ��
	void Delete(T* obj)
	{
		obj->~T();
		*(void**)obj = _freeList;//���ָ���ָ������
		_freeList = obj;
	}

private:
	char* _memory = nullptr;//ָ����ڴ��ָ��
	size_t		_remainBytes = 0;//����ڴ��зֺ�ʣ����ֽ���
	void* _freeList = nullptr;//�洢�����������ڴ�

};

//�����ڴ����malloc���ܲ���
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
//	//������ѯ����
//	const size_t Rounds = 5;
//	//ÿ����ѯ�ͷŶ��ٴ�(����,���ٵĴ���)
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
//			v1.push_back(new TreeNode);//�����ڴ�
//		}
//		for (int k = 0; k < N; k++)
//		{
//			delete v1[k];//�ͷ��ڴ�
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
//			v2.push_back(TNPool.New());//�����ڴ�
//		}
//		for (int k = 0; k < N; k++)
//		{
//			TNPool.Delete(v2[k]);//�ͷ��ڴ�
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
