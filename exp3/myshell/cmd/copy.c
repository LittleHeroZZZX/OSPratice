/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-10-25 13:38:28
 * @LastEditTime: 2022-10-25 13:40:27
 * @Software: VSCode
 */
#include <stdio.h>
int main(int argc, char const *argv[])
{
    FILE *r_file = fopen(argv[1], "rb");
    FILE *w_file = fopen(argv[2], "w");
    char temp[10];
    int judge = 0;
    if (r_file)
    {
        do
        {
            judge = fread(temp, sizeof(char), 10, r_file);
            if (!judge)
            {
                break;
            }
            fwrite(temp, sizeof(char), 10, w_file);
        } while (1);
    }
    fclose(r_file);
    fclose(w_file);
    r_file = NULL;
    w_file = NULL;
    return 0;
}