/*
# Copyright(C) 2010-2016 Vyacheslav Makoveychuk (email: slv709@gmail.com, skype: vyacheslavm81)
# This file is part of DniproDB.
#
# DniproDB is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# DniproDB is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifndef _HARRAY_FIX_BASE		 // Allow use of features specific to Windows XP or later.                   
#define _HARRAY_FIX_BASE 0x709 // Change this to the appropriate value to target other versions of Windows.

#endif	

#include <atomic>
#include "stdafx.h"
#include "BinFile.h"
#include "ReadedList.h"
#include "AttrValuesPool.h"

//#define _RELEASE 0x1234567

const uint32 REPOSITORY_VERSION = 1;

const uint32 COUNT_TEMPS = 50;

const uint32 BLOCK_ENGINE_BITS = 4; //bits of block
const uint32 BLOCK_ENGINE_SIZE = 16; //size of block
const uint32 BLOCK_ENGINE_SHIFT = 32 - BLOCK_ENGINE_BITS;
const uint32 BLOCK_ENGINE_STEP = 4;

//const uint32 BLOCK_ENGINE_BITS = 16; //bits of block
//const uint32 BLOCK_ENGINE_SIZE = 65536; //size of block
//const uint32 BLOCK_ENGINE_SHIFT = 32 - BLOCK_ENGINE_BITS;
//const uint32 BLOCK_ENGINE_STEP = 16;

const uint32 BRANCH_ENGINE_SIZE = 4; //can be changed

const uchar8 EMPTY_TYPE = 0;
const uchar8 MIN_BRANCH_TYPE1 = 1;
const uchar8 MAX_BRANCH_TYPE1 = BRANCH_ENGINE_SIZE;
const uchar8 MIN_BRANCH_TYPE2 = BRANCH_ENGINE_SIZE + 1;
const uchar8 MAX_BRANCH_TYPE2 = BRANCH_ENGINE_SIZE * 2;
const uchar8 MIN_BLOCK_TYPE = 50;
const uchar8 MAX_BLOCK_TYPE = MIN_BLOCK_TYPE + (32 / BLOCK_ENGINE_STEP) - 1;
const uchar8 VAR_TYPE = 125; //for var value
const uchar8 CONTINUE_VAR_TYPE = 126; //for continue var value
const uchar8 CURRENT_VALUE_TYPE = 127;
const uchar8 VALUE_TYPE = 128;
const uchar8 VALUE_LIST_TYPE = 129;
const uchar8 ONLY_CONTENT_TYPE = 130;

typedef bool HARRAY_ITEM_VISIT_FUNC(uint32* key, uint32 keyLen, uint32 value, uchar8 valueType, void* pData);

typedef void ON_CONTENT_CELL_MOVED_FUNC(uchar8 tranID, std::atomic<uchar8>* oldAddress, std::atomic<uchar8>* newAddress);

typedef bool CHECK_DEADLOCK_FUNC(uchar8 tranID);

struct HArrayFixBaseInfo
{
	uint32 Version;

	uint32 KeyLen;
	uint32 ValueLen;

	uint32 HeaderBase;

	uint32 ContentPagesCount;
	uint32 VarPagesCount;
	uint32 BranchPagesCount;
	uint32 BlockPagesCount;

	uint32 LastContentOffset;
	uint32 LastVarOffset;
	uint32 LastBranchOffset;
	uint32 LastBlockOffset;
};

struct HACursor
{
	uint32 CountFullContentPage;
	uint32 SizeLastContentPage;

	uint32 Page;
	uint32 Index;

	uint32* Value;
};

//struct ContentTypeCell
//{
//	uchar8 Type;
//};

struct HArrayFixPair
{
	uint32 Key[16];
	uint32 Value;
	uint32 KeyLen;

	int compareTo(HArrayFixPair& pair)
	{
		for (uint32 i = 0; i < KeyLen; i++)
		{
			if (Key[i] < pair.Key[i])
				return -1;

			if (Key[i] > pair.Key[i])
				return 1;
		}

		return 0;
	}
};

struct BranchCell
{
	uint32 Values[BRANCH_ENGINE_SIZE];
	uint32 Offsets[BRANCH_ENGINE_SIZE];
};

struct BlockCell
{
	uchar8 Type;
	uint32 Offset;
	uint32 ValueOrOffset;
};

struct ContentCell
{
	std::atomic<uchar8> ReadedByTranID;
	uchar8 Type;
	uint32 Value;
};

struct VarCell
{
	ContentCell ValueContentCell;
	ContentCell ContCell;
};

class ContentPage
{
public:
	ContentCell pContent[MAX_SHORT];
};

class VarPage
{
public:
	VarCell pVar[MAX_SHORT];
};

class BranchPage
{
public:
	BranchCell pBranch[MAX_SHORT];
};

class BlockPage
{
public:
	BlockPage()
	{
		for (uint32 i = 0; i < MAX_SHORT; i++)
		{
			pBlock[i].Type = 0;
		}
	}

	BlockCell pBlock[MAX_SHORT];
};

class ValueList
{
public:
	ValueList(uint32 size)
	{
		Count = 0;
		Size = size;

		pValues = new uint32[Size];

		for (uint32 i = 0; i < Size; i++)
			pValues[i] = 0;
	}

	ValueList()
	{
		Count = 0;
		Size = 4;

		pValues = new uint32[Size];
	}

	uint32* pValues;

	uint32 Count;
	uint32 Size;

	//returns index of insertion
	inline uint32 addValue(uint32 value, bool isUnique = false)
	{
		if (Count == Size)
		{
			//reallocate
			Size <<= 1; //*2

			uint32* pValuesTemp = new uint32[Size];

			uint32 i = 0;
			for (; i < Count; i++)
			{
				pValuesTemp[i] = pValues[i];
			}

			for (; i < Size; i++)
			{
				pValuesTemp[i] = 0;
			}

			delete[] pValues;

			pValues = pValuesTemp;
		}

		if (isUnique)
		{
			for (uint32 i = 0; i < Count; i++)
			{
				if (!pValues[i]) //insert in empty slot
				{
					pValues[i] = value;

					return i;
				}
				else if (pValues[i] == value) //if value is exists, just return
				{
					return i;
				}
			}
		}

		pValues[Count] = value;

		return Count++;
	}

	inline void addValues(ValueList* pValueList)
	{
		for (uint32 i = 0; i < pValueList->Count; i++)
		{
			if (pValueList->pValues[i])
			{
				addValue(pValueList->pValues[i]);
			}
		}
	}

	inline void addUniqueValue(uint32 value)
	{
		for (uint32 i = 0; i < Count; i++)
		{
			if (pValues[i] == value)
			{
				return;
			}
		}

		addValue(value);
	}

	inline void delValue(uint32 value, uint32 index = 0)
	{
		if (pValues[index] == value) //fast way
		{
			pValues[index] = 0;

			return;
		}

		for (uint32 i = 1; i < Count; i++) //long way
		{
			if (pValues[i] == value)
			{
				pValues[i] = 0;

				return;
			}
		}
	}

	inline void delValues(ValueList* pValueList)
	{
		for (uint32 i = 0; i < pValueList->Count; i++)
		{
			delValue(pValueList->pValues[i]);
		}
	}

	inline bool hasValue(uint32 value)
	{
		for (uint32 i = 0; i < Count; i++)
		{
			if (pValues[i] == value)
			{
				return true;
			}
		}

		return false;
	}

	inline bool hasValues(ValueList* pValueList)
	{
		for (uint32 i = 0; i < pValueList->Count; i++)
		{
			if (hasValue(pValueList->pValues[i]))
			{
				return true;
			}
		}

		return false;
	}

	void read(BinFile *pFile)
	{
		pFile->readInt(&Count);
		pFile->readInt(&Size);

		pValues = new uint32[Size];
		pFile->read(pValues, Size * sizeof(uint32));
	}

	bool save(BinFile* pFile)
	{
		if (!pFile->writeInt(&Count))
			return false;

		if (!pFile->writeInt(&Size))
			return false;

		if (!pFile->write(pValues, Size * sizeof(uint32)))
			return false;

		return true;
	}

	uint32 getUsedMemory()
	{
		return sizeof(ValueList) + Count * sizeof(uint32);
	}

	uint32 getTotalMemory()
	{
		return sizeof(ValueList) + Size * sizeof(uint32);
	}

	~ValueList()
	{
		delete[] pValues;
	}
};

class ValueListPool
{
public:
	ValueListPool()
	{
		Count = 1;
		Size = 0;

		ValueListsSize = 4;

		pValueLists = new ValueList*[ValueListsSize];
	}

	ValueList** pValueLists;

	uint32 ValueListsSize;

	uint32 Count;
	uint32 Size;

	bool read(BinFile* pFile)
	{
		pFile->readInt(&Count);
		pFile->readInt(&Size);

		uint32 pages = Count >> 16;

		//read pages
		uint32 i = 0;
		for (; i < pages; i++)
		{
			pValueLists[i] = new ValueList[MAX_SHORT];

			for (uint32 j = 0; j < MAX_SHORT; j++)
			{
				pValueLists[i][j].read(pFile);
			}
		}

		//read last page
		uint32 lastPageSize = Count & 0xFFFF;

		if (lastPageSize)
		{
			pValueLists[i] = new ValueList[MAX_SHORT];

			for (uint32 j = 0; j < lastPageSize; j++)
			{
				pValueLists[i][j].read(pFile);
			}
		}

		return true;
	}

	bool save(BinFile* pFile)
	{
		//header
		if (!pFile->writeInt(&Count))
			return false;

		if (!pFile->writeInt(&Size))
			return false;

		uint32 pages = Count >> 16;

		//read pages
		uint32 i = 0;
		for (; i < pages; i++)
		{
			for (uint32 j = 0; j < MAX_SHORT; j++)
			{
				if (!pValueLists[i][j].save(pFile))
					return false;
			}
		}

		//read last page
		uint32 lastPageCount = Count & 0xFFFF;

		for (uint32 j = 0; j < lastPageCount; j++)
		{
			if (!pValueLists[i][j].save(pFile))
				return false;
		}

		return true;
	}

	void reallocateValueLists()
	{
		uint32 newValueListsSize = ValueListsSize * 2;
		ValueList** pTempValueLists = new ValueList*[newValueListsSize];

		uint32 j = 0;
		for (; j < ValueListsSize; j++)
		{
			pTempValueLists[j] = pValueLists[j];
		}

		for (; j < newValueListsSize; j++)
		{
			pTempValueLists[j] = 0;
		}

		delete[] pValueLists;
		pValueLists = pTempValueLists;

		ValueListsSize = newValueListsSize;
	}

	inline ValueList* newObject()
	{
		uint32 page = Count >> 16;
		uint32 index = Count & 0xFFFF;

		if (Count >= Size)
		{
			if (page >= ValueListsSize)
			{
				reallocateValueLists();
			}

			//store pages
			ValueList* pNewValueLists = new ValueList[MAX_SHORT];
			pValueLists[page] = pNewValueLists;

			Size += MAX_SHORT;
		}

		//get free item
		ValueList* pValueList = &pValueLists[page][index];
		pValueList->Count = 0;

		Count++;

		return pValueList;
	}

	inline ValueList* newSerObject(uint32& serPointer)
	{
		uint32 page = Count >> 16;
		uint32 index = Count & 0xFFFF;

		if (Count >= Size)
		{
			if (page >= ValueListsSize)
			{
				reallocateValueLists();
			}

			//store pages
			ValueList* pNewValueLists = new ValueList[MAX_SHORT];
			pValueLists[page] = pNewValueLists;

			Size += MAX_SHORT;
		}

		//get free item
		ValueList* pValueList = &pValueLists[page][index];
		pValueList->Count = 0;

		serPointer = Count++;

		return pValueList;
	}

	inline ValueList* fromSerPointer(uint32 serPointer)
	{
		return &pValueLists[serPointer >> 16][serPointer & 0xFFFF];
	}

	uint32 getUsedMemory()
	{
		uint32 bytes = 0;

		for (uint32 i = 1; i < Count; i++)
		{
			uint32 page = i >> 16;
			uint32 index = i & 0xFFFF;

			bytes += pValueLists[page][index].getUsedMemory();
		}

		return bytes + sizeof(ValueListPool);
	}

	uint32 getTotalMemory()
	{
		uint32 bytes = 0;

		for (uint32 i = 1; i < Count; i++)
		{
			uint32 page = i >> 16;
			uint32 index = i & 0xFFFF;

			bytes += pValueLists[page][index].getTotalMemory();
		}

		return bytes + sizeof(ValueListPool);
	}

	void printMemory()
	{
		printf("================= ValueListPool =========================\n");
		printf("Amount ValueLists: %d\n", Count);
		printf("Total memory: %d\n", getTotalMemory());
		printf("=========================================================\n");
	}

	void clear()
	{
		Count = 1;
	}

	void destroy()
	{
		uint32 page = Size >> 16;

		for (uint32 i = 0; i<page; i++)
		{
			delete[] pValueLists[i];
		}

		if (pValueLists)
		{
			delete[] pValueLists;
			pValueLists = 0;
		}

		Count = 0;
		Size = 0;
	}

	~ValueListPool()
	{
		destroy();
	}
};