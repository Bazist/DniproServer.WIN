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

#ifndef _HARRAY_VAR_RAM		 // Allow use of features specific to Windows XP or later.                   
#define _HARRAY_VAR_RAM 0x778 // Change this to the appropriate value to target other versions of Windows.

#endif

#include "HArrayFixBase.h"

class HArrayVarRAM
{
public:
	HArrayVarRAM()
	{
		pContentPages = 0;
		pVarPages = 0;
		pBranchPages = 0;
		pBlockPages = 0;

		ContentPagesCount = 0;
		VarPagesCount = 0;
		BranchPagesCount = 0;
		BlockPagesCount = 0;
	}

	char Name[256];

	uint ContentPagesCount;
	uint VarPagesCount;
	uint BranchPagesCount;
	uint BlockPagesCount;

	uint ContentPagesSize;
	uint VarPagesSize;
	uint BranchPagesSize;
	uint BlockPagesSize;

	uint* pHeader;
	
	/*uint* pActiveContent;
	ContentTypeCell* pActiveContentType;
	BranchCell* pActiveBranch;
	BlockCell* pActiveBlock;*/
	
	ContentPage** pContentPages;
	VarPage** pVarPages;
	BranchPage** pBranchPages;
	BlockPage** pBlockPages;

	uint HeaderBase;
	uint HeaderBits;
	uint HeaderSize;

	uint* freeBranchCells;
	uint countFreeBranchCell;

	uint ValueLen;
	
	uint MAX_SAFE_SHORT;

	//support value lists
	bool AllowValueList;

	ValueListPool valueListPool;
	AttrValuesPool attrValuesPool;
	
	void init(uint valueLen, 
			  uchar headerBase)
	{
		init(valueLen,
			 headerBase,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES);
	}

	void init(uint valueLen, 
			  uchar headerBase,
			  uint contentPagesSize,
			  uint varPagesSize,
			  uint branchPagesSize,
			  uint blockPagesSize)
	{
		ValueLen = valueLen / 4;
		
		HeaderBase = headerBase;
		HeaderBits = 32-headerBase;
		HeaderSize = (0xFFFFFFFF>>HeaderBits) + 1;

		countFreeBranchCell = 0;

		MAX_SAFE_SHORT = MAX_SHORT - valueLen - 1;

		pHeader = new uint[HeaderSize];
		for(uint i=0; i<HeaderSize; i++)
		{
			pHeader[i] = 0;
		}

		#ifndef _RELEASE

		for(uint i=0; i<COUNT_TEMPS; i++)
		{
			tempValues[i] = 0;
			tempCaptions[i] = 0;
		}

		tempCaptions[0] = "Moves Level1";
		tempCaptions[1] = "Moves Level2";
		tempCaptions[2] = "Moves Level3";
		tempCaptions[3] = "Moves Level4";
		tempCaptions[4] = "Amount Blocks";
		tempCaptions[5] = "Fill Blocks 1..64";
		tempCaptions[6] = "Fill Blocks 64..128";
		tempCaptions[7] = "Fill Blocks 128..192";
		tempCaptions[8] = "Fill Blocks 192..256";
		tempCaptions[9] = "Next blocks";
		tempCaptions[10] = "Short way";
		tempCaptions[11] = "Long way";
		tempCaptions[12] = "CURRENT_VALUE_TYPE";
		tempCaptions[13] = "ONLY_CONTENT_TYPE ";
		tempCaptions[14] = "MAX_BRANCH_TYPE1  ";
		tempCaptions[15] = "MAX_BLOCK_TYPE    ";
		tempCaptions[16] = "Fill Blocks 1..16";
		tempCaptions[17] = "Fill Blocks 16..32";
		tempCaptions[18] = "Fill Blocks 32..48";
		tempCaptions[19] = "Fill Blocks 48..64";
		tempCaptions[20] = "Fill Blocks 1..8";
		tempCaptions[21] = "Next Block Level 3";

		#endif

		pContentPages = new ContentPage*[contentPagesSize];
		pVarPages = new VarPage*[varPagesSize];
		pBranchPages = new BranchPage*[branchPagesSize];
		pBlockPages = new BlockPage*[blockPagesSize];

		memset(pContentPages, 0, contentPagesSize * sizeof(uint));
		memset(pVarPages, 0, varPagesSize * sizeof(uint));
		memset(pBranchPages, 0, branchPagesSize * sizeof(uint));
		memset(pBlockPages, 0, blockPagesSize * sizeof(uint));

		ContentPagesCount = 0;
		VarPagesCount = 0;
		BranchPagesCount = 0;
		BlockPagesCount = 0;

		ContentPagesSize = INIT_MAX_PAGES;
		VarPagesSize = INIT_MAX_PAGES;
		BranchPagesSize = INIT_MAX_PAGES;
		BlockPagesSize = INIT_MAX_PAGES;
		
		lastContentOffset = 1;
		lastVarOffset = 0;
		lastBranchOffset = 0;
		lastBlockOffset = 0;

		freeBranchCells = new uint[MAX_SHORT];

		AllowValueList = false;

		checkDeadlockFunc = 0;
		onContentCellMovedFunc = 0;

		attrValuesPool.init();
	}

