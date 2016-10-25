#ifndef __STDLIB_WRAPPER_H__
#define __STDLIB_WRAPPER_H__

int memset_s(void *v, size_t smax, int c, size_t n);
void logmsg(int level, const char *format, ...);

#endif /* __STDLIB_WRAPPER_H__ */
