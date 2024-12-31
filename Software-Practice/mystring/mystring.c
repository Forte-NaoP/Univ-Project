//-----------------------------------------------------------
//
// SWE2007: Software Experiment II (Fall 2017)
//
// Skeleton code for PA #0
// September 6, 2017
//
// Ha-yun Lee, Jong-won Park
// Embedded Software Laboratory
// Sungkyunkwan University
//
//-----------------------------------------------------------

#include "mystring.h"
#include <stdbool.h>

#define isspace(ch) ((ch) == ' ' || (ch) == '\t' || (ch) == '\n' || \
					 (ch) == '\v' || (ch) == '\f' || (ch) == '\r')

char *new_string(size_t size) {
    char* str = reserve_exact(NULL, size);
    return str;
}

char *reserve_exact(char *str, size_t size) {
    char *new = (char *)calloc(size, sizeof(char));
    if (str != NULL) {
        strcpy(new, str);
        free(str);
    }
    return new;
}

char *reserve(char *str, size_t size) {
    size = 1 << (32 - __builtin_clz(size - 1));
    char *new = (char *)calloc(size, sizeof(char));
    if (str != NULL) {
        strcpy(new, str);
        free(str);
    }
    return new;
}
/* Conversions string to numeric formats */
int32_t atoi_32(const char *str) {
    int32_t result = 0;
    char *cur = str;
    size_t len = strlen(str);
    bool sign = false;

    if (str[0] == '-') {
        sign = true;
        ++cur;
        --len;
    }

    int32_t digit = 1;
    for (size_t i = 0; i < len - 1; ++i) {
        digit *= 10;
    }
    
    while (*cur != '\0') {
        result += (*cur - '0') * digit;
        ++cur; digit /= 10;
    }

    if (sign) {
        result = -result;
    }

    return result;
}

int64_t atoi_64(const char *str) {
    int64_t result = 0;
    char *cur = str;
    size_t len = strlen(str);
    bool sign = false;

    if (str[0] == '-') {
        sign = true;
        ++cur;
        --len;
    }

    int64_t digit = 1;
    for (size_t i = 0; i < len - 1; ++i) {
        digit *= 10;
    }
    
    while (*cur != '\0') {
        result += (*cur - '0') * digit;
        ++cur; digit /= 10;
    }

    if (sign) {
        result = -result;
    }

    return result;
}

/* Conversions numeric formats to string */
char *int2str(char *dest, int32_t num) {
    char *ptr = dest == NULL ? new_string(16) : dest;
    size_t len = 0;
    bool sign = false;
    
    if (num < 0) {
        sign = true;
        num = -num;
    }

    while (num > 0) {
        ptr[len++] = num % 10 + '0';
        num /= 10;
    }
    
    if (sign) {
        ptr[len++] = '-';
    }

    for (size_t i = 0; i < len / 2; ++i) {
        ptr[i] ^= ptr[len - i - 1] ^= ptr[i] ^= ptr[len - i - 1];
    }

    return ptr;
}

/* String manipulation */
char *strcpy(char *dst, const char *src) {
    char *cur = dst;
    while(*src != '\0') {
        *cur++ = *src++;
    }
    *cur='\0';
    return dst;
}

char *strncpy(char *dst, const char *src, size_t count) {
    char *cur = dst;
    while(*src != '\0' && count > 0) {
        *cur++ = *src++;
        --count;
    }
    while(count--) {
        *cur++ = '\0';
    }
    return dst;
}

char *strcat(char *dst, const char *src) {
    char *cur = dst;
    while(*cur != '\0') ++cur;
    while(*src != '\0') {
        *cur++ = *src++;
    }
    *cur='\0';
    return dst;
}

char *strncat(char *dst, const char *src, size_t count) {
    char *cur = dst;
    while(*cur != '\0') ++cur;
    while(*src != '\0' && count > 0) {
        *cur++ = *src++;
        --count;
    }
    *cur='\0';
    return dst;
}

char *strdup(const char *str) {
    size_t len = strlen(str);
    char *dst = new_string(len + 1);
    strcpy(dst, str);
    return dst;
}

