//#include"kmp.h"
//int main()
//{
//	//BBC ABCDAB ABCDABCDABDE
//	string s1("BBCABCDABABCDABCDABDE");
//	string s2("ABCDABD");
//	int ret=multiSearch( s1, s2);
//	cout << ret << endl;
//	/*vector<int>vec1 = GetNext(s2);
//	for (auto e : vec1)
//		cout << e << endl;*/
//	return 0;
//}
#include"kmp.h"
int main()
{
	//BBC ABCDAB ABCDABCDABDE
	/*string s1("BBCABCDABABCDABCDABDE");
	string s2("ABCDABD");*/
	string s1("abc");
	vector<string> s2 = { "","a"};
	vector<vector<int>> ret = multiSearch(s1, s2);
	int m = ret.size();
	
	for (int i = 0; i < m; i++)
	{
		int n = ret[i].size();
		for (int j = 0; j < n; j++)
		{
			cout << ret[i][j];
		}
		cout << endl;
	}
	/*vector<int>vec1 = GetNext(s2);
	for (auto e : vec1)
		cout << e << endl;*/
	return 0;
}