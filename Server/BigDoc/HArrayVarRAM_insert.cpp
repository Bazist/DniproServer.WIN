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

#include "stdafx.h"
#include "HArrayVarRAM.h"

uint32 HArrayVarRAM::insert(uint32* key,
						  uint32 keyLen,
						  uint32 value)
{
	keyLen >>= 2; //in 4 bytes 

	uint32 maxSafeShort = MAX_SAFE_SHORT - keyLen;

	//create new page =======================================================================================
	if(!pContentPages[(lastContentOffset + keyLen + ValueLen)>>16])
	{
		pContentPages[ContentPagesCount++] = new ContentPage();

		if(ContentPagesCount == ContentPagesSize)
		{
			reallocateContentPages();
		}
	}

	//insert value ==========================================================================================
	uint32 keyOffset = 0;
	uint32 contentOffset = 0;

	uint32 headerOffset = key[0]>>HeaderBits;
	contentOffset = pHeader[headerOffset];

	if(!contentOffset)
	{
		pHeader[headerOffset] = lastContentOffset;
		
		ContentPage* pContentPage = pContentPages[lastContentOffset>>16];
		uint32 contentIndex = lastContentOffset&0xFFFF;

		pContentPage->pContent[contentIndex].Type = (ONLY_CONTENT_TYPE + keyLen);

		if(contentIndex < maxSafeShort) //content in one page
		{
			//fill key
			for(; keyOffset < keyLen; contentIndex++, keyOffset++, lastContentOffset++)
			{
				pContentPage->pContent[contentIndex].Value = key[keyOffset];
			}

			pContentPage->pContent[contentIndex].Type = VALUE_TYPE;
			pContentPage->pContent[contentIndex].Value = value;

			lastContentOffset++;

			return 0;
		}
		else
		{
			for(; keyOffset < keyLen; lastContentOffset++, keyOffset++)
			{
				pContentPage = pContentPages[lastContentOffset>>16];
				pContentPage->pContent[lastContentOffset&0xFFFF].Value = key[keyOffset];
			}

			pContentPage = pContentPages[lastContentOffset>>16];

			pContentPage->pContent[lastContentOffset&0xFFFF].Type = VALUE_TYPE;
			pContentPage->pContent[lastContentOffset&0xFFFF].Value = value;

			lastContentOffset++;

			return 0;
		}

		#ifndef _RELEASE
		tempValues[10]++;
		#endif
	}
	
	#ifndef _RELEASE
	tempValues[11]++;
	#endif

	//TWO KEYS =============================================================================================
NEXT_KEY_PART:

	ContentPage* pContentPage = pContentPages[contentOffset>>16];
	uint32 contentIndex = contentOffset & 0xFFFF;

	uchar8 contentCellType = pContentPage->pContent[contentIndex].Type;

	if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================
	{
		#ifndef _RELEASE
		tempValues[13]++;
		#endif

		uint32 originKeyLen = keyOffset + contentCellType - ONLY_CONTENT_TYPE;
		
		if(contentIndex < maxSafeShort) //content in one page
		{
			for(; keyOffset < originKeyLen; contentOffset++, contentIndex++, keyOffset++)
			{
				ContentCell& contentCell = pContentPage->pContent[contentIndex];
					
				if(contentCell.Value != key[keyOffset])
				{
					//create branch
					contentCell.Type = MIN_BRANCH_TYPE1 + 1;

					//get free branch cell
					BranchCell* pBranchCell;
					if(countFreeBranchCell)
					{
						uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
						pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

						pBranchCell->Values[0] = contentCell.Value;
						contentCell.Value = branchOffset;
					}
					else
					{
						BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
						if(!pBranchPage)
						{
							pBranchPage = new BranchPage();
							pBranchPages[BranchPagesCount++] = pBranchPage;

							if(BranchPagesCount == BranchPagesSize)
							{
								reallocateBranchPages();
							}
						}

						pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

						pBranchCell->Values[0] = contentCell.Value;
						contentCell.Value = lastBranchOffset++;
					}
			
					pBranchCell->Offsets[0] = contentOffset + 1;

					pBranchCell->Values[1] = key[keyOffset];
					pBranchCell->Offsets[1] = lastContentOffset;

					if((keyOffset + 1) < originKeyLen)
					{
						pContentPage->pContent[contentIndex + 1].Type = (ONLY_CONTENT_TYPE + originKeyLen - keyOffset - 1);
					}

					goto FILL_KEY;
				}
				else
				{
					contentCell.Type = CURRENT_VALUE_TYPE; //reset to current value	
				}

				//key less than original, insert
				if(keyOffset == keyLen)
				{
					VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
					if(!pVarPage)
					{
						pVarPage = new VarPage();
						pVarPages[VarPagesCount++] = pVarPage;

						if(VarPagesCount == VarPagesSize)
						{
							reallocateVarPages();
						}
					}
				
					VarCell& varCell = pVarPage->pVar[lastVarOffset & 0xFFFF];
					varCell.ContCell.Type = contentCell.Type;
					varCell.ContCell.Value = contentCell.Value;

					//shounted, notify trans !
					if (onContentCellMovedFunc)
					{
						uchar8 tranID = contentCell.ReadedByTranID.load();
						if (tranID)
						{
							onContentCellMovedFunc(tranID,
								&contentCell.ReadedByTranID,
								&varCell.ValueContentCell.ReadedByTranID);

							varCell.ValueContentCell.ReadedByTranID.store(contentCell.ReadedByTranID.load());
							contentCell.ReadedByTranID.store(0);
						}
					}

					varCell.ValueContentCell.Type = VALUE_TYPE;
					varCell.ValueContentCell.Value = value;

					contentCell.Type = VAR_TYPE;
					contentCell.Value = lastVarOffset++;

					//set rest of key and value
					for(contentIndex++, keyOffset++; keyOffset < originKeyLen; contentIndex++, keyOffset++)
					{
						pContentPage->pContent[contentIndex].Type = CURRENT_VALUE_TYPE;
					}

					pContentPage->pContent[contentIndex].Type = VALUE_TYPE;

					return 0;
				}
			}
			
			if(keyLen > originKeyLen) //key more than original
			{
				VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
				if(!pVarPage)
				{
					pVarPage = new VarPage();
					pVarPages[VarPagesCount++] = pVarPage;

					if(VarPagesCount == VarPagesSize)
					{
						reallocateVarPages();
					}
				}
				
				ContentCell& contentCell = pContentPage->pContent[contentIndex];

				VarCell& varCell = pVarPage->pVar[lastVarOffset & 0xFFFF];

				//shounted, notify trans !
				if (onContentCellMovedFunc)
				{
					uchar8 tranID = contentCell.ReadedByTranID.load();
					if (tranID)
					{
						onContentCellMovedFunc(tranID,
							&contentCell.ReadedByTranID,
							&varCell.ValueContentCell.ReadedByTranID);

						varCell.ValueContentCell.ReadedByTranID.store(contentCell.ReadedByTranID.load());
						contentCell.ReadedByTranID.store(0);
					}
				}

				varCell.ValueContentCell.Type = VALUE_TYPE;
				varCell.ValueContentCell.Value = contentCell.Value;

				varCell.ContCell.Type = CONTINUE_VAR_TYPE;
				varCell.ContCell.Value = lastContentOffset;

				contentCell.Type = VAR_TYPE;
				contentCell.Value = lastVarOffset++;

				goto FILL_KEY2;
			}
			else //key is exists, update
			{
				ContentCell& contentCell = pContentPage->pContent[contentIndex];

				if(AllowValueList)
				{
					uint32 index;

					if(contentCell.Type == VALUE_LIST_TYPE)
					{
						ValueList* pValueList = valueListPool.fromSerPointer(contentCell.Value);
						index = pValueList->addValue(value);
					}
					else //VALUE_TYPE
					{
						uint32 prevValue = contentCell.Value;
						ValueList* pValueList = valueListPool.newSerObject(contentCell.Value);
						pValueList->addValue(prevValue);
						index = pValueList->addValue(value);
					
						contentCell.Type = VALUE_LIST_TYPE;
					}

					return index;
				}
				else
				{
					contentCell.Value = value;
				}

				return 0;
			}
		}
		else  //content in two pages
		{
			for(; keyOffset < originKeyLen; contentOffset++, keyOffset++)
			{
				pContentPage = pContentPages[contentOffset>>16];
				contentIndex = contentOffset & 0xFFFF;

				if(pContentPage->pContent[contentIndex].Value != key[keyOffset])
				{
					//create branch
					pContentPage->pContent[contentIndex].Type = MIN_BRANCH_TYPE1 + 1;
								
					//get free branch cell
					BranchCell* pBranchCell;
					if(countFreeBranchCell)
					{
						uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
						pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

						pBranchCell->Values[0] = pContentPage->pContent[contentIndex].Value;
						pContentPage->pContent[contentIndex].Value = branchOffset;
					}
					else
					{
						BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
						if(!pBranchPage)
						{
							pBranchPage = new BranchPage();
							pBranchPages[BranchPagesCount++] = pBranchPage;

							if(BranchPagesCount == BranchPagesSize)
							{
								reallocateBranchPages();
							}
						}

						pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

						pBranchCell->Values[0] = pContentPage->pContent[contentIndex].Value;
						pContentPage->pContent[contentIndex].Value = lastBranchOffset++;
					}
									
					pBranchCell->Offsets[0] = contentOffset + 1;

					pBranchCell->Values[1] = key[keyOffset];
					pBranchCell->Offsets[1] = lastContentOffset;

					if((keyOffset + 1) < originKeyLen)
					{
						//set rest of key
						contentOffset++;

						pContentPage = pContentPages[contentOffset>>16];
						contentIndex = contentOffset & 0xFFFF;
					
						pContentPage->pContent[contentIndex].Type = (ONLY_CONTENT_TYPE + originKeyLen - keyOffset - 1);
					}
					
					goto FILL_KEY;
				}
				else
				{
					pContentPage->pContent[contentIndex].Type = CURRENT_VALUE_TYPE; //reset to current value	
				}

				//key less than original, insert
				if(keyOffset == keyLen)
				{
					VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
					if(!pVarPage)
					{
						pVarPage = new VarPage();
						pVarPages[VarPagesCount++] = pVarPage;

						if(VarPagesCount == VarPagesSize)
						{
							reallocateVarPages();
						}
					}
				
					VarCell& varCell = pVarPage->pVar[lastVarOffset & 0xFFFF];

					ContentCell& currContentCell = pContentPage->pContent[contentIndex];

					varCell.ContCell.Type = currContentCell.Type;
					varCell.ContCell.Value = currContentCell.Value;

					//shounted, notify trans !
					if (onContentCellMovedFunc)
					{
						uchar8 tranID = currContentCell.ReadedByTranID.load();
						if (tranID)
						{
							onContentCellMovedFunc(tranID,
								&currContentCell.ReadedByTranID,
								&varCell.ValueContentCell.ReadedByTranID);

							varCell.ValueContentCell.ReadedByTranID.store(currContentCell.ReadedByTranID.load());
							currContentCell.ReadedByTranID.store(0);
						}
					}

					varCell.ValueContentCell.Type = VALUE_TYPE;
					varCell.ValueContentCell.Value = value;

					pContentPage->pContent[contentIndex].Type = VAR_TYPE;
					pContentPage->pContent[contentIndex].Value = lastVarOffset++;

					//set rest of key and value
					for(contentOffset++, keyOffset++; keyOffset < originKeyLen; contentOffset++, keyOffset++)
					{
						pContentPages[contentOffset>>16]->pContent[contentOffset & 0xFFFF].Type = CURRENT_VALUE_TYPE;
					}

					pContentPages[contentOffset>>16]->pContent[contentOffset & 0xFFFF].Type = VALUE_TYPE;

					return 0;
				}
			}

			pContentPage = pContentPages[contentOffset>>16];
			contentIndex = contentOffset & 0xFFFF;

			if(keyLen > originKeyLen) //key more than original
			{
				VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
				if(!pVarPage)
				{
					pVarPage = new VarPage();
					pVarPages[VarPagesCount++] = pVarPage;

					if(VarPagesCount == VarPagesSize)
					{
						reallocateVarPages();
					}
				}
				
				ContentCell& contentCell = pContentPage->pContent[contentIndex];

				VarCell& varCell = pVarPage->pVar[lastVarOffset & 0xFFFF];

				//shounted, notify trans !
				if (onContentCellMovedFunc)
				{
					uchar8 tranID = contentCell.ReadedByTranID.load();
					if (tranID)
					{
						onContentCellMovedFunc(tranID,
							&contentCell.ReadedByTranID,
							&varCell.ValueContentCell.ReadedByTranID);

						varCell.ValueContentCell.ReadedByTranID.store(contentCell.ReadedByTranID.load());
						contentCell.ReadedByTranID.store(0);
					}
				}

				varCell.ValueContentCell.Type = VALUE_TYPE;
				varCell.ValueContentCell.Value = contentCell.Value;

				varCell.ContCell.Type = CONTINUE_VAR_TYPE;
				varCell.ContCell.Value = lastContentOffset;

				contentCell.Type = VAR_TYPE;
				contentCell.Value = lastVarOffset++;

				goto FILL_KEY2;
			}
			else //key is exists, update
			{
				ContentCell& contentCell = pContentPage->pContent[contentIndex];

				if(AllowValueList)
				{
					uint32 index;

					if(contentCell.Type == VALUE_LIST_TYPE)
					{
						ValueList* pValueList = valueListPool.fromSerPointer(contentCell.Value);
						index = pValueList->addValue(value);
					}
					else //VALUE_TYPE
					{
						uint32 prevValue = contentCell.Value;
						ValueList* pValueList = valueListPool.newSerObject(contentCell.Value);
						pValueList->addValue(prevValue);
						index = pValueList->addValue(value);
					
						contentCell.Type = VALUE_LIST_TYPE;
					}

					return index;
				}
				else
				{
					contentCell.Value = value;
				}

				return 0;
			}
		}

		return 0; 
	}

	uint32 keyValue;

	keyValue = key[keyOffset];

	ContentCell* pContentCell;

	pContentCell = &pContentPage->pContent[contentIndex];

	uint32 contentCellValueOrOffset;

	contentCellValueOrOffset = pContentPage->pContent[contentIndex].Value;
	
	if(contentCellType == VAR_TYPE) //VAR =====================================================================
	{
		VarPage* pVarPage = pVarPages[contentCellValueOrOffset >> 16];
		VarCell& varCell = pVarPage->pVar[contentCellValueOrOffset & 0xFFFF];

		if(keyOffset < keyLen)
		{
			contentCellType = varCell.ContCell.Type; //read from var cell
			contentCellValueOrOffset = varCell.ContCell.Value;
			
			if(contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
			{
				contentOffset = contentCellValueOrOffset;
		
				goto NEXT_KEY_PART;
			}

			pContentCell = &varCell.ContCell;
		}
		else
		{
			//update existing value
			ContentCell& contentCell = varCell.ValueContentCell;

			if(AllowValueList)
			{
				uint32 index;

				if(contentCell.Type == VALUE_LIST_TYPE)
				{
					ValueList* pValueList = valueListPool.fromSerPointer(contentCell.Value);
					index = pValueList->addValue(value);
				}
				else //VALUE_TYPE
				{
					uint32 prevValue = contentCell.Value;
					ValueList* pValueList = valueListPool.newSerObject(contentCell.Value);
					pValueList->addValue(prevValue);
					index = pValueList->addValue(value);
				
					contentCell.Type = VALUE_LIST_TYPE;
				}

				return index;
			}
			else
			{
				contentCell.Value = value;
			}
			
			return 0;
		}
	}
	else if(contentCellType == VALUE_TYPE ||
			contentCellType == VALUE_LIST_TYPE) //update existing value
	{
		if(keyOffset < keyLen)
		{
			VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
			if(!pVarPage)
			{
				pVarPage = new VarPage();
				pVarPages[VarPagesCount++] = pVarPage;

				if(VarPagesCount == VarPagesSize)
				{
					reallocateVarPages();
				}
			}
				
			ContentCell& contentCell = pContentPage->pContent[contentIndex];

			VarCell& varCell = pVarPage->pVar[lastVarOffset & 0xFFFF];

			//shounted, notify trans !
			if (onContentCellMovedFunc)
			{
				uchar8 tranID = contentCell.ReadedByTranID.load();

				if (tranID)
				{
					onContentCellMovedFunc(tranID,
						&contentCell.ReadedByTranID,
						&varCell.ValueContentCell.ReadedByTranID);

					varCell.ValueContentCell.ReadedByTranID.store(contentCell.ReadedByTranID.load());
					contentCell.ReadedByTranID.store(0);
				}
			}

			varCell.ValueContentCell.Type = VALUE_TYPE;
			varCell.ValueContentCell.Value = contentCell.Value;

			varCell.ContCell.Type = CONTINUE_VAR_TYPE;
			varCell.ContCell.Value = lastContentOffset;

			contentCell.Type = VAR_TYPE;
			contentCell.Value = lastVarOffset++;

			goto FILL_KEY2;
		}
		else
		{
			if(AllowValueList)
			{
				uint32 index;

				if(pContentCell->Type == VALUE_LIST_TYPE)
				{
					ValueList* pValueList = valueListPool.fromSerPointer(pContentCell->Value);
					index = pValueList->addValue(value);
				}
				else //VALUE_TYPE
				{
					uint32 prevValue = pContentCell->Value;
					ValueList* pValueList = valueListPool.newSerObject(pContentCell->Value);
					pValueList->addValue(prevValue);
					index = pValueList->addValue(value);
				
					pContentCell->Type = VALUE_LIST_TYPE;
				}

				return index;
			}
			else
			{
				pContentCell->Value = value;
			}
			
			return 0;
		}
	}
	else if(keyOffset == keyLen) //STOP =====================================================================
	{
		VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
		if(!pVarPage)
		{
			pVarPage = new VarPage();
			pVarPages[VarPagesCount++] = pVarPage;

			if(VarPagesCount == VarPagesSize)
			{
				reallocateVarPages();
			}
		}

		ContentCell& contentCell = pContentPage->pContent[contentIndex];
				
		VarCell& varCell = pVarPage->pVar[lastVarOffset & 0xFFFF];
		varCell.ContCell.Type = contentCell.Type;
		varCell.ContCell.Value = contentCell.Value;

		//shounted, notify trans !
		if (onContentCellMovedFunc)
		{
			uchar8 tranID = contentCell.ReadedByTranID.load();
			if (tranID)
			{
				onContentCellMovedFunc(tranID,
					&contentCell.ReadedByTranID,
					&varCell.ValueContentCell.ReadedByTranID);

				varCell.ValueContentCell.ReadedByTranID.store(contentCell.ReadedByTranID.load());
				contentCell.ReadedByTranID.store(0);
			}
		}

		varCell.ValueContentCell.Type = VALUE_TYPE;
		varCell.ValueContentCell.Value = value; //save value

		contentCell.Type = VAR_TYPE;
		contentCell.Value = lastVarOffset++;

		return 0;
	}
		
	if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
	{
		#ifndef _RELEASE
		tempValues[14]++;
		#endif

		BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
		BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

		//try find value in the list
		for(uint32 i=0; i<contentCellType; i++)
		{
			if(branchCell.Values[i] == keyValue)
			{
				contentOffset = branchCell.Offsets[i];
				keyOffset++;
				
				goto NEXT_KEY_PART;
			}
		}

		//create value in branch
		if(contentCellType < MAX_BRANCH_TYPE1)
		{
			branchCell.Values[contentCellType] = keyValue;
			branchCell.Offsets[contentCellType] = lastContentOffset;

			pContentCell->Type++;
			
			goto FILL_KEY;
		}
		else
		{
			//EXTRACT BRANCH AND CREATE BLOCK =========================================================
			uchar8 idxKeyValue = 0;
			uchar8 currContentCellType = MIN_BLOCK_TYPE; 

			if(countFreeBranchCell < MAX_SHORT)
			{
				freeBranchCells[countFreeBranchCell++] = contentCellValueOrOffset;
			}

			const ushort16 countCell = BRANCH_ENGINE_SIZE + 1;

EXTRACT_BRANCH:
			
			BlockCell blockCells[countCell];
			uchar8 indexes[countCell];

			for(uint32 i=0; i < MAX_BRANCH_TYPE1; i++)
			{
				BlockCell& currBlockCell = blockCells[i];
				currBlockCell.Type = CURRENT_VALUE_TYPE;
				currBlockCell.Offset = branchCell.Offsets[i];

				uint32& value = branchCell.Values[i];
				currBlockCell.ValueOrOffset = value;

				indexes[i] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT); //((uchar8*)&value)[idxKeyValue];
			}

			//add current value
			BlockCell& currBlockCell4 = blockCells[BRANCH_ENGINE_SIZE];
			currBlockCell4.Type = CURRENT_VALUE_TYPE;
			currBlockCell4.Offset = lastContentOffset;
			currBlockCell4.ValueOrOffset = keyValue;

			indexes[BRANCH_ENGINE_SIZE] = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			
			//clear map
			uchar8 mapIndexes[BLOCK_ENGINE_SIZE];
			for(uint32 i=0; i < countCell; i++)
			{
				mapIndexes[indexes[i]] = 0;
			}

			//fill map
			for(uint32 i=0; i < countCell; i++)
			{
				uchar8& countIndexes = mapIndexes[indexes[i]];
				countIndexes++;

				if(countIndexes > BRANCH_ENGINE_SIZE)
				{
					idxKeyValue += BLOCK_ENGINE_STEP;
					currContentCellType++;
					goto EXTRACT_BRANCH; //use another byte
				}
			}

			//allocate page
			uint32 maxLastBlockOffset = lastBlockOffset + BLOCK_ENGINE_SIZE*2;
			if(!pBlockPages[maxLastBlockOffset>>16])
			{
				pBlockPages[BlockPagesCount++] = new BlockPage();

				if(BlockPagesCount == BlockPagesSize)
				{
					reallocateBlockPages();
				}
			}

			//fill block
			pContentCell->Type = currContentCellType;
			pContentCell->Value = lastBlockOffset;

			for(uint32 i = 0; i < countCell; i++)
			{
				uchar8 idx = indexes[i];
				uint32 offset = lastBlockOffset + idx;

				BlockPage* pBlockPage = pBlockPages[offset >> 16];
				BlockCell& currBlockCell = pBlockPage->pBlock[offset & 0xFFFF];
				
				uchar8 count = mapIndexes[idx];
				if(count == 1) //one value in block cell
				{
					currBlockCell = blockCells[i];
				}
				else if(count <= BRANCH_ENGINE_SIZE) //create branch cell
				{
					if(currBlockCell.Type == 0) //create branch
					{
						//get free branch cell
						BranchCell* pCurrBranchCell;
						if(countFreeBranchCell)
						{
							uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
							pCurrBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];
							currBlockCell.Offset = branchOffset;
						}
						else
						{
							BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
							if(!pBranchPage)
							{
								pBranchPage = new BranchPage();
								pBranchPages[BranchPagesCount++] = pBranchPage;

								if(BranchPagesCount == BranchPagesSize)
								{
									reallocateBranchPages();
								}
							}

							pCurrBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];
							currBlockCell.Offset = lastBranchOffset++;
						}

						currBlockCell.Type = MIN_BRANCH_TYPE1;
						
						pCurrBranchCell->Values[0] = blockCells[i].ValueOrOffset;
						pCurrBranchCell->Offsets[0] = blockCells[i].Offset;
					}
					else //types 1..4
					{
						BranchPage* pBranchPage = pBranchPages[currBlockCell.Offset >> 16];
						BranchCell& currBranchCell = pBranchPage->pBranch[currBlockCell.Offset & 0xFFFF];

						currBranchCell.Values[currBlockCell.Type] = blockCells[i].ValueOrOffset;
						currBranchCell.Offsets[currBlockCell.Type] = blockCells[i].Offset;
						currBlockCell.Type++;
					}
				}
				else
				{
					printf("FAIL STATE.");
				}
			}

			lastBlockOffset += BLOCK_ENGINE_SIZE;

			#ifndef _RELEASE
			tempValues[4]++;
			#endif

			goto FILL_KEY;
		}
	}
	else if(contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
	{
		#ifndef _RELEASE
		tempValues[15]++;
		#endif

		//uint32 level = 1;

		uchar8 idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

		uint32 startOffset = contentCellValueOrOffset;

	NEXT_BLOCK:
		uint32 subOffset = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
		uint32 blockOffset = startOffset + subOffset;
		
		BlockPage* pBlockPage = pBlockPages[blockOffset >> 16];
		if(!pBlockPage)
		{
			pBlockPage = new BlockPage();
			pBlockPages[BlockPagesCount++] = pBlockPage;

			if(BlockPagesCount == BlockPagesSize)
			{
				reallocateBlockPages();
			}
		}

		BlockCell& blockCell = pBlockPage->pBlock[blockOffset & 0xFFFF];

		uchar8& blockCellType = blockCell.Type;

		if(blockCellType == EMPTY_TYPE) //free cell, fill
		{
			blockCellType = CURRENT_VALUE_TYPE;
			blockCell.ValueOrOffset = keyValue;
			blockCell.Offset = lastContentOffset;
			
			goto FILL_KEY;
		}
		else //block cell 
		{
			if(blockCellType == CURRENT_VALUE_TYPE) //current value
			{
				if(blockCell.ValueOrOffset == keyValue) //value is exists
				{
					contentOffset = blockCell.Offset;
					keyOffset++;

					goto NEXT_KEY_PART;
				}
				else //create branch with two values
				{
					blockCellType = MIN_BRANCH_TYPE1 + 1;
			
					//get free branch cell
					BranchCell* pBranchCell;
					if(countFreeBranchCell)
					{
						uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
						pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

						pBranchCell->Offsets[0] = blockCell.Offset;
						blockCell.Offset = branchOffset;
					}
					else
					{
						BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
						if(!pBranchPage)
						{
							pBranchPage = new BranchPage();
							pBranchPages[BranchPagesCount++] = pBranchPage;

							if(BranchPagesCount == BranchPagesSize)
							{
								reallocateBranchPages();
							}
						}

						pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

						pBranchCell->Offsets[0] = blockCell.Offset;
						blockCell.Offset = lastBranchOffset++;
					}
					
					pBranchCell->Values[0] = blockCell.ValueOrOffset;
					
					pBranchCell->Values[1] = keyValue;
					pBranchCell->Offsets[1] = lastContentOffset;

					goto FILL_KEY;
				}
			}
			else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell 1
			{
				BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
				BranchCell& branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];
				
				//first branch
				//try find value in the list
				for(uint32 i=0; i<blockCellType; i++)
				{
					if(branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				if(blockCellType < MAX_BRANCH_TYPE1)
				{
					branchCell1.Values[blockCellType] = keyValue;
					branchCell1.Offsets[blockCellType] = lastContentOffset;

					blockCellType++;

					goto FILL_KEY;
				}

				//create second branch
				blockCellType = MIN_BRANCH_TYPE2;
			
				//get free branch cell
				BranchCell* pBranchCell;
				if(countFreeBranchCell)
				{
					uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
					pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

					blockCell.ValueOrOffset = branchOffset;
				}
				else
				{
					BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
					if(!pBranchPage)
					{
						pBranchPage = new BranchPage();
						pBranchPages[BranchPagesCount++] = pBranchPage;

						if(BranchPagesCount == BranchPagesSize)
						{
							reallocateBranchPages();
						}
					}

					pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

					blockCell.ValueOrOffset = lastBranchOffset++;
				}
					
				pBranchCell->Values[0] = keyValue;
				pBranchCell->Offsets[0] = lastContentOffset;
				
				goto FILL_KEY;
			}
			else if(blockCellType <= MAX_BRANCH_TYPE2) //branch cell 2
			{
				BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
				BranchCell& branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];
				
				//first branch
				//try find value in the list
				for(uint32 i=0; i < BRANCH_ENGINE_SIZE; i++)
				{
					if(branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				//second branch
				BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
				BranchCell& branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

				//try find value in the list
				uint32 countValues = blockCellType - MAX_BRANCH_TYPE1;

				for(uchar8 i=0; i<countValues; i++)
				{
					if(branchCell2.Values[i] == keyValue)
					{
						contentOffset = branchCell2.Offsets[i];
						keyOffset++;
						
						goto NEXT_KEY_PART;
					}
				}

				//create value in branch
				if(blockCellType < MAX_BRANCH_TYPE2) //add to branch value
				{
					branchCell2.Values[countValues] = keyValue;
					branchCell2.Offsets[countValues] = lastContentOffset;

					blockCellType++;

					goto FILL_KEY;
				}
				else
				{
					//CREATE NEXT BLOCK ==========================================================
					#ifndef _RELEASE
					tempValues[9]++;

					#endif

					const ushort16 branchesSize = BRANCH_ENGINE_SIZE * 2;
					const ushort16 countCell = branchesSize + 1;

					BlockCell blockCells[countCell];
					uchar8 indexes[countCell];

					if(countFreeBranchCell < MAX_SHORT)
					{
						freeBranchCells[countFreeBranchCell++] = blockCell.Offset;
					}

					if(countFreeBranchCell < MAX_SHORT)
					{
						freeBranchCells[countFreeBranchCell++] = blockCell.ValueOrOffset;
					}

EXTRACT_BRANCH2:
					idxKeyValue += BLOCK_ENGINE_STEP;
					if(idxKeyValue > BLOCK_ENGINE_SHIFT)
						idxKeyValue = 0;
					
					//idxKeyValue &= BLOCK_ENGINE_MASK;

					uchar8 j=0;

					//first branch
					for(uint32 i=0; i < BRANCH_ENGINE_SIZE; i++, j++)
					{
						BlockCell& currBlockCell = blockCells[j];
						currBlockCell.Type = CURRENT_VALUE_TYPE;
						currBlockCell.Offset = branchCell1.Offsets[i];

						uint32& value = branchCell1.Values[i];
						currBlockCell.ValueOrOffset = value;

						indexes[j] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
					}

					//second branch
					for(uint32 i=0; i < BRANCH_ENGINE_SIZE; i++, j++)
					{
						BlockCell& currBlockCell = blockCells[j];
						currBlockCell.Type = CURRENT_VALUE_TYPE;
						currBlockCell.Offset = branchCell2.Offsets[i];

						uint32& value = branchCell2.Values[i];
						currBlockCell.ValueOrOffset = value;

						indexes[j] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
					}

					//add current value
					BlockCell& currBlockCell8 = blockCells[branchesSize];
					currBlockCell8.Type = CURRENT_VALUE_TYPE;
					currBlockCell8.Offset = lastContentOffset;
					currBlockCell8.ValueOrOffset = keyValue;

					indexes[branchesSize] = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			
					//clear map
					uchar8 mapIndexes[BLOCK_ENGINE_SIZE];
					for(uchar8 i=0; i < countCell; i++)
					{
						mapIndexes[indexes[i]] = 0;
					}

					//fill map
					for(uint32 i=0; i < countCell; i++)
					{
						uchar8& countIndexes = mapIndexes[indexes[i]];
						countIndexes++;

						if(countIndexes > branchesSize)
						{
							goto EXTRACT_BRANCH2; //use another byte
						}
					}

					//allocate page
					uint32 maxLastBlockOffset = lastBlockOffset + BLOCK_ENGINE_SIZE*2;
					if(!pBlockPages[maxLastBlockOffset>>16])
					{
						pBlockPages[BlockPagesCount++] = new BlockPage();

						if(BlockPagesCount == BlockPagesSize)
						{
							reallocateBlockPages();
						}
					}

					//fill block
					blockCellType = MIN_BLOCK_TYPE + (idxKeyValue / BLOCK_ENGINE_STEP);
					blockCell.Offset = lastBlockOffset;

					for(uint32 i = 0; i < countCell; i++)
					{
						uchar8 idx = indexes[i];
						uint32 offset = lastBlockOffset + idx;

						BlockPage* pBlockPage = pBlockPages[offset >> 16];
						BlockCell& currBlockCell = pBlockPage->pBlock[offset & 0xFFFF];

						uchar8 count = mapIndexes[idx];
						if(count == 1) //one value in block cell
						{
							currBlockCell = blockCells[i];
						}
						else
						{
							if(currBlockCell.Type == 0) //create branch
							{
								//get free branch cell
								BranchCell* pCurrBranchCell;
								if(countFreeBranchCell)
								{
									uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
									pCurrBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

									currBlockCell.Offset = branchOffset;
								}
								else
								{
									BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
									if(!pBranchPage)
									{
										pBranchPage = new BranchPage();
										pBranchPages[BranchPagesCount++] = pBranchPage;

										if(BranchPagesCount == BranchPagesSize)
										{
											reallocateBranchPages();
										}
									}

									pCurrBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

									currBlockCell.Offset = lastBranchOffset++;
								}
																	
								currBlockCell.Type = MIN_BRANCH_TYPE1;
								
								pCurrBranchCell->Values[0] = blockCells[i].ValueOrOffset;
								pCurrBranchCell->Offsets[0] = blockCells[i].Offset;
							}
							else if(currBlockCell.Type < MAX_BRANCH_TYPE1)
							{
								BranchPage* pBranchPage = pBranchPages[currBlockCell.Offset >> 16];
								BranchCell& currBranchCell = pBranchPage->pBranch[currBlockCell.Offset & 0xFFFF];

								currBranchCell.Values[currBlockCell.Type] = blockCells[i].ValueOrOffset;
								currBranchCell.Offsets[currBlockCell.Type] = blockCells[i].Offset;

								currBlockCell.Type++;
							}
							else if(currBlockCell.Type == MAX_BRANCH_TYPE1)
							{
								//get free branch cell
								BranchCell* pCurrBranchCell;
								if(countFreeBranchCell)
								{
									uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
									pCurrBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

									currBlockCell.ValueOrOffset = branchOffset;
								}
								else
								{
									BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
									if(!pBranchPage)
									{
										pBranchPage = new BranchPage();
										pBranchPages[BranchPagesCount++] = pBranchPage;

										if(BranchPagesCount == BranchPagesSize)
										{
											reallocateBranchPages();
										}
									}

									pCurrBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

									currBlockCell.ValueOrOffset = lastBranchOffset++;
								}

								currBlockCell.Type = MIN_BRANCH_TYPE2;
								
								pCurrBranchCell->Values[0] = blockCells[i].ValueOrOffset;
								pCurrBranchCell->Offsets[0] = blockCells[i].Offset;
							}
							else
							{
								uint32 countValues = currBlockCell.Type - MAX_BRANCH_TYPE1;

								BranchPage* pBranchPage = pBranchPages[currBlockCell.ValueOrOffset >> 16];
								BranchCell& currBranchCell = pBranchPage->pBranch[currBlockCell.ValueOrOffset & 0xFFFF];

								currBranchCell.Values[countValues] = blockCells[i].ValueOrOffset;
								currBranchCell.Offsets[countValues] = blockCells[i].Offset;
								currBlockCell.Type++;
							}
						}
					}

					lastBlockOffset += BLOCK_ENGINE_SIZE;

					#ifndef _RELEASE
					tempValues[4]++;
					#endif

					goto FILL_KEY;
				}
			}
			else if(blockCell.Type <= MAX_BLOCK_TYPE)
			{
				//go to block
				idxKeyValue = (blockCell.Type - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;
				startOffset = blockCell.Offset;

				//level++;

				goto NEXT_BLOCK;
			}
		}
	}
	else if(contentCellType == CURRENT_VALUE_TYPE) //PART OF KEY =========================================================================
	{
		#ifndef _RELEASE
		tempValues[12]++;
		#endif

		if(contentCellValueOrOffset == keyValue)
		{
			contentOffset++;
			keyOffset++;

			goto NEXT_KEY_PART;
		}
		else //create branch
		{
			pContentCell->Type = MIN_BRANCH_TYPE1 + 1;
			
			//get free branch cell
			BranchCell* pBranchCell;
			if(countFreeBranchCell)
			{
				uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
				pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

				pBranchCell->Values[0] = contentCellValueOrOffset;
				pContentCell->Value = branchOffset;
			}
			else
			{
				BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
				if(!pBranchPage)
				{
					pBranchPage = new BranchPage();
					pBranchPages[BranchPagesCount++] = pBranchPage;

					if(BranchPagesCount == BranchPagesSize)
					{
						reallocateBranchPages();
					}
				}

				pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

				pBranchCell->Values[0] = contentCellValueOrOffset;
				pContentCell->Value = lastBranchOffset++;
			}

			pBranchCell->Offsets[0] = contentOffset + 1;

			pBranchCell->Values[1] = keyValue;
			pBranchCell->Offsets[1] = lastContentOffset;

			goto FILL_KEY;
		}
	}
	
FILL_KEY:

	keyOffset++;

FILL_KEY2:

	//fill key
	pContentPage = pContentPages[lastContentOffset>>16];
	contentIndex = lastContentOffset&0xFFFF;
	
	pContentPage->pContent[contentIndex].Type = ONLY_CONTENT_TYPE + keyLen - keyOffset;

	if(contentIndex < maxSafeShort) //content in one page
	{
		for(; keyOffset < keyLen; contentIndex++, keyOffset++, lastContentOffset++)
		{
			pContentPage->pContent[contentIndex].Value = key[keyOffset];
		}

		pContentPage->pContent[contentIndex].Type = VALUE_TYPE;
		pContentPage->pContent[contentIndex].Value = value;
		
		lastContentOffset++;

		return 0;
	}
	else //content in two pages
	{
		for(; keyOffset < keyLen; lastContentOffset++, keyOffset++)
		{
			pContentPage = pContentPages[lastContentOffset>>16];
			contentIndex = lastContentOffset&0xFFFF;

			pContentPage->pContent[contentIndex].Value = key[keyOffset];
		}

		ContentCell& contentCell = pContentPages[lastContentOffset>>16]->pContent[lastContentOffset&0xFFFF];
		contentCell.Type = VALUE_TYPE;
		contentCell.Value = value;

		lastContentOffset++;

		return 0;
	}

	return 0;
}