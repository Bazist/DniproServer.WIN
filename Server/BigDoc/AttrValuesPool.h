/*
# Copyright(C) 2010-2021 Viacheslav Makoveichuk (email: BigDoc@gmail.com, skype: vyacheslavm81)
# This file is part of BigDoc.
#
# BigDoc is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# BigDoc is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifndef _ATTRVALUES_POOL		 // Allow use of features specific to Windows XP or later.                   
#define _ATTRVALUES_POOL 0x780 // Change this to the appropriate value to target other versions of Windows.

#endif

#include <atomic>
#include "stdafx.h"
#include "BinFile.h"

class AttrValuesPage
{
public:
	std::atomic<uchar8> Type; //0 - Free, 1 - In use, 2 - Full
	ushort16 Index;
	uint32 CurrPos;
	char* Values;

	uint32 getUsedMemory()
	{
		return CurrPos;
	}

	uint32 getTotalMemory()
	{
		return MAX_SHORT;
	}
};

class AttrValuesPool
{
public:

	AttrValuesPool()
	{
		memset(this, 0, sizeof(AttrValuesPool));
		maxSafeShort = MAX_SHORT - 128;
	}

	void init()
	{
		attrValues[0].Type = 0;
		attrValues[0].CurrPos = 1;
		attrValues[0].Values = new char[MAX_SHORT];
		attrValues[0].Values[0] = 0;

		for (uint32 i = 1; i < MAX_ATTR_VALUES_PAGES; i++)
		{
			attrValues[i].Type = 0;
			attrValues[i].Index = i;
			attrValues[i].Values = 0;
		}

		CountPage = 1;
	}

	AttrValuesPage* checkPage(AttrValuesPage* pPage)
	{
		if (pPage)
		{
			if (pPage->CurrPos < maxSafeShort) //page is full
			{
				return pPage; //just continue with this page
			}
			else
			{
				pPage->Type.store(2); //page is full, find new
			}
		}

		//try find new page
		while (true)
		{
			for (uint32 i = 0; i < MAX_ATTR_VALUES_PAGES; i++)
			{
				uchar8 val = 0;

				if (attrValues[i].Type.compare_exchange_strong(val, 1)) //set in use
				{
					//check space
					if (!attrValues[i].Values)
					{
						attrValues[i].Values = new char[MAX_SHORT];
						attrValues[i].Values[0] = 0;

						CountPage++;
					}

					return &attrValues[i];
				}
			}
		}

		return 0;
	}
		
	//from serializable pointer
	inline char* fromSerPointer(uint32 serPointer)
	{
		if (serPointer)
		{
			return attrValues[serPointer >> 16].Values + (serPointer & 0xFFFF);
		}
		else
		{
			return 0;
		}
	}

	//to serializable pointer
	inline uint32 toSerPointer(AttrValuesPage* pPage, char* pointer)
	{
		return (pPage->Index << 16) | (pointer - pPage->Values);
	}

	std::atomic<uint32> CountPage;
	uint32 maxSafeShort;

	AttrValuesPage attrValues[MAX_ATTR_VALUES_PAGES];
	//uint32 lastAttrValues;
	
	//char* currAttrValues;
	//uint32 currPosAttrValues;

	bool read(BinFile* pFile)
	{
		/*pFile->readInt(&lastAttrValues);
		pFile->readInt(&currPosAttrValues);

		for(uint32 i=0; i <= lastAttrValues; i++)
		{
			attrValues[i] = new char[MAX_SHORT];
			
			pFile->read(attrValues[i], MAX_SHORT);
		}
			
		currAttrValues = attrValues[lastAttrValues];*/

		return true;
	}

	bool save(BinFile* pFile)
	{
		//EnterCriticalSection(&Lock);

		//header
		/*if(!pFile->writeInt(&lastAttrValues))
			return false;

		if(!pFile->writeInt(&currPosAttrValues))
			return false;

		for(uint32 i=0; i <= lastAttrValues; i++)
		{
			if(!pFile->write(attrValues[i], MAX_SHORT))
				return false;
		}*/

		//LeaveCriticalSection(&Lock);

		return true;
	}

	uint32 getUsedMemory()
	{
		uint32 usedMemory = sizeof(AttrValuesPool);

		for (uint32 i = 0; i < CountPage; i++)
		{
			usedMemory += attrValues[i].getUsedMemory();
		}

		return usedMemory;
	}

	uint32 getTotalMemory()
	{
		uint32 usedMemory = sizeof(AttrValuesPool);

		for (uint32 i = 0; i < CountPage; i++)
		{
			usedMemory += attrValues[i].getTotalMemory();
		}

		return usedMemory;
	}

	void printMemory()
	{
		printf("================= AttrValuesPool =========================\n");
		printf("Amount pages: %d\n", CountPage.load());
		printf("Total memory: %d\n", getTotalMemory());
		printf("==========================================================\n");
	}

	void clear()
	{
		destroy();

		init();
	}

	void destroy()
	{
		for(uint32 i=0; i < CountPage; i++)
		{
			delete[] attrValues[i].Values;
		}
	}
};