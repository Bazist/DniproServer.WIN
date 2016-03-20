#include "StdAfx.h"
#include "HArrayFix.h"

uint* HArrayFix::getValueByVarKey(uint* key, uint keyLen)
{
	uint contentOffset = pHeader[key[0]>>HeaderBits];
	
	if(contentOffset)
	{
		uint keyOffset = 0;
	
NEXT_KEY_PART:
		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		ushort contentIndex = contentOffset&0xFFFF;

		uchar& contentCellType = pContentPage->pContentType[contentIndex]; //move to type part
	
		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			if(contentIndex < MAX_SAFE_SHORT) //content in one page
			{
				for(; keyOffset < KeyLen; contentIndex++, keyOffset++)
				{
					if(pContentPage->pContent[contentIndex] != key[keyOffset])
						return 0;
				}

				return &pContentPage->pContent[contentIndex]; //return value
			}
			else //content in two pages
			{
				for(; keyOffset < KeyLen; contentOffset++, keyOffset++)
				{
					if(pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF] != key[keyOffset])
						return 0;
				}

				return &pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF]; //return value
			}
		}

		uint& keyValue = key[keyOffset];
		uint& contentCellValueOrOffset = pContentPage->pContent[contentIndex];
	
		if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//try find value in the list
			uint* values = branchCell.Values;

			for(uint i=0; i<contentCellType; i++)
			{
				if(values[i] == keyValue)
				{
					contentOffset = branchCell.Offsets[i];
					keyOffset++;

					goto NEXT_KEY_PART;
				}
			}

			return 0;
		}
		else if(contentCellType == VALUE_TYPE)
		{
			return &contentCellValueOrOffset;
		}
		else if(contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			uchar idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

			uint blockOffset = contentCellValueOrOffset;

NEXT_BLOCK:
			uint offset = blockOffset + ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
		
			BlockPage* pBlockPage = pBlockPages[offset >> 16];
			BlockCell& blockCell = pBlockPage->pBlock[offset & 0xFFFF];

			uchar& blockCellType = blockCell.Type;

			if(blockCellType == CURRENT_VALUE_TYPE) //current value
			{
				if(blockCell.CurrentValueOrParentCodeAndIndex == keyValue) //value is exists
				{
					contentOffset = blockCell.Offset;
					keyOffset++;

					goto NEXT_KEY_PART;
				}
				else
				{
					return 0;
				}
			}
			else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
			{
				BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
				BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

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

				return 0;
			}
			else if(blockCellType <= MAX_BRANCH_TYPE2) //branch cell
			{
				BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
				BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

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

				BranchPage* pBranchPage2 = pBranchPages[blockCell.CurrentValueOrParentCodeAndIndex >> 16];
				BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.CurrentValueOrParentCodeAndIndex & 0xFFFF];

				//try find value in the list
				uint countValues = blockCellType - MAX_BRANCH_TYPE1;

				for(uint i=0; i<countValues; i++)
				{
					if(branchCell2.Values[i] == keyValue)
					{
						contentOffset = branchCell2.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				return 0;
			}
			else if(blockCell.Type <= MAX_BLOCK_TYPE)
			{
				//go to block
				idxKeyValue = (blockCell.CurrentValueOrParentCodeAndIndex * BLOCK_ENGINE_BITS);
				blockOffset = blockCell.Offset;

				goto NEXT_BLOCK;
			}
			else
			{
				return 0;
			}
		}
		else if(contentCellType == CURRENT_VALUE_TYPE) //PART OF KEY =========================================================================
		{
			if(contentCellValueOrOffset == keyValue)
			{
				contentOffset++;
				keyOffset++;
			
				goto NEXT_KEY_PART;
			}
			else 
			{
				return 0;
			}
		}
	}

	return 0;
}