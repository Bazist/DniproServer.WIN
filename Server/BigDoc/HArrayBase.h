/*
# Copyright(C) 2010-2017 Viacheslav Makoveichuk (email: slv709@gmail.com, skype: vyacheslavm81)
# This file is part of HArray.
#
# HArray is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# HArray is distributed in the hope that it will be useful,
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

#include "stdafx.h"
#include "BinFile.h"
#include <atomic>

#define _RELEASE 0x1234567

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
const uchar8 MIN_BLOCK_TYPE = MAX_BRANCH_TYPE2 + 1;
const uchar8 MAX_BLOCK_TYPE = MIN_BLOCK_TYPE + (32 / BLOCK_ENGINE_STEP) - 1;
const uchar8 VAR_TYPE = MAX_BLOCK_TYPE + 1; //for var value
const uchar8 CONTINUE_VAR_TYPE = VAR_TYPE + 1; //for continue var value
const uchar8 CURRENT_VALUE_TYPE = CONTINUE_VAR_TYPE + 1;
const uchar8 VALUE_TYPE = CURRENT_VALUE_TYPE + 1;
const uchar8 VALUE_LIST_TYPE = VALUE_TYPE + 1;
const uchar8 ONLY_CONTENT_TYPE = VALUE_LIST_TYPE + 1;

const uchar8 MOVES_LEVEL1_STAT = 0;
const uchar8 MOVES_LEVEL2_STAT = 1;
const uchar8 MOVES_LEVEL3_STAT = 2;
const uchar8 MOVES_LEVEL4_STAT = 3;
const uchar8 MOVES_LEVEL5_STAT = 4;
const uchar8 MOVES_LEVEL6_STAT = 5;
const uchar8 MOVES_LEVEL7_STAT = 6;
const uchar8 MOVES_LEVEL8_STAT = 7;
const uchar8 SHORT_WAY_STAT = 8;
const uchar8 LONG_WAY_STAT = 9;
const uchar8 CONTENT_BRANCH_STAT = 10;

const uchar8 CURRENT_VALUE_SEGMENT_TYPE = 1;
const uchar8 BRANCH_SEGMENT_TYPE = CURRENT_VALUE_SEGMENT_TYPE + 1;
const uchar8 BLOCK_VALUE_SEGMENT_TYPE = BRANCH_SEGMENT_TYPE + 1;
const uchar8 BLOCK_BRANCH1_SEGMENT_TYPE = BLOCK_VALUE_SEGMENT_TYPE + 1;
const uchar8 BLOCK_BRANCH2_SEGMENT_TYPE = BLOCK_BRANCH1_SEGMENT_TYPE + 1;
const uchar8 BLOCK_OFFSET_SEGMENT_TYPE = BLOCK_BRANCH2_SEGMENT_TYPE + 1;
const uchar8 VAR_SHUNT_SEGMENT_TYPE = BLOCK_OFFSET_SEGMENT_TYPE + 1;
const uchar8 VAR_VALUE_SEGMENT_TYPE = VAR_SHUNT_SEGMENT_TYPE + 1;

const uint32 MIN_COUNT_RELEASED_CONTENT_CELLS = MAX_SHORT * 2; //two pages
const uint32 MIN_COUNT_RELEASED_BRANCH_CELLS = MAX_SHORT;
const uint32 MIN_COUNT_RELEASED_BLOCK_CELLS = MAX_SHORT;
const uint32 MIN_COUNT_RELEASED_VAR_CELLS = MAX_SHORT;

const uchar8 MAX_KEY_SEGMENTS = MAX_CHAR - ONLY_CONTENT_TYPE;
const uchar8 MIN_HEADER_BASE_BITS = 14;		  //16384 slots
const uchar8 MAX_HEADER_FILL_FACTOR_BITS = 4; //fill factor 1/16 of header size

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

struct HArrayPair
{
public:
	uint32 Key[16];
	uint32 Value;
	uint32 KeyLen;

	void print()
	{
		for(int i=0; i<KeyLen; i++)
		{
			printf("%u ", Key[i]);
		}

		printf("=> %u\n", Value);
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

//struct ContentCell
//{
//	uchar8 Type;
//	uint32 Value;
//};

struct VarCell
{
	std::atomic<uchar8> ValueContCellReadByTranID;
	uchar8 ValueContCellType;
	uint32 ValueContCellValue;
	
	std::atomic<uchar8> ContCellReadByTranID;
	uchar8 ContCellType;
	uint32 ContCellValue;
};

struct ContentPage
{
	std::atomic<uchar8> ReadByTranID[MAX_SHORT];
	uchar8 pType[MAX_SHORT];
	uint32 pContent[MAX_SHORT];
};

struct VarPage
{
	VarCell pVar[MAX_SHORT];
};

struct BranchPage
{
	BranchCell pBranch[MAX_SHORT];
};

struct BlockPage
{
	BlockPage()
	{
		for (uint32 i = 0; i < MAX_SHORT; i++)
		{
			pBlock[i].Type = 0;
		}
	}

	BlockCell pBlock[MAX_SHORT];
};

struct CompactPage
{
	CompactPage()
	{
		for (uint32 i = 0; i < MAX_CHAR; i++)
		{
			Values[i] = 0;
			Offsets[i] = 0;
		}

		Count = 0;
	}

	uint32 Values[MAX_CHAR];
	uint32 Offsets[MAX_CHAR];

	uint32 Count;

	CompactPage* pNextPage;
};

struct SegmentPath
{
	uchar8 Type;

	uchar8* pContentCellType;
	uint32* pContentCellValue;

	BlockCell* pBlockCell;
	uint32 StartBlockOffset;

	BranchCell* pBranchCell1;
	uint32 BranchOffset1;

	BranchCell* pBranchCell2;
	uint32 BranchOffset2;

	VarCell* pVarCell;
	uint32 VarOffset;

	uint32 BranchIndex;
	uint32 ContentOffset;
	uint32 BlockSubOffset;

	void print()
	{
		/*
		printf("Type: %u, ", Type);
		
		if(pContentCell)
			printf("ContentCell: Type=%u, Value=%u, ", pContentCell->Type, pContentCell->Value);

		if(pBlockCell)
			printf("BlockCell: Type=%u, Offset=%u, ValueOrOffset=%u, ", pBlockCell->Type, pBlockCell->Offset, pBlockCell->ValueOrOffset);

		printf("StartBlockOffset: %u, ", StartBlockOffset);

		if (pBranchCell1)
			printf("pBranchCell1: Type=%u, Offset=%u, ValueOrOffset=%u, ", pBlockCell->Type, pBlockCell->Offset, pBlockCell->ValueOrOffset);
		*/
	}
};

typedef uint32 (*NormalizeFunc)(void* key);

typedef int (*CompareFunc)(void* key1, uint32 keyLen1, void* key2, uint32 keyLen2);

typedef int(*CompareSegmentFunc)(void* keySeg1, void* keySeg2, uint32 index);

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

	inline bool delValue(uint32 value, uint32 index = 0)
	{
		if (pValues[index] == value) //fast way
		{
			pValues[index] = 0;

			return true;
		}

		for (uint32 i = 1; i < Count; i++) //long way
		{
			if (pValues[i] == value)
			{
				pValues[i] = 0;

				return true;
			}
		}

		return false;
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

	void read(BinFile* pFile)
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

		pValueLists = new ValueList * [ValueListsSize];
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
		ValueList** pTempValueLists = new ValueList * [newValueListsSize];

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

	inline void releaseObject(ValueList* pValueList)
	{
		//todo implement
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

		for (uint32 i = 0; i < page; i++)
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
