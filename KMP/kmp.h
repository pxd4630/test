//#pragma once
//#include<iostream>
//#include<string>
//#include<vector>
//using namespace std;
// 单对单
////怎么获得next数组  ?
////怎么获得最大的前缀和后缀的最大值  ?
//vector<int> GetNext(string t)
//{
//	int n = t.size();
//	int j = 0;
//	//next[i]的值表示下标为i的字符前的字符串最长相等前后缀的长度。
//	//表示该处字符不匹配时应该回溯到的字符的下标
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
//	//构造失败函数(构造部分匹配表)
//	//就是获得next数组
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
//单对多
#pragma once
#include<iostream>
#include<string>
#include<vector>
using namespace std;
//怎么获得next数组  ?
//怎么获得最大的前缀和后缀的最大值  ?
vector<int> GetNext(string t)
{
	int n = t.size();
	if (n == 0)
	{
		vector<int> v = {0};
		return v;
	}
	int j = 0;
	//next[i]的值表示下标为i的字符前的字符串最长相等前后缀的长度。
	//表示该处字符不匹配时应该回溯到的字符的下标
	
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
	//构造失败函数(构造部分匹配表)
	//就是获得next数组
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
