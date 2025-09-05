#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((__noreturn__))
void abort(void);

// i put this in abort.c sorry
__attribute__((__noreturn__))
void exit(int exit_status);

#ifdef __cplusplus
}
#endif

#endif
