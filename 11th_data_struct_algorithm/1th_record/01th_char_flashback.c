/*字符串倒叙
 * 核心：使用gets从stdin获取字符串，然后使用中间变量做一个交换即可
 * */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void reverse(char src[]);

int main()
{
	char src[100];
	printf("please input char scr:");
	gets(src);
	reverse(src);
	printf("reverse:%s\n", src);
	return 0;
}

void reverse(char src[]){
	int tmp,n,i;
	n = strlen(src);
	for(i = 0; i < n/2; i++) {
		tmp = src[i];
		src[i] = src[n - i -1];
		src[n - i -1] = tmp;
	}
}
