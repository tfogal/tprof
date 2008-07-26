/** Tom Fogal, Thu Jul 24 17:33:58 EDT 2008
 * Simple profiler code. */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif /* !_POSIX_C_SOURCE */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

struct func {
    void *addr;
    void *caller;
    uint64_t enter;
};

/** Converts two addresses to the input to our hash function.
 * We want to hash on both the function address plus the address of the call
 * site.  We get them as void ptrs, so they must be cast before we can
 * meaningfully combine them. */
#define HASH_INPUT_ADDRV(a,b)                \
    ({ const int _ca = (const int) a;        \
       const int _cb = (const int) b;        \
       (gpointer)(_ca + _cb); })

/** Profiling data will be stored in this table. */
static GHashTable *profile;
static gboolean initialized = FALSE;

static inline uint64_t rdtsc();
static gboolean func_equal(gconstpointer a, gconstpointer b);

/** Initialize the hash table so we can assume it's values are valid. */
void __attribute__((constructor))
tprof_initialize()
{
    profile = g_hash_table_new_full(g_direct_hash, func_equal, NULL, g_free);
    puts("tprof startup ..");
    initialized = TRUE;
}

/** Frees any memory left in the hash table. */
static void __attribute__((destructor)) 
tprof_destroy()
{
    initialized = FALSE;
    g_hash_table_destroy(profile);
}

/** Inserts the given call into our hash table.
 * Note we assume that the function is not already in the table.  This is most
 * likely true.  However, for recursive functions (or a series of functions
 * with mutual recursion), this scheme totally breaks down.  We will overwrite
 * the older entry with the newer entry, effectively reporting the time of the
 * inner-most call of the given function. */
void
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
    struct func *fqn = g_malloc(sizeof(struct func));
    fqn->addr = this_fn;
    fqn->caller = call_site;

    /* This should not be necessary.  Our initialization function is marked
     * with the `constructor' attribute, so it should be called automagically
     * when the loader starts up the program.
     * However, this simply isn't happening inside VisIt, for reasons I don't
     * currently fathom.  Therefore we have a branch here and check if the
     * library has been initialized, and initialize it if so ... */
    if(initialized == FALSE) {
        tprof_initialize();
    }

    /* We do the insert and THEN record the time into the structure; we have no
     * idea how expensive the insert will be, and we want to get the timing
     * differences with as little overhead as possible. */
    g_hash_table_insert(profile, HASH_INPUT_ADDRV(this_fn, call_site), fqn);
    fqn->enter = rdtsc();
}


/** Finds the given function in our table.  Outputs the time difference between
 * then and now. */
void
__cyg_profile_func_exit(void *this_fn, void *call_site)
{
    Dl_info dl_fqn, dl_caller;
    uint64_t leave = rdtsc();

    gpointer *f = g_hash_table_lookup(profile,
                                      HASH_INPUT_ADDRV(this_fn, call_site));
    struct func *fqn = (struct func *) f;

    /* Now lookup the symbol name. */
    if(dladdr(this_fn, &dl_fqn) == 0 || dladdr(call_site, &dl_caller) == 0) {
        g_error("Could not resolve %p or %p", this_fn, call_site);
    }

    /* FIXME this should be changed to async file i/o; terminal i/o is slow. */
    if(strcmp(dl_fqn.dli_fname, dl_caller.dli_fname) == 0) {
        printf("%lld units: %s from %s (%s)\n", leave - fqn->enter,
               dl_fqn.dli_sname, dl_caller.dli_sname,
               dl_fqn.dli_fname);
    } else {
        printf("%s(%s) from %s(%s): %lld units.\n",
               dl_fqn.dli_sname, dl_fqn.dli_fname,
               dl_caller.dli_sname, dl_caller.dli_fname,
               leave - fqn->enter);
    }

    if(g_hash_table_remove(profile, HASH_INPUT_ADDRV(this_fn, call_site))
       != TRUE) {
        g_error("Could not remove function from internal hash.");
    }
}

static inline uint64_t
rdtsc()
{
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return (uint64_t)hi << 32 | lo;
}

static gboolean
func_equal(gconstpointer a, gconstpointer b) { return a == b; }
