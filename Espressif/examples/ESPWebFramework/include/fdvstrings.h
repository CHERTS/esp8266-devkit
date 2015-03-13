/*
# Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com)
# Copyright (c) 2015 Fabrizio Di Vittorio.
# All rights reserved.

# GNU GPL LICENSE
#
# This module is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; latest version thereof,
# available at: <http://www.gnu.org/licenses/gpl.txt>.
#
# This module is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this module; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*/


#ifndef _FDVSTRINGS_H_
#define _FDVSTRINGS_H_


#include "fdv.h"


extern "C"
{
	#include "stdlib.h"
}



namespace fdv
{



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_strstr
// return value is "IteratorSTR()" if not found
// If str or substr is in RAM or Flash use CharIterator
// If str or substr is in Chunk use ChunkBuffer<...>::Iterator
template <typename IteratorSTR, typename IteratorSUBSTR>
static IteratorSTR FUNC_FLASHMEM t_strstr(IteratorSTR str, IteratorSUBSTR substr)
{
	IteratorSTR string(str);
	IteratorSUBSTR b(substr);
	if (*b == 0)
		return string;
	for ( ; *string != 0; ++string)
	{
		if (*string != *b)
			continue;
		IteratorSTR a(string);
		while (true)
		{
			if (*b == 0)
				return string;			
			if (*a++ != *b++)
				break;	    
		}
		b = IteratorSUBSTR(substr);
	}
	return IteratorSTR();
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_strstr
// return value is "IteratorSTR()" if not found
// If str or substr is in RAM or Flash use CharIterator
// If str or substr is in Chunk use ChunkBuffer<...>::Iterator
template <typename IteratorSTR, typename IteratorSUBSTR>
static IteratorSTR FUNC_FLASHMEM t_strstr(IteratorSTR str, IteratorSTR strEnd, IteratorSUBSTR substr)
{
	IteratorSUBSTR b(substr);
	if (*b == 0)
		return str;
	for ( ; str != strEnd; ++str)
	{
		if (*str != *b)
			continue;
		IteratorSTR a(str);
		while (a != strEnd)
		{
			if (*a++ != *b++)
				break;	    
			if (*b == 0)
				return str;
		}
		b = IteratorSUBSTR(substr);
	}
	return IteratorSTR();
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_strcmp
// If s1 or s2 is in RAM or Flash use CharIterator
// If s1 or s2 is in Chunk use ChunkBuffer<...>::Iterator
template <typename IteratorS1, typename IteratorS2>
static int32 FUNC_FLASHMEM t_strcmp(IteratorS1 s1, IteratorS2 s2)
{
	while(*s1 && (*s1 == *s2))
		++s1, ++s2;
	return (uint8_t)*s1 - (uint8_t)*s2;		
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_compare
template <typename IteratorS1, typename IteratorS2>
static bool FUNC_FLASHMEM t_compare(IteratorS1 s1, IteratorS1 s1End, IteratorS2 s2, IteratorS2 s2End)
{
	while(s1 != s1End && s2 != s2End && *s1 == *s2)
		++s1, ++s2;
	return s1 == s1End && s2 == s2End;
}

// s1 must be zero terminated
template <typename IteratorS1, typename IteratorS2>
static bool FUNC_FLASHMEM t_compare(IteratorS1 s1, IteratorS2 s2, IteratorS2 s2End)
{
	while(*s1 && s2 != s2End && *s1 == *s2)
		++s1, ++s2;
	return *s1 == 0 && s2 == s2End;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_memcmp
// If s1 or s2 is in RAM or Flash use CharIterator
// If s1 or s2 is in Chunk use ChunkBuffer<...>::Iterator
template <typename IteratorS1, typename IteratorS2>
static int32 FUNC_FLASHMEM t_memcmp(IteratorS1 s1, IteratorS2 s2, uint32_t length)
{
	while(length--)
	{
		if (*s1 != *s2)
			return (uint8_t)*s1 - (uint8_t)*s2;
		++s1, ++s2;
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_strcpy
// If source is in RAM or Flash use CharIterator
// If source is in Chunk use ChunkBuffer<...>::Iterator
template <typename Iterator>
static char* FUNC_FLASHMEM t_strcpy(char* destination, Iterator source)
{
	char* dest = destination;
	while (*dest++ = *source++);
	return destination;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_memcpy
// If source is in RAM or Flash use ByteIterator
// If source is in Chunk use ChunkBuffer<...>::Iterator
template <typename Iterator>
static void* FUNC_FLASHMEM t_memcpy(void* destination, Iterator source, uint32_t length)
{
	uint8_t* dest = (uint8_t*)destination;
	while(length--)
		*dest++ = *source++;
	return destination;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_strlen
// If str is in RAM or Flash use CharIterator
// If str is in Chunk use ChunkBuffer<...>::Iterator
template <typename Iterator>
static uint32_t FUNC_FLASHMEM t_strlen(Iterator str)
{
	uint32_t len = 0;
	for (; *str; ++str, ++len);
	return len;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_strnlen
// If str is in RAM or Flash use CharIterator
// If str is in Chunk use ChunkBuffer<...>::Iterator
template <typename Iterator>
static uint32_t FUNC_FLASHMEM t_strnlen(Iterator str, uint32_t maxlen)
{
	uint32_t len = 0;
	for (; len != maxlen && *str; ++str, ++len);
	return len;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_strdup
// If source is in RAM or Flash use CharIterator
// If source is in Chunk use ChunkBuffer<...>::Iterator
// use delete[] to free memory
template <typename Iterator>
static char* FUNC_FLASHMEM t_strdup(Iterator source)
{
	return t_strcpy(new char[t_strlen(source) + 1], source);
}



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_memdup
// If source is in RAM or Flash use ByteIterator
// If source is in Chunk use ChunkBuffer<...>::Iterator
// use delete[] to free memory
template <typename Iterator>
static void* FUNC_FLASHMEM t_memdup(Iterator source, uint32_t length)
{
	return t_memcpy(new uint8_t[length], source, length);
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// t_strtol
template <typename Iterator>
static int32_t FUNC_FLASHMEM t_strtol(Iterator str, int32_t base)
{
	APtr<char> tempbuf(t_strdup(str));
	return strtol(tempbuf.get(), NULL, base);
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// String utilities to work with both Flash stored strings and RAM stored strings
//
// Example:
//   static char const STR1[] FLASHMEM = "1234567890";
//   
//   if (f_strcmp(STR1, otherstring) == 0)
//     dosomething();
// 
// Example:
//   if (f_strcmp(FSTR("1234567890"), otherstring) == 0)
//     dosomething();
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// f_strlen
// str can be stored in Flash or/and in RAM
static inline uint32_t FUNC_FLASHMEM f_strlen(char const* str)
{
	return t_strlen(CharIterator(str));
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// f_strnlen
// str can be stored in Flash or/and in RAM
static inline uint32_t FUNC_FLASHMEM f_strnlen(char const* str, uint32_t maxlen)
{
	return t_strnlen(CharIterator(str), maxlen);
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// f_strcpy
// source can be stored in Flash or/and in RAM
static inline char* FUNC_FLASHMEM f_strcpy(char* destination, char const* source)
{
	return t_strcpy(destination, CharIterator(source));
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// f_strdup
// use delete[] to free memory
// str can be stored in Flash or/and in RAM
static inline char* FUNC_FLASHMEM f_strdup(char const* str)
{
	return t_strdup(CharIterator(str));
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// f_memdup
// use delete[] to free memory
// str can be stored in Flash or/and in RAM
static inline void* FUNC_FLASHMEM f_memdup(void const* buffer, uint32_t length)
{
	return t_memdup(ByteIterator((uint8_t const*)buffer), length);
}
		

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// f_strcmp
// both s1 and s2 can be stored in Flash or/and in RAM
static inline int32_t FUNC_FLASHMEM f_strcmp(char const* s1, char const* s2)
{
	return t_strcmp(CharIterator(s1), CharIterator(s2));
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// f_memcmp
// both s1 and s2 can be stored in Flash or/and in RAM
static inline int32_t FUNC_FLASHMEM f_memcmp(void const* s1, void const* s2, uint32_t length)
{
	return t_memcmp(ByteIterator((uint8_t const*)s1), ByteIterator((uint8_t const*)s2), length);
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// f_memcpy
// source can be stored in Flash or/and in RAM
static void* FUNC_FLASHMEM f_memcpy(void* destination, void const* source, uint32_t length)
{
	return t_memcpy(destination, ByteIterator((uint8_t const*)source), length);
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// f_strstr
// both str and substr can be stored in Flash or/and RAM
static inline char const* FUNC_FLASHMEM f_strstr(char const* str, char const* substr)
{
	return t_strstr(CharIterator(str), CharIterator(substr)).get();
}

static inline char const* FUNC_FLASHMEM f_strstr(char const* str, char const* strEnd, char const* substr)
{
	return t_strstr(CharIterator(str), CharIterator(strEnd), CharIterator(substr)).get();
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// isspace
static inline bool FUNC_FLASHMEM isspace(char c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r');
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// isalpha
static inline bool FUNC_FLASHMEM isalpha(char c)
{
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// isdigit
static inline bool FUNC_FLASHMEM isdigit(char c)
{
	return (c >= '0' && c <= '9');
}







}

#endif