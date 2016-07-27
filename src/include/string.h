#pragma once

void *memcpy(void *dst, const void *src, size_t n);

void *memmove(void *dst, const void *src, size_t len);

char *strcpy(char * dst, const char * src);

char *strncpy(char * dst, const char * src, size_t len);

int strcoll(const char *s1, const char *s2);

char *strcat(char *s1, const char *s2);

char *strncat(char *s1, const char *s2, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);

int strcmp(const char *s1, const char *s2);

int strncmp(const char *s1, const char *s2, size_t n);

size_t strxfrm(char *s1, const char *s2, size_t n);

void *memchr(const void *s, int c, size_t n);

char *strchr(const char *s, int c);

size_t strcspn(const char *s1, const char *s2);

char *strpbrk(const char *s1, const char *s2);

char *strrchr(const char *s, int c);

size_t strspn(const char *s1, const char *s2);

char *strstr(const char *big, const char *little);

char *strtok(char *str, const char *sep);

void *memset(void *b, int c, size_t len);

char *strerror(int errnum);

size_t strlen(const char *s);
