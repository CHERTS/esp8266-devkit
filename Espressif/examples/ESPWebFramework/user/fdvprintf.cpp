/*
This code is based on a file that contains the following:
 Copyright (C) 2002 Michael Ringgaard. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 
 1. Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.  
 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.  
 3. Neither the name of the project nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission. 
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 SUCH DAMAGE.

*/


#include "fdv.h"

extern "C"
{
	#include <stdarg.h>
	#include <math.h>
}



// Enable if you need "%f"
//#define HAS_FLOAT 1

#define ZEROPAD  	(1<<0)	/* Pad with zero */
#define SIGN    	(1<<1)	/* Unsigned/signed long */
#define PLUS    	(1<<2)	/* Show plus */
#define SPACE   	(1<<3)	/* Spacer */
#define LEFT    	(1<<4)	/* Left justified */
#define HEX_PREP 	(1<<5)	/* 0x */
#define UPPERCASE   (1<<6)	/* 'ABCDEF' */

#define is_digit(c) ((c) >= '0' && (c) <= '9')

	

namespace fdv
{


struct Str
{
	// set buffer=NULL to only know the string length
	Str(char* buffer)
		: m_buffer(buffer), m_length(0)
	{
	}
	char* operator++(int)
	{
		if (m_buffer)
			return &m_buffer[m_length++];
		else
		{
			++m_length;
			return &m_dummyChar;
		}
	}
	char& operator=(char const c)
	{
		if (m_buffer)
		{
			m_buffer[m_length] = c;
			return m_buffer[m_length];
		}
		else
			return m_dummyChar;
	}
	char& operator*()
	{
		if (m_buffer)
			return m_buffer[m_length];
		else
			return m_dummyChar;
	}
	uint16_t getLength()
	{
		return m_length;
	}
private:
	char*    m_buffer;
	uint16_t m_length;
	char     m_dummyChar;
};



static char const lower_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static char const upper_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


static size_t FUNC_FLASHMEM strnlen(const char *s, size_t count)
{
  const char *sc;
  for (sc = s; *sc != '\0' && count--; ++sc);
  return sc - s;
}

static int FUNC_FLASHMEM ee_skip_atoi(const char **s)
{
  int i = 0;
  while (is_digit(fdv::getChar(*s))) 
	  i = i*10 + fdv::getChar((*s)++) - '0';
  return i;
}

static void FUNC_FLASHMEM ee_number(Str& str, long num, int base, int size, int precision, int type)
{
  char c, sign, tmp[66];
  char const * dig = lower_digits;
  int i;

  if (type & UPPERCASE)  dig = upper_digits;
  if (type & LEFT) type &= ~ZEROPAD;
  if (base < 2 || base > 36) return;
  
  c = (type & ZEROPAD) ? '0' : ' ';
  sign = 0;
  if (type & SIGN)
  {
    if (num < 0)
    {
      sign = '-';
      num = -num;
      size--;
    }
    else if (type & PLUS)
    {
      sign = '+';
      size--;
    }
    else if (type & SPACE)
    {
      sign = ' ';
      size--;
    }
  }

  if (type & HEX_PREP)
  {
    if (base == 16)
      size -= 2;
    else if (base == 8)
      size--;
  }

  i = 0;

  if (num == 0)
    tmp[i++] = '0';
  else
  {
    while (num != 0)
    {
      tmp[i++] = dig[((unsigned long) num) % (unsigned) base];
      num = ((unsigned long) num) / (unsigned) base;
    }
  }

  if (i > precision) precision = i;
  size -= precision;
  if (!(type & (ZEROPAD | LEFT))) while (size-- > 0) *str++ = ' ';
  if (sign) *str++ = sign;
  
  if (type & HEX_PREP)
  {
    if (base == 8)
      *str++ = '0';
    else if (base == 16)
    {
      *str++ = '0';
      *str++ = lower_digits[33];
    }
  }

  if (!(type & LEFT)) while (size-- > 0) *str++ = c;
  while (i < precision--) *str++ = '0';
  while (i-- > 0) *str++ = tmp[i];
  while (size-- > 0) *str++ = ' ';

  return;
}

static void FUNC_FLASHMEM eaddr(Str& str, unsigned char *addr, int size, int precision, int type)
{
  char tmp[24];
  char const * dig = lower_digits;
  int i, len;

  if (type & UPPERCASE)  dig = upper_digits;
  len = 0;
  for (i = 0; i < 6; i++)
  {
    if (i != 0) tmp[len++] = ':';
    tmp[len++] = dig[addr[i] >> 4];
    tmp[len++] = dig[addr[i] & 0x0F];
  }

  if (!(type & LEFT)) while (len < size--) *str++ = ' ';
  for (i = 0; i < len; ++i) *str++ = tmp[i];
  while (len < size--) *str++ = ' ';

  return;
}

static void FUNC_FLASHMEM iaddr(Str& str, unsigned char *addr, int size, int precision, int type)
{
  char tmp[24];
  int i, n, len;

  len = 0;
  for (i = 0; i < 4; i++)
  {
    if (i != 0) tmp[len++] = '.';
    n = addr[i];
    
    if (n == 0)
      tmp[len++] = lower_digits[0];
    else
    {
      if (n >= 100) 
      {
        tmp[len++] = lower_digits[n / 100];
        n = n % 100;
        tmp[len++] = lower_digits[n / 10];
        n = n % 10;
      }
      else if (n >= 10) 
      {
        tmp[len++] = lower_digits[n / 10];
        n = n % 10;
      }

      tmp[len++] = lower_digits[n];
    }
  }

  if (!(type & LEFT)) while (len < size--) *str++ = ' ';
  for (i = 0; i < len; ++i) *str++ = tmp[i];
  while (len < size--) *str++ = ' ';

  return;
}

#ifdef HAS_FLOAT

#define CVTBUFSIZE 80

static char * FUNC_FLASHMEM cvt(double arg, int ndigits, int *decpt, int *sign, char *buf, int eflag)
{
  int r2;
  double fi, fj;
  char *p, *p1;

  if (ndigits < 0) ndigits = 0;
  if (ndigits >= CVTBUFSIZE - 1) ndigits = CVTBUFSIZE - 2;
  r2 = 0;
  *sign = 0;
  p = &buf[0];
  if (arg < 0)
  {
    *sign = 1;
    arg = -arg;
  }
  arg = modf(arg, &fi);
  p1 = &buf[CVTBUFSIZE];

  if (fi != 0) 
  {
    p1 = &buf[CVTBUFSIZE];
    while (fi != 0) 
    {
      fj = modf(fi / 10, &fi);
      *--p1 = (int)((fj + .03) * 10) + '0';
      r2++;
    }
    while (p1 < &buf[CVTBUFSIZE]) *p++ = *p1++;
  } 
  else if (arg > 0)
  {
    while ((fj = arg * 10) < 1) 
    {
      arg = fj;
      r2--;
    }
  }
  p1 = &buf[ndigits];
  if (eflag == 0) p1 += r2;
  *decpt = r2;
  if (p1 < &buf[0]) 
  {
    buf[0] = '\0';
    return buf;
  }
  while (p <= p1 && p < &buf[CVTBUFSIZE])
  {
    arg *= 10;
    arg = modf(arg, &fj);
    *p++ = (int) fj + '0';
  }
  if (p1 >= &buf[CVTBUFSIZE]) 
  {
    buf[CVTBUFSIZE - 1] = '\0';
    return buf;
  }
  p = p1;
  *p1 += 5;
  while (*p1 > '9') 
  {
    *p1 = '0';
    if (p1 > buf)
      ++*--p1;
    else 
    {
      *p1 = '1';
      (*decpt)++;
      if (eflag == 0) 
      {
        if (p > buf) *p = '0';
        p++;
      }
    }
  }
  *p = '\0';
  return buf;
}

static char * FUNC_FLASHMEM ecvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf)
{
  return cvt(arg, ndigits, decpt, sign, buf, 1);
}

