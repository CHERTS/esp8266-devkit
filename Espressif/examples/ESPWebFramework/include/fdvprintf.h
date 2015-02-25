#ifndef _FDVPRINTF_H_
#define _FDVPRINTF_H_

extern "C"
{
#include <stdarg.h>
}


namespace fdv
{
	
	uint16_t vsprintf(char *buf, const char *fmt, va_list args);
	
}



#endif