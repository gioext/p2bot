#ifndef GSTACK_H
#define GSTACK_H


typedef struct gstack_node {
    void *datap;
    struct gstack_node *nextp;
} gstack_node_t;

typedef struct gstack {
    int length;
    gstack_node_t *head;
    pthread_mutex_t mutex;
} gstack_t;

gstack_t *gstack_new();
void gstack_destroy(gstack_t *stack);
int gstack_push(gstack_t *stack, void *datap);
void *gstack_pop(gstack_t *stack);


#endif
