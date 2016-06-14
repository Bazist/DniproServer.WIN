#pragma once

#ifndef _ATTRVALUES_POOL		 // Allow use of features specific to Windows XP or later.                   
#define _ATTRVALUES_POOL 0x780 // Change this to the appropriate value to target other versions of Windows.

#endif

#include <atomic>
#include "stdafx.h"
#include "BinaryFile.h"

class AttrValuesPage
{
public:
	std::atomic<uchar> Type; //0 - Free, 1 - In use, 2 - Full
	ushort Index;
	uint CurrPos;
	char* Values;

	uint getUsedMemory()
	{
		return CurrPos;
	}

	uint getTotalMemory()
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

		for (uint i = 1; i < MAX_ATTR_VALUES_PAGES; i++)
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
			for (uint i = 0; i < MAX_ATTR_VALUES_PAGES; i++)
			{
				uchar val = 0;

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
	inline char* fromSerPointer(uint serPointer)
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
	inline uint toSerPointer(AttrValuesPage* pPage, char* pointer)
	{
		return (pPage->Index << 16) | (pointer - pPage->Values);
	}

	std::atomic<uint> CountPage;
	uint maxSafeShort;

	AttrValuesPage attrValues[MAX_ATTR_VALUES_PAGES];
	//uint lastAttrValues;
	
	//char* currAttrValues;
	//uint currPosAttrValues;

	bool read(BinaryFile* pFile)
	{
		/*pFile->readInt(&lastAttrValues);
		pFile->readInt(&currPosAttrValues);

		for(uint i=0; i <= lastAttrValues; i++)
		{
			attrValues[i] = new char[MAX_SHORT];
			
			pFile->read(attrValues[i], MAX_SHORT);
		}
			
		currAttrValues = attrValues[lastAttrValues];*/

		return true;
	}

	bool save(BinaryFile* pFile)
	{
		//EnterCriticalSection(&Lock);

		//header
		/*if(!pFile->writeInt(&lastAttrValues))
			return false;

		if(!pFile->writeInt(&currPosAttrValues))
			return false;

		for(uint i=0; i <= lastAttrValues; i++)
		{
			if(!pFile->write(attrValues[i], MAX_SHORT))
				return false;
		}*/

		//LeaveCriticalSection(&Lock);

		return true;
	}

	uint getUsedMemory()
	{
		uint usedMemory = sizeof(AttrValuesPool);

		for (uint i = 0; i < CountPage; i++)
		{
			usedMemory += attrValues[i].getUsedMemory();
		}

		return usedMemory;
	}

	uint getTotalMemory()
	{
		uint usedMemory = sizeof(AttrValuesPool);

		for (uint i = 0; i < CountPage; i++)
		{
			usedMemory += attrValues[i].getTotalMemory();
		}

		return usedMemory;
	}

	void printMemory()
	{
		printf("================= AttrValuesPool =========================\n");
		printf("Amount pages: %d\n", CountPage);
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
		for(uint i=0; i < CountPage; i++)
		{
			delete[] attrValues[i].Values;
		}
	}
};