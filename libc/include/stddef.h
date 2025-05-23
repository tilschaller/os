#ifndef _STDDEF_H
#define _STDDEF_H

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef unsigned long size_t;

typedef long ptrdiff_t;

#define offsetof(type, member) ((size_t)&(((type *)0)->member))

#endif // _STDDEF_H
