#include "list.h"
#include <stdlib.h>

#define listSetDupMethod(l,m) ((l)->dup = (m))
#define listSetFreeMethod(l,m) ((l)->free = (m))

#define listGetDupMethod(l) ((l)->dup)
#define listGetFreeMethod(l) ((l)->free)

List *listNew(void){
    List *list = (List*)(calloc(sizeof(List),1));
    return list;
}
void listDel(List *list){
    ListNode *cur, *nxt;
    cur = list->head;
    while(cur){
        nxt = cur->nxt;
        if(list->free) list->free(cur->val);
        free(cur);
        cur = nxt;
    }
    free(list);
}
void listAddNodeHead(List *list, void *val){
    ListNode *node = (ListNode*)malloc(sizeof(ListNode));
    node->val = val;
    if(!list->len){
        node->pre = node->nxt = NULL;
        list->head = list->tail = node;
    } else {
        node->pre = NULL;
        node->nxt = list->head;
        list->head->pre = node;
        list->head = node;
    }
    list->len++;
}
void listAddNodeTail(List *list, void *val){
    ListNode *node = (ListNode*)malloc(sizeof(ListNode));
    node->val = val;
    if(!list->len){
        node->pre = node->nxt = NULL;
        list->head = list->tail = node;
    } else {
        node->nxt = NULL;
        node->pre = list->tail;
        list->tail->nxt = node;
        list->tail = node;
    }
    list->len++;
}
void listAddNodeAfter(List *list, void *val, ListNode *pre_node){
    ListNode *node = (ListNode*)malloc(sizeof(ListNode));
    node->val = val;
    node->pre = pre_node;
    node->nxt = pre_node->nxt;
    pre_node->nxt = node;
    if(node->nxt) {
        node->nxt->pre = node;
    } else list->tail = node;
    list->len++;
}
void listRemoveNode(List *list, ListNode *node){
    if(node->pre){
        node->pre->nxt = node->nxt;
    } else list->head = node->nxt;
    if(node->nxt){
        node->nxt->pre = node->pre;
    } else list->tail = node->pre;

    if(list->free) list->free(node->val);
    free(node);
    list->len--;
}

void listForeach(List *list, void (*fun)(void *)){
    ListNode *cur = list->head;
    while(cur){
        fun(cur->val);
        cur = cur->nxt;
    }
}

void listForeachFilter(List *list, FilterFun filter, void (*fun)(void *)){
    ListNode *cur = list->head;
    while(cur){
        if(filter(cur->val))fun(cur->val);
        cur = cur->nxt;
    }
}
ListNode* listFindNode(List *list, FilterFun filter){
    ListNode *cur = list->head;
    while(cur){
        if(filter(cur->val)) return cur;
        cur = cur->nxt;
    } 
    return NULL;
}
