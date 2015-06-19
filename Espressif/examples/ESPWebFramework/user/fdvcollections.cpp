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


void MTD_FLASHMEM LinkedCharChunks::operator=(LinkedCharChunks& c)
{
    clear();
    addChunks(&c);
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



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// FlashDictionary

// clear the entire available space and write MAGIC at the beginning of the dictionary
// This is required only if you want to remove previous content
// erase all values to 0xFF
void STC_FLASHMEM FlashDictionary::eraseContent()
{
    Critical critical;
    spi_flash_erase_sector(FLASH_DICTIONARY_POS / SPI_FLASH_SEC_SIZE);
    uint32 magic = MAGIC;
    spi_flash_write(FLASH_DICTIONARY_POS, &magic, sizeof(magic));
}


// requires 4K free heap to execute!
void STC_FLASHMEM FlashDictionary::setValue(char const* key, void const* value, uint32_t valueLength)
{
    if (!value)
        return;
    
    // find key or a free space
    uint8_t const* keyPosPtr = (uint8_t const*)findKey(key);

    // copy the Flash page in RAM buffer (page)
    // memcpy is ok, because source alredy aligned then can be read directly from Flash
    uint8_t* page = new uint8_t[SPI_FLASH_SEC_SIZE];
    memcpy(page, (void const*)(FLASH_MAP_START + FLASH_DICTIONARY_POS), SPI_FLASH_SEC_SIZE);	
    
    // get key position as index into page[]
    uint32_t keyPos = (uint32_t)keyPosPtr - FLASH_MAP_START - FLASH_DICTIONARY_POS;
    uint32_t curPos = keyPos;
    
    // is new key?
    bool isNewKey = (page[keyPos] == 0xFF);
    
    // store key length (byte)
    uint32_t keyLen = f_strlen(key);
    page[curPos++] = keyLen;
    
    // store key (and terminating zero)
    f_strcpy((char *)&page[curPos], key);
    curPos += keyLen + 1;

    // need to move next key->values?
    if (!isNewKey)
    {			
        uint32_t prevValueLength = page[curPos] | (page[curPos + 1] << 8);	// little-endian
        bool isLastKey = (page[curPos + 2 + prevValueLength] == 0xFF);
        int diff = (int32_t)valueLength - prevValueLength;
        uint8_t* dst = &page[curPos + max(0, diff)];
        uint8_t* src = &page[curPos - min(0, diff)];
        uint8_t* end = &page[SPI_FLASH_SEC_SIZE];
        memmove(dst, src, min(end - src, end - dst));
        // mark the new end of items
        if (isLastKey)
            page[curPos + 2 + valueLength] = 0xFF;			
    }
    
    // store low and high byte of value length word (little-endian)
    page[curPos++] = valueLength & 0xFF;
    page[curPos++] = (valueLength >> 8) & 0xFF;
    
    // store value data
    f_memcpy(&page[curPos], value, valueLength);
            
    // write back into the flash
    Critical critical;
    spi_flash_erase_sector(FLASH_DICTIONARY_POS / SPI_FLASH_SEC_SIZE);
    spi_flash_write(FLASH_DICTIONARY_POS, (uint32*)page, SPI_FLASH_SEC_SIZE);
    
    delete[] page;
}


// return NULL if key point to a free space (aka key doesn't exist)
// return pointer may be Unaligned pointer to Flash
uint8_t const* STC_FLASHMEM FlashDictionary::getValue(char const* key, uint32_t* valueLength)
{
    uint8_t const* pos = (uint8_t const*)findKey(key);
    uint8_t keyLen = getByte(pos);
    if (keyLen == 0xFF)
        return NULL;
    pos += 1 + keyLen + 1;
    if (valueLength)
        *valueLength = getWord(pos);
    return pos + 2;
}


void STC_FLASHMEM FlashDictionary::setString(char const* key, char const* value)
{
    if (value)
        setValue(key, value, f_strlen(value) + 1);
}


// always returns a Flash stored string (if not found return what contained in defaultValue)
char const* STC_FLASHMEM FlashDictionary::getString(char const* key, char const* defaultValue)
{
    char const* value = (char const*)getValue(key);
    return value? value : defaultValue;
}


void STC_FLASHMEM FlashDictionary::setInt(char const* key, int32_t value)
{
    setValue(key, &value, sizeof(value));
}


int32_t STC_FLASHMEM FlashDictionary::getInt(char const* key, int32_t defaultValue)
{
    void const* value = (void const*)getValue(key);
    return value? (int32_t)getDWord(value) : defaultValue;
}


void STC_FLASHMEM FlashDictionary::setBool(char const* key, bool value)
{
    uint8_t b = value? 1 : 0;
    setValue(key, &b, 1);
}


bool STC_FLASHMEM FlashDictionary::getBool(char const* key, bool defaultValue)
{
    void const* value = (void const*)getValue(key);
    return value? (getByte(value) == 1? true : false) : defaultValue;
}


// return starting of specified key or starting of next free position
// what is stored:
//   byte: key length (or 0xFF for end of keys, hence starting of free space). Doesn't include ending zero
//   ....: key string
//   0x00: key ending zero
//   word: value length (little endian)
//   ....: value data
// automatically erase content if not already initialized
void const* MTD_FLASHMEM FlashDictionary::findKey(char const* key)
{		
    if (!isContentValid())
        eraseContent();
    uint8_t const* curpos = (uint8_t const*)(FLASH_MAP_START + FLASH_DICTIONARY_POS + sizeof(MAGIC));	
    while (true)
    {
        uint8_t keylen = getByte(curpos);	// keylen doesn't include ending zero
        if (keylen == 0xFF || (key && f_strcmp((char const*)(curpos + 1), key) == 0))
            return curpos;	// return start of free space or start of key->value block
        // goto next block
        curpos += 1 + keylen + 1;		// 1 (keylen field) + keylen + 1 (ending zero)
        curpos += 2 + getWord(curpos);	// 2 (valuelen field) + valuelen
    }
}


bool MTD_FLASHMEM FlashDictionary::isContentValid()
{
    // already aligned, can be read directly from Flash
    return *((uint32_t const*)(FLASH_MAP_START + FLASH_DICTIONARY_POS)) == MAGIC;
}


uint32_t MTD_FLASHMEM FlashDictionary::getUsedSpace()
{
    return (uint32_t)findKey(NULL) - (FLASH_MAP_START + FLASH_DICTIONARY_POS);
}


}

