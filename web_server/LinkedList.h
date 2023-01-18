

typedef struct LinkedList {
    void *data;
    LinkedList *next;
} LinkedList;

LinkedList *make_linked_list(void *data, LinkedList *next);

