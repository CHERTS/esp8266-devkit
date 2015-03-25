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


#ifndef _FDVUTILS_H_
#define _FDVUTILS_H_


#include "fdv.h"




namespace fdv
{

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// reboot
// creates a task and reboot after specified time (ms)

void reboot(uint32_t time);



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Memory

struct Memory
{
	static void* malloc(uint32_t size);
	static void free(void* ptr);
};


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Ptr
// A very simple smart pointer (without copy or ref counting functionalities)
// For arrays (new[]) use APtr

template <typename T>
class Ptr
{	
private:
	Ptr(Ptr const& c);	// no copy constructor
	
public:
	explicit Ptr(T* ptr)
		: m_ptr(ptr)
	{
	}
	
	Ptr()
		: m_ptr(NULL)
	{
	}
	
	
	~Ptr()
	{
		delete m_ptr;
	}
	
	T& operator*()
	{
		return *m_ptr;
	}
	
	T* operator->()
	{
		return m_ptr;
	}
	
	T* get()
	{
		return m_ptr;
	}
	
	void reset(T* ptr)
	{
		delete m_ptr;
		m_ptr = ptr;
	}
	
private:
	T* m_ptr;
};


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// APtr
// A very simple smart pointer (without copy or ref counting functionalities)
// For single objects (new) use Ptr

template <typename T>
class APtr
{	
private:
	APtr(APtr const& c);	// no copy constructor
	
public:
	explicit APtr(T* ptr)
		: m_ptr(ptr)
	{
	}
	
	APtr()
		: m_ptr(NULL)
	{
	}
	
	~APtr()
	{
		delete[] m_ptr;
	}
	
	T& operator*()
	{
		return *m_ptr;
	}
	
	T* operator->()
	{
		return m_ptr;
	}
	
	T* get()
	{
		return m_ptr;
	}
	
	T& operator[](uint32_t index)
	{
		return m_ptr[index];
	}

	void reset(T* ptr)
	{
		delete[] m_ptr;
		m_ptr = ptr;
	}
	
private:
	T* m_ptr;
};


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// min / max

template <typename T>
T min(T const& v1, T const& v2)
{
	return v1 < v2? v1 : v2;
}

template <typename T>
T max(T const& v1, T const& v2)
{
	return v1 > v2? v1 : v2;
}




}


#endif