	uint lastContentOffset;
	uint lastVarOffset;
	uint lastBranchOffset;
	uint lastBlockOffset;

	uint getHash()
	{
		return lastContentOffset +
			   lastVarOffset +
			   lastBranchOffset +
			   lastBlockOffset;
	}

	uint getHeaderSize()
	{
		return HeaderSize * sizeof(uint);
	}

	uint getContentSize()
	{
		return ContentPagesCount * sizeof(ContentPage);
	}

	uint getVarSize()
	{
		return VarPagesCount * sizeof(VarPage);
	}

	uint getBranchSize()
	{
		return BranchPagesCount * sizeof(BranchPage);
	}

	uint getBlockSize()
	{
		return BlockPagesCount * sizeof(BlockPage);
	}
	
	uint getUsedMemory()
	{
		return	getHeaderSize() + 
				getContentSize() + 
				getVarSize() + 
				getBranchSize() +
				getBlockSize() + 
				valueListPool.getUsedMemory() +
				attrValuesPool.getUsedMemory();
	}

	uint getTotalMemory()
	{
		return	getHeaderSize() + 
				getContentSize() + 
				getVarSize() + 
				getBranchSize() +
				getBlockSize() + 
				valueListPool.getTotalMemory() +
				attrValuesPool.getTotalMemory();
	}

	void printMemory()
	{
		printf("=================== HArrayVarRAM =========================\n");
		printf("Header size: %d\n", getHeaderSize());
		printf("Content size: %d\n", getContentSize());
		printf("Var size: %d\n", getVarSize());
		printf("Branch size: %d\n", getBranchSize());
		printf("Block size: %d\n", getBlockSize());
		printf("Value list pool size: %d\n", valueListPool.getTotalMemory());
		printf("Attr value pool size: %d\n", attrValuesPool.getTotalMemory());
		printf("Total size: %d\n", getTotalMemory());
		valueListPool.printMemory();
		attrValuesPool.printMemory();
	}

	void clear()
	{
		destroy();
		init(4, 24);
	}

	bool read(BinaryFile* pInfoFile,
			  BinaryFile* pHeaderFile,
			  BinaryFile* pContentPagesFile,
			  BinaryFile* pVarPagesFile,
			  BinaryFile* pBranchPagesFile,
			  BinaryFile* pBlockPagesFile,
			  BinaryFile* pValueListFile)
	{
		HArrayFixBaseInfo info;

		//header
		pInfoFile->read(&info, sizeof(HArrayFixBaseInfo));

		init(info.ValueLen,
			 info.HeaderBase,
			 info.ContentPagesCount + 1,
			 info.VarPagesCount + 1,
			 info.BranchPagesCount + 1,
			 info.BlockPagesCount + 1);

		ContentPagesCount = info.ContentPagesCount;
		VarPagesCount = info.VarPagesCount;
		BranchPagesCount = info.BranchPagesCount;
		BlockPagesCount = info.BlockPagesCount;
		
		lastContentOffset = info.LastContentOffset;
		lastVarOffset = info.LastVarOffset;
		lastBranchOffset = info.LastBranchOffset;
		lastBlockOffset = info.LastBlockOffset;
						
		//header
		pHeaderFile->read(pHeader, HeaderSize*sizeof(uint));
		
		//content
		for(uint i=0; i<ContentPagesCount; i++)
		{
			pContentPages[i] = new ContentPage();
			pContentPagesFile->read(pContentPages[i], sizeof(ContentPage));
		}

		//var
		for(uint i=0; i<VarPagesCount; i++)
		{
			pVarPages[i] = new VarPage();
			pVarPagesFile->read(pVarPages[i], sizeof(VarPage));
		}

		//barnches
		for(uint i=0; i<BranchPagesCount; i++)
		{
			pBranchPages[i] = new BranchPage();
			pBranchPagesFile->read(pBranchPages[i], sizeof(BranchPage));
		}

		//blocks
		for(uint i=0; i<BlockPagesCount; i++)
		{
			pBlockPages[i] = new BlockPage();
			pBlockPagesFile->read(pBlockPages[i], sizeof(BlockPage));
		}

		//value lists
		valueListPool.read(pValueListFile);
		//attrValuesPool.read

		return true;
	}

