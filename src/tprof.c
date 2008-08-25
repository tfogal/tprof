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
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif /* !_POSIX_C_SOURCE */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "attributes.h"

/** Converts two addresses to the input to our hash function.
 * We want to hash on both the function address and the address of the call
 * site.  We get them as void ptrs, so they must be cast before we can
 * meaningfully combine them. */
#define HASH_INPUT_ADDRV(a,b)                \
    ({ const int _ca = (const int) a;        \
       const int _cb = (const int) b;        \
       (gpointer)(_ca + _cb); })

/** Profiling data will be stored in this table. */
static GHashTable *profile = NULL;
static gboolean initialized = FALSE;

static inline guint64 rdtsc();
static gboolean func_equal(gconstpointer a, gconstpointer b);
static void _free_chain(gpointer key, gpointer value,  gpointer user_data);

/** Initialize the hash table so we can assume it's values are valid. */
void __attribute__((constructor))
tprof_initialize()
{
    profile = g_hash_table_new(g_direct_hash, func_equal);
    initialized = TRUE;
}

/** Frees any memory left in the hash table. */
static void __attribute__((destructor)) 
tprof_destroy()
{
    initialized = FALSE;
    g_hash_table_foreach(profile, _free_chain, NULL);
    g_hash_table_destroy(profile);
}

/** Inserts the given call into our hash table. */
void
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
    guint64 *entry;
    /* This should not be necessary.  Our initialization function is marked
     * with the `constructor' attribute, so it should be called automagically
     * when the loader starts up the program.
     * However, this simply isn't happening inside VisIt, for reasons I don't
     * currently fathom.  Therefore we have a branch here and check if the
     * library has been initialized, and initialize it if so ... */
    if(unlikely(initialized == FALSE)) {
        tprof_initialize();
    }

    GSList *entries = (GSList *)
        g_hash_table_lookup(profile, HASH_INPUT_ADDRV(this_fn, call_site));
    entry = g_malloc(sizeof(guint64));
    entries = g_slist_prepend(entries, entry);
    
    /* We do the insert and THEN record the time into the structure; our
     * instrumentation might have been expensive, and we want to get the timing
     * differences with as little overhead as possible. */
    g_hash_table_insert(profile, HASH_INPUT_ADDRV(this_fn, call_site), entries);
    *entry = rdtsc();
}

/** Finds the given function in our table.  Outputs the time difference between
 * then and now. */
void
__cyg_profile_func_exit(void *this_fn, void *call_site)
{
    Dl_info dl_fqn, dl_caller;
    guint64 leave = rdtsc();

    GSList *entries = (GSList *)
        g_hash_table_lookup(profile, HASH_INPUT_ADDRV(this_fn, call_site));
    g_assert(entries);
    guint64 *enter = (guint64*) entries->data;

    /* Now lookup the symbol name. */
    if(dladdr(this_fn, &dl_fqn) == 0 || dladdr(call_site, &dl_caller) == 0) {
        g_error("Could not resolve %p or %p", this_fn, call_site);
    }

    /* FIXME this should be changed to async file i/o; terminal i/o is slow. */
    if(strcmp(dl_fqn.dli_fname, dl_caller.dli_fname) == 0) {
        printf("%lld units: %s from %s (%s)\n", leave - *enter,
               dl_fqn.dli_sname, dl_caller.dli_sname,
               dl_fqn.dli_fname);
    } else {
        printf("%lld units: %s(%s) from %s(%s)\n", leave - *enter,
               dl_fqn.dli_sname, dl_fqn.dli_fname,
               dl_caller.dli_sname, dl_caller.dli_fname);
    }

    /* Interestingly, we don't need to do an explicit HT removal.  We simply
     * pop the head off the list and then re-hash the list.  We'll insert a
     * NULL into the hash if the list empties -- which is equivalent to saying
     * the hash doesn't exist!  Perfect. */
    GSList *newhead = g_slist_next(entries);
    g_hash_table_insert(profile, HASH_INPUT_ADDRV(this_fn, call_site), newhead);

    /* Remove the data and set it to NULL.  It must be nullified so that
     * _free_chain (called at program exit) knows not to free it again. */
    g_free(entries->data);
    entries->data = NULL;
    g_slist_free_1(entries); /* ... and, of course, delete the old head. */
}

static inline guint64
rdtsc()
{
    guint32 lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return (guint64)hi << 32 | lo;
}

static gboolean
func_equal(gconstpointer a, gconstpointer b) { return a == b; }

/** This makes sure al the memory is cleaned up from a given element in the
 * hash table.  The second parameter is assumed to be the head of a GSList
 * (potentially null, for the empty list).  This iterates over the list and
 * free's all the data pointers.  Finally, it deletes the list altogether. */
static void
_free_chain(G_GNUC_UNUSED gpointer key, gpointer value,
            G_GNUC_UNUSED gpointer user_data)
{
    GSList *list = (GSList *) value;
    GSList *elem = list;
    /* Yes, we iterate over the list twice; once right now, and then
     * g_slist_free will do it's own iteration.  We could fairly easily
     * implement this in one pass, but this is only called at the end of
     * program execution anyway. */
    while(elem) {
        g_free(elem->data);
        elem = g_slist_next(elem);
    }
    g_slist_free(list);
}