/* String examination */
size_t strlen(const char *str) {
    size_t len = 0;
    while(*str != '\0'){
        ++len;
        ++str;
    }
    return len;
}

int strcmp(const char *lhs, const char *rhs) {
    while(*lhs != '\0' && *rhs != '\0') {
        if(*lhs > *rhs) return 1;
        if(*lhs < *rhs) return -1;
        ++lhs;
        ++rhs;
    }
    if (*lhs > *rhs) return 1;
    if (*lhs < *rhs) return -1;
    return 0;
}

int strncmp(const char *lhs, const char *rhs, size_t count) {
    while(*lhs != '\0' && *rhs != '\0' && count > 0) {
        if(*lhs > *rhs) return 1;
        if(*lhs < *rhs) return -1;
        ++lhs;
        ++rhs;
        --count;
    }
    if(count > 0) {
        if(*lhs > *rhs) return 1;
        if(*lhs < *rhs) return -1;
    }
    return 0;
}

char *strchr(const char *str, int ch) {
    while(*str != '\0'){
        if(*str == ch) return (char *)str;
        ++str;
    }
    if (ch == '\0') return (char *)str;
    return NULL;
}

char *strrchr(const char *str, int ch) {
    const char *ptr = NULL;
    while(*str != '\0'){
        if(*str == (char)ch) ptr = str;
        ++str;
    }
    if(ch == '\0') return (char *)str;
    return (char *)ptr;
}

char *strpbrk(const char *str, const char *accept) {
    while(*str!='\0') {
        if (strchr(accept, *str) != NULL) return (char *)str;
        ++str;
    }
    return NULL;
}

char *strstr(const char *str, const char *substr) {
    if (*substr == '\0') return (char *)str;

    const char *ptr, *subptr;
    while (*str != '\0') {
        ptr = str;
        subptr = substr;
        while (*ptr != '\0' && *subptr != '\0' && *ptr == *subptr) {
            if (*ptr == *subptr) {
                ++ptr;
                ++subptr;
            }
        }
        if (*subptr == '\0') return (char *)str;
        ++str;
    }
    return NULL;
}

char *strtok(char *str, const char *delim) {
    static char *last;
    if (str != NULL) last = str;
    if (last == NULL) return NULL;

    while (*last != '\0' && strchr(delim, *last) != NULL) {
        ++last;
    }

    if (*last == '\0') {
        last = NULL;
        return NULL;
    }

    char *token = last;
    while (*last != '\0' && strchr(delim, *last) == NULL) {
        ++last;
    }

    if (*last == '\0') {
        last = NULL;
    } else {
        *last++ = '\0';
    }

    return token;
}

char *strtok_r(char *str, const char *delim, char **saveptr) {
    if (str != NULL) *saveptr = str;
    if (*saveptr == NULL) return NULL;

    char *last = *saveptr;
    while (*last != '\0' && strchr(delim, *last) != NULL) {
        ++last;
    }

    if (*last == '\0') {
        *saveptr = NULL;
        return NULL;
    }

    char *token = last;
    while (*last != '\0' && strchr(delim, *last) == NULL) {
        ++last;
    }

    if (*last != '\0') {
        *last++ = '\0';
    }
    *saveptr = last;

    return token;
}

/* Character array manipulation */
void *memcpy(void *dest, const void *str, size_t n) {
    uint8_t *dest_8 = (uint8_t *)dest;
    uint8_t *src_8 = (uint8_t *)str;
    while (n--) {
        *dest_8++ = *src_8++;
    }
    return dest;
}

void *memset(void *dest, int ch, size_t count) {
    uint64_t value = (uint8_t)ch;
    value |= value << 8;
    value |= value << 16;
    value |= value << 32;

    uint64_t *ptr_64 = (uint64_t *)dest;
    while (count >= 8) {
        *ptr_64++ = value;
        count -= 8;
    }

    uint8_t *ptr_8 = (uint8_t *)ptr_64;
    while (count--) {
        *ptr_8++ = (uint8_t)ch;
    }

    return dest;
}
