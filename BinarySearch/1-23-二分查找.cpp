//���ֲ���--�ݹ鷨
/*#include <stdio.h>

int binarySearchRecursive(int arr[], int key, int left, int right) 
{
    if (left <= right) 
    {
        int mid = (left + right) / 2;

        if (arr[mid] == key)
        {
            return mid;
        }
        else if (arr[mid] > key) 
        {
            return binarySearchRecursive(arr, key, left, mid - 1);
        }
        else {
            return binarySearchRecursive(arr, key, mid + 1, right);
        }
    }
    else
    {
        return -1;
    }
}

int main() {
    int arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int key = 0;
    scanf_s("%d", &key);
    int sz = sizeof(arr) / sizeof(arr[0]);
    int result = binarySearchRecursive(arr, key, 0, sz - 1);

    if (result != -1)
    {
        printf("�ҵ��ˣ��±���%d\n", result);
    }
    else 
    {
        printf("�Ҳ���\n");
    }

    return 0;
}*/

/*#include <stdio.h>
int main()
{
    int arr[] = { 1,2,3,4,5,6,7,8,9,10 };
    int left = 0;
    int right = sizeof(arr) / sizeof(arr[0]) - 1;
    int key =0;//Ҫ�ҵ�����
    scanf_s("%d", &key);
    int mid = 0;//��¼�м�Ԫ�ص��±�
    int find = 0;
    while (left <= right)
    {
        mid = (left + right) / 2;
        if (arr[mid] > key)
        {
            right = mid - 1;
        }
        else if (arr[mid] < key)
        {
            left = mid + 1;
        }
        else
        {
            find = 1;
            break;
        }
    }
    if (1 == find)
        printf("�ҵ���,�±���%d\n", mid);
    else
        printf("�Ҳ���\n");
}*/


#include <stdio.h>
// �ݹ�ʵ�ֵĶ��ֲ��Һ���
// ����˵����arrΪ�������飬targetΪҪ���ҵ�Ŀ��ֵ��left��right�ֱ�Ϊ���ҷ�Χ�����ұ߽�
int binarySearchRecursive(int arr[], int target, int left, int right) {
    if (left <= right) {
        int mid = left + (right - left) / 2;

        if (arr[mid] == target) {
            return mid;  // ����м�Ԫ�ص���Ŀ��ֵ���򷵻��м�Ԫ�ص��±�
        }
        else if (arr[mid] > target) {
            // ����м�Ԫ�ش���Ŀ��ֵ������벿�ֵݹ����
            return binarySearchRecursive(arr, target, left, mid - 1);
        }
        else {
            // ����м�Ԫ��С��Ŀ��ֵ�����Ұ벿�ֵݹ����
            return binarySearchRecursive(arr, target, mid + 1, right);
        }
    }
    else {
        return -1;  // �����ָ�������ָ�룬˵��Ŀ��ֵ�����ڣ����� -1
    }
}

int main() {
    int arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int target = 0;
    scanf_s("%d",&target);
    int result = binarySearchRecursive(arr, target, 0, sizeof(arr) / sizeof(arr[0]) - 1);

    if (result != -1) {
        printf("�ҵ��ˣ��±���%d\n", result);
    }
    else {
        printf("�Ҳ���\n");
    }

    return 0;
}