	bool save(BinaryFile* pInfoFile,
			  BinaryFile* pHeaderFile,
			  BinaryFile* pContentPagesFile,
			  BinaryFile* pVarPagesFile,
			  BinaryFile* pBranchPagesFile,
			  BinaryFile* pBlockPagesFile,
			  BinaryFile* pValueListFile)
	{
		//header
		HArrayFixBaseInfo info;

		info.Version = 1;
		info.ValueLen = ValueLen;
		info.HeaderBase = HeaderBase;
		info.ContentPagesCount = ContentPagesCount;
		info.VarPagesCount = VarPagesCount;
		info.BranchPagesCount = BranchPagesCount;
		info.BlockPagesCount = BlockPagesCount;
		info.LastContentOffset = lastContentOffset;
		info.LastBranchOffset = lastBranchOffset;
		info.LastBlockOffset = lastBlockOffset;

		if(!pInfoFile->write(&info, sizeof(HArrayFixBaseInfo)))
			return false;
		
		//header
		if(!pHeaderFile->write(pHeader, HeaderSize*sizeof(uint)))
			return false;
		
		//content
		for(uint i=0; i<ContentPagesCount; i++)
		{
			if(!pContentPagesFile->write(pContentPages[i], sizeof(ContentPage)))
				return false;
		}
				
		//var
		for(uint i=0; i<VarPagesCount; i++)
		{
			if(!pVarPagesFile->write(pVarPages[i], sizeof(VarPage)))
				return false;
		}

		//barnches
		for(uint i=0; i<BranchPagesCount; i++)
		{
			if(!pBranchPagesFile->write(pBranchPages[i], sizeof(BranchPage)))
				return false;
		}

		//blocks
		for(uint i=0; i<BlockPagesCount; i++)
		{
			if(!pBlockPagesFile->write(pBlockPages[i], sizeof(BlockPage)))
				return false;
		}
		
		//value lists
		if(!valueListPool.save(pValueListFile))
			return false;

		return true;
	}
	
	//types: 0-empty, 1..4 branches, 5 value, 6..9 blocks offset, 10 empty branch, 11 value
#ifndef _RELEASE

	uint tempValues[COUNT_TEMPS];
	char* tempCaptions[COUNT_TEMPS];

#endif
	
	void reallocateContentPages()
	{
		uint newSizeContentPages = ContentPagesSize * 2;
		ContentPage** pTempContentPages = new ContentPage*[newSizeContentPages];
			
		uint j=0;
		for(; j < ContentPagesSize ; j++)
		{
			pTempContentPages[j] = pContentPages[j];
		}

		for(; j < newSizeContentPages ; j++)
		{
			pTempContentPages[j] = 0;
		}

		delete[] pContentPages;
		pContentPages = pTempContentPages;

		ContentPagesSize = newSizeContentPages;
	}

	void reallocateVarPages()
	{
		uint newSizeVarPages = VarPagesSize * 2;
		VarPage** pTempVarPages = new VarPage*[newSizeVarPages];
			
		uint j=0;
		for(; j < VarPagesSize ; j++)
		{
			pTempVarPages[j] = pVarPages[j];
		}

		for(; j < newSizeVarPages ; j++)
		{
			pTempVarPages[j] = 0;
		}

		delete[] pVarPages;
		pVarPages = pTempVarPages;

		VarPagesSize = newSizeVarPages;
	}

	void reallocateBranchPages()
	{
		uint newSizeBranchPages = BranchPagesSize * 2;
		BranchPage** pTempBranchPages = new BranchPage*[newSizeBranchPages];
			
		uint j=0;
		for(; j < BranchPagesSize ; j++)
		{
			pTempBranchPages[j] = pBranchPages[j];
		}

		for(; j < newSizeBranchPages ; j++)
		{
			pTempBranchPages[j] = 0;
		}

		delete[] pBranchPages;
		pBranchPages = pTempBranchPages;

		BranchPagesSize = newSizeBranchPages;
	}

	void reallocateBlockPages()
	{
		uint newSizeBlockPages = BlockPagesSize * 2;
		BlockPage** pTempBlockPages = new BlockPage*[newSizeBlockPages];
			
		uint j=0;
		for(; j < BlockPagesSize ; j++)
		{
			pTempBlockPages[j] = pBlockPages[j];
		}

		for(; j < newSizeBlockPages ; j++)
		{
			pTempBlockPages[j] = 0;
		}

		delete[] pBlockPages;
		pBlockPages = pTempBlockPages;

		BlockPagesSize = newSizeBlockPages;
	}
	//CALLBACKS ==========================================================================================================
	ON_CONTENT_CELL_MOVED_FUNC* onContentCellMovedFunc;
	CHECK_DEADLOCK_FUNC* checkDeadlockFunc;

