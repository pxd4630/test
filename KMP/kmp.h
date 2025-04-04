//#pragma once
//#include<iostream>
//#include<string>
//#include<vector>
//using namespace std;
// ���Ե�
////��ô���next����  ?
////��ô�������ǰ׺�ͺ�׺�����ֵ  ?
//vector<int> GetNext(string t)
//{
//	int n = t.size();
//	int j = 0;
//	//next[i]��ֵ��ʾ�±�Ϊi���ַ�ǰ���ַ�������ǰ��׺�ĳ��ȡ�
//	//��ʾ�ô��ַ���ƥ��ʱӦ�û��ݵ����ַ����±�
//	vector<int> vec(n);
//	vec[0] = 0;
//	for (int i = 1; i < n; i++)
//	{
//		while (j > 0 && t[i] != t[j])
//		{
//			j = vec[j - 1];
//		}
//		if (t[i] == t[j])
//			j++;
//		vec[i] = j;
//	}
//	return vec;
//}
//int multiSearch(string s,string t)
//{
//	//����ʧ�ܺ���(���첿��ƥ���)
//	//���ǻ��next����
//	 int ret = -1;
//	vector<int> Next = GetNext(t);
//	int n = s.size();
//	int j = 0;
//	for (int i = 0; i < n; i++)
//	{
//		while (j > 0 && s[i] != t[j])
//		{
//			j = Next[j - 1];
//		}
//			if (s[i] == t[j])
//		{
//			j++;
//		}
//		if (j == t.size()) 
//		{
//			ret = i - t.size() + 1;
//		}
//	}
//	return ret;
//}
//���Զ�
#pragma once
#include<iostream>
#include<string>
#include<vector>
using namespace std;
//��ô���next����  ?
//��ô�������ǰ׺�ͺ�׺�����ֵ  ?
vector<int> GetNext(string t)
{
	int n = t.size();
	if (n == 0)
	{
		vector<int> v = {0};
		return v;
	}
	int j = 0;
	//next[i]��ֵ��ʾ�±�Ϊi���ַ�ǰ���ַ�������ǰ��׺�ĳ��ȡ�
	//��ʾ�ô��ַ���ƥ��ʱӦ�û��ݵ����ַ����±�
	
		vector<int> vec(n);
		vec[0] = 0;
		for (int i = 1; i < n; i++)
		{
			while (j > 0 && t[i] != t[j])
			{
				j = vec[j - 1];
			}
			if (t[i] == t[j])
				j++;
			vec[i] = j;
		}
	return vec;
}
vector<vector<int>> multiSearch(string s, vector<string>& t)
{
	//����ʧ�ܺ���(���첿��ƥ���)
	//���ǻ��next����
	vector<vector<int>> vec(t.size());
	int ret = -1;
	//vector<int> Next = GetNext(t);
	int n = s.size();
	int key = t.size();
	for (size_t x = 0; x < key; x++)
	{
		if (t[x].empty())
		{
			x++;
			continue;
		}
		vector<int> matches;
		int j = 0;
		vector<int> Next = GetNext(t[x]);
		string str = t[x];
		int m = str.size();
		for (int i = 0; i < n; i++)
		{
			while (j > 0 && s[i] != str[j])
			{
				j = Next[j - 1];
			}
			if (s[i] == str[j])
			{
				j++;
			}
			if (j == m)
			{
				ret = i - m + 1;
				vec[x].push_back(ret);
				j = Next[j - 1];
			}
		}
		/*if (vec[x].size() == 0)
		{
			vec.push_back(matches);
		}*/
	}
	return vec;
}
