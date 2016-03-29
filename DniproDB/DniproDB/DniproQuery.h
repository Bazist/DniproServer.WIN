#pragma once

#ifndef _DNIPRO_QUERY		 // Allow use of features specific to Windows XP or later.                   
#define _DNIPRO_QUERY 0x709 // Change this to the appropriate value to target other versions of Windows.

#endif	

#include "DniproDB.h"

class DniproQuery
{
public:
	DniproQuery(DniproDB* pDB)
	{
		this->pDB = pDB;
		CountValueList = 0;

		TranID = 0;

		//memset(pValueList, 0, sizeof(uint) * 16);
		//memset(pIndexes, 0, sizeof(uint) * 16);
	}

	DniproDB* pDB;
	
	ValueList* pValueList[16];
	ValueList* pIndexes[16];

	uint TranID;

	bool hasBeginTran;

	uint CountValueList;

	bool isIndexesEquals(uint* indexes1, uint* indexes2)
	{
		if(indexes1 && indexes2)
		{
			uint i=0;
			while(indexes1[i] == indexes2[i])
			{
				if(!indexes1[i])
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

	uint sum(char* query)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		uint index = 0;

		uint sum = 0;

		char jsonResult[128];

		for(uint i=0; i<pValueList[index]->Count; i++)
		{
			if(pValueList[index]->pValues[i])
			{
				pDB->getPartDoc(query, jsonResult, i, TranID, true, pValueList);

				sum += atoi(jsonResult);
			}
		}

		if (hasBeginTran)
		{
			pDB->clearTran(TranID);

			TranID = 0;
		}

		return sum;
	}

	/*
	uint minValue(char* query)
	{
		uint minValue = 0xFFFFFFFF;

		for(uint i=0; i<pValueList->Count; i++)
		{
			if(pValueList->pValues[i])
			{
				uint value = pDB->getAttrValueByDocID(query, pValueList->pValues[i]);
				
				if(value < minValue)
				{
					minValue = value;
				}
			}
		}

		return minValue;
	}

	uint maxValue(char* query)
	{
		uint maxValue = 0;

		for(uint i=0; i<pValueList->Count; i++)
		{
			if(pValueList->pValues[i])
			{
				uint value = pDB->getAttrValueByDocID(query, pValueList->pValues[i]);
				
				if(value > maxValue)
				{
					maxValue = value;
				}
			}
		}

		return maxValue;
	}*/

	uint avg(char* query)
	{
		return sum(query) / count();
	}

	DniproQuery* getWhere(char* query)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		this->pValueList[0] = pDB->getDocsByAttr(query, 0, TranID);
		this->pIndexes[0] = 0;
		
		this->CountValueList = 1;

		return this;
	}

	DniproQuery* getWhereElems(char* query)
	{
		return getWhereElems(query, 0);
	}

	DniproQuery* getWhereElems(char* query, uint docID)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		this->pValueList[0] = pDB->getDocsByAttr(query,
												 docID,
												 TranID,
												 &this->pIndexes[0]);
		
		this->CountValueList = 1;

		return this;
	}

	//???
	DniproQuery* andWhere(char* query)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		uint index = 0;

		if (pValueList[index])
		{
			ValueList* pAndValueList = pDB->getDocsByAttr(query, 0, TranID);

			for (uint i = 0; i < pValueList[index]->Count; i++)
			{
				if (pValueList[index]->pValues[i])
				{
					bool bFind = false;

					for (uint j = 0; j < pAndValueList->Count; j++)
					{
						if (pAndValueList->pValues[j] == pValueList[index]->pValues[i])
						{
							bFind = true;
							break;
						}
					}

					if (!bFind)
					{
						for (uint j = 0; j < CountValueList; j++)
						{
							pValueList[j]->pValues[i] = 0;

							if (pIndexes[j])
							{
								pIndexes[j]->pValues[i] = 0;
							}
						}
					}
				}
			}
		}

