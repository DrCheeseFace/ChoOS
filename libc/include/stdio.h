#ifndef _STDIO_H
#define _STDIO_H 1

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/cdefs.h>

#define EOF (-1)
#define EOVERFLOW (75)

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char *__restrict, ...);
int vprintf(const char *restrict format, va_list parameters);
int putchar(int);
int puts(const char *);

// TODO: Implement stdio and the write system call.

// putc(3)              output a character or word to a stream
// putw(3)              output a character or word to a stream
// clearerr(3)          check and reset stream status
// fclose(3)            close a stream
// fdopen(3)            stream open functions
// feof(3)              check and reset stream status
// ferror(3)            check and reset stream status
// fflush(3)            flush a stream
// fgetc(3)             get next character or word from input stream
// fgetpos(3)           reposition a stream
// fgets(3)             get a line from a stream
// fileno(3)            return the integer descriptor of the argument stream
// fmemopen(3)          open memory as stream
// fopen(3)             stream open functions
// fopencookie(3)       open a custom stream
// fprintf(3)           formatted output conversion
// fpurge(3)            flush a stream
// fputc(3)             output a character or word to a stream
// fputs(3)             output a line to a stream
// fread(3)             binary stream input/output
// freopen(3)           stream open functions
// fscanf(3)            input format conversion
// fseek(3)             reposition a stream
// fsetpos(3)           reposition a stream
// ftell(3)             reposition a stream
// fwrite(3)            binary stream input/output
// getc(3)              get next character or word from input stream
// getchar(3)           get next character or word from input stream
// gets(3)              get a line from a stream
// getw(3)              get next character or word from input stream
// mktemp(3)            make temporary filename (unique)
// open_memstream(3)    open a dynamic memory buffer stream
// open_wmemstream(3)   open a dynamic memory buffer stream
// perror(3)            system error messages
// remove(3)            remove directory entry
// rewind(3)            reposition a stream
// scanf(3)             input format conversion
// setbuf(3)            stream buffering operations
// setbuffer(3)         stream buffering operations
// setlinebuf(3)        stream buffering operations
// setvbuf(3)           stream buffering operations
// sprintf(3)           formatted output conversion
// sscanf(3)            input format conversion
// strerror(3)          system error messages
// sys_errlist(3)       system error messages
// sys_nerr(3)          system error messages
// tempnam(3)           temporary file routines
// tmpfile(3)           temporary file routines
// tmpnam(3)            temporary file routines
// ungetc(3)            un-get character from input stream
// vfprintf(3)          formatted output conversion
// vfscanf(3)           input format conversion
// vprintf(3)           formatted output conversion
// vscanf(3)            input format conversion
// vsprintf(3)          formatted output conversion
// vsscanf(3)           input format conversion
//
// ssize_t write(size_t count; int fd, const void buf[count], size_t count);

#ifdef __cplusplus
}
#endif

#endif
