//
// Created by yanayg on 10/27/19.
//

#ifndef MYOS_LINKED_LIST_H

typedef struct node_t {
    struct node_t *next;
    struct node_t *prev;
} node_t;

void insert_after(node_t *src, node_t *target);

void insert_before(node_t *src, node_t *target);

void append(node_t *head, node_t *item);

void remove(node_t *node);

u32 length(node_t *head);

#define MYOS_LINKED_LIST_H

#endif //MYOS_LINKED_LIST_H