static char * FUNC_FLASHMEM fcvtbuf(double arg, int ndigits, int *decpt, int *sign, char *buf)
{
  return cvt(arg, ndigits, decpt, sign, buf, 0);
}

static void FUNC_FLASHMEM ee_bufcpy(char *pd, char *ps, int count) 
{
	char *pe=ps+count;
	while (ps!=pe)
		*pd++=*ps++;
}

static void FUNC_FLASHMEM parse_float(double value, char *buffer, char fmt, int precision)
{
  int decpt, sign, exp, pos;
  char *fdigits = NULL;
  char * cvtbuf = (char*)Memory::malloc(CVTBUFSIZE);
  int capexp = 0;
  int magnitude;

  if (fmt == 'G' || fmt == 'E')
  {
    capexp = 1;
    fmt += 'a' - 'A';
  }

  if (fmt == 'g')
  {
    fdigits = ecvtbuf(value, precision, &decpt, &sign, cvtbuf);
    magnitude = decpt - 1;
    if (magnitude < -4  ||  magnitude > precision - 1)
    {
      fmt = 'e';
      precision -= 1;
    }
    else
    {
      fmt = 'f';
      precision -= decpt;
    }
  }

  if (fmt == 'e')
  {
    fdigits = ecvtbuf(value, precision + 1, &decpt, &sign, cvtbuf);

    if (sign) *buffer++ = '-';
    *buffer++ = *fdigits;
    if (precision > 0) *buffer++ = '.';
    ee_bufcpy(buffer, fdigits + 1, precision);
    buffer += precision;
    *buffer++ = capexp ? 'E' : 'e';

    if (decpt == 0)
    {
      if (value == 0.0)
        exp = 0;
      else
        exp = -1;
    }
    else
      exp = decpt - 1;

    if (exp < 0)
    {
      *buffer++ = '-';
      exp = -exp;
    }
    else
      *buffer++ = '+';

    buffer[2] = (exp % 10) + '0';
    exp = exp / 10;
    buffer[1] = (exp % 10) + '0';
    exp = exp / 10;
    buffer[0] = (exp % 10) + '0';
    buffer += 3;
  }
  else if (fmt == 'f')
  {
    fdigits = fcvtbuf(value, precision, &decpt, &sign, cvtbuf);
    if (sign) *buffer++ = '-';
    if (*fdigits)
    {
      if (decpt <= 0)
      {
        *buffer++ = '0';
        *buffer++ = '.';
        for (pos = 0; pos < -decpt; pos++) *buffer++ = '0';
        while (*fdigits) *buffer++ = *fdigits++;
      }
      else
      {
        pos = 0;
        while (*fdigits)
        {
          if (pos++ == decpt) *buffer++ = '.';
          *buffer++ = *fdigits++;
        }
      }
    }
    else
    {
      *buffer++ = '0';
      if (precision > 0)
      {
        *buffer++ = '.';
        for (pos = 0; pos < precision; pos++) *buffer++ = '0';
      }
    }
  }

  *buffer = '\0';
  Memory::free(cvtbuf);
}

