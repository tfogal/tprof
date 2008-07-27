/*
 * This file is part of libtprof.
 *
 * libtprof is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libtprof is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libtprof.  If not, see <http://www.gnu.org/licenses/>.
 */
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
