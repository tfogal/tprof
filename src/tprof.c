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

/** Converts two addresses to the input to our hash function.
 * We want to hash on both the function address plus the address of the call
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
static void g_free_null(gpointer *data, gpointer *user_data);

/** Initialize the hash table so we can assume it's values are valid. */
void __attribute__((constructor))
tprof_initialize()
{
    profile = g_hash_table_new_full(g_direct_hash, func_equal, NULL, NULL);
    initialized = TRUE;
}

/** Frees any memory left in the hash table. */
static void __attribute__((destructor)) 
tprof_destroy()
{
    initialized = FALSE;
    /* FIXME: g_hash_table_foreach (g_slist_destroy on the value) ... */
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
    if(initialized == FALSE) {
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
        printf("%s(%s) from %s(%s): %lld units.\n",
               dl_fqn.dli_sname, dl_fqn.dli_fname,
               dl_caller.dli_sname, dl_caller.dli_fname,
               leave - *enter);
    }

    /* Interestingly, we don't need to do an explicit HT removal.  We simply
     * pop the head off the list and then re-hash the list.  We'll insert a
     * NULL into the hash if the list empties! */
    g_free(entries->data);
    entries = g_slist_remove(entries, entries);
    g_hash_table_insert(profile, HASH_INPUT_ADDRV(this_fn, call_site), entries);
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

/** g_free but takes an extra parameter which is not used.  This gives us
 * g_free in a GFunc-compatible interface. */
static void
g_free_null(gpointer *data, G_GNUC_UNUSED gpointer *user_data)
{
    g_free(data);
}
