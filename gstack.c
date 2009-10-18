#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "gstack.h"

gstack_t *gstack_new()
{
    gstack_t *stack;
    stack = (gstack_t *)malloc(sizeof(gstack_t));
    if (stack == NULL) {
        return NULL;
    }
    stack->length = 0;
    stack->head = NULL;
    if (pthread_mutex_init(&(stack->mutex), NULL) < 0) {
        fprintf(stderr, "error\n");
    }

    return stack;
}

void gstack_destroy(gstack_t *stack)
{
    pthread_mutex_destroy(&(stack->mutex));
    free(stack);
}

int gstack_push(gstack_t *stack, void *datap)
{
    gstack_node_t *node;
    
    node = (gstack_node_t *)malloc(sizeof(gstack_node_t));
    if (node == NULL) {
        return -1;
    }
    node->nextp = NULL;
    node->datap = datap;

    pthread_mutex_lock(&(stack->mutex));
    if (stack->head != NULL) {
        node->nextp = stack->head;
    }
    stack->head = node;
    stack->length++;
    pthread_mutex_unlock(&(stack->mutex));

    return 0;
}

void *gstack_pop(gstack_t *stack)
{
    gstack_node_t *node;
    void *datap;

    if (stack->head == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&(stack->mutex));
    node = stack->head;
    datap = node->datap;
    stack->head = node->nextp;
    stack->length--;
    free(node);
    pthread_mutex_unlock(&(stack->mutex));

    return datap;
}

//void *run(void *arg)
//{
//    gstack_node_t *datap;
//    gstack_t *stack;
//    stack = (gstack_t *)arg;
//
//    while((datap = gstack_pop(stack)) != NULL) {
//        int i = (int)datap;
//        printf("%u: %d\n", pthread_self(), i);
//    }
//    return NULL;
//}

//int main(int argc, char *argv[])
//{
//    int i;
//    gstack_t *stack;
//    stack = gstack_new();
//    pthread_t thread1, thread2;
//
//    for (i = 1; i < 1000; i++) {
//        gstack_push(stack, (void *)i);
//    }
//
//    pthread_create(&thread1, NULL, run, stack);
//    pthread_create(&thread2, NULL, run, stack);
//
//    pthread_join(thread1, NULL);
//    pthread_join(thread2, NULL);
//    gstack_destroy(stack);
//    return 0;
//}
