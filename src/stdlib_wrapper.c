#define _BSD_SOURCE
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include "stdlib_wrapper.h"

#ifndef LOG_AUTHPRIV
#define LOG_AUTHPRIV LOG_AUTH
#endif

int memset_s(void *v, size_t smax, int c, size_t n)
{
	volatile unsigned char *p = v;

	if (!v || smax > n || n > smax)
		return -EINVAL;

	while (smax-- && n--)
		*p++ = (unsigned char)c;

	return 0;
}

void logmsg(int level, const char *format, ...)
{
	va_list args;

	va_start(args, format);

#ifdef DEBUG_BUILD
	switch (level) {
	case LOG_EMERG:
		printf("[emerg  ] ");
		break;
	case LOG_ALERT:
		printf("[alert  ] ");
		break;
	case LOG_CRIT:
		printf("[crit   ] ");
		break;
	case LOG_ERR:
		printf("[err    ] ");
		break;
	case LOG_WARNING:
		printf("[warning] ");
		break;
	case LOG_NOTICE:
		printf("[notice ] ");
		break;
	case LOG_INFO:
		printf("[info   ] ");
		break;
	case LOG_DEBUG:
		printf("[debug  ] ");
		break;
	default:
		printf("[<unknown>] ");
		break;
	}
	vprintf(format, args);
	printf("\n");
#else
	vsyslog(level | LOG_AUTHPRIV, format, args);
#endif
	va_end(args);
}
