/** Tom Fogal, CS820
 * Attributes useful when using GCC. */
#ifndef RB_ATT_H
#define RB_ATT_H

#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#ifdef __linux__
#  undef __USE_BSD
#  undef __USE_MISC
#  define __USE_BSD
#  define __USE_MISC
#  define _XOPEN_SOURCE 600
#elif defined(__sun__)
#  define __EXTENSIONS__
#endif

/** These were taken from Robert Love: http://rlove.org/log/2005102601
 * Except I changed them to use one underscore because I would get problems
 * clashing with internal glibc usages / redefinitions. */
#if __GNUC__ >= 3
#  define _pure      __attribute__ ((pure))
#  define _const  __attribute__ ((const))
#  define _noreturn  __attribute__ ((noreturn))
#  define _malloc __attribute__ ((malloc))
#  define _must_check   __attribute__ ((warn_unused_result))
#  define _deprecated   __attribute__ ((deprecated))
#  define _nonnull      __attribute__ ((nonnull))
#  define _used      __attribute__ ((used))
#  define _unused __attribute__ ((unused))
#  define _packed __attribute__ ((packed))
#  define likely(x)   __builtin_expect (!!(x), 1)
#  define unlikely(x) __builtin_expect (!!(x), 0)
#else
#  define inline      /* no inline */
#  define _pure      /* no pure */
#  define _const  /* no const */
#  define _noreturn  /* no noreturn */
#  define _malloc /* no malloc */
#  define _must_check   /* no warn_unused_result */
#  define _nonnull      /* no nonnull */
#  define _deprecated   /* no deprecated */
#  define _used      /* no used */
#  define _unused /* no unused */
#  define _packed /* no packed */
#  define likely(x)   (x)
#  define unlikely(x) (x)
#endif

#endif /* RB_ATT_H */
