//
// Created by yanayg on 10/27/19.
//

#include "linked_list.h"
#include "../utils.h"

void insert_after(node_t *src, node_t *target) {
    target->next = src->next;
    target->prev = src;

    src->next = target;

    if (src->next) src->next->prev = target;
}

void insert_before(node_t *src, node_t *target) {
    target->next = src;
    target->prev = src->prev;

    src->prev = target;

    if (src->prev) src->prev->next = target;
}

void append(node_t *head, node_t *item) {
    node_t *curr =  head;
    while(curr->next != 0) {
        curr = curr->next;
    }
    curr->next = item;
    item->prev = curr;
}

u32 list_length(node_t *head) {
    node_t *curr =  head;
    u32 length = 0;
    while(curr->next != 0) {
        curr = curr->next;
        length++;
    }
    return length;
}

void remove(node_t *node) {
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
}