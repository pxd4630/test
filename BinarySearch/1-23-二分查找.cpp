//二分查找--递归法
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
        printf("找到了，下标是%d\n", result);
    }
    else 
    {
        printf("找不到\n");
    }

    return 0;
}*/

/*#include <stdio.h>
int main()
{
    int arr[] = { 1,2,3,4,5,6,7,8,9,10 };
    int left = 0;
    int right = sizeof(arr) / sizeof(arr[0]) - 1;
    int key =0;//要找的数字
    scanf_s("%d", &key);
    int mid = 0;//记录中间元素的下标
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
        printf("找到了,下标是%d\n", mid);
    else
        printf("找不到\n");
}*/


#include <stdio.h>
// 递归实现的二分查找函数
// 参数说明：arr为有序数组，target为要查找的目标值，left和right分别为查找范围的左右边界
int binarySearchRecursive(int arr[], int target, int left, int right) {
    if (left <= right) {
        int mid = left + (right - left) / 2;

        if (arr[mid] == target) {
            return mid;  // 如果中间元素等于目标值，则返回中间元素的下标
        }
        else if (arr[mid] > target) {
            // 如果中间元素大于目标值，在左半部分递归查找
            return binarySearchRecursive(arr, target, left, mid - 1);
        }
        else {
            // 如果中间元素小于目标值，在右半部分递归查找
            return binarySearchRecursive(arr, target, mid + 1, right);
        }
    }
    else {
        return -1;  // 如果左指针大于右指针，说明目标值不存在，返回 -1
    }
}

int main() {
    int arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int target = 0;
    scanf_s("%d",&target);
    int result = binarySearchRecursive(arr, target, 0, sizeof(arr) / sizeof(arr[0]) - 1);

    if (result != -1) {
        printf("找到了，下标是%d\n", result);
    }
    else {
        printf("找不到\n");
    }

    return 0;
}