static void FUNC_FLASHMEM decimal_point(char *buffer)
{
  while (*buffer)
  {
    if (*buffer == '.') return;
    if (*buffer == 'e' || *buffer == 'E') break;
    buffer++;
  }

  if (*buffer)
  {
    int n = strnlen(buffer,256);
    while (n > 0) 
    {
      buffer[n + 1] = buffer[n];
      n--;
    }

    *buffer = '.';
  }
  else
  {
    *buffer++ = '.';
    *buffer = '\0';
  }
}

static void FUNC_FLASHMEM cropzeros(char *buffer)
{
  char *stop;

  while (*buffer && *buffer != '.') buffer++;
  if (*buffer++)
  {
    while (*buffer && *buffer != 'e' && *buffer != 'E') buffer++;
    stop = buffer--;
    while (*buffer == '0') buffer--;
    if (*buffer == '.') buffer--;
    while (buffer!=stop)
		*++buffer=0;
  }
}

static void FUNC_FLASHMEM flt(Str& str, double num, int size, int precision, char fmt, int flags)
{
  char tmp[80];
  char c, sign;
  int n, i;

  // Left align means no zero padding
  if (flags & LEFT) flags &= ~ZEROPAD;

  // Determine padding and sign char
  c = (flags & ZEROPAD) ? '0' : ' ';
  sign = 0;
  if (flags & SIGN)
  {
    if (num < 0.0)
    {
      sign = '-';
      num = -num;
      size--;
    }
    else if (flags & PLUS)
    {
      sign = '+';
      size--;
    }
    else if (flags & SPACE)
    {
      sign = ' ';
      size--;
    }
  }

  // Compute the precision value
  if (precision < 0)
    precision = 6; // Default precision: 6

  // Convert floating point number to text
  parse_float(num, tmp, fmt, precision);

  if ((flags & HEX_PREP) && precision == 0) decimal_point(tmp);
  if (fmt == 'g' && !(flags & HEX_PREP)) cropzeros(tmp);

  n = strnlen(tmp,256);

  // Output number with alignment and padding
  size -= n;
  if (!(flags & (ZEROPAD | LEFT))) while (size-- > 0) *str++ = ' ';
  if (sign) *str++ = sign;
  if (!(flags & LEFT)) while (size-- > 0) *str++ = c;
  for (i = 0; i < n; i++) *str++ = tmp[i];
  while (size-- > 0) *str++ = ' ';

  return;
}

