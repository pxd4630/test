#pragma once
//手动实现一个unique_ptr
//以下只针对于析构普通的指针
#include<iostream>
template <typename T, typename Deleter = std::default_delete<T>>
class Uniqueptr {

	//构造函数
	Uniqueptr() :ptr(nullptr)
	{

	};

	//右值构造
	Uniqueptr(Uniqueptr&& other)
	{
		ptr(other.release());
	}

	//移动赋值运算符
	Uniqueptr& operator=(Uniqueptr&& other)
	{
		if (other != this)
		{
			reset(other.release())
		}
	}

	~Uniqueptr() {

	}

	//提供将亡值
	T* release()
	{
		T* temp = ptr;
		ptr = nullptr;
		return temp;
	}

	//移动资源 p是release提供的将亡值
	void reset(T* p)
	{
		if (p != ptr)
		{
			/*
			创建删除器实例：Deleter() 通过类型 Deleter 创建一个临时对象。
			调用操作符：使用() 运算符重载（即 operator()）对临时对象进行调用。
			传递指针：将指针 ptr 作为参数传递给删除器的 operator()。
			处理不同参数的类型类型的资源
			*/
			Deleter()(ptr);
			ptr = p;
		}
	}

private:
	T* ptr;
};
