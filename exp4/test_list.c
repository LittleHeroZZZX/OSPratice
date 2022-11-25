/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-11-25 14:40:23
 * @LastEditTime: 2022-11-25 14:56:18
 * @Software: VSCode
 */

#include "list.h"
#include <stdio.h>
#include <stdlib.h>

struct person
{
    struct list_head list;
    int age;
};


int main()
{
    int i;
    struct person *p;
    struct person person1;
    struct list_head *l;
    INIT_LIST_HEAD(&person1.list);
    for (int i=0; i<5; i++)
    {
        p = (struct person *)malloc(sizeof(struct person));
        p->age = i;
        list_add(&p->list, &person1.list);
    }
    list_for_each(l, &person1.list)
    {
        printf("%d\n", ((struct person *)l)->age);
    }
    list_for_each_entry(p, &person1.list, list)
    {
        printf("%d\n", p->age);
    }
    return 0;

}