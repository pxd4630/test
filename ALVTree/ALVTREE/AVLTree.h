#pragma once
#include<iostream>
#include<vector>
#include<assert.h>
using namespace std;
//左子树上的所有节点的键值都小于该节点的键值。
//右子树上的所有节点的键值都大于该节点的键值。
//考虑一个树节点的结构
template<class K,class V>
struct AVLTreeNode {
	AVLTreeNode<K, V>* _left ;
	AVLTreeNode<K, V>* _right ;
	AVLTreeNode<K, V>* _parent ;
	std::pair<K, V> _kv;
	int _bf;//平衡因子--->左右树的高度差


	AVLTreeNode(const pair<K,V> &kv)
	:_left(nullptr),
	_right(nullptr),
	_parent(nullptr),
	_kv(kv),
	_bf(0)
	{}
};

template <class K,class V>
class AVLTree {
	//using Node=AVLTreeNode<K, V>
	typedef AVLTreeNode<K, V> Node;
public:
	bool Insert(const std::pair<K, V>& kv)
	{
		//首先考虑根节点是否为空
		if (_root == nullptr)
		{
			_root = new Node(kv);
			return true;
		}
		Node* parent = nullptr;
		Node* cur = _root;
		//更新节点
		while (cur)
		{
			if (cur->_kv.first > kv.first)
			{
				parent = cur;
				cur = cur->_left;
			}
			else if(cur->_kv.first<kv.first)
			{
				parent = cur;
				cur = cur->_right;
			}
			else
			{
				//去除健值相同的情况
				return false;
			}
		}
		//创建节点
		cur = new Node(kv);
		//判断该新创节点是左节点还是右节点
		//左小右大
		if (parent->_kv.first < kv.first)
		{
			parent->_right = cur;
		}
		else
		{
			parent->_left = cur;
		}
		cur->_parent = parent;
		//更新平衡因子
		while (parent)
		{
			if (cur == parent->_left)
			{
				parent->_bf--;
			}
			else
			{
				parent->_bf++;
			}

			if (parent->_bf == 0)
			{
				//更新结束
				break;
			}
			//继续更新
			else if (parent->_bf== - 1 || parent->_bf == 1)
			{
				//向上更新
				cur = parent;
				parent = parent->_parent;
			}
			//子树出现问题 -1-->-2 || 1-->2
			//左左-->右单旋
			else if(parent->_bf==-2&&cur->_bf==-1)
			{
				RotateR(parent);
			}
			//右右-->左单旋
			else if (parent->_bf == 2 && cur->_bf == 1)
			{
				RotateL(parent);
			}
			//左右-->左右双旋
			else if (parent->_bf == -2 && cur->_bf == 1)
			{
				RotateLR(parent);
			}
			//右左-->右左双旋
			else if (parent->_bf == 2 && cur->_bf == -1)
			{
			//	cout << &parent << endl;
				RotateRL(parent);
			}
			else
			{
				assert(false);
			}

		}

	}
	//左左-->右单旋
	void RotateR(Node* parent)
	{
		//影响因子节点的左节点
		Node* subL = parent->_left;//这里的subL指针相当于上层调用函数的cur指针
		Node* subLR = subL->_right;

		parent->_left = subLR;
		//防止出现空指针指向问题
		if (subLR)
		subLR->_parent = parent;

		subL->_right = parent;
		Node* ppNode = parent->_parent;
		parent->_parent = subL;

		if (parent == _root)
		{
			_root = subL;
			_root->_parent = nullptr;
		}
		else
		{
			if (ppNode->_left==parent)
			{
				ppNode->_left = subL;
			}
			else
			{
				ppNode->_right = subL;
			}
			//更新节点指针指向
			subL->_parent = ppNode;
		}
		parent->_bf = subL->_bf = 0;
		cout << "右单旋没有问题" << endl;
	}
	//右右-->左单旋
	//注意!!!上下两层之后之间的指向关系
	void RotateL(Node* parent)
	{
		Node* subR = parent->_right;
		Node* subRR = subR->_right;
		
		parent->_right = subRR;
		//判断是否为空,避免造成空指针的指向问题
		if (subRR)
		{
			subRR->_parent = parent;
		}
		
		subR->_left = parent;
		//存放参入参数的父亲指针
		Node* ppNode = parent->_parent;
		parent->_parent = subR;
		if (parent == _root)
		{
			_root = subR;
			_root->_parent = nullptr;
		}
		else{
			if (ppNode->_left == parent)
				ppNode->_left = subR;
			else
				ppNode->_right = subR;

			//更新新头节点的指向问题
			subR->_parent = ppNode;
		}
		//更新平衡因子
		parent->_bf = subR->_bf = 0;
		cout << "左单旋没有问题" << endl;
	}
	//左右-->左右双旋
	void RotateLR(Node* parent)
	{
		Node* subL = parent->_left;
		Node* subLR = subL->_right;
		//更新最后的节点的平衡因子
		int bf = subLR->_bf;

		RotateL(parent->_left);
		RotateR(parent);


		if (bf == -1)
		{
			subLR->_bf = 0;
			subL->_bf = 0;
			parent->_bf = 1;
		}
		else if (bf == 1)
		{
			subLR->_bf = 0;
			subL->_bf = -1;
			parent->_bf = 0;
		}
		else if (bf == 0)
		{
			subLR->_bf = 0;
			subL->_bf = 0;
			parent->_bf = 0;
		}
		else
		{
			assert(false);
		}
		cout << "左双旋没有问题" << endl;
	}
	//右左-->右左双旋
	void RotateRL(Node* parent)
	{
		Node* subR = parent->_right;
		Node* subRL = subR->_left;
		//cout << &subRL << endl;
		int bf = subRL->_bf;

		RotateR(subR);
		RotateL(parent);

		subRL->_bf = 0;
		if (bf == 1)
		{
			subR->_bf = 0;
			parent->_bf = -1;
		}
		else if (bf == -1)
		{
			parent->_bf = 0;
			subR->_bf = 1;
		}
		else
		{
			parent->_bf = 0;
			subR->_bf = 0;
		}
		cout << "右左双旋没有问题" << endl;
	}
	Node* Find(const K& key)
	{
		Node* cur = _root;
		while (cur)
		{
			if (cur->_kv.first < key)
			{
				cur = cur->_right;
			}
			else if (cur->_kv.first > key)
			{
				cur = cur->_left;
			}
			else
			{
				return cur;
			}
		}

		return nullptr;
	}


