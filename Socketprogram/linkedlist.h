#include <stdio.h>
#include <stdlib.h>

#define MAXLENGTH 1024 


typedef struct node
{
    char neighbour[100];
    int cost;
    struct node *next;
}node;



void insertatend(struct node* x, node* data);

void insertatbeginning(node **head ,node *data);

void print(struct node *x);

void count(struct node *x);
