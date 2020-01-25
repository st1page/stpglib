#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include "rc_ptr.h"

typedef struct ListNode{
    ListNode* pre;
    ListNode* nxt;
    void *val;
}ListNode;

typedef struct List{
    ListNode* head;
    ListNode* tail;
    size_t len;
    void *(*dup)(void *ptr);
    void (*free)(void *ptr);
}List;

#define listSetDupMethod(l,m) ((l)->dup = (m))
#define listSetFreeMethod(l,m) ((l)->free = (m))

#define listGetDupMethod(l) ((l)->dup)
#define listGetFreeMethod(l) ((l)->free)

List *listNew(void);
void listDel(List *list);
void listAddNodeHead(List *list, void *val);
void listAddNodeTail(List *list, void *val);
void listAddNodeAfter(List *list, void *val, ListNode *pre_node);
void listRemoveNode(List *list, ListNode *node);

void listForeach(List *list, void (*fun)(void *));

//符合要求的节点返回1 否则返回0
typedef int (*FilterFun)(ListNode*);
void listForeachFilter(List *list, FilterFun, void (*fun)(void *));
// first 
void listFindNode(List *list, FilterFun);

#endif //LIST_H
