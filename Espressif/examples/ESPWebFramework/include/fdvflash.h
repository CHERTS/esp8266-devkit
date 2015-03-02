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

#ifndef _FDVFLASH_H_
#define _FDVFLASH_H_



#include "fdv.h"



// Flash from 0x0 to 0x8000      mapped to 0x40100000, len = 0x8000 (32KBytes)    - ".text"
// Flash from 0x40000 to 0x7C000 mapped to 0x40240000, len = 0x3C000 (240KBytes)  - ".irom0.text"
//
// We use flash from 0x14000 for a maximum size of 180KBytes (usable in blocks of 4K)
static uint32_t const FLASHFILESYSTEMSTART   = 0x14000;
static uint32_t const FLASHFILESYSTEMLENGTH  = 0x2C000;

static uint16_t const FLASHFILESYSTEMSTART_SECTOR  = FLASHFILESYSTEMSTART / SPI_FLASH_SEC_SIZE;
static uint16_t const FLASHFILESYSTEMLENGTH_SECTOR =  FLASHFILESYSTEMLENGTH / SPI_FLASH_SEC_SIZE;



namespace fdv
{

	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// isStoredInFlash
	static inline bool FUNC_FLASHMEM isStoredInFlash(void const* ptr)
	{
		return (uint32_t)ptr >= 0x40200000 && (uint32_t)ptr < 0x40300000;
	}
	

	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// getChar
	// flash mapped "text" is readable as 4 byte aligned blocks
	// works with both RAM and Flash stored data
	// str must be 32 bit aligned
	static inline char FUNC_FLASHMEM getChar(char const* str, uint32_t index)
	{
		if (isStoredInFlash(str))
		{
			uint32_t u32 = ((uint32_t const*)str)[index >> 2];
			return ((char const*)&u32)[index & 0x3];
		}
		else
			return str[index];
	}


	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// getChar
	// flash mapped "text" is readable as 4 byte aligned blocks
	// works with both RAM and Flash stored data
	// Returns first char of str
	// str may Not be 32 bit aligned
	static inline char FUNC_FLASHMEM getChar(char const* str)
	{
		if (isStoredInFlash(str))
		{
			uint32_t index = (uint32_t)str & 0x3;
			str = (char const*)((uint32_t)str & 0xFFFFFFFC);  // align str
			uint32_t u32 = *((uint32_t const*)str);
			return ((char const*)&u32)[index];
		}
		else
			return *str;
	}		
	
	
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// CharIterator
	// Can iterate both RAM and Flash strings

	struct CharIterator
	{
		CharIterator(char const* str = NULL)
			: m_str(str)
		{
		}
		
		char const* MTD_FLASHMEM get()
		{
			return m_str;
		}
		
		char MTD_FLASHMEM operator*()
		{
			return getChar(m_str);
		}
		
		CharIterator MTD_FLASHMEM operator++(int)
		{
			CharIterator p = *this;
			++m_str;
			return p;
		}
		
		CharIterator MTD_FLASHMEM operator++()
		{
			++m_str;
			return *this;
		}
		
		bool MTD_FLASHMEM operator==(char const* rhs)
		{
			return getChar(m_str) == *rhs;
		}
		
		bool MTD_FLASHMEM operator!=(char const* rhs)
		{
			return getChar(m_str) != *rhs;
		}
		
	private:
		char const* m_str;
	};

	

	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	// FlashFileSystem
	//
	// filename: only first four characters are stored
	// note: cannot free an already allocated file
	// max 44 files of 4096 bytes can be stored. No check is done on available space. 
	//
	// Example:
	//
	//   struct MyStore
	//  {
	//    char name[16];
	//    char surname[16];
	//  };
	//
	//  Save data:
	//    MyStore t;
	//    strcpy(t.name, "Fabrizio");
	//    strcpy(t.surname, "Di Vittorio");
	//    fdv::FlashFileSystem::save("USR0", &t, sizeof(MyStore));
	// 
	//  Load data:
	//    MyStore t;
	//    fdv::FlashFileSysten::load("USR0", &t);
	//    printf("%s %s", t.name, t.surname);
	
	
	struct FlashFileSystem
	{
		
