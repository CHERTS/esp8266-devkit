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


#ifndef _FDVCOLLECTIONS_H_
#define _FDVCOLLECTIONS_H_


#include "fdv.h"


namespace fdv
{



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// ChunkedBuffer

template <typename T>
struct ChunkedBuffer
{

	struct Chunk
	{
		Chunk*   next;
		uint32_t items;
		T*       data;
		
		Chunk(uint32_t capacity)
			: next(NULL), items(0), data(new T[capacity])
		{
		}
		~Chunk()
		{
			delete[] data;
		}
	};
	
	
	struct Iterator
	{
		Iterator(Chunk* chunk = NULL)
			: m_chunk(chunk), m_pos(0)
		{
		}
		T& MTD_FLASHMEM operator*()
		{			
			return m_chunk->data[m_pos];
		}
		MTD_FLASHMEM operator bool()
		{
			return m_chunk != NULL;
		}
		Iterator MTD_FLASHMEM operator++(int)
		{
			Iterator c = *this;
			next();
			return c;
		}
		Iterator& MTD_FLASHMEM operator++()
		{
			next();
			return *this;
		}
		Iterator& MTD_FLASHMEM operator+=(int32_t rhs)
		{
			while (rhs-- > 0)
				next();
			return *this;
		}
		bool MTD_FLASHMEM operator==(Iterator const& rhs)
		{
			return m_chunk == rhs.m_chunk && m_pos == rhs.m_pos;
		}
		bool MTD_FLASHMEM operator!=(Iterator const& rhs)
		{
			return m_chunk != rhs.m_chunk || m_pos != rhs.m_pos;
		}
		T* MTD_FLASHMEM dup()
		{
			return t_strdup(*this);
		}
	private:
		void MTD_FLASHMEM next()
		{
			++m_pos;
			if (m_pos == m_chunk->items)
			{
				m_pos = 0;
				m_chunk = m_chunk->next;
			}
		}
	private:
		Chunk*   m_chunk;
		uint32_t m_pos;
	};

	
	ChunkedBuffer()
		: m_chunks(NULL), m_current(NULL)
	{
	}
	
	
	~ChunkedBuffer()
	{
		clear();
	}
	
	
	void MTD_FLASHMEM clear()
	{
		Chunk* chunk = m_chunks;
		while (chunk)
		{
			Chunk* next = chunk->next;
			delete chunk;
			chunk = next;
		}
		m_chunks = m_current = NULL;
	}
	
	
	Chunk* MTD_FLASHMEM addChunk(uint32_t capacity)
	{
		if (m_chunks == NULL)
			m_chunks = m_current = new Chunk(capacity);
		else
			m_current = m_current->next = new Chunk(capacity);		
		return m_current;
	}
	

	Iterator MTD_FLASHMEM begin()
	{
		return Iterator(m_chunks);
	}

	
	Iterator MTD_FLASHMEM end()
	{
		return Iterator();
	}

private:
	Chunk* m_chunks;
	Chunk* m_current;
};



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// IterDict
// A dictionary where key and value are iterators

template <typename KeyIterator, typename ValueIterator>
class IterDict
{
public:

	struct Item
	{
		Item*         next;
		KeyIterator   key;
		ValueIterator value;
		
		Item(KeyIterator key_, ValueIterator value_)
			: next(NULL), key(key_), value(value_)
		{
		}
	};

	IterDict()
		: m_items(NULL), m_current(NULL), m_itemsCount(0)
	{
	}
	
	~IterDict()
	{
		clear();
	}
	
	void MTD_FLASHMEM clear()
	{
		Item* item = m_items;
		while (item)
		{
			Item* next = item->next;
			delete item;
			item = next;
		}
		m_items = m_current = NULL;
		m_itemsCount = 0;
	}
	
	void MTD_FLASHMEM add(KeyIterator key, ValueIterator value)
	{
		if (m_items)
		{
			m_current = m_current->next = new Item(key, value);
			++m_itemsCount;
		}
		else
		{
			m_current = m_items = new Item(key, value);
			++m_itemsCount;
		}
	}
	
	uint32_t MTD_FLASHMEM getItemsCount()
	{
		return m_itemsCount;
	}
	
	// warn: this doesn't check "index" range!
	Item& MTD_FLASHMEM getItem(uint32_t index)
	{
		Item* item = m_items;
		for (; index > 0; --index)
			item = item->next;
		return *item;
	}
	
	// warn: this doesn't check "index" range!
	Item& MTD_FLASHMEM operator[](uint32_t index)
	{
		return getItem(index);
	}
	
	// key stay in RAM or Flash
	ValueIterator MTD_FLASHMEM operator[](char const* key)
	{
		Item* item = m_items;
		while (item)
		{
			if (t_strcmp(item->key, CharIterator(key)) == 0)
				return item->value;
			item = item->next;
		}
		return ValueIterator();
	}
	
	// debug
	void dump()
	{
		for (uint32_t i = 0; i != m_itemsCount; ++i)
		{
			APtr<char> key(getItem(i).key.dup());
			APtr<char> value(getItem(i).value.dup());
			debug("Key = %s  Value = %s\n\r", key.get(), value.get());
		}
	}		
	
private:
	
	Item*    m_items;
	Item*    m_current;
	uint32_t m_itemsCount;
};














}

#endif