		return this;
	}

	DniproQuery* andWhereElems(char* query)
	{
		return andWhereElems(query, 0);
	}

	//???
	DniproQuery* andWhereElems(char* query, uint docID)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		uint index = 0;

		if (pValueList[index])
		{
			ValueList* pAndIndexes;
			ValueList* pAndValueList = pDB->getDocsByAttr(query, docID, TranID, &pAndIndexes);

			for (uint i = 0; i < pValueList[index]->Count; i++)
			{
				if (pValueList[index]->pValues[i])
				{
					bool bFind = false;

					for (uint j = 0; j < pAndValueList->Count; j++)
					{
						if (pAndValueList->pValues[j] == pValueList[index]->pValues[i] &&
							(!pIndexes[index] ||
								isIndexesEquals((uint*)pAndIndexes->pValues[j], (uint*)pIndexes[index]->pValues[i])))
						{
							bFind = true;
							break;
						}
					}

					if (!bFind)
					{
						for (uint j = 0; j < CountValueList; j++)
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

	//???
	DniproQuery* orWhere(char* query)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		if (CountValueList && pValueList[0])
		{
			ValueList* pOrValueList = pDB->getDocsByAttr(query, 0, TranID);

			uint count = pValueList[0]->Count;

			for (uint i = 0; i < pOrValueList->Count; i++)
			{
				if (pOrValueList->pValues[i])
				{
					bool bFind = false;

					for (uint j = 0; j < count; j++)
					{
						if (pValueList[0]->pValues[j] == pOrValueList->pValues[i])
						{
							bFind = true;
							break;
						}
					}

					if (!bFind)
					{
						pValueList[0]->addValue(pOrValueList->pValues[i], false);

						if (pIndexes[0])
						{
							pIndexes[0]->addValue(0, false);
						}
					}
				}
			}
		}

		return this;
	}

	DniproQuery* orWhereElems(char* query)
	{
		return orWhereElems(query, 0);
	}

	//???
	DniproQuery* orWhereElems(char* query, uint docID)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		if (CountValueList && pValueList[0])
		{
			ValueList* pOrIndexes;
			ValueList* pOrValueList = pDB->getDocsByAttr(query, docID, TranID, &pOrIndexes);

			uint count = pValueList[0]->Count;

			for (uint i = 0; i < pOrValueList->Count; i++)
			{
				if (pOrValueList->pValues[i])
				{
					bool bFind = false;

					for (uint j = 0; j < count; j++)
					{
						if (pOrValueList->pValues[j] == pValueList[0]->pValues[i] &&
							(!pIndexes[0] ||
							 isIndexesEquals((uint*)pOrIndexes->pValues[j], (uint*)pIndexes[0]->pValues[i])))
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

	DniproQuery* join(char* query1,
					  char* query2)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		char outValue[256];
		char queryTemp[1024];

		ValueList* pLeftValueList = pValueList[CountValueList - 1];
		//ValueList* pLeftIndexes = pIndexes[CountValueList - 1];

		ValueList* pRightValueList = pDB->_trans[TranID].valueListPool.newObject();

		pRightValueList->Count = pLeftValueList->Count;

		pValueList[CountValueList++] = pRightValueList;

		for(uint i=0; i <pLeftValueList->Count; i++)
		{
			if(pLeftValueList->pValues[i])
			{
				//get attr value from left table
				pDB->getPartDoc(query1, outValue, pLeftValueList->pValues[i], TranID, true);

				//create query
				for(uint j=0; ; j++)
				{
					if(query2[j] != '$')
					{
						queryTemp[j] = query2[j];
					}
					else
					{
						queryTemp[j++] = '\'';

						uint s = j;
					
						for(uint k=0; outValue[k] != 0; k++, j++)
						{
							queryTemp[j] = outValue[k];
						}

						queryTemp[j++] = '\'';

						for(; query2[s] != 0; s++, j++)
						{
							queryTemp[j] = query2[s];
						}

						queryTemp[j] = 0;

						break;
					}
				}

				//join
				ValueList* pTempValueList = pDB->getDocsByAttr(queryTemp, 0, TranID);
			
				if(pTempValueList->Count == 1) //one to one
				{
					pRightValueList->pValues[i] = pTempValueList->pValues[0];
				}
				else if(pTempValueList->Count > 1) //one to many
				{
					for(uint j = 0; j < pTempValueList->Count; j++)
					{
						pLeftValueList->addValue(0, false);
						pRightValueList->addValue(0, false);
					}

					for(uint j = pLeftValueList->Count-1; j > i; j--)
					{
						pLeftValueList->pValues[j] = pLeftValueList->pValues[j - pTempValueList->Count];
						pRightValueList->pValues[j] = pTempValueList->pValues[j - pTempValueList->Count];
					}

					for(uint j = 0; j < pTempValueList->Count; j++)
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

	DniproQuery* getAll()
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		pValueList[0] = pDB->_trans[TranID].valueListPool.newObject();
		this->CountValueList = 1;

		pIndexes[0] = 0;
		
		for(uint i=1; i <= pDB->lastDocID; i++)
		{
			pValueList[0]->addValue(i, false);
		}

		return this;
	}

	uint select(char* query, char* jsonResult)
	{
		return select(query, jsonResult, false, false);
	}

	uint select(char* query,
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
		uint index = 0;

		uint fullLen = 4;

		uint* pCount = (uint*)jsonResult;
		*pCount = 0;

		if(pValueList[index])
		{
			for(uint i=0; i<pValueList[index]->Count; i++)
			{
				if(pValueList[index]->pValues[i])
				{
					ushort* pLen = (ushort*)(jsonResult + fullLen);

					fullLen += 2;

					uint len;

					if(!pIndexes[index])
					{
						len = pDB->getPartDoc(query,
											  jsonResult + fullLen,
											  i,
											  TranID,
											  onlyValue,
											  pValueList);
					}
					else
					{
						len = pDB->getPartDoc(query,
											  jsonResult + fullLen,
											  i,
											  TranID,
											  onlyValue,
											  pValueList,
											  (uint*)pIndexes[index]->pValues[i]);
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
		}

		return fullLen;
	}

	//all results in one str with ';' delimeter
	uint selectStr(char* query,
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
		uint index = 0;

		uint fullLen = 0;

		if (pValueList[index])
		{
			for (uint i = 0; i<pValueList[index]->Count; i++)
			{
				if (pValueList[index]->pValues[i])
				{
					uint len;

					if (!pIndexes[index])
					{
						len = pDB->getPartDoc(query,
							jsonResult + fullLen,
							i,
							TranID,
							onlyValue,
							pValueList);
					}
					else
					{
						len = pDB->getPartDoc(query,
							jsonResult + fullLen,
							i,
							TranID,
							onlyValue,
							pValueList,
							(uint*)pIndexes[index]->pValues[i]);
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

		uint index = 0;

		char jsonResult[246];

		for(uint i=0; i<pValueList[index]->Count; i++)
		{
			if(pValueList[index]->pValues[i])
			{
				if(!pIndexes[index])
				{
					pDB->getPartDoc(query,
									jsonResult,
									i,
									TranID,
									false,
									pValueList);
				}
				else
				{
					pDB->getPartDoc(query,
									jsonResult,
									i,
									TranID,
									false,
									pValueList,
									(uint*)pIndexes[index]->pValues[i]);
				}

				printf("%s\n", jsonResult);
			}
		}

		if (hasBeginTran)
		{
			pDB->clearTran(TranID);

			TranID = 0;
		}

		return;
	}

	/*
	DniproQuery* print(char* query1, uint index1,
					   char* query2, uint index2)
	{
		char jsonResult[246];

		for(uint i=0; i<pValueList[index1]->Count; i++)
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
									(uint*)pIndexes[index1]->pValues[i],
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
									(uint*)pIndexes[index2]->pValues[i],
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

	DniproQuery* skip(uint count)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		if(CountValueList && pValueList[0]->Count < count)
		{
			count = pValueList[0]->Count;
		}

		for(uint i=0; i < count; i++)
		{
			for(uint j=0; j<CountValueList; j++)
			{
				pValueList[j]->pValues[i] = 0;

				if(!pIndexes[j])
				{
					pIndexes[j]->pValues[i] = 0;
				}
			}
		}

		return this;
	}

	DniproQuery* take(uint count)
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		if (CountValueList && pValueList[0])
		{
			for (uint i = count; i < pValueList[0]->Count; i++)
			{
				for (uint j = 0; j < CountValueList; j++)
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

	DniproQuery* sort(char* query, bool isAsc)
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

		for (uint i = 0; i<pValueList[0]->Count; i++)
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
									true,
									pValueList);
				}
				else
				{
					pDB->getPartDoc(query,
									strs[i],
									i,
									TranID,
									true,
									pValueList,
									(uint*)pIndexes[0]->pValues[i]);
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

	void quickSort(char** strs, uint count, bool isAsc)
	{
		int i = 0, j = count - 1;	// поставить указатели на исходные места
		
		uint tempID;
		char* tempStr;
		char* p;

		p = strs[count >> 1];	// центральный элемент
								// процедура разделения

		do
		{
			if (isAsc)
			{
				while (strcmp(strs[i], p) < 0)
				{
					i++;
				}

				while (strcmp(strs[j], p) > 0)
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
		}
		while (i <= j);


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

	void drop(char* query)
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

			hasBeginTran = false;
		}

		//method
		uint index = 0;

		if (pValueList[index])
		{
			for (uint i = 0; i < pValueList[index]->Count; i++)
			{
				if (pValueList[index]->pValues[i])
				{
					if (!pIndexes[index])
					{
						pDB->delPartDoc(query,
										i,
										TranID,
										pValueList);
					}
					else
					{
						pDB->delPartDoc(query,
										i,
										TranID,
										pValueList,
										(uint*)pIndexes[index]->pValues[i]);
					}
				}
			}
		}

		//commit tran
		if (hasBeginTran)
		{
			pDB->commitTran(TranID);

			TranID = 0;
		}
	}

	void update(char* query)
	{
		//begin tran
		if (!TranID)
		{
			TranID = pDB->beginTran(1);

			hasBeginTran = true;
		}
		else
		{
			if (!pDB->_trans[TranID].TranType)
			{
				pDB->_trans[TranID].TranType = READ_COMMITED_TRAN; //commitable tran
			}

			hasBeginTran = false;
		}

		//method
		uint index = 0;

		if(pValueList[index])
		{
			for (uint i = 0; i < pValueList[index]->Count; i++)
			{
				if (pValueList[index]->pValues[i])
				{
					if (!pIndexes[index])
					{
						pDB->updPartDoc(query,
										i,
										TranID,
										pValueList);
					}
					else
					{
						pDB->updPartDoc(query,
										i,
										TranID,
										pValueList,
										(uint*)pIndexes[index]->pValues[i]);
					}
				}
			}
		}

		//commit tran
		if (hasBeginTran)
		{
			pDB->commitTran(TranID);

			TranID = 0;
		}

		return;
	}

	void insert(char* query)
	{
		//begin tran
		if (!TranID)
		{
			TranID = pDB->beginTran(1);

			hasBeginTran = true;
		}
		else
		{
			if (!pDB->_trans[TranID].TranType)
			{
				pDB->_trans[TranID].TranType = READ_COMMITED_TRAN; //commitable tran
			}

			hasBeginTran = false;
		}

		//method
		uint index = 0;

		if (pValueList[index])
		{
			for (uint i = 0; i < pValueList[index]->Count; i++)
			{
				if (pValueList[index]->pValues[i])
				{
					if (!pIndexes[index])
					{
						pDB->insPartDoc(query,
										i,
										TranID,
										pValueList,
										0);
					}
					else
					{
						pDB->insPartDoc(query,
										i,
										TranID,
										pValueList,
										(uint*)pIndexes[index]->pValues[i]);
					}
				}
			}
		}

		//commit tran
		if (hasBeginTran)
		{
			pDB->commitTran(TranID);

			TranID = 0;
		}

		return;
	}
	
	uint count()
	{
		if (!TranID)
		{
			TranID = pDB->beginTran(READONLY_TRAN); //readonly tran

			hasBeginTran = true;
		}

		uint cnt = 0;

		if (CountValueList && pValueList[0])
		{
			for (uint i = 0; i < pValueList[0]->Count; i++)
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
		}

		return cnt;
	}
};
