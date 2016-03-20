#include "StdAfx.h"
#include "HArrayFix.h"

void HArrayFix::insertVar(uint* key, uint keyLen, uint* value, uint valLen)
{
	//create new page =======================================================================================
	if(!pContentPages[(lastContentOffset + RowLen)>>16])
	{
		pContentPages[CountContentPages++] = new ContentPage();

		if(CountContentPages == SizeContentPages)
		{
			reallocateContentPages();
		}
	}

	//insert value ==========================================================================================
	uint keyOffset = 0;
	uint contentOffset = 0;

	uint headerOffset = key[0]>>HeaderBits;
	contentOffset = pHeader[headerOffset];

	if(!contentOffset)
	{
		pHeader[headerOffset] = lastContentOffset;
		
		ContentPage* pContentPage = pContentPages[lastContentOffset>>16];
		uint contentIndex = lastContentOffset&0xFFFF;

		pContentPage->pContentType[contentIndex] = (ONLY_CONTENT_TYPE + KeyLen);

		if(contentIndex < MAX_SAFE_SHORT) //content in one page
		{
			//fill key
			for(; keyOffset < KeyLen; contentIndex++, keyOffset++, lastContentOffset++)
			{
				pContentPage->pContent[contentIndex] = key[keyOffset];
			}

			for(uint i=0; i<ValueLen; contentIndex++, i++, lastContentOffset++)
			{
				pContentPage->pContent[contentIndex] = value[i];
			}
		}
		else
		{
			for(; keyOffset < KeyLen; lastContentOffset++, keyOffset++)
			{
				pContentPage = pContentPages[lastContentOffset>>16];
				pContentPage->pContent[lastContentOffset&0xFFFF] = key[keyOffset];
			}

			for(uint i=0; i < ValueLen; lastContentOffset++, i++)
			{
				pContentPage = pContentPages[lastContentOffset>>16];
				pContentPage->pContent[lastContentOffset&0xFFFF] = value[i];
			}
		}

		#ifndef _RELEASE
		tempValues[10]++;
		#endif

		return;
	}
	
	#ifndef _RELEASE
	tempValues[11]++;
	#endif

	//TWO KEYS =============================================================================================
NEXT_KEY_PART:

	ContentPage* pContentPage = pContentPages[contentOffset>>16];
	uint contentIndex = contentOffset & 0xFFFF;

	uchar& contentCellType = pContentPage->pContentType[contentIndex];

	if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================
	{
		#ifndef _RELEASE
		tempValues[13]++;
		#endif

		if(contentIndex < MAX_SAFE_SHORT) //content in one page
		{
			for(; keyOffset < KeyLen; contentOffset++, contentIndex++, keyOffset++)
			{
				if(pContentPage->pContent[contentIndex] != key[keyOffset])
				{
					//create branch
					pContentPage->pContentType[contentIndex] = MIN_BRANCH_TYPE1 + 1;

					//get free branch cell
					BranchCell* pBranchCell;
					if(countFreeBranchCell)
					{
						uint branchOffset = freeBranchCells[--countFreeBranchCell];
						pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

						pBranchCell->Values[0] = pContentPage->pContent[contentIndex];
						pContentPage->pContent[contentIndex] = branchOffset;
					}
					else
					{
						BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
						if(!pBranchPage)
						{
							pBranchPage = new BranchPage();
							pBranchPages[CountBranchPages++] = pBranchPage;

							if(CountBranchPages == SizeBranchPages)
							{
								reallocateBranchPages();
							}
						}

						pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

						pBranchCell->Values[0] = pContentPage->pContent[contentIndex];
						pContentPage->pContent[contentIndex] = lastBranchOffset++;
					}
			
					pBranchCell->Offsets[0] = contentOffset + 1;

					pBranchCell->Values[1] = key[keyOffset];
					pBranchCell->Offsets[1] = lastContentOffset;

					pContentPage->pContentType[contentIndex + 1] = (ONLY_CONTENT_TYPE + KeyLen - keyOffset - 1);

					goto FILL_KEY;
				}
				else
				{
					pContentPage->pContentType[contentIndex] = CURRENT_VALUE_TYPE; //reset to current value	
				}
			}

			pContentPage->pContentType[contentIndex] = VALUE_TYPE;

			for(uint i=0; i<ValueLen; contentIndex++, i++)
			{
				pContentPage->pContent[contentIndex] = value[i]; //key is exists, update
			}
		}
		else  //content in two pages
		{
			for(; keyOffset < KeyLen; contentOffset++, keyOffset++)
			{
				pContentPage = pContentPages[contentOffset>>16];
				contentIndex = contentOffset & 0xFFFF;

				if(pContentPage->pContent[contentIndex] != key[keyOffset])
				{
					//create branch
					pContentPage->pContentType[contentIndex] = MIN_BRANCH_TYPE1 + 1;
								
					//get free branch cell
					BranchCell* pBranchCell;
					if(countFreeBranchCell)
					{
						uint branchOffset = freeBranchCells[--countFreeBranchCell];
						pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

						pBranchCell->Values[0] = pContentPage->pContent[contentIndex];
						pContentPage->pContent[contentIndex] = branchOffset;
					}
					else
					{
						BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
						if(!pBranchPage)
						{
							pBranchPage = new BranchPage();
							pBranchPages[CountBranchPages++] = pBranchPage;

							if(CountBranchPages == SizeBranchPages)
							{
								reallocateBranchPages();
							}
						}

						pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

						pBranchCell->Values[0] = pContentPage->pContent[contentIndex];
						pContentPage->pContent[contentIndex] = lastBranchOffset++;
					}
									
					pBranchCell->Offsets[0] = contentOffset + 1;

					pBranchCell->Values[1] = key[keyOffset];
					pBranchCell->Offsets[1] = lastContentOffset;

					//set rest of key
					contentOffset++;

					pContentPage = pContentPages[contentOffset>>16];
					contentIndex = contentOffset & 0xFFFF;
					
					pContentPage->pContentType[contentIndex] = ONLY_CONTENT_TYPE + KeyLen - keyOffset - 1;
					
					goto FILL_KEY;
				}
				else
				{
					pContentPage->pContentType[contentIndex] = CURRENT_VALUE_TYPE; //reset to current value	
				}
			}

			pContentPage = pContentPages[contentOffset>>16];
			contentIndex = contentOffset & 0xFFFF;

			pContentPage->pContentType[contentIndex] = VALUE_TYPE;

			for(uint i=0; i<ValueLen; contentOffset++, i++)
			{
				pContentPage = pContentPages[contentOffset>>16];
				pContentPage->pContent[contentOffset&0xFFFF] = value[i]; //key is exists, update
			}
		}

		return; 
	}

	uint& keyValue = key[keyOffset];
	uint& contentCellValueOrOffset = pContentPage->pContent[contentIndex];
	
	if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
	{
		#ifndef _RELEASE
		tempValues[14]++;
		#endif

		BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
		BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

		//try find value in the list
		for(uint i=0; i<contentCellType; i++)
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

			contentCellType++;

			goto FILL_KEY;
		}
		else
		{
			//EXTRACT BRANCH AND CREATE BLOCK =========================================================
			uchar idxKeyValue = 0;
			uchar currContentCellType = MIN_BLOCK_TYPE; 

			if(countFreeBranchCell < MAX_SHORT)
			{
				freeBranchCells[countFreeBranchCell++] = contentCellValueOrOffset;
			}

EXTRACT_BRANCH:
			const ushort countCell = BRANCH_ENGINE_SIZE + 1;

			BlockCell blockCells[countCell];
			uchar indexes[countCell];

			for(uint i=0; i < MAX_BRANCH_TYPE1; i++)
			{
				BlockCell& currBlockCell = blockCells[i];
				currBlockCell.Type = CURRENT_VALUE_TYPE;
				currBlockCell.Offset = branchCell.Offsets[i];

				uint& value = branchCell.Values[i];
				currBlockCell.CurrentValueOrParentCodeAndIndex = value;

				indexes[i] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT); //((uchar*)&value)[idxKeyValue];
			}

			//add current value
			BlockCell& currBlockCell4 = blockCells[BRANCH_ENGINE_SIZE];
			currBlockCell4.Type = CURRENT_VALUE_TYPE;
			currBlockCell4.Offset = lastContentOffset;
			currBlockCell4.CurrentValueOrParentCodeAndIndex = keyValue;

			indexes[BRANCH_ENGINE_SIZE] = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			
			//clear map
			uchar mapIndexes[BLOCK_ENGINE_SIZE];
			for(uint i=0; i <= MAX_BRANCH_TYPE1; i++)
			{
				mapIndexes[indexes[i]] = 0;
			}

			//fill map
			for(uint i=0; i <= MAX_BRANCH_TYPE1; i++)
			{
				uchar& countIndexes = mapIndexes[indexes[i]];
				countIndexes++;

				if(countIndexes > BRANCH_ENGINE_SIZE)
				{
					idxKeyValue += BLOCK_ENGINE_STEP;
					currContentCellType++;
					goto EXTRACT_BRANCH; //use another byte
				}
			}

			//allocate page
			uint maxLastBlockOffset = lastBlockOffset + BLOCK_ENGINE_SIZE*2;
			if(!pBlockPages[maxLastBlockOffset>>16])
			{
				pBlockPages[CountBlockPages++] = new BlockPage();

				if(CountBlockPages == SizeBlockPages)
				{
					reallocateBlockPages();
				}
			}

			//fill block
			contentCellType = currContentCellType;
			contentCellValueOrOffset = lastBlockOffset;

			for(uint i = 0; i < countCell; i++)
			{
				uchar idx = indexes[i];
				uint offset = lastBlockOffset + idx;

				BlockPage* pBlockPage = pBlockPages[offset >> 16];
				BlockCell& currBlockCell = pBlockPage->pBlock[offset & 0xFFFF];
				
				uchar count = mapIndexes[idx];
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
							uint branchOffset = freeBranchCells[--countFreeBranchCell];
							pCurrBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];
							currBlockCell.Offset = branchOffset;
						}
						else
						{
							BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
							if(!pBranchPage)
							{
								pBranchPage = new BranchPage();
								pBranchPages[CountBranchPages++] = pBranchPage;

								if(CountBranchPages == SizeBranchPages)
								{
									reallocateBranchPages();
								}
							}

							pCurrBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];
							currBlockCell.Offset = lastBranchOffset++;
						}

						currBlockCell.Type = MIN_BRANCH_TYPE1;
						
						pCurrBranchCell->Values[0] = blockCells[i].CurrentValueOrParentCodeAndIndex;
						pCurrBranchCell->Offsets[0] = blockCells[i].Offset;
					}
					else //types 1..4
					{
						BranchPage* pBranchPage = pBranchPages[currBlockCell.Offset >> 16];
						BranchCell& currBranchCell = pBranchPage->pBranch[currBlockCell.Offset & 0xFFFF];

						currBranchCell.Values[currBlockCell.Type] = blockCells[i].CurrentValueOrParentCodeAndIndex;
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

		//uint level = 1;

		uchar idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

		uint* pStartOffset = &contentCellValueOrOffset;

NEXT_BLOCK:
		uint subOffset = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
		uint offset = *pStartOffset + subOffset;
		
		BlockPage* pBlockPage = pBlockPages[offset >> 16];
		if(!pBlockPage)
		{
			pBlockPage = new BlockPage();
			pBlockPages[CountBlockPages++] = pBlockPage;

			if(CountBlockPages == SizeBlockPages)
			{
				reallocateBlockPages();
			}
		}

		BlockCell& blockCell = pBlockPage->pBlock[offset & 0xFFFF];

		uchar& blockCellType = blockCell.Type;

		if(blockCellType == EMPTY_TYPE) //free cell, fill
		{
			blockCellType = CURRENT_VALUE_TYPE;
			blockCell.CurrentValueOrParentCodeAndIndex = keyValue;
			blockCell.Offset = lastContentOffset;
			
			goto FILL_KEY;
		}
		else //block cell 
		{
			if(blockCellType == CURRENT_VALUE_TYPE) //current value
			{
				if(blockCell.CurrentValueOrParentCodeAndIndex == keyValue) //value is exists
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
						uint branchOffset = freeBranchCells[--countFreeBranchCell];
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
							pBranchPages[CountBranchPages++] = pBranchPage;

							if(CountBranchPages == SizeBranchPages)
							{
								reallocateBranchPages();
							}
						}

						pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

						pBranchCell->Offsets[0] = blockCell.Offset;
						blockCell.Offset = lastBranchOffset++;
					}
					
					pBranchCell->Values[0] = blockCell.CurrentValueOrParentCodeAndIndex;
					
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
				for(uint i=0; i<blockCellType; i++)
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
					uint branchOffset = freeBranchCells[--countFreeBranchCell];
					pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

					blockCell.CurrentValueOrParentCodeAndIndex = branchOffset;
				}
				else
				{
					BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
					if(!pBranchPage)
					{
						pBranchPage = new BranchPage();
						pBranchPages[CountBranchPages++] = pBranchPage;

						if(CountBranchPages == SizeBranchPages)
						{
							reallocateBranchPages();
						}
					}

					pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

					blockCell.CurrentValueOrParentCodeAndIndex = lastBranchOffset++;
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
				for(uint i=0; i < BRANCH_ENGINE_SIZE; i++)
				{
					if(branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				//second branch
				BranchPage* pBranchPage2 = pBranchPages[blockCell.CurrentValueOrParentCodeAndIndex >> 16];
				BranchCell& branchCell2 = pBranchPage2->pBranch[blockCell.CurrentValueOrParentCodeAndIndex & 0xFFFF];

				//try find value in the list
				uint countValues = blockCellType - MAX_BRANCH_TYPE1;

				for(uchar i=0; i<countValues; i++)
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

					/*if(level >= 3)
					{
						tempValues[21]++;
					}*/
					#endif

					const ushort branchesSize = BRANCH_ENGINE_SIZE * 2;
					const ushort countCell = branchesSize + 1;

					BlockCell blockCells[countCell];
					uchar indexes[countCell];

					if(countFreeBranchCell < MAX_SHORT)
					{
						freeBranchCells[countFreeBranchCell++] = blockCell.Offset;
					}

					if(countFreeBranchCell < MAX_SHORT)
					{
						freeBranchCells[countFreeBranchCell++] = blockCell.CurrentValueOrParentCodeAndIndex;
					}

EXTRACT_BRANCH2:
					idxKeyValue += BLOCK_ENGINE_STEP;
					if(idxKeyValue > BLOCK_ENGINE_SHIFT)
						idxKeyValue = 0;
					
					//idxKeyValue &= BLOCK_ENGINE_MASK;

					uchar j=0;

					//first branch
					for(uint i=0; i < BRANCH_ENGINE_SIZE; i++, j++)
					{
						BlockCell& currBlockCell = blockCells[j];
						currBlockCell.Type = CURRENT_VALUE_TYPE;
						currBlockCell.Offset = branchCell1.Offsets[i];

						uint& value = branchCell1.Values[i];
						currBlockCell.CurrentValueOrParentCodeAndIndex = value;

						indexes[j] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
					}

					//second branch
					for(uint i=0; i < BRANCH_ENGINE_SIZE; i++, j++)
					{
						BlockCell& currBlockCell = blockCells[j];
						currBlockCell.Type = CURRENT_VALUE_TYPE;
						currBlockCell.Offset = branchCell2.Offsets[i];

						uint& value = branchCell2.Values[i];
						currBlockCell.CurrentValueOrParentCodeAndIndex = value;

						indexes[j] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
					}

					//add current value
					BlockCell& currBlockCell8 = blockCells[branchesSize];
					currBlockCell8.Type = CURRENT_VALUE_TYPE;
					currBlockCell8.Offset = lastContentOffset;
					currBlockCell8.CurrentValueOrParentCodeAndIndex = keyValue;

					indexes[branchesSize] = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			
					//clear map
					uchar mapIndexes[BLOCK_ENGINE_SIZE];
					for(uchar i=0; i < countCell; i++)
					{
						mapIndexes[indexes[i]] = 0;
					}

					//fill map
					for(uint i=0; i < countCell; i++)
					{
						uchar& countIndexes = mapIndexes[indexes[i]];
						countIndexes++;

						if(countIndexes > branchesSize)
						{
							goto EXTRACT_BRANCH2; //use another byte
						}
					}

					//allocate page
					uint maxLastBlockOffset = lastBlockOffset + BLOCK_ENGINE_SIZE*2;
					if(!pBlockPages[maxLastBlockOffset>>16])
					{
						pBlockPages[CountBlockPages++] = new BlockPage();

						if(CountBlockPages == SizeBlockPages)
						{
							reallocateBlockPages();
						}
					}

					//fill block
					blockCellType = MIN_BLOCK_TYPE;
					
					blockCell.CurrentValueOrParentCodeAndIndex = (idxKeyValue/BLOCK_ENGINE_STEP);
					blockCell.Offset = lastBlockOffset;

					for(uint i = 0; i < countCell; i++)
					{
						uchar idx = indexes[i];
						uint offset = lastBlockOffset + idx;

						BlockPage* pBlockPage = pBlockPages[offset >> 16];
						BlockCell& currBlockCell = pBlockPage->pBlock[offset & 0xFFFF];

						uchar count = mapIndexes[idx];
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
									uint branchOffset = freeBranchCells[--countFreeBranchCell];
									pCurrBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

									currBlockCell.Offset = branchOffset;
								}
								else
								{
									BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
									if(!pBranchPage)
									{
										pBranchPage = new BranchPage();
										pBranchPages[CountBranchPages++] = pBranchPage;

										if(CountBranchPages == SizeBranchPages)
										{
											reallocateBranchPages();
										}
									}

									pCurrBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

									currBlockCell.Offset = lastBranchOffset++;
								}
																	
								currBlockCell.Type = MIN_BRANCH_TYPE1;
								
								pCurrBranchCell->Values[0] = blockCells[i].CurrentValueOrParentCodeAndIndex;
								pCurrBranchCell->Offsets[0] = blockCells[i].Offset;
							}
							else if(currBlockCell.Type < MAX_BRANCH_TYPE1)
							{
								BranchPage* pBranchPage = pBranchPages[currBlockCell.Offset >> 16];
								BranchCell& currBranchCell = pBranchPage->pBranch[currBlockCell.Offset & 0xFFFF];

								currBranchCell.Values[currBlockCell.Type] = blockCells[i].CurrentValueOrParentCodeAndIndex;
								currBranchCell.Offsets[currBlockCell.Type] = blockCells[i].Offset;

								currBlockCell.Type++;
							}
							else if(currBlockCell.Type == MAX_BRANCH_TYPE1)
							{
								//get free branch cell
								BranchCell* pCurrBranchCell;
								if(countFreeBranchCell)
								{
									uint branchOffset = freeBranchCells[--countFreeBranchCell];
									pCurrBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

									currBlockCell.CurrentValueOrParentCodeAndIndex = branchOffset;
								}
								else
								{
									BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
									if(!pBranchPage)
									{
										pBranchPage = new BranchPage();
										pBranchPages[CountBranchPages++] = pBranchPage;

										if(CountBranchPages == SizeBranchPages)
										{
											reallocateBranchPages();
										}
									}

									pCurrBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

									currBlockCell.CurrentValueOrParentCodeAndIndex = lastBranchOffset++;
								}

								currBlockCell.Type = MIN_BRANCH_TYPE2;
								
								pCurrBranchCell->Values[0] = blockCells[i].CurrentValueOrParentCodeAndIndex;
								pCurrBranchCell->Offsets[0] = blockCells[i].Offset;
							}
							else
							{
								uint countValues = currBlockCell.Type - MAX_BRANCH_TYPE1;

								BranchPage* pBranchPage = pBranchPages[currBlockCell.CurrentValueOrParentCodeAndIndex >> 16];
								BranchCell& currBranchCell = pBranchPage->pBranch[currBlockCell.CurrentValueOrParentCodeAndIndex & 0xFFFF];

								currBranchCell.Values[countValues] = blockCells[i].CurrentValueOrParentCodeAndIndex;
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
				idxKeyValue = blockCell.CurrentValueOrParentCodeAndIndex * BLOCK_ENGINE_STEP;
				pStartOffset = &blockCell.Offset;

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
			contentCellType = MIN_BRANCH_TYPE1 + 1;
			
			//get free branch cell
			BranchCell* pBranchCell;
			if(countFreeBranchCell)
			{
				uint branchOffset = freeBranchCells[--countFreeBranchCell];
				pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

				pBranchCell->Values[0] = contentCellValueOrOffset;
				contentCellValueOrOffset = branchOffset;
			}
			else
			{
				BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
				if(!pBranchPage)
				{
					pBranchPage = new BranchPage();
					pBranchPages[CountBranchPages++] = pBranchPage;

					if(CountBranchPages == SizeBranchPages)
					{
						reallocateBranchPages();
					}
				}

				pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

				pBranchCell->Values[0] = contentCellValueOrOffset;
				contentCellValueOrOffset = lastBranchOffset++;
			}

			pBranchCell->Offsets[0] = contentOffset + 1;

			pBranchCell->Values[1] = keyValue;
			pBranchCell->Offsets[1] = lastContentOffset;

			goto FILL_KEY;
		}
	}
	else if(contentCellType == VALUE_TYPE) //update existing value
	{
		if(contentIndex < MAX_SAFE_SHORT) //content in one page
		{
			//fill key
			for(uint i=0; i<ValueLen; contentIndex++, i++, lastContentOffset++)
			{
				pContentPage->pContent[contentIndex] = value[i];
			}
		}
		else
		{
			for(uint i=0; i < ValueLen; contentOffset++, i++)
			{
				pContentPage = pContentPages[contentOffset>>16];
				pContentPage->pContent[contentOffset & 0xFFFF] = value[i];
			}
		}

		return;
	}

FILL_KEY:

	//fill key
	pContentPage = pContentPages[lastContentOffset>>16];
	contentIndex = lastContentOffset&0xFFFF;

	keyOffset++;

	pContentPage->pContentType[contentIndex] = ONLY_CONTENT_TYPE + KeyLen - keyOffset;

	if(contentIndex < MAX_SAFE_SHORT) //content in one page
	{
		for(; keyOffset < KeyLen; contentIndex++, keyOffset++, lastContentOffset++)
		{
			pContentPage->pContent[contentIndex] = key[keyOffset];
		}

		for(uint i=0; i<ValueLen; contentIndex++, i++, lastContentOffset++)
		{
			pContentPage->pContent[contentIndex] = value[i];
		}
	}
	else //content in two pages
	{
		for(; keyOffset < KeyLen; lastContentOffset++, keyOffset++)
		{
			pContentPage = pContentPages[lastContentOffset>>16];
			contentIndex = lastContentOffset&0xFFFF;

			pContentPage->pContent[contentIndex] = key[keyOffset];
		}

		pContentPage = pContentPages[lastContentOffset>>16];
		contentIndex = lastContentOffset&0xFFFF;

		for(uint i=0; i<ValueLen; lastContentOffset++, i++)
		{
			pContentPage = pContentPages[lastContentOffset>>16];
			pContentPage->pContent[lastContentOffset&0xFFFF] = value[i];
		}
	}
	
	return;
}