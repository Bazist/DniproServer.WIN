/*
# Copyright(C) 2010-2016 Viacheslav Makoveichuk (email: BigDoc@gmail.com, skype: vyacheslavm81)
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

#ifndef _BigDoc_QUERY		 // Allow use of features specific to Windows XP or later.                   
#define _BigDoc_QUERY 0x709 // Change this to the appropriate value to target other versions of Windows.

#endif	

#include "BigDoc.h"

class BigDocQuery
{
public:
	BigDocQuery(BigDoc* pDB)
	{
		this->pDB = pDB;
		CountValueList = 0;

		DefCollID = 0;

		TranID = 0;

		hasBeginTran = false;

		//memset(pValueList, 0, sizeof(uint32) * 16);
		//memset(pIndexes, 0, sizeof(uint32) * 16);
	}

	BigDoc* pDB;

	ValueList* pValueList[MAX_JOINED_DOCUMENTS];
	ValueList* pIndexes[MAX_JOINED_DOCUMENTS];
	uchar8 CollIDs[MAX_JOINED_DOCUMENTS];

	uint32 DefCollID;
	uint32 TranID;

	bool hasBeginTran;

	uint32 CountValueList;

	bool isIndexesEquals(uint32* indexes1, uint32* indexes2)
	{
		if (indexes1 && indexes2)
		{
			uint32 i = 0;
			while (indexes1[i] == indexes2[i])
			{
				if (!indexes1[i])
					return true;

				i++;
			}

			return false;
		}
		else
		{
			return (!indexes1 && !indexes2);
		}
	}

	void clearState()
	{
		CountValueList = 0;
	}

	uint32 getFirstID()
	{
		if (this->pValueList[0])
		{
			if (pValueList[0]->Count)
			{
				return pValueList[0]->pValues[0];
			}
		}

		return 0;
	}

	uint32 sum(char* query)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		uint32 index = 0;

		uint32 sum = 0;

		char jsonResult[128];

		for (uint32 i = 0; i < pValueList[index]->Count; i++)
		{
			if (pValueList[index]->pValues[i])
			{
				pDB->getPartDoc(query,
					jsonResult,
					i,
					TranID,
					CollIDs[index],
					true,
					pValueList);

				sum += atoi(jsonResult);
			}
		}

		if (hasBeginTran)
		{
			pDB->clearTran(TranID);

			TranID = 0;

			hasBeginTran = false;
		}

		return sum;
	}

	/*
	uint32 minValue(char* query)
	{
		uint32 minValue = 0xFFFFFFFF;

		for(uint32 i=0; i<pValueList->Count; i++)
		{
			if(pValueList->pValues[i])
			{
				uint32 value = pDB->getAttrValueByDocID(query, pValueList->pValues[i]);

				if(value < minValue)
				{
					minValue = value;
				}
			}
		}

		return minValue;
	}

	uint32 maxValue(char* query)
	{
		uint32 maxValue = 0;

		for(uint32 i=0; i<pValueList->Count; i++)
		{
			if(pValueList->pValues[i])
			{
				uint32 value = pDB->getAttrValueByDocID(query, pValueList->pValues[i]);

				if(value > maxValue)
				{
					maxValue = value;
				}
			}
		}

		return maxValue;
	}*/

	uint32 avg(char* query)
	{
		return sum(query) / count();
	}

	BigDocQuery* getWhere(char* query)
	{
		return getWhere(query, 0);
	}

	BigDocQuery* getWhere(char* query, uint32 docID)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		this->CollIDs[0] = DefCollID;
		this->pValueList[0] = pDB->getDocsByAttr(query,
			docID,
			TranID,
			DefCollID,
			&this->pIndexes[0]);

		this->CountValueList = 1;

		return this;
	}

	BigDocQuery* andWhere(char* query)
	{
		return andWhere(query, 0);
	}

	BigDocQuery* andWhere(char* query, uint32 docID)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		if (pValueList[0])
		{
			ValueList* pAndIndexes;
			ValueList* pAndValueList = pDB->getDocsByAttr(query,
				docID,
				TranID,
				CollIDs[0],
				&pAndIndexes);

			for (uint32 i = 0; i < pValueList[0]->Count; i++)
			{
				if (pValueList[0]->pValues[i])
				{
					bool bFind = false;

					for (uint32 j = 0; j < pAndValueList->Count; j++)
					{
						if (pAndValueList->pValues[j] == pValueList[0]->pValues[i] &&
							(!pIndexes[0] ||
								isIndexesEquals((uint32*)pAndIndexes->pValues[j], (uint32*)pIndexes[0]->pValues[i])))
						{
							bFind = true;
							break;
						}
					}

					if (!bFind)
					{
						for (uint32 j = 0; j < CountValueList; j++)
						{
							pValueList[j]->pValues[i] = 0;
							pIndexes[j]->pValues[i] = 0;
						}
					}
				}
			}
		}

		return this;
	}

	BigDocQuery* orWhere(char* query)
	{
		return orWhere(query, 0);
	}

	BigDocQuery* orWhere(char* query, uint32 docID)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		if (CountValueList && pValueList[0])
		{
			ValueList* pOrIndexes;
			ValueList* pOrValueList = pDB->getDocsByAttr(query,
				docID,
				TranID,
				CollIDs[0],
				&pOrIndexes);

			uint32 count = pValueList[0]->Count;

			for (uint32 i = 0; i < pOrValueList->Count; i++)
			{
				if (pOrValueList->pValues[i])
				{
					bool bFind = false;

					for (uint32 j = 0; j < count; j++)
					{
						if (pOrValueList->pValues[j] == pValueList[0]->pValues[i] &&
							(!pIndexes[0] ||
								isIndexesEquals((uint32*)pOrIndexes->pValues[j], (uint32*)pIndexes[0]->pValues[i])))
						{
							bFind = true;
							break;
						}
					}

					if (!bFind)
					{
						if (!pIndexes[0])
						{
							pIndexes[0] = pDB->_trans[TranID].valueListPool.newObject(); //new ValueList[pValueList[0]->Count];
							pIndexes[0]->Count = pValueList[0]->Count;
						}

						pValueList[0]->addValue(pOrValueList->pValues[i], false);
						pIndexes[0]->addValue(pOrIndexes->pValues[i], false);
					}
				}
			}
		}

		return this;
	}

	BigDocQuery* join(char* query1,
		char* query2,
		uchar8 rightCollID = 0)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		char outValue[256];
		char queryTemp[1024];

		uchar8 leftCollID = CollIDs[CountValueList - 1];
		ValueList* pLeftValueList = pValueList[CountValueList - 1];

		ValueList* pLeftIndexes = pIndexes[CountValueList - 1];

		ValueList* pRightValueList = pDB->_trans[TranID].valueListPool.newObject();

		pRightValueList->Count = pLeftValueList->Count;

		CollIDs[CountValueList] = rightCollID;
		pValueList[CountValueList] = pRightValueList;
		pIndexes[CountValueList] = 0;

		CountValueList++;
		
		for (uint32 i = 0; i < pLeftValueList->Count; i++)
		{
			if (pLeftValueList->pValues[i])
			{
				if (!pLeftIndexes)
				{
					//get attr value from left table
					pDB->getPartDoc(query1,
						outValue,
						pLeftValueList->pValues[i],
						TranID,
						leftCollID,
						true);
				}
				else
				{
					//get attr value from left table with index
					pDB->getPartDoc(query1,
						outValue,
						pLeftValueList->pValues[i],
						TranID,
						leftCollID,
						true,
						0,
						(uint32*)pLeftIndexes->pValues[i]);
				}

				//create query
				for (uint32 j = 0; ; j++)
				{
					if (query2[j] != '$')
					{
						queryTemp[j] = query2[j];
					}
					else
					{
						queryTemp[j++] = '\'';

						uint32 s = j;

						for (uint32 k = 0; outValue[k] != 0; k++, j++)
						{
							queryTemp[j] = outValue[k];
						}

						queryTemp[j++] = '\'';

						for (; query2[s] != 0; s++, j++)
						{
							queryTemp[j] = query2[s];
						}

						queryTemp[j] = 0;

						break;
					}
				}

				//join
				ValueList* pTempValueList = pDB->getDocsByAttr(queryTemp,
					0,
					TranID,
					rightCollID);

				if (pTempValueList->Count == 1) //one to one
				{
					pRightValueList->pValues[i] = pTempValueList->pValues[0];
				}
				else if (pTempValueList->Count > 1) //one to many
				{
					for (uint32 j = 0; j < pTempValueList->Count; j++)
					{
						pLeftValueList->addValue(0, false);
						pRightValueList->addValue(0, false);
					}

					for (uint32 j = pLeftValueList->Count - 1; j > i; j--)
					{
						pLeftValueList->pValues[j] = pLeftValueList->pValues[j - pTempValueList->Count];
						pRightValueList->pValues[j] = pTempValueList->pValues[j - pTempValueList->Count];
					}

					for (uint32 j = 0; j < pTempValueList->Count; j++)
					{
						pLeftValueList->pValues[i + j] = pLeftValueList->pValues[i];
						pRightValueList->pValues[i + j] = pTempValueList->pValues[j];
					}
				}
				else //one to nothing
				{
					pLeftValueList->pValues[i] = 0;
					pRightValueList->pValues[i] = 0;
				}
			}
		}

		return this;
	}

	BigDocQuery* getAll()
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		this->CollIDs[0] = DefCollID;
		this->pValueList[0] = pDB->_trans[TranID].valueListPool.newObject();

		this->CountValueList = 1;

		pIndexes[0] = 0;

		for (uint32 i = 1; i <= pDB->lastDocID; i++)
		{
			pValueList[0]->addValue(i, false);
		}

		return this;
	}

	uint32 select(char* query, char* jsonResult)
	{
		return select(query, jsonResult, false, false);
	}

	uint32 select(char* query,
		char* jsonResult,
		bool isDistinct,
		bool onlyValue)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		//format count|len|string|len|string|len|string
		uint32 index = 0;

		uint32 fullLen = 4;

		uint32* pCount = (uint32*)jsonResult;
		*pCount = 0;

		if (pValueList[index])
		{
			for (uint32 i = 0; i < pValueList[index]->Count; i++)
			{
				if (pValueList[index]->pValues[i])
				{
					ushort16* pLen = (ushort16*)(jsonResult + fullLen);

					fullLen += 2;

					uint32 len;

					if (!pIndexes[index])
					{
						len = pDB->getPartDoc(query,
							jsonResult + fullLen,
							i,
							TranID,
							DefCollID,
							onlyValue,
							pValueList,
							0,
							CollIDs);
					}
					else
					{
						len = pDB->getPartDoc(query,
							jsonResult + fullLen,
							i,
							TranID,
							DefCollID,
							onlyValue,
							pValueList,
							(uint32*)pIndexes[index]->pValues[i],
							CollIDs);
					}

					*pLen = len;

					fullLen += len;

					//count
					(*pCount)++;
				}
			}
		}

		if (hasBeginTran)
		{
			pDB->clearTran(TranID);

			TranID = 0;

			hasBeginTran = false;
		}

		return fullLen;
	}

	//all results in one str with ';' delimeter
	uint32 selectStr(char* query,
		char* jsonResult,
		bool isDistinct = false,
		bool onlyValue = false)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		//format string|;
		uint32 index = 0;

		uint32 fullLen = 0;

		if (pValueList[index])
		{
			for (uint32 i = 0; i < pValueList[index]->Count; i++)
			{
				if (pValueList[index]->pValues[i])
				{
					uint32 len;

					if (!pIndexes[index])
					{
						len = pDB->getPartDoc(query,
							jsonResult + fullLen,
							i,
							TranID,
							DefCollID,
							onlyValue,
							pValueList);
					}
					else
					{
						len = pDB->getPartDoc(query,
							jsonResult + fullLen,
							i,
							TranID,
							DefCollID,
							onlyValue,
							pValueList,
							(uint32*)pIndexes[index]->pValues[i]);
					}

					fullLen += len;

					jsonResult[fullLen++] = ';';
				}
			}
		}

		if (hasBeginTran)
		{
			pDB->clearTran(TranID);

			TranID = 0;

			hasBeginTran = false;
		}

		return fullLen;
	}

	void print(char* query)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		char jsonResult[246];

		for (uint32 i = 0; i < pValueList[0]->Count; i++)
		{
			if (pValueList[0]->pValues[i])
			{
				if (!pIndexes[0])
				{
					pDB->getPartDoc(query,
						jsonResult,
						i,
						TranID,
						CollIDs[0],
						false,
						pValueList);
				}
				else
				{
					pDB->getPartDoc(query,
						jsonResult,
						i,
						TranID,
						CollIDs[0],
						false,
						pValueList,
						(uint32*)pIndexes[0]->pValues[i]);
				}

				printf("%s\n", jsonResult);
			}
		}

		if (hasBeginTran)
		{
			pDB->clearTran(TranID);

			TranID = 0;

			hasBeginTran = false;
		}

		return;
	}

	/*
	BigDocQuery* print(char* query1, uint32 index1,
					   char* query2, uint32 index2)
	{
		char jsonResult[246];

		for(uint32 i=0; i<pValueList[index1]->Count; i++)
		{
			if(pValueList[index1]->pValues[i])
			{
				if(!pIndexes[index1])
				{
					pDB->getPartDoc(query1,
									jsonResult,
									pValueList[index1]->pValues[i],
									false);
				}
				else
				{
					pDB->getPartDoc(query1,
									jsonResult,
									pValueList[index1]->pValues[i],
									(uint32*)pIndexes[index1]->pValues[i],
									false);
				}

				printf("%s ", jsonResult);
			}
			else
			{
				printf("NULL ", jsonResult);
			}

			if(pValueList[index2]->pValues[i])
			{
				if(!pIndexes[index2])
				{
					pDB->getPartDoc(query2,
									jsonResult,
									pValueList[index2]->pValues[i],
									false);
				}
				else
				{
					pDB->getPartDoc(query2,
									jsonResult,
									pValueList[index2]->pValues[i],
									(uint32*)pIndexes[index2]->pValues[i],
									false);
				}

				printf("%s ", jsonResult);
			}
			else
			{
				printf("NULL ", jsonResult);
			}

			printf("\n");
		}

		return this;
	}
	*/

	BigDocQuery* skip(uint32 count)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		if (CountValueList && pValueList[0]->Count < count)
		{
			count = pValueList[0]->Count;
		}

		for (uint32 i = 0; i < count; i++)
		{
			for (uint32 j = 0; j < CountValueList; j++)
			{
				pValueList[j]->pValues[i] = 0;

				if (!pIndexes[j])
				{
					pIndexes[j]->pValues[i] = 0;
				}
			}
		}

		return this;
	}

	BigDocQuery* take(uint32 count)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		if (CountValueList && pValueList[0])
		{
			for (uint32 i = count; i < pValueList[0]->Count; i++)
			{
				for (uint32 j = 0; j < CountValueList; j++)
				{
					pValueList[j]->pValues[i] = 0;

					if (pIndexes[j])
					{
						pIndexes[j]->pValues[i] = 0;
					}
				}
			}
		}

		return this;
	}

	BigDocQuery* sort(char* query, bool isAsc)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		if (!CountValueList || pValueList[0]->Count < 2)
			return this;

		//1. Get all doc values
		char** strs = new char*[pValueList[0]->Count];
		char* vals = new char[pValueList[0]->Count * 256];

		for (uint32 i = 0; i < pValueList[0]->Count; i++)
		{
			strs[i] = vals + (i * 256); //init values

			if (pValueList[0]->pValues[i])
			{
				if (!pIndexes[0])
				{
					pDB->getPartDoc(query,
						strs[i],
						i,
						TranID,
						DefCollID,
						true,
						pValueList,
						0,
						CollIDs);
				}
				else
				{
					pDB->getPartDoc(query,
						strs[i],
						i,
						TranID,
						DefCollID,
						true,
						pValueList,
						(uint32*)pIndexes[0]->pValues[i],
						CollIDs);
				}
			}
			else
			{
				strs[i][0] = 0; //empty string
			}
		}

		//2. Sort all doc values
		quickSort(strs, pValueList[0]->Count, isAsc);

		delete[] strs;
		delete[] vals;

		return this;
	}

	void quickSort(char** strs, uint32 count, bool isAsc)
	{
		int i = 0, j = count - 1;	// поставить указатели на исходные места

		uint32 tempID;
		char* tempStr;
		char* p;

		p = strs[count >> 1];	// центральный элемент
								// процедура разделения

		do
		{
			if (isAsc)
			{
				while (i < count && strcmp(strs[i], p) < 0)
				{
					i++;
				}

				while (j > 0 && strcmp(strs[j], p) > 0)
				{
					j--;
				}
			}
			else
			{
				while (strcmp(strs[i], p) > 0)
				{
					i++;
				}

				while (strcmp(strs[j], p) < 0)
				{
					j--;
				}
			}

			if (i <= j)
			{
				tempStr = strs[i];
				tempID = pValueList[0]->pValues[i];

				strs[i] = strs[j];
				pValueList[0]->pValues[i] = pValueList[0]->pValues[j];

				strs[j] = tempStr;
				pValueList[0]->pValues[j] = tempID;

				i++;
				j--;
			}
		} while (i <= j);

		// рекурсивные вызовы, если есть, что сортировать 
		if (j > 0)
		{
			quickSort(strs, j, isAsc);
		}

		if (count > i)
		{
			quickSort(strs + i, count - i, isAsc);
		}
	}

	void drop(char* query,
			  uint32 tableIndex = 0)
	{
		//begin tran
		if (!TranID)
		{
			TranID = pDB->beginTran(READ_COMMITED_TRAN);

			hasBeginTran = true;
		}
		else
		{
			if (!pDB->_trans[TranID].TranType)
			{
				pDB->_trans[TranID].TranType = READ_COMMITED_TRAN; //commitable tran
			}
		}

		//method
		if (pValueList[0])
		{
			for (uint32 i = 0; i < pValueList[0]->Count; i++)
			{
				if (pValueList[0]->pValues[i])
				{
					if (pIndexes && pIndexes[tableIndex])
					{
						pDB->delPartDoc(query,
							pValueList[tableIndex]->pValues[i],
							TranID,
							CollIDs[tableIndex],
							(uint32*)pIndexes[tableIndex]->pValues[i]);
					}
					else
					{
						pDB->delPartDoc(query,
							pValueList[tableIndex]->pValues[i],
							TranID,
							CollIDs[tableIndex],
							0);
					}
				}
			}
		}

		//commit tran
		if (hasBeginTran)
		{
			pDB->commitTran(TranID);

			TranID = 0;

			hasBeginTran = false;
		}
	}

	void update(char* query,
				uint32 tableIndex = 0)
	{
		//begin tran
		if (!TranID)
		{
			TranID = pDB->beginTran(READ_COMMITED_TRAN);

			hasBeginTran = true;
		}
		else
		{
			if (!pDB->_trans[TranID].TranType)
			{
				pDB->_trans[TranID].TranType = READ_COMMITED_TRAN; //commitable tran
			}
		}

		//method
		if (pValueList[0])
		{
			for (uint32 i = 0; i < pValueList[0]->Count; i++)
			{
				if (pValueList[0]->pValues[i])
				{
					if (pIndexes && pIndexes[tableIndex])
					{
						pDB->updPartDoc(query,
							pValueList[tableIndex]->pValues[i],
							TranID,
							CollIDs[tableIndex],
							(uint32*)pIndexes[tableIndex]->pValues[i]);
					}
					else
					{
						pDB->updPartDoc(query,
							pValueList[tableIndex]->pValues[i],
							TranID,
							CollIDs[tableIndex],
							0);
					}
				}
			}
		}

		//commit tran
		if (hasBeginTran)
		{
			pDB->commitTran(TranID);

			TranID = 0;

			hasBeginTran = false;
		}

		return;
	}

	void insert(char* query,
				uint32 tableIndex = 0)
	{
		//begin tran
		if (!TranID)
		{
			TranID = pDB->beginTran(READ_COMMITED_TRAN);

			hasBeginTran = true;
		}
		else
		{
			if (!pDB->_trans[TranID].TranType)
			{
				pDB->_trans[TranID].TranType = READ_COMMITED_TRAN; //commitable tran
			}
		}

		//method
		if (pValueList[0])
		{
			for (uint32 i = 0; i < pValueList[0]->Count; i++)
			{
				if (pIndexes && pIndexes[tableIndex])
				{
					pDB->insPartDoc(query,
						pValueList[tableIndex]->pValues[i],
						TranID,
						CollIDs[tableIndex],
						(uint32*)pIndexes[tableIndex]->pValues[i]);
				}
				else
				{
					pDB->insPartDoc(query,
						pValueList[tableIndex]->pValues[i],
						TranID,
						CollIDs[tableIndex],
						0);
				}
			}
		}

		//commit tran
		if (hasBeginTran)
		{
			pDB->commitTran(TranID);

			TranID = 0;

			hasBeginTran = false;
		}

		return;
	}

	uint32 count()
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		uint32 cnt = 0;

		if (CountValueList && pValueList[0])
		{
			for (uint32 i = 0; i < pValueList[0]->Count; i++)
			{
				if (pValueList[0]->pValues[i])
				{
					cnt++;
				}
			}
		}

		if (hasBeginTran)
		{
			pDB->clearTran(TranID);

			TranID = 0;

			hasBeginTran = false;
		}

		return cnt;
	}
};