#endif

// buf can stay in RAM or Flash
// "strings" of args can stay in RAM or Flash
// buf = NULL -> just count required buffer length
uint16_t FUNC_FLASHMEM vsprintf(char *buf, const char *fmt, va_list args)
{
  int len;
  unsigned long num;
  int i, base;
  char const * s;

  int flags;            // Flags to number()

  int field_width;      // Width of output field
  int precision;        // Min. # of digits for integers; max number of chars for from string
  int qualifier;        // 'h', 'l', or 'L' for integer fields

  Str str(buf);

  for (; fdv::getChar(fmt); fmt++)
  {
    if (fdv::getChar(fmt) != '%')
    {
      *str++ = fdv::getChar(fmt);
      continue;
    }
                  
    // Process flags
    flags = 0;
repeat:
    fmt++; // This also skips first '%'
    switch (fdv::getChar(fmt))
    {
      case '-': flags |= LEFT; goto repeat;
      case '+': flags |= PLUS; goto repeat;
      case ' ': flags |= SPACE; goto repeat;
      case '#': flags |= HEX_PREP; goto repeat;
      case '0': flags |= ZEROPAD; goto repeat;
    }
          
    // Get field width
    field_width = -1;
    if (is_digit(fdv::getChar(fmt)))
      field_width = ee_skip_atoi(&fmt);
    else if (fdv::getChar(fmt) == '*')
    {
      fmt++;
      field_width = va_arg(args, int);
      if (field_width < 0)
      {
        field_width = -field_width;
        flags |= LEFT;
      }
    }

    // Get the precision
    precision = -1;
    if (fdv::getChar(fmt) == '.')
    {
      ++fmt;    
      if (is_digit(fdv::getChar(fmt)))
        precision = ee_skip_atoi(&fmt);
      else if (fdv::getChar(fmt) == '*')
      {
        ++fmt;
        precision = va_arg(args, int);
      }
      if (precision < 0) precision = 0;
    }

    // Get the conversion qualifier
    qualifier = -1;
    if (fdv::getChar(fmt) == 'l' || fdv::getChar(fmt) == 'L')
    {
      qualifier = fdv::getChar(fmt);
      fmt++;
    }

    // Default base
    base = 10;

    switch (fdv::getChar(fmt))
    {
      case 'c':
        if (!(flags & LEFT)) while (--field_width > 0) *str++ = ' ';
        *str++ = (unsigned char) va_arg(args, int);
        while (--field_width > 0) *str++ = ' ';
        continue;

      case 's':
        s = va_arg(args, char *);
        if (!s) s = FSTR("<NULL>");
        len = f_strnlen(s, precision);
        if (!(flags & LEFT)) while (len < field_width--) *str++ = ' ';
        for (i = 0; i < len; ++i) *str++ = fdv::getChar(s++);
        while (len < field_width--) *str++ = ' ';
        continue;

      case 'p':
        if (field_width == -1)
        {
          field_width = 2 * sizeof(void *);
          flags |= ZEROPAD;
        }
        ee_number(str, (unsigned long) va_arg(args, void *), 16, field_width, precision, flags);
        continue;

      case 'A':
        flags |= UPPERCASE;

      case 'a':
        if (qualifier == 'l')
          eaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
        else
          iaddr(str, va_arg(args, unsigned char *), field_width, precision, flags);
        continue;

      // Integer number formats - set up the flags and "break"
      case 'o':
        base = 8;
        break;

      case 'X':
        flags |= UPPERCASE;

      case 'x':
        base = 16;
        break;

      case 'd':
      case 'i':
        flags |= SIGN;

      case 'u':
        break;

#ifdef HAS_FLOAT

      case 'f':
        flt(str, va_arg(args, double), field_width, precision, fdv::getChar(fmt), flags | SIGN);
        continue;

#endif

      default:
        if (fdv::getChar(fmt) != '%') *str++ = '%';
        if (fdv::getChar(fmt))
          *str++ = fdv::getChar(fmt);
        else
          --fmt;
        continue;
    }

    if (qualifier == 'l')
      num = va_arg(args, unsigned long);
    else if (flags & SIGN)
      num = va_arg(args, int);
    else
      num = va_arg(args, unsigned int);

    ee_number(str, num, base, field_width, precision, flags);
  }

  *str = '\0';
  return str.getLength();
}


}	// end of fdv namespace