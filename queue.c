#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

#define MAX 1000
/*
static void push_f(struct queue *obj, int data, int n)
{
    obj->offset = (obj->offset + n)%obj->q_max;
    obj->data[obj->offset] = data;
}

static int pop_f(struct queue *obj)
{
    int reg = obj->data[obj->sp];

    obj->sp = (obj->sp + 1)%obj->q_max;

    return reg;
}
*/
static struct data read_f(struct queue *obj, int off)
{
    return obj->data[obj->sp + off];
}
static void wirte_f(struct queue *obj, struct data in, int off)
{
    obj->data[obj->sp + off] = in;
}
static void addsp_f(struct queue *obj)
{
    obj->sp++;
}

struct queue *queue_new()
{
    struct queue *reg = malloc(sizeof(struct queue));
    struct data n = {};

    reg->data = malloc(sizeof(struct data) * MAX);
    reg->q_max = MAX;
    reg->sp = 0;
    reg->read = read_f;
    reg->wirte = wirte_f;
    reg->addsp = addsp_f;

    for(int i = 0; i < reg->q_max; i++)
        reg->data[i] = n;
    return reg;
}

void queue_del(struct queue **obj)
{
    if(*obj != NULL)
    {
        if((*obj)->data != NULL)
        {
            free((*obj)->data);
            (*obj)->data = NULL;
        }
        free(*obj);
        *obj = NULL;
    }
}
#if 0
int main()
{
    struct queue *que = queue_new();
    struct data n = {.pip[0] = 1, .pip[1] = 2};

    que->wirte(que, n, 3);
    que->addsp(que);
    struct data s = que->read(que, 2);
    printf("%d %d\n", s.pip[0], s.pip[1]);
}
#endif
