#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    tty,
    debug,
} output_stream;

#define printf(...) fprintf(tty, __VA_ARGS__)
int fprintf(output_stream output, const char* __restrict, ...);
#define putchar(...) fputchar(tty, __VA_ARGS__)
int fputchar(output_stream output, int);
#define puts(...) fputs(tty, __VA_ARGS__)
int fputs(output_stream, const char*);

#ifdef __cplusplus
}
#endif

#endif