	//INSERT =============================================================================================================

	//returns ha1DocIndex
	uint insert(uint* key,
				uint keyLen,
				uint value);
		
	//GET =============================================================================================================

	uint getValueByKey(uint* key,
					   uint keyLen,
					   uchar& valueType,
					   uchar readModeType = 0,
					   uchar tranID = 0,
					   ReadedList* pReadedList = 0);

	bool hasPartKey(uint* key, uint keyLen);
	void delValueByKey(uint* key, uint keyLen, uint value, uint index = 0);

	//uint* getValueByVarKey(uint* key, uint keyLen);
	
	//ISREADED ===========================================================================================================
	inline uint processReadByTranID(ContentCell& contentCell,
									uchar readedModeType,
									uchar tranID,
									ReadedList* pReadedList)
	{
		switch (readedModeType)
		{
			case 0: //0. Without any check (ha1)
			{
				return contentCell.Value;
			}
			case 1: //1. Read data with check blocking (ha2)
			{
				if (contentCell.ReadedByTranID.load() != tranID) //not mine blocking
				{
					uint timeout = 10000;

					while (contentCell.ReadedByTranID.load())
					{
						if (--timeout)
						{
							pReadedList->BlockedByTranID = contentCell.ReadedByTranID.load();
							pReadedList->BlockedOnValue = contentCell.Value;

							Sleep(1); //do time for other threads
						}
						else //deadlock ??
						{
							if (checkDeadlockFunc(tranID))
							{
								return 0;
							}
							else
							{
								timeout = 10000;
							}
						}
					}
				}

				return contentCell.Value;
			}
			case 2: //2. Read data and check blocking and block with put to array blocked cell (ha2)
			{
				if (contentCell.ReadedByTranID.load() != tranID) //not mine blocking
				{
					uint timeout = 10000;

					uchar val = 0;

					while (!contentCell.ReadedByTranID.compare_exchange_weak(val, tranID)) //block
					{
						val = 0;

						if (--timeout)
						{
							pReadedList->BlockedByTranID = contentCell.ReadedByTranID.load();
							pReadedList->BlockedOnValue = contentCell.Value;

							Sleep(1); //do time for other threads
						}
						else //deadlock ??
						{
							if (checkDeadlockFunc(tranID))
							{
								return 0;
							}
							else
							{
								timeout = 10000;
							}
						}
					}

					pReadedList->addValue(&contentCell.ReadedByTranID);
				}

				return contentCell.Value;
			}
			case 3: //3. Check if key blocked (ha2 for commit)
			{
				return contentCell.ReadedByTranID.load();
			}
			case 4: //4. Check if key blocked, but excluded my cells
			{
				if(contentCell.ReadedByTranID.load() == tranID)
				{
					return 0; //mine blocking
				}
				else
				{
					return 1; //blocked by other tran
				}
			}
			default: //undefined
			{
				return 0;
			}
		}
	}

	//RANGE =============================================================================================================
	HACursor getCursorByValue()
	{
		HACursor cursor;
		cursor.CountFullContentPage = ContentPagesCount - 1;
		cursor.SizeLastContentPage = lastContentOffset & 0xFFFF;

		cursor.Page = 0;
		cursor.Index = 0;

		return cursor;
	}

