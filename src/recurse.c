/** Simple program to test recursion with libtprof. */
#include <stdio.h>
#include <glib.h>

struct node {
    int d;
    const struct node *next;
};

/** adds at head.  returns new head. */
struct node *
ll_add(const struct node *list, int x)
{
    struct node *p = g_malloc(sizeof(struct node));
    p->d = x;
    p->next = list;
    return p;
}

void
ll_print(const struct node * list)
{
    const struct node * iter = list;
    if(iter) {
        printf("item: %d\n", iter->d);
        ll_print(iter->next);
    }
}

int
main()
{
    struct node *head = NULL;
    head = ll_add(head, 1);
    head = ll_add(head, 2);
    head = ll_add(head, 3);
    head = ll_add(head, 5);

    ll_print(head);
    return 0;
}
