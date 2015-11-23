#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"







void count(struct node *x)
{
    struct node *temp = x;
    int count = 0;
    while(temp!=NULL)
    {
        count++;
        temp=temp->next;
    }
    printf("total elements = %d",count);
}