	bool moveCursor(HACursor& cursor)
	{
		uint& page = cursor.Page;
		uint& index = cursor.Index;

		//pages
		while(page < cursor.CountFullContentPage)
		{
			ContentPage* pContentPage = pContentPages[page];
			
			while(index < MAX_SHORT)
			{
				uchar& contentType = pContentPage->pContent[index].Type;

				if(contentType >= ONLY_CONTENT_TYPE)
				{
					index += contentType - ONLY_CONTENT_TYPE;

					if(index >= MAX_SHORT)
					{
						//next page
						page++;
						index &= 0xFFFF;
						
						cursor.Value = &pContentPages[page]->pContent[index].Value;
					}
					else
					{
						cursor.Value = &pContentPage->pContent[index].Value;
					}

					index += ValueLen;
					
					if(index >= MAX_SHORT)
					{
						cursor.Page++;
						index &= 0xFFFF;
					}

					return true;
				}
				else if(contentType == VALUE_TYPE ||
						contentType == VALUE_LIST_TYPE)
				{
					cursor.Value = &pContentPage->pContent[index].Value;
				
					index += ValueLen;

					if(index >= MAX_SHORT)
					{
						cursor.Page++;
						index &= 0xFFFF;
					}

					return true;
				}
				else
				{
					index++;
				}
			}

			cursor.Page++;
			cursor.Index = 0;
		}
		
		//last page
		ContentPage* pContentPage = pContentPages[page];
		
		while(index < cursor.SizeLastContentPage)
		{
			uchar& contentType = pContentPage->pContent[index].Type;
			
			if(contentType >= ONLY_CONTENT_TYPE)
			{
				index += contentType - ONLY_CONTENT_TYPE;

				if(index >= MAX_SHORT)
				{
					//next page
					page++;
					index &= 0xFFFF;
					
					cursor.Value = &pContentPages[page]->pContent[index].Value;
				}
				else
				{
					cursor.Value = &pContentPage->pContent[index].Value;
				}

				index += ValueLen;
				
				if(index >= MAX_SHORT)
				{
					cursor.Page++;
					index &= 0xFFFF;
				}

				return true;
			}
			else if(contentType == VALUE_TYPE ||
					contentType == VALUE_LIST_TYPE)
			{
				cursor.Value = &pContentPage->pContent[index].Value;
			
				index += ValueLen;

				if(index >= MAX_SHORT)
				{
					cursor.Page++;
					index &= 0xFFFF;
				}

				return true;
			}
			else
			{
				index++;
			}
		}

		return false;
	}

	void getValuesByRangeFromBlock(uint** values, 
									uint& count,
									uint size,
									uint contentOffset,
									uint keyOffset,
									uint blockOffset,
									uint* minKey,
									uint* maxKey);
	
	void getValuesByRange(uint** values, 
								uint& count,
								uint size,
								uint keyOffset, 
								uint contentOffset,
								uint* minKey,
								uint* maxKey);
	
	uint getValuesByRange(uint** values, 
						 uint size, 
						 uint* minKey, 
						 uint* maxKey);

	//RANGE keys and values =============================================================================================================
	void sortLastItem(HArrayFixPair* pairs, 
					  uint count);
	
	void getKeysAndValuesByRangeFromBlock(HArrayFixPair* pairs,
										  uint& count,
										  uint size,
										  uint contentOffset,
										  uint keyOffset,
										  uint blockOffset,
										  uint* minKey,
										  uint* maxKey);
	
	void getKeysAndValuesByRange(HArrayFixPair* pairs,
								 uint& count,
								 uint size,
								 uint keyOffset, 
								 uint contentOffset,
								 uint* minKey,
								 uint* maxKey);
	
	uint getKeysAndValuesByRange(HArrayFixPair* pairs,
								 uint size, 
								 uint* minKey, 
								 uint* maxKey);

	//TEMPLATE ====================================================================================================
	void scanKeysAndValuesFromBlock(uint* key,
									uint contentOffset,
									uint keyOffset,
									uint blockOffset,
									HARRAY_ITEM_VISIT_FUNC visitor,
									void* pData);
	
	void scanKeysAndValues(uint* key,
						   uint keyOffset, 
						   uint contentOffset,
						   HARRAY_ITEM_VISIT_FUNC visitor,
						   void* pData);
	
	uint scanKeysAndValues(uint* key, 
						   uint keyLen,
						   HARRAY_ITEM_VISIT_FUNC visitor,
						   void* pData);

	uint scanKeysAndValues(uint* key,
						   HARRAY_ITEM_VISIT_FUNC visitor,
						   void* pData);

	//=============================================================================================================

	void destroy()
	{
		if(pHeader)
		{
			delete[] pHeader;
			pHeader = 0;
		}

		if(pContentPages)
		{
			for(uint i=0; i<ContentPagesCount; i++)
			{
				delete pContentPages[i];
			}

			delete[] pContentPages;
			pContentPages = 0;
		}

		if(pVarPages)
		{
			for(uint i=0; i<VarPagesCount; i++)
			{
				delete pVarPages[i];
			}

			delete[] pVarPages;
			pVarPages = 0;
		}

		if(pBranchPages)
		{
			for(uint i=0; i<BranchPagesCount; i++)
			{
				delete pBranchPages[i];
			}
			
			delete[] pBranchPages;
			pBranchPages = 0;
		}

		if(pBlockPages)
		{
			for(uint i=0; i<BlockPagesCount; i++)
			{
				delete pBlockPages[i];
			}

			delete[] pBlockPages;
			pBlockPages = 0;
		}

		delete[] freeBranchCells;

		valueListPool.destroy();
		attrValuesPool.destroy();
	}
};