	void InOrder()
	{
		_InOrder(_root);
		cout << endl;
	}

	bool IsBalance()
	{
		return _IsBalance(_root);
	}

	int Height()
	{
		return _Height(_root);
	}

	int Size()
	{
		return _Size(_root);
	}

	private:
		int _Size(Node* root)
		{
			return root == nullptr ? 0 : _Size(root->_left) + _Size(root->_right) + 1;
		}

		int _Height(Node* root)
		{
			if (root == nullptr)
				return 0;

			return max(_Height(root->_left), _Height(root->_right)) + 1;
		}

		bool _IsBalance(Node* root)
		{
			if (root == nullptr)
				return true;

			int leftHeight = _Height(root->_left);
			int rightHeight = _Height(root->_right);
			// 不平衡
			if (abs(leftHeight - rightHeight) >= 2)
			{
				cout << root->_kv.first << endl;
				return false;
			}

			// 顺便检查一下平衡因子是否正确
			if (rightHeight - leftHeight != root->_bf)
			{
				cout << root->_kv.first << endl;
				return false;
			}

			return _IsBalance(root->_left)
				&& _IsBalance(root->_right);
		}

		void _InOrder(Node* root)
		{
			if (root == nullptr)
			{
				return;
			}

			_InOrder(root->_left);
			cout << root->_kv.first << ":" << root->_kv.second << endl;
			_InOrder(root->_right);
		}
private:
	Node* _root = nullptr;
};

void TestAVLTree2()
{
	const int N = 1000;
	vector<int> v;
	v.reserve(N);
	srand(time(0));

	for (size_t i = 0; i < N; i++)
	{
		v.push_back(rand() + i);
		cout << v.back() << endl;
	}

	size_t begin2 = clock();
	AVLTree<int, int> t;
	for (auto e : v)
	{
		t.Insert(make_pair(e, e));
		//cout << "Insert:" << e << "->" << t.IsBalance() << endl;
	}
	size_t end2 = clock();

	cout << "Insert:" << end2 - begin2 << endl;
	//cout << t.IsBalance() << endl;

	cout << "Height:" << t.Height() << endl;
	cout << "Size:" << t.Size() << endl;

	size_t begin1 = clock();
	// 确定在的值
	for (auto e : v)
	{
		t.Find(e);
	}

	// 随机值
	/*for (size_t i = 0; i < N; i++)
	{
		t.Find((rand() + i));
	}*/

	size_t end1 = clock();

	cout << "Find:" << end1 - begin1 << endl;
}

