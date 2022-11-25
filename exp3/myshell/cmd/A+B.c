/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-10-25 13:32:24
 * @LastEditTime: 2022-10-25 13:35:33
 * @Software: VSCode
 */
#include<stdio.h>

int str2num(char *str)
{
    int num = 0;
    while(*str != '\0')
    {
        num = num * 10 + (*str - '0');
        str++;
    }
    return num;
}


int main(int argc, char const *argv[])
{
    int num1 = str2num(argv[1]);
    int num2 = str2num(argv[2]);
    printf("%d + %d = %d\n", num1, num2, num1 + num2);
}