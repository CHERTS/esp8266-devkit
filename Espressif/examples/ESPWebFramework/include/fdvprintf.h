#ifndef _FDVPRINTF_H_
#define _FDVPRINTF_H_

extern "C"
{
#include <stdarg.h>
}


namespace fdv
{
	
	// buf can stay in RAM or Flash
	// "strings" of args can stay in RAM or Flash
	uint16_t vsprintf(char *buf, const char *fmt, va_list args);
	uint16_t sprintf(char* str, char const *fmt, ...);
	
}



#endif