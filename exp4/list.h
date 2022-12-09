#ifndef LIST_H
#define LIST_H
#define offset_of(type, member) ((size_t) &((type *)0)->member)
#define container_of(ptr, type, member) ({ \
    const __typeof__(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offset_of(type, member)); \
})


struct list_head
{
	struct list_head* next, * prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head* list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head* new_node,
	struct list_head* prev,
	struct list_head* next)
{
	next->prev = new_node;
	new_node->next = next;
	new_node->prev = prev;
	prev->next = new_node;
}

static inline void list_add(struct list_head* new_node, struct list_head* head)
{
	__list_add(new_node, head, head->next);
}

static inline void list_del(struct list_head* entry)
{
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
}


#define list_for_each(pos, head)\
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_for_each_entry(pos, head, member) \
    for (pos = list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = list_entry(pos->member.next, typeof(*pos), member))

// 修正链表指针偏移
#define update_list(ptr, member, delta) \
        struct list_head *pos;          \
        (ptr)->member.next = (struct list_head *)((char *) (ptr)->member.next + delta); \
        (ptr)->member.prev = (struct list_head *)((char *) (ptr)->member.prev + delta); \
        list_for_each(pos, &(ptr)->member) { \
            pos->next = (struct list_head *)((char *) pos->next + delta); \
            pos->prev = (struct list_head *)((char *) pos->prev + delta); \
        }
#endif //LIST_H