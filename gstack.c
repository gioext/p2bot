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
    stack->wait = 0;
    stack->head = NULL;
    if (pthread_mutex_init(&(stack->mutex), NULL) < 0) {
        fprintf(stderr, "mutex_init error\n");
    }
    if (pthread_cond_init(&(stack->cond), NULL) < 0) {
        fprintf(stderr, "cond_init error\n");
    }

    return stack;
}

void gstack_destroy(gstack_t *stack)
{
    pthread_mutex_destroy(&(stack->mutex));
    pthread_cond_destroy(&(stack->cond));
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
    if (stack->length > 0) {
        node->nextp = stack->head;
    } else {
        pthread_cond_signal(&(stack->cond));
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

    pthread_mutex_lock(&(stack->mutex));
    while (stack->length == 0) {
        stack->wait = 1;
        pthread_cond_wait(&(stack->cond), &(stack->mutex));
    }

    node = stack->head;
    datap = node->datap;
    stack->head = node->nextp;
    stack->length--;
    stack->wait = 0;
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