		// format the entire available space
		// This is required only if you want to remove previous files
		static void MTD_FLASHMEM format()
		{
			Critical critical;
			for (uint16_t s = 0; s != FLASHFILESYSTEMLENGTH_SECTOR; ++s)
				spi_flash_erase_sector(FLASHFILESYSTEMSTART_SECTOR + s);
		}
				
		// ret false if file doesn't exist
		static bool MTD_FLASHMEM load(char const* filename, void* buffer)
		{
			SectorHeader header;
			uint16_t sector = find(filename, &header);
			if (sector > 0)
			{
				// found
				Critical critical;
				spi_flash_read((uint32_t)(sector * SPI_FLASH_SEC_SIZE + sizeof(SectorHeader)), (uint32*)buffer, header.length);
				return true;
			}
			return false;	// not found			
		}
		
		// length cannot exceed initial allocated sectors
		// buffer can be NULL: in this case the specified space is allocated and erased
		static void MTD_FLASHMEM save(char const* filename, void* buffer, uint16_t length)
		{
			SectorHeader header;
			uint16_t sector = find(filename, &header);
			if (sector == 0)				
				sector = findFreeSector(filename, length, &header); // initial file allocation, prepare header
			else				
				header.length = length; // already allocated, update header
			Critical critical;
			for (uint16_t s = 0; s != header.sectors; ++s)
				spi_flash_erase_sector(sector + s);
			spi_flash_write((uint32_t)(sector * SPI_FLASH_SEC_SIZE), (uint32*)&header, sizeof(SectorHeader));
			if (buffer)
				// todo: can spi_flash_write write more than one sector?
				spi_flash_write((uint32_t)(sector * SPI_FLASH_SEC_SIZE + sizeof(SectorHeader)), (uint32*)buffer, length);
		}
		
		// ret -1 if doesn't exist
		static int32_t MTD_FLASHMEM getLength(char const* filename)
		{
			SectorHeader header;
			if (find(filename, &header))
				return header.length;
			return -1;
		}
		
		
	private:

		static uint32_t const MAGIC = 0x46445631;
		
		struct SectorHeader
		{
			uint32_t magic;		// MAGIC if allocated
			uint32_t filename;	// 4 bytes filename
			uint16_t sectors;	// used sectors
			uint16_t length;    // used bytes (not including size of SectorHeader)
		};
		
		// header is filled with valid values
		static uint16_t MTD_FLASHMEM findFreeSector(char const* filename, uint16_t length, SectorHeader* header)
		{
			// find free position
			for (uint16_t s = 0; s != FLASHFILESYSTEMLENGTH_SECTOR; s += header->sectors)
			{
				readSectorHeader(FLASHFILESYSTEMSTART_SECTOR + s, header);
				if (header->magic != MAGIC)
				{
					// write header
					header->magic    = MAGIC;
					header->filename = *((uint32_t const*)filename);
					header->sectors  = calcRequiredSectors(length);
					header->length   = length;
					return FLASHFILESYSTEMSTART_SECTOR + s;
				}
			}
			return 0;
		}		
	
		static uint16_t MTD_FLASHMEM calcRequiredSectors(uint16_t length)
		{
			return (sizeof(SectorHeader) + length + SPI_FLASH_SEC_SIZE - 1) / SPI_FLASH_SEC_SIZE;
		}
		
		static void MTD_FLASHMEM readSectorHeader(uint16_t sector, SectorHeader* header)
		{
			Critical critical;
			spi_flash_read((uint32_t)(sector * SPI_FLASH_SEC_SIZE), (uint32*)header, sizeof(SectorHeader));
		}
		
		// ret 0 if not found
		static uint16_t MTD_FLASHMEM find(char const* filename, SectorHeader* header)
		{
			for (uint16_t s = 0; s != FLASHFILESYSTEMLENGTH_SECTOR; s += header->sectors)
			{
				readSectorHeader(FLASHFILESYSTEMSTART_SECTOR + s, header);
				if (header->magic != MAGIC)
					break;
				if (header->filename == *((uint32_t const*)filename))
					return FLASHFILESYSTEMSTART_SECTOR + s;	// found
			}
			return 0;	// not found			
		}
	};

	
	
	
}


#endif