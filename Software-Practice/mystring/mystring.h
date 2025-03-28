#ifndef MYSTRING_H
#define MYSTRING_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>

extern char *new_string(size_t size);
extern char *reserve_exact(char *str, size_t size);
extern char *reserve(char *str, size_t size);

/* Conversions string to numeric formats */
extern int32_t atoi_32(const char *str);
extern int64_t atol_64(const char *str);

/* Conversions numeric formats to string */
extern char *int2str(char *dest, int num);

/* String manipulation */
extern char *strcpy(char *dst, const char *src);
extern char *strncpy(char *dst, const char *src, size_t count);
extern char *strcat(char *dst, const char *src);
extern char *strncat(char *dst, const char *src, size_t count);
extern char *strdup(const char *str);

/* String examination */
extern size_t strlen(const char *str);
extern int strcmp(const char *lhs, const char *rhs);
extern int strncmp(const char *lhs, const char *rhs, size_t count);
extern char *strchr(const char *str, int ch);
extern char *strrchr(const char *str, int ch);
extern char *strpbrk(const char *str, const char *accept);
extern char *strstr(const char *str, const char *substr);
extern char *strtok(char *str, const char *delim);
extern char *strtok_r(char *str, const char *delim, char **saveptr);

/* Character array manipulation */
extern void *memcpy(void *dest, const void *str, size_t n);
extern void *memset(void *dest, int ch, size_t count);

#endif