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



#include "fdv.h"


namespace fdv
{



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// CharChunksIterator

char& MTD_FLASHMEM CharChunksIterator::operator*()
{			
	return m_chunk->data[m_pos];
}

CharChunksIterator MTD_FLASHMEM CharChunksIterator::operator++(int)
{
	CharChunksIterator c = *this;
	next();
	return c;
}

CharChunksIterator& MTD_FLASHMEM CharChunksIterator::operator++()
{
	next();
	return *this;
}

CharChunksIterator& MTD_FLASHMEM CharChunksIterator::operator+=(int32_t rhs)
{
	while (rhs-- > 0)
		next();
	return *this;
}

CharChunksIterator MTD_FLASHMEM CharChunksIterator::operator+(int32_t rhs)
{
	CharChunksIterator newval = *this;
	newval += rhs;
	return newval;
}

// *this must be > rhs
int32_t MTD_FLASHMEM CharChunksIterator::operator-(CharChunksIterator rhs)
{
	int32_t dif = 0;
	while (*this != rhs)
		++rhs, ++dif;
	return dif;
}

bool MTD_FLASHMEM CharChunksIterator::operator==(CharChunksIterator const& rhs)
{
	return m_chunk == rhs.m_chunk && m_pos == rhs.m_pos;
}

bool MTD_FLASHMEM CharChunksIterator::operator!=(CharChunksIterator const& rhs)
{
	return m_chunk != rhs.m_chunk || m_pos != rhs.m_pos;
}

uint32_t MTD_FLASHMEM CharChunksIterator::getPosition()
{
	return m_absPos;
}

bool MTD_FLASHMEM CharChunksIterator::isLast()
{
	return m_chunk->next == NULL && m_pos + 1 >= m_chunk->items;
}

bool MTD_FLASHMEM CharChunksIterator::isValid()
{
	return m_chunk != NULL;
}

void MTD_FLASHMEM CharChunksIterator::next()
{
	++m_absPos;
	++m_pos;
	if (m_pos == m_chunk->items)
	{
		m_pos = 0;
		m_chunk = m_chunk->next;
	}
}

	

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// LinkedCharChunks
//
// Warn: copy constructor copies only pointers. Should not used when with heap or stack buffers.

void MTD_FLASHMEM LinkedCharChunks::clear()
{
	CharChunk* chunk = m_chunks;
	while (chunk)
	{
		CharChunk* next = chunk->next;
		delete chunk;
		chunk = next;
	}
	m_chunks = m_current = NULL;
}


CharChunk* MTD_FLASHMEM LinkedCharChunks::addChunk(uint32_t capacity)
{
	CharChunk* newChunk = new CharChunk(capacity);
	if (m_chunks == NULL)
		m_current = m_chunks = newChunk;
	else
		m_current = m_current->next = newChunk;
	return newChunk;
}


CharChunk* MTD_FLASHMEM LinkedCharChunks::addChunk(char* data, uint32_t items, bool freeOnDestroy)
{
	CharChunk* newChunk = new CharChunk(data, items, freeOnDestroy);
	if (m_chunks == NULL)
		m_current = m_chunks = newChunk;
	else
		m_current = m_current->next = newChunk;
	return m_current;
}


// const to non const cast
CharChunk* MTD_FLASHMEM LinkedCharChunks::addChunk(char const* data, uint32_t items, bool freeOnDestroy)
{
	return addChunk((char*)data, items, freeOnDestroy);
}


void MTD_FLASHMEM LinkedCharChunks::addChunk(char const* str, bool freeOnDestroy)
{
	addChunk(str, f_strlen(str), freeOnDestroy);	// "items" field doesn't include ending zero
}


// adds all chunks of src
// Only data pointers are copied and they will be not freed
void MTD_FLASHMEM LinkedCharChunks::addChunks(LinkedCharChunks* src)
{
	CharChunk* srcChunk = src->m_chunks;
	while (srcChunk)
	{
		addChunk(srcChunk->data, srcChunk->items, false);
		srcChunk = srcChunk->next;
	}
}


void MTD_FLASHMEM LinkedCharChunks::append(char value, uint32_t newChunkSize)
{
	if (m_current && m_current->items < m_current->capacity)
	{
		// can use current chunk
		m_current->data[m_current->items++] = value;
	}
	else
	{
		// need another chunk
		CharChunk* chunk = addChunk(newChunkSize);
		chunk->data[0] = value;
		chunk->items = 1;
	}
}


CharChunk* MTD_FLASHMEM LinkedCharChunks::getFirstChunk()
{
	return m_chunks;
}


CharChunksIterator MTD_FLASHMEM LinkedCharChunks::getIterator()
{
	return CharChunksIterator(m_chunks);
}


uint32_t MTD_FLASHMEM LinkedCharChunks::getItemsCount()
{
	uint32_t len = 0;
	CharChunk* chunk = m_chunks;
	while (chunk)
	{
		len += chunk->items;
		chunk = chunk->next;
	}
	return len;
}


void MTD_FLASHMEM LinkedCharChunks::dump()
{
	for (CharChunksIterator i = getIterator(); i.isValid(); ++i)
		debug(getChar(&*i));		
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// FlashFileSystem


// filename can stay in Ram or Flash
bool MTD_FLASHMEM FlashFileSystem::find(char const* filename, char const** mimetype, void const** data, uint16_t* dataLength)
{
	char const* curc = (char const*)(FLASH_MAP_START + FLASHFILESYSTEM_POS);
	
	// check magic
	if (MAGIC != *((uint32_t const*)curc))
		return false;	// not found
	curc += 4;
	
	// find file
	while (true)
	{
		// filename length
		uint8_t filenamelen = getByte(curc);
		curc += 1;
		uint8_t mimetypelen = getByte(curc);
		curc += 1;
		uint16_t filecontentlen = getWord(curc); 
		curc += 2;
		if (filenamelen == 0)
			return false;	// not found
		// check filename
		if (f_strcmp(filename, curc) == 0)
		{
			// found
			*mimetype   = curc + filenamelen;
			*data       = (void*)(*mimetype + mimetypelen);
			*dataLength = filecontentlen;
			return true;
		}
		// bypass this file
		curc += filenamelen + mimetypelen + filecontentlen;			
	}
}



}

