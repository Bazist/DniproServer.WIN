/*
# Copyright(C) 2010-2016 Viacheslav Makoveichuk (email: dniprodb@gmail.com, skype: vyacheslavm81)
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

#include "DniproDB.h"

void DniproDB::procAlias(char* json,
						char* key,
						uint32 currPos,
						uint32* arrayPos,
						uint32** arrayMaxPos,
						uint32* repeatPos,
						uint32 level,
						uint32& i,
						uint32 docID,
						uchar8& typeCommand,
						uint32* indexes,
						HArrayTran* pTran)
{
	arrayPos[level] = 1; //by default

	if (arrayMaxPos)
	{
		//set count in array, format: path + docID + indexes (0) =========================================
		//add docID
		*(uint32*)&key[currPos] = docID;

		//add indexes
		currPos += 4;

		setPathIndexes(arrayPos, indexes, key, level, currPos);

		*(uint32*)&key[currPos - 4] = 0; //last index is zero (Array.Count)

		//get reference
		uchar8 valueType;

		//uint32 serPointer = ha2.getValueByKey((uint32*)key, currPos, valueType);
		uint32 serPointer;
		
		while (blockReaders.load());

		amountReaders++;

		if (!pTran->TranType) //READ_ONLY_TRAN
		{
			serPointer = has2[pTran->CollID]->getValueByKey((uint32*)key, currPos, valueType, 1); //1. Read data with check blocking (ha2)
		}
		else 
		{
			if (pTran->TranType == READ_COMMITED_TRAN)
			{
				serPointer = has2[pTran->CollID]->getValueByKey((uint32*)key, currPos, valueType, 1); //1. Read data with check blocking (ha2)
			}
			else //pTran->TranType == REPEATABLE_READ_TRAN || pTran->TranType == SNAPSHOT_TRAN
			{
				serPointer = has2[pTran->CollID]->getValueByKey((uint32*)key,
																currPos,
																valueType,
																2,
																pTran->TranID,
																&pTran->readedList); //2. Read data and check blocking and block with put to array blocked cell (ha2)
			}
		}

		amountReaders--;

		print(key, currPos, level);

		uint32* pValue = 0;

		if (!serPointer)
		{
			if (typeCommand) //ins part doc, create array
			{
				pValue = (uint32*)(pTran->pAttrValuesPage->Values + pTran->pAttrValuesPage->CurrPos);

				pTran->pAttrValuesPage->CurrPos += 4;

				*pValue = 0;

				pTran->insertOrDelete2((uint32*)key,
									   currPos,
									   has2[pTran->CollID]->attrValuesPool.toSerPointer(pTran->pAttrValuesPage, (char*)pValue),
									   Inserted,
									   pTran->TranID);
			}
		}
		else
		{
			if (typeCommand) //ins part doc, amount of array items will be modified
			{
				//need clone Arr.Count value, to create new version of value
				pValue = (uint32*)(pTran->pAttrValuesPage->Values + pTran->pAttrValuesPage->CurrPos);

				pTran->pAttrValuesPage->CurrPos += 4;

				//clone
				*pValue = *(uint32*)has2[pTran->CollID]->attrValuesPool.fromSerPointer(serPointer);

				pTran->insertOrDelete2((uint32*)key,
										currPos,
										has2[pTran->CollID]->attrValuesPool.toSerPointer(pTran->pAttrValuesPage, (char*)pValue),
										Inserted,
										pTran->TranID);

			}
			else //get part doc
			{
				pValue = (uint32*)has2[pTran->CollID]->attrValuesPool.fromSerPointer(serPointer);
			}
		}

		arrayMaxPos[level] = pValue;
	}

	//==========================================================================================
	char nc = json[i + 1];
	if (nc == '@' &&
		('0' <= json[i + 2] && json[i + 2] <= '9'))
	{
		uint32 num = 0;

		for (i += 2; '0' <= json[i] && json[i] <= '9'; i++)
		{
			num = num * 10 + json[i] - '0';
		}

		arrayPos[level] = num + 1;
	}
	else if (nc == 'A' &&
		json[i + 2] == 'd') //Add
	{
		arrayPos[level] = ++(*arrayMaxPos[level]);

		i += 4;
	}
	else if (nc == 'L') //Last
	{
		arrayPos[level] = (*arrayMaxPos[level]);

		i += 5;
	}
	//else if (nc == 'C') //Count //Supported only through GetAttrValue
	//{
	//	arrayPos[level] = 0;

	//	i += 6;
	//}
	else if (nc == 'I') //In
	{
		typeCommand = 1;

		i += 3;
	}
	else if (nc == 'N') //Nin
	{
		typeCommand = 2;

		i += 3;
	}
	else if (nc == 'A' &&
		json[i + 2] == 'n') //Any
	{
		typeCommand = 3;

		i += 4;
	}
	else if (nc == 'R') //Repeat
	{
		arrayPos[level] = 1; //start from

		i += 2;

		repeatPos[level] = i;
	}
	else if (nc == 'P') //Portion
	{
		i += 3; //skip P,

		//read numbers
		uint32 j = 0;
		char num[2][10];

		for (uint32 k = 0; k < 2; k++)
		{
			while (true)
			{
				if (('0' <= json[i] && json[i] <= '9'))
				{
					num[k][j++] = json[i++];
				}
				else if (json[i] == ' ')
				{
					i++;
				}
				else
				{
					num[k][j] = 0;

					j = 0;

					if (k == 0)
					{
						i++; //skip ,
					}

					break;
				}
			}
		}

		arrayPos[level] = atoi(num[0]); //start from

		if (arrayMaxPos[level]) //possible only if there is array
		{
			pTran->tempInt = arrayPos[level] + atoi(num[1]) - 1; //to position

			if (pTran->tempInt < *(arrayMaxPos[level])) //not last portion
			{
				arrayMaxPos[level] = &pTran->tempInt;
			}

			repeatPos[level] = i;
		}
	}
}

uint32 DniproDB::setPathIndexes(uint32* arrayPos,
								uint32* indexes,
								char* key,
								uint32 level,
								uint32& currPos)
{
	uint32 amountIndexes = 0;

	uint32 currIndex = 0;

	for (uint32 i = 1; i <= level; i++)
	{
		if (arrayPos[i])
		{
			if (indexes && indexes[currIndex])
			{
				*(uint32*)&key[currPos] = indexes[currIndex];

				currIndex++;
			}
			else
			{
				*(uint32*)&key[currPos] = arrayPos[i];
			}

			currPos += 4;

			amountIndexes++;
		}
	}

	return amountIndexes;
}

ValueList* DniproDB::getDocsByAttr(char* json,
									uint32 docID ,
									uint32 tranID,
									uint32 collID,
									ValueList** pQueryIndexes)
{
	bool hasBeginTran;

	if (!tranID)
	{
		tranID = beginTran(READONLY_TRAN);

		hasBeginTran = true;
	}
	else
	{
		hasBeginTran = false;
	}

	HArrayTran* pTran = &_trans[tranID];
	pTran->CollID = collID;
	
	pTran->LastJson = json;

	uint32 level = 0;
	uint32 depthPos[MAX_DOC_DEPTH];
	depthPos[0] = 0;

	uint32 arrayPos[MAX_DOC_DEPTH];
	arrayPos[0] = 0;

	uint32 arrayLevel = 0;

	uint32* arrayMaxPos[MAX_DOC_DEPTH];

	uint32 currPos = 0;

	bool hasValue = false;

	uchar8 typeCommand = 0;

	ValueList* pValueListCommand[16];
	ValueList* pIndexesListCommand[16];

	ValueList** pIndexes = 0;

	uint32 valueListCommandCount = 0;

	ScanKeyData scanKeyData;
	
	/*scanKeyData.Scans = 0;
									scanKeyData.Bitmap = new char[100000];
									memset(scanKeyData.Bitmap, 0, 100000);*/

	uint32* arrayIndex = 0;

	char key[1000];

	for (uint32 i = 0; json[i];)
	{
		char c = json[i];

		switch (c)
		{
		case '[':
		{
			depthPos[++level] = currPos;
			procAlias(json,
				key,
				currPos,
				arrayPos,
				0,
				0,
				++arrayLevel,
				i,
				0,
				typeCommand,
				0,
				pTran);

			//need indexes
			if (!pIndexes && pQueryIndexes)
			{
				scanKeyData.DocID = docID;
				scanKeyData.pValueListPool = &has1[pTran->CollID]->valueListPool;
				scanKeyData.pIndexesPool = &pTran->indexesPool;
				scanKeyData.pTran = pTran;

				pIndexes = pQueryIndexes;
			}

			break;
		}
		case '{':
		{
			depthPos[++level] = currPos;
			arrayPos[level] = 0;

			break;
		}
		//case '@':
										//	{
										//		i++;

		//		//alingment
										//		ValueList* pNewValueList = new ValueList();

		//		ValueList* pValueList = getDocsByAttr(json + i);

		//		for(uint32 j=0; j < pValueList->Count; j++)
										//		{
										//			if(pValueList->pValues[j])
										//			{
										//				uint32 newCurrPos = currPos + getAttrValueByDocID("{'pk':$}",
										//								  								key + currPos,
										//																pValueList->pValues[j]);

		//				//alingment
										//				while(newCurrPos & 0x3)
										//				{
										//					key[newCurrPos++] = 0;
										//				}

		//				uchar8 valueType;
										//				uint32 value = ha1.getValueByKey((uint32*)key, newCurrPos, valueType);

		//				if(value)
										//				{
										//					if(valueType == VALUE_TYPE)
										//					{
										//						pNewValueList->addValue(value);
										//					}
										//					else if(valueType == VALUE_LIST_TYPE)
										//					{
										//						pNewValueList->addValues((ValueList*)value);		
										//					}
										//				}
										//			}
										//		}

		//		return pNewValueList;
										//	}
		case '@': //expression
		{
			//format #"<10"

			scanKeyData.pValueList = pTran->valueListPool.newObject();
			scanKeyData.KeyLen = currPos >> 2;

			i += 2;

			//template by default
			scanKeyData.MethodType = 5;

			if (json[i] == '<')
			{
				//<
				if (json[i + 1] != '=')
				{
					i++;
					scanKeyData.MethodType = 1;
				}
				else //<=
				{
					i += 2;
					scanKeyData.MethodType = 2;
				}
			}
			else if (json[i] == '>')
			{
				//>
				if (json[i + 1] != '=')
				{
					i++;
					scanKeyData.MethodType = 3;
				}
				else //>=
				{
					i += 2;
					scanKeyData.MethodType = 4;
				}
			}
			else if (json[i] == '!')
			{
				scanKeyData.MethodType = 7;
			}

			//read operand
			uint32 k1 = 0;
			uint32 k2 = 0;

			for (; json[i] != '"' && json[i] != '\''; i++, k1++) //copy operand 1
			{
				if (json[i] == '\\')
				{
					i++;
				}

				scanKeyData.Operand1Str[k1] = json[i];

				if (json[i] == '-') //range
				{
					scanKeyData.MethodType = 6;

					for (i++; json[i] != '"' && json[i] != '\''; i++, k2++) //copy operand 2
					{
						if (json[i] == '\\')
						{
							i++;
						}

						scanKeyData.Operand2Str[k2] = json[i];
					}

					break;
				}
			}

			if (scanKeyData.MethodType != 5) //not template
			{
				scanKeyData.Operand1Str[k1] = 0;
				scanKeyData.Operand1 = atoi(scanKeyData.Operand1Str);
				scanKeyData.Operand1Len = strlen(scanKeyData.Operand1Str);

				if (scanKeyData.MethodType == 6) //range
				{
					scanKeyData.Operand2Str[k2] = 0;
					scanKeyData.Operand2 = atoi(scanKeyData.Operand2Str);
					scanKeyData.Operand2Len = strlen(scanKeyData.Operand2Str);
				}
			}

			while (blockReaders.load());

			amountReaders++;

			//copy value;
			has1[pTran->CollID]->scanKeysAndValues((uint32*)key,
														currPos,
														&getDocsByAttr_scanCond,
														&scanKeyData);

			amountReaders--;

			if (pTran->TranType) //only for not readonly trans
			{
				//scan tran
				pTran->scanKeysAndValues1((uint32*)key,
					currPos,
					&getDocsByAttr_scanCond,
					&scanKeyData);
			}

			/*for(uint32 j=0; j<100000; j++)
											{
												if(!scanKeyData.Bitmap[j])
												{
													printf("%d\n", j);
												}
											}*/

			if (hasBeginTran)
			{
				pTran->clear();
			}

			return scanKeyData.pValueList;

		}
		case '"':
		case '\'':
		{
			for (i++; json[i] != '"' && json[i] != '\''; i++)
			{
				if (json[i] == '\\')
				{
					i++;
				}

				key[currPos++] = json[i];
			}

			hasValue = true;

			break;
		}
		case '0':case'1':case'2':case'3':case'4':case'5':case'6':case'7':case'8':case'9':
		{
			for (; ('0' <= json[i] && json[i] <= '9') || json[i] == '.'; i++)
			{
				key[currPos++] = json[i];
			}

			hasValue = true;

			continue;
		}
		case 't':
		case 'f':
		case 'n':
		{
			uint32 len = (c == 't' || c == 'n' ? 4 : 5);

			for (uint32 j = 0; j < len; i++, j++)
			{
				key[currPos++] = json[i];
			}

			hasValue = true;

			continue;
		}
		case ':':
		{
			//alingment
			while (currPos & 0x3)
			{
				key[currPos++] = 0;
			}

			break;
		}
		case '}':
		case ']':
		{
			level--;
		}
		case ',':
		{
			if (hasValue)
			{
				//alingment
				while (currPos & 0x3)
				{
					key[currPos++] = 0;
				}

				//get without indexes
				if (!pIndexes)
				{
					if (!typeCommand)
					{
						if (arrayLevel)
						{
							setPathIndexes(arrayPos, 0, key, arrayLevel, currPos);
						}

						print(key, currPos, level);

						uchar8 valueType;
						//uint32 value = ha1.getValueByKey((uint32*)key, currPos, valueType);

						uint32 value;

						while (blockReaders.load());

						amountReaders++;
						
						if (pTran->TranType) //only for not readonly trans
						{
							value = pTran->getValueByKey1((uint32*)key, currPos, valueType);
						}
						else
						{
							value = has1[pTran->CollID]->getValueByKey((uint32*)key, currPos, valueType);
						}

						amountReaders--;

						if (value)
						{
							if (valueType == VALUE_LIST_TYPE)
							{
								pValueListCommand[valueListCommandCount] = has1[pTran->CollID]->valueListPool.fromSerPointer(value);
								pIndexesListCommand[valueListCommandCount] = 0;

								valueListCommandCount++;
							}
							else if (valueType == VALUE_TYPE)
							{
								ValueList* pValueList = pTran->valueListPool.newObject();
								pValueList->addValue((uint32)value);

								pValueListCommand[valueListCommandCount] = pValueList;
								pIndexesListCommand[valueListCommandCount] = 0;

								valueListCommandCount++;
							}
							else
							{
								printf("Error !!!");
							}
						}
					}
					else //In, Any, Nin
					{
						scanKeyData.pValueList = pTran->valueListPool.newObject();

						print(key, currPos, level);

						while (blockReaders.load());

						amountReaders++;

						//scan store
						has1[pTran->CollID]->scanKeysAndValues((uint32*)key,
																	currPos,
																	getDocsByAttr_scanIn,
																	&scanKeyData);

						amountReaders--;

						if (pTran->TranType) //only for not readonly trans
						{
							//scan tran
							pTran->scanKeysAndValues1((uint32*)key,
								currPos,
								getDocsByAttr_scanIn,
								&scanKeyData);
						}

						pValueListCommand[valueListCommandCount] = scanKeyData.pValueList;
						pIndexesListCommand[valueListCommandCount] = 0;

						valueListCommandCount++;
					}

					//continue;
				}
				else //get with indexes
				{
					scanKeyData.pValueList = pTran->valueListPool.newObject();
					scanKeyData.pIndexes = pTran->valueListPool.newObject();

					scanKeyData.KeyLen = currPos >> 2;

					while (blockReaders.load());

					amountReaders++;

					//scan tran
					has1[pTran->CollID]->scanKeysAndValues((uint32*)key,
															currPos,
															getDocsByAttr_scanKey,
															&scanKeyData);

					amountReaders--;

					if (pTran->TranType) //only for not readonly trans
					{
						//scan store
						pTran->scanKeysAndValues1((uint32*)key,
							currPos,
							getDocsByAttr_scanKey,
							&scanKeyData);
					}

					pValueListCommand[valueListCommandCount] = scanKeyData.pValueList;
					pIndexesListCommand[valueListCommandCount] = scanKeyData.pIndexes;

					valueListCommandCount++;
				}

				currPos = depthPos[level];

				hasValue = false;
			}

			break;
		}
		default:
			break;
		}

		i++;
	}

	//no indexes
	if (!pIndexes && pQueryIndexes)
	{
		*pQueryIndexes = 0;
	}

	//merge results
	if (valueListCommandCount)
	{
		if (valueListCommandCount == 1)
		{
			if (pIndexes)
			{
				*pIndexes = pIndexesListCommand[0];
			}

			if (hasBeginTran)
			{
				pTran->clear();
			}

			return pValueListCommand[0];
		}

		switch (typeCommand)
		{
		case 0: //And (,)
		case 1: //In
		{
			if (!pIndexes) //without indexes
			{
				for (uint32 i = 0; i < pValueListCommand[0]->Count; i++)
				{
					for (uint32 j = 1; j < valueListCommandCount; j++)
					{
						if (!pValueListCommand[j]->hasValue(pValueListCommand[0]->pValues[i]))
						{
							pValueListCommand[0]->pValues[i] = 0;

							break;
						}
					}
				}

				if (hasBeginTran)
				{
					pTran->clear();
				}

				return pValueListCommand[0];
			}
			else //with indexes
			{
				uint32 lastIdx = valueListCommandCount - 1; //last because it is deepest level, and it should be much all previous levels

				for (uint32 i = 0; i < pValueListCommand[lastIdx]->Count; i++) //each doc
				{
					for (uint32 j = 0; j < lastIdx; j++) //and each doc
					{
						bool bFind = false;

						for (uint32 k = 0; k < pValueListCommand[j]->Count; k++) //any value
						{
							if (pValueListCommand[j]->pValues[k] == pValueListCommand[lastIdx]->pValues[i]) //check DocID
							{
								if (pIndexesListCommand[lastIdx] && pIndexesListCommand[j])
								{
									//check indexes
									uint32* indexes1 = (uint32*)pIndexesListCommand[lastIdx]->pValues[i];
									uint32* indexes2 = (uint32*)pIndexesListCommand[j]->pValues[k];

									for (uint32 l = 0; indexes2[l]; l++)
									{
										if (indexes1[l] != indexes2[l])
										{
											goto NOT_FOUND;
										}
									}
								}

								bFind = true;

								break;
							}

						NOT_FOUND:;
						}

						//not found !
						if (!bFind)
						{
							pValueListCommand[lastIdx]->pValues[i] = 0;
							pIndexesListCommand[lastIdx]->pValues[i] = 0;

							break;
						}
					}
				}

				*pIndexes = pIndexesListCommand[lastIdx];

				if (hasBeginTran)
				{
					pTran->clear();
				}

				return pValueListCommand[lastIdx];
			}
		}
		case 2: //Nin
		{
			break;
		}
		case 3: //Any
		{
			for (uint32 i = 1; i < valueListCommandCount; i++)
			{
				for (uint32 j = 0; j < pValueListCommand[i]->Count; j++)
				{
					pValueListCommand[0]->addUniqueValue(pValueListCommand[i]->pValues[j]);
				}
			}

			if (hasBeginTran)
			{
				pTran->clear();
			}

			return pValueListCommand[0];
		}
		default:
		{
			printf("Command type is undefined.");
		}
		}
	}

	return 0;
}

uint32 DniproDB::getPartDoc(char* jsonTemplate,
						  char* jsonResult,
						  uint32 rowNumOrDocID,
						  uint32 tranID,
						  uint32 collID,
						  bool onlyValue,
						  ValueList** pDocIDs,
						  uint32* indexes,
						  uchar8* collIDs)
{
	bool hasBeginTran;

	if (!tranID)
	{
		tranID = beginTran(READONLY_TRAN);

		hasBeginTran = true;
	}
	else
	{
		hasBeginTran = false;
	}

	HArrayTran* pTran = &_trans[tranID];
	pTran->CollID = collID;

	pTran->LastJson = jsonTemplate;

	uint32 docID = 0;
	uint32 tableIndex = 0;

	uint32 level = 0;

	uint32 depthPos[MAX_DOC_DEPTH];
	depthPos[0] = 0;

	uint32 repeatPos[MAX_DOC_DEPTH];
	repeatPos[0] = 0;

	uint32 arrayPos[MAX_DOC_DEPTH];
	arrayPos[0] = 0;

	uint32* arrayMaxPos[MAX_DOC_DEPTH];

	uint32 currPos = 0;

	uint32 extractFromLevel = 0;

	uchar8 typeCommand = 0;

	char key[1024];

	uint32 j = 0;
	for (uint32 i = 0; jsonTemplate[i];)
	{
		char c = jsonTemplate[i];

		switch (c)
		{
		case '[':
		{
			depthPos[++level] = currPos;
			repeatPos[level] = 0;

			procAlias(jsonTemplate,
						key,
						currPos,
						arrayPos,
						arrayMaxPos,
						repeatPos,
						level,
						i,
						docID,
						typeCommand,
						indexes,
						pTran);

			//if no elements in array, then skip all inside and write null
			if (!arrayMaxPos[level] || !(*arrayMaxPos[level]))
			{
				if (jsonTemplate[i] != '[') //skip
				{
					int arrLevel = 1;

					while (true)
					{
						if (jsonTemplate[i] == '[')
							arrLevel++;
						else if (jsonTemplate[i] == ']')
							arrLevel--;

						i++;

						if (!arrLevel)
							break;
					}

					jsonResult[j++] = '[';
					jsonResult[j++] = ']';

					level--;

					continue;
				}
				else //as is
				{
					int arrLevel = 0;

					while (true)
					{
						if (jsonTemplate[i] == '[')
							arrLevel++;
						else if (jsonTemplate[i] == ']')
							arrLevel--;

						jsonResult[j++] = jsonTemplate[i];

						i++;

						if (!arrLevel)
							break;

					}

					level--;

					continue;
				}
			}

			break;
		}
		case '{':
		{
			//next join table
			if (!level)
			{
				if (!pDocIDs)
				{
					pTran->CollID = collID;
					docID = rowNumOrDocID;
				}
				else
				{
					pTran->CollID = collIDs[tableIndex];
					docID = pDocIDs[tableIndex++]->pValues[rowNumOrDocID]; //row number
				}
			}

			depthPos[++level] = currPos;
			arrayPos[level] = 0;
			repeatPos[level] = 0;

			break;
		}
		case '!':
		{
			extractFromLevel = level;

			j = 0;

			i++;

			continue;
		}
		case '"':
		case '\'':
		{
			jsonResult[j++] = jsonTemplate[i];

			for (i++; jsonTemplate[i] != '"' && jsonTemplate[i] != '\''; i++)
			{
				if (jsonTemplate[i] == '\\')
				{
					i++;
				}

				key[currPos++] = jsonTemplate[i];

				if (level >= extractFromLevel)
				{
					jsonResult[j++] = jsonTemplate[i];
				}
			}

			if (level >= extractFromLevel)
			{
				jsonResult[j++] = jsonTemplate[i];
			}

			i++;

			continue;
		}
		case '0':case'1':case'2':case'3':case'4':case'5':case'6':case'7':case'8':case'9':
		{
			for (; ('0' <= jsonTemplate[i] && jsonTemplate[i] <= '9') || jsonTemplate[i] == '.'; i++)
			{
				key[currPos++] = jsonTemplate[i];

				if (level >= extractFromLevel)
				{
					jsonResult[j++] = jsonTemplate[i];
				}
			}

			continue;
		}
		case 't':
		case 'f':
		case 'n':
		{
			uint32 len = (c == 't' || c == 'n' ? 4 : 5);

			for (uint32 j = 0; j < len; i++, j++)
			{
				key[currPos++] = jsonTemplate[i];

				if (level >= extractFromLevel)
				{
					jsonResult[j++] = jsonTemplate[i];
				}
			}

			continue;
		}
		case ':':
		{
			//alingment
			while (currPos & 0x3)
			{
				key[currPos++] = 0;
			}

			break;
		}
		case '$':
		{
			*(uint32*)&key[currPos] = docID;

			currPos += 4;

			setPathIndexes(arrayPos, indexes, key, level, currPos);

			print(key, currPos, level);

			bool needQuotas = true;

			//get tail
			uchar8 valueType;
			
			//char* attrValue = attrValuesPool.fromSerPointer(ha2.getValueByKey((uint32*)key, currPos, valueType));
			char* attrValue;

			while (blockReaders.load());

			amountReaders++;
			
			if (!pTran->TranType) //READONLY_TRAN
			{
				//1. Read data check blocking (ha2)
				attrValue = has2[pTran->CollID]->attrValuesPool.fromSerPointer(has2[pTran->CollID]->getValueByKey((uint32*)key, currPos, valueType, 1, pTran->TranID));
			}
			else
			{
				if (pTran->TranType == READ_COMMITED_TRAN)
				{
					attrValue = has2[pTran->CollID]->attrValuesPool.fromSerPointer(pTran->getValueByKey2((uint32*)key, currPos, valueType, 1)); //1. Read data with check blocking (ha2)
				}
				else //pTran->TranType == REPEATABLE_READ_TRAN || pTran->TranType == SNAPSHOT_TRAN
				{
					//2. Read data and check blocking and block with put to array blocked cell (ha2)
					attrValue = has2[pTran->CollID]->attrValuesPool.fromSerPointer(pTran->getValueByKey2((uint32*)key, currPos, valueType, 2, &pTran->readedList));
				}
			}

			amountReaders--;

			//copy value
			if (attrValue)
			{
				attrValue += 5; //reserve 4 bytes for ha1DocIndex and 1 bytes for amount indexes

				if ((attrValue[0] == 't' && strcmp(attrValue, "true") == 0) ||
					(attrValue[0] == 'f' && strcmp(attrValue, "false") == 0) ||
					(attrValue[0] == 'n' && strcmp(attrValue, "null") == 0))
				{
					needQuotas = false;
				}
			}

			if (onlyValue)
			{
				j = 0;

				needQuotas = false;
			}

			if (needQuotas)
			{
				jsonResult[j++] = '"';
			}

			uint32 k = 0;

			if (attrValue)
			{
				while (attrValue[k])
				{
					if (attrValue[k] == '"' ||
						attrValue[k] == '\'')
					{
						jsonResult[j++] = '\\'; //encode
					}

					jsonResult[j++] = attrValue[k++];
				}
			}

			if (needQuotas)
			{
				jsonResult[j++] = '"';
			}

			i++;

			if (onlyValue)
			{
				jsonResult[j] = 0;

				if (hasBeginTran)
				{
					pTran->clear();
				}

				return j;
			}
			else
			{
				continue;
			}
		}
		case ']':
		case '}':
		case ',':
		{
			currPos = depthPos[level];

			if (c == ',')
			{
				//increase index
				if (arrayPos[level])
				{
					arrayPos[level]++;
				}
			}
			else if (c == ']')
			{
				if (repeatPos[level])
				{
					if (arrayPos[level] < *arrayMaxPos[level])
					{
						arrayPos[level]++;

						i = repeatPos[level] + 1;

						if (level >= extractFromLevel)
						{
							jsonResult[j++] = ',';
						}

						continue;
					}
				}

				level--;
			}
			else if (c == '}')
			{
				level--;

				if (!level)
				{

				}
			}

			break;
		}
		default:
			break;
		}

		if (level >= extractFromLevel)
		{
			//copy
			jsonResult[j++] = c;
		}

		i++;
	}

	jsonResult[j] = 0;

	if (hasBeginTran)
	{
		pTran->clear();
	}
	else
	{
		pTran->CollID = collID; //restore default
	}
	
	return j; //size of document
}

//modify methods

uint32 DniproDB::addDocFromFile(char* fileName)
{
	BinFile* pFile = new BinFile(fileName, false, false);

	if (pFile->open())
	{
		char* pData = new char[1024];

		uint32 len = pFile->read(pData, 1024);
		pData[len] = 0;

		addDoc(pData, 0);

		delete[] pData;

		pFile->close();
	}

	return 0;
}

uint32 DniproDB::insPartDoc(char* json,
						  uint32 docID,
						  uint32 tranID,
						  uint32 collID,
						  uint32* indexes)
{
	//begin tran
	HArrayTran* pTran;

	bool hasBeginTran;

	if (!tranID)
	{
		tranID = beginTran(READ_COMMITED_TRAN);

		pTran = &_trans[tranID];

		hasBeginTran = true;
	}
	else
	{
		pTran = &_trans[tranID];

		if (!pTran->TranType)
		{
			pTran->TranType = READ_COMMITED_TRAN; //not readonly tran
		}

		hasBeginTran = false;

		if (writeTranOnHDD)
		{
			pTran->IsWritable = true;

			//add tran log
			addTranLog('i', json, docID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity, indexes);

			pTran->LastWritedOnTranPage = amountWritedTranPages;
		}
	}
	
	pTran->CollID = collID;
	pTran->LastJson = json;
	
	//allocate new page
	pTran->pAttrValuesPage = has2[pTran->CollID]->attrValuesPool.checkPage(pTran->pAttrValuesPage);

	//method
	uint32 level = 0;

	uint32 depthPos[MAX_DOC_DEPTH];
	depthPos[0] = 0;

	uint32 arrayPos[MAX_DOC_DEPTH];
	arrayPos[0] = 0;

	uint32* arrayMaxPos[MAX_DOC_DEPTH];

	uint32 currPos = 0;

	char* attrValue = 0;
	uint32 attrCurrPos = 0;

	bool hasValue = false;

	uchar8 typeCommand = 1;

	char key[1024];

	uint32 i = 0;

	for (; json[i];)
	{
		char c = json[i];

		switch (c)
		{
		case '[':
		{
			depthPos[++level] = currPos;
			procAlias(json,
						key,
						currPos,
						arrayPos,
						arrayMaxPos,
						0,
						level,
						i,
						docID,
						typeCommand,
						indexes,
						pTran);

			attrCurrPos = currPos;
			
			//format attr value: ha1DocIndex(4 bytes) | attr value
			attrValue = pTran->pAttrValuesPage->Values + pTran->pAttrValuesPage->CurrPos;

			pTran->pAttrValuesPage->CurrPos += 5; //reserve 4 bytes for ha1DocIndex and 1 bytes for amount indexes

			break;
		}
		case '{':
		{
			depthPos[++level] = currPos;
			arrayPos[level] = 0;

			break;
		}
		case '"':
		case '\'':
		{
			for (i++; json[i] != '"' && json[i] != '\''; i++)
			{
				if (json[i] == '\\')
				{
					i++;
				}

				if (attrCurrPos)
				{
					pTran->pAttrValuesPage->Values[pTran->pAttrValuesPage->CurrPos++] = json[i];
				}

				key[currPos++] = json[i];
			}

			hasValue = true;

			break;
		}
		case '0':case'1':case'2':case'3':case'4':case'5':case'6':case'7':case'8':case'9':
		{
			for (; ('0' <= json[i] && json[i] <= '9') || json[i] == '.'; i++)
			{
				if (attrCurrPos)
				{
					pTran->pAttrValuesPage->Values[pTran->pAttrValuesPage->CurrPos++] = json[i];
				}

				key[currPos++] = json[i];
			}

			hasValue = true;

			continue;
		}
		case 't':
		case 'f':
		case 'n':
		{
			uint32 len = (c == 't' || c == 'n' ? 4 : 5);

			for (uint32 j = 0; j < len; i++, j++)
			{
				if (attrCurrPos)
				{
					pTran->pAttrValuesPage->Values[pTran->pAttrValuesPage->CurrPos++] = json[i];
				}

				key[currPos++] = json[i];
			}

			hasValue = true;

			continue;
		}
		case ':':
		{
			//alingment
			while (currPos & 0x3)
			{
				key[currPos++] = 0;
			}

			attrCurrPos = currPos;
			
			//format attr value: ha1DocIndex(4 bytes) | attr value
			attrValue = pTran->pAttrValuesPage->Values + pTran->pAttrValuesPage->CurrPos;
			 
			pTran->pAttrValuesPage->CurrPos += 5; //reserve 4 bytes for ha1DocIndex and 1 bytes for amount indexes

			hasValue = false;

			break;
		}
		case '}':
		case ']':
		case ',':
		{
			if (hasValue)
			{
				//alingment
				while (currPos & 0x3)
				{
					key[currPos++] = 0;
				}

				//set last symbols
				setPathIndexes(arrayPos, indexes, key, level, currPos);

				print(key, currPos, level);

				/*if(lastDocID == 99991)
				{
				lastDocID = lastDocID;
				}*/

				//if (pTran)
				//{
					
				pTran->insertOrDelete1((uint32*)key,
									   currPos,
									   docID,
									   (uint32*)attrValue,
									   Inserted,
									   pTran->TranID); //first 4 bytes will update later
				
				//}
				//else
				//{
				//	ha1.insert((uint32*)key, currPos, docID);
				//}

				/*if(lastDocID >= 10000)
				{
				ValueList* pvl = getDocsByAttr("{'ServicePlanID':9999}");

				if(!pvl)
				{
				pvl = pvl;
				}
				}*/

				//invert index
				*(uint32*)&key[attrCurrPos] = docID;

				attrCurrPos += 4;

				*(uchar8*)(attrValue + 4) = setPathIndexes(arrayPos, indexes, key, level, attrCurrPos);

				print(key, attrCurrPos, level);

				pTran->pAttrValuesPage->Values[pTran->pAttrValuesPage->CurrPos++] = 0;

				//if (pTran)
				//{
				
				//key has format: path + docID + indexes | ha1docIndex + amountIndexes + value
				pTran->insertOrDelete2((uint32*)key,
										attrCurrPos,
										has2[pTran->CollID]->attrValuesPool.toSerPointer(pTran->pAttrValuesPage, attrValue),
										Inserted,
										pTran->TranID);
				//}
				//else
				//{
				//	ha2.insert((uint32*)key,
				//		attrCurrPos,
				//		attrValuesPool.toSerPointer(attrValue));
				//}

				print(key, attrCurrPos, level);

				pTran->pAttrValuesPage = has2[pTran->CollID]->attrValuesPool.checkPage(pTran->pAttrValuesPage);

				attrCurrPos = 0;
			}

			//update level
			switch (c)
			{
			case ']':
			{
				if (attrCurrPos != currPos) //is not empty array
				{
					if (arrayPos[level] > *arrayMaxPos[level])
					{
						*arrayMaxPos[level] = arrayPos[level];
					}
				}
			}
			case '}':
			{
				currPos = depthPos[--level];
				break;
			}
			case ',':
			{
				currPos = depthPos[level];

				//increase index
				if (arrayPos[level])
				{
					attrCurrPos = currPos;
					
					//format attr value: ha1DocIndex(4 bytes) | attr value
					attrValue = pTran->pAttrValuesPage->Values + pTran->pAttrValuesPage->CurrPos;

					pTran->pAttrValuesPage->CurrPos += 5; //reserve 4 bytes for ha1DocIndex and 1 bytes for amount indexes

					arrayPos[level]++;

					if (arrayPos[level] > *arrayMaxPos[level])
					{
						*arrayMaxPos[level] = arrayPos[level];
					}
				}

				break;
			}
			default:
				break;
			}

			hasValue = false;

			break;
		}
		default:
			break;
		}

		i++;
	}

	//commit tran
	if (hasBeginTran)
	{
		pTran->IsWritable = false;

		//add tran log
		if (writeTranOnHDD)
		{
			addTranLogWithCommit('i', json, docID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity, indexes);
		}

		commitTran(tranID);
	}

	return i;
}

uint32 DniproDB::updPartDoc(char* json,
						uint32 docID,
						uint32 tranID,
						uint32 collID,
						uint32* indexes,
						bool onlyDelete)
{
	//begin tran
	HArrayTran* pTran;

	bool hasBeginTran;

	if (!tranID)
	{
		tranID = beginTran(READ_COMMITED_TRAN);

		pTran = &_trans[tranID];

		hasBeginTran = true;
	}
	else
	{
		pTran = &_trans[tranID];

		if (!pTran->TranType)
		{
			pTran->TranType = READ_COMMITED_TRAN; //not readonly tran
		}

		hasBeginTran = false;

		if (writeTranOnHDD)
		{
			pTran->IsWritable = true;

			//add tranLog
			if (!onlyDelete)
			{
				addTranLog('u', json, docID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity, indexes);
			}
			else
			{
				addTranLog('d', json, docID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity, indexes);
			}

			pTran->LastWritedOnTranPage = amountWritedTranPages;
		}
	}

	pTran->CollID = collID;
	
	pTran->LastJson = json;
		
	//allocate new page
	pTran->pAttrValuesPage = has2[pTran->CollID]->attrValuesPool.checkPage(pTran->pAttrValuesPage);

	//method
	uint32 level = 0;
	uint32 depthPos[MAX_DOC_DEPTH];
	depthPos[0] = 0;

	uint32 arrayPos[MAX_DOC_DEPTH];
	arrayPos[0] = 0;

	uint32 currPos = 0;

	char* attrValue = 0;
	uint32 attrCurrPos = 0;

	bool hasValue = false;

	uchar8 typeCommand;

	char key[1024];

	uint32 i = 0;

	for (; json[i];)
	{
		char c = json[i];

		switch (c)
		{
			case '[':
			{
				depthPos[++level] = currPos;
				procAlias(json,
							key,
							currPos,
							arrayPos,
							0,
							0,
							level,
							i,
							0,
							typeCommand,
							indexes,
							pTran);

				break;
			}
			case '{':
			{
				depthPos[++level] = currPos;
				arrayPos[level] = 0;

				break;
			}
			case '"':
			case '\'':
			{
				for (i++; json[i] != '"' && json[i] != '\''; i++)
				{
					if (json[i] == '\\')
					{
						i++;
					}

					if (attrCurrPos)
					{
						pTran->pAttrValuesPage->Values[pTran->pAttrValuesPage->CurrPos++] = json[i];
					}

					key[currPos++] = json[i];
				}

				hasValue = true;

				break;
			}
			case '0':case'1':case'2':case'3':case'4':case'5':case'6':case'7':case'8':case'9':
			{
				for (; ('0' <= json[i] && json[i] <= '9') || json[i] == '.'; i++)
				{
					if (attrCurrPos)
					{
						pTran->pAttrValuesPage->Values[pTran->pAttrValuesPage->CurrPos++] = json[i];
					}

					key[currPos++] = json[i];
				}

				hasValue = true;

				continue;
			}
			case 't':
			case 'f':
			case 'n':
			{
				uint32 len = (c == 't' || c == 'n' ? 4 : 5);

				for (uint32 j = 0; j < len; i++, j++)
				{
					if (attrCurrPos)
					{
						pTran->pAttrValuesPage->Values[pTran->pAttrValuesPage->CurrPos++] = json[i];
					}

					key[currPos++] = json[i];
				}

				hasValue = true;

				continue;
			}
			case ':': //delete item
			{
				//alingment
				while (currPos & 0x3)
				{
					key[currPos++] = 0;
				}

				attrCurrPos = currPos;

				//format attr value: ha1DocIndex(4 bytes) | attr value
				attrValue = pTran->pAttrValuesPage->Values + pTran->pAttrValuesPage->CurrPos;

				pTran->pAttrValuesPage->CurrPos += 5; //reserve 4 bytes for ha1DocIndex and 1 bytes for amount indexes

				//get value
				*(uint32*)&key[currPos] = docID;

				currPos += 4;

				//add indexes
				setPathIndexes(arrayPos, indexes, key, level, currPos);

				print(key, currPos, level);

				uchar8 valueType;

				//char* attrVal = attrValuesPool.fromSerPointer(ha2.getValueByKey((uint32*)key, currPos, valueType));
				char* delAttrVal;
				
				/*while (blockReaders.load());

				amountReaders++;*/

				//if (pTran->TranType)
				//{
				
				//attrVal = attrValuesPool.fromSerPointer(pTran->getValueByKey(false, (uint32*)key, currPos, valueType));
				//delAttrVal = attrValuesPool.fromSerPointer(has2[pTran->CollID]->getValueByKey((uint32*)key, currPos, valueType, 0, pTran->TranID));

				//delAttrVal = attrValuesPool.fromSerPointer(pTran->getValueByKey2((uint32*)key, currPos, valueType, 0));

				//optimization for SELFTEST
				delAttrVal = has2[pTran->CollID]->attrValuesPool.fromSerPointer(has2[pTran->CollID]->getValueByKey((uint32*)key, currPos, valueType, 0, pTran->TranID));

				//}
				//else
				//{
				//	attrVal = attrValuesPool.fromSerPointer(ha2.getValueByKey((uint32*)key, currPos, valueType));
				//}

				//amountReaders--;

				if (delAttrVal)
				{
					char* delAttrValStr = delAttrVal;
					delAttrValStr += 5; //skip ha1DocIndex
					
					if (delAttrValStr[0])
					{
						currPos = attrCurrPos;

						//copy value
						for (uint32 k = 0; delAttrValStr[k]; k++, currPos++)
						{
							key[currPos] = delAttrValStr[k];
						}

						//delete
						pTran->insertOrDelete2((uint32*)key, currPos, 0, Deleted, pTran->TranID);
												
						//alingment
						while (currPos & 0x3)
						{
							key[currPos++] = 0;
						}

						//add indexes
						setPathIndexes(arrayPos, indexes, key, level, currPos);

						print(key, currPos, level);

						//delete
						//if (pTran)
						//{

						pTran->insertOrDelete1((uint32*)key,
												currPos,
												docID,
												(uint32*)delAttrVal,
												Deleted,
												pTran->TranID);

						//}
						//else
						//{
						//	ha1.detValueByKey((uint32*)key, currPos, docID);
						//}
					}
				}

				currPos = attrCurrPos;

				break;
			}
			case '}':
			case ']':
			case ',': //insert item
			{
				if (hasValue && !onlyDelete)
				{
					//alingment
					while (currPos & 0x3)
					{
						key[currPos] = 0;

						currPos++;
					}

					//add indexes
					setPathIndexes(arrayPos, indexes, key, level, currPos);

					print(key, currPos, level);

					//if (pTran)
					//{
					
					pTran->insertOrDelete1((uint32*)key,
											currPos,
											docID,
											(uint32*)attrValue,
											Inserted,
											pTran->TranID);
					
					//}
					//else
					//{
					//	ha1.insert((uint32*)key, currPos, docID);
					//}

					pTran->pAttrValuesPage->Values[pTran->pAttrValuesPage->CurrPos++] = 0;

					//invert index
					*(uint32*)&key[attrCurrPos] = docID;

					attrCurrPos += 4;

					*(uchar8*)(attrValue + 4) = setPathIndexes(arrayPos, indexes, key, level, attrCurrPos);

					print(key, currPos, level);

					//if (pTran)
					//{
					
					//key has format: path + docID + indexes | ha1docIndex + amountIndexes + value
					pTran->insertOrDelete2((uint32*)key,
											attrCurrPos,
											has2[pTran->CollID]->attrValuesPool.toSerPointer(pTran->pAttrValuesPage, attrValue),
											Inserted,
											pTran->TranID);

					//}
					//else
					//{
					//	ha2.insert((uint32*)key,
					//		attrCurrPos,
					//		attrValuesPool.toSerPointer(attrValue));
					//}

					currPos = depthPos[level];

					attrCurrPos = 0;

					hasValue = false;

					pTran->pAttrValuesPage = has2[pTran->CollID]->attrValuesPool.checkPage(pTran->pAttrValuesPage);
				}

				//update level
				switch (c)
				{
					case '}':
					case ']':
					{
						currPos = depthPos[--level];
						break;
					}
					case ',':
					{
						currPos = depthPos[level];

						//increase index
						if (arrayPos[level])
						{
							attrCurrPos = currPos;
							
							//format attr value: ha1DocIndex(4 bytes) | attr value
							attrValue = pTran->pAttrValuesPage->Values + pTran->pAttrValuesPage->CurrPos;

							pTran->pAttrValuesPage->CurrPos += 5; //reserve 4 bytes for ha1DocIndex and 1 bytes for amount indexes

							arrayPos[level]++;
						}

						break;
					}
					default:
						break;
				}

				break;
			}
			default:
			{
				break;
			}
		}

		i++;
	}

	//commit tran
	if (hasBeginTran)
	{
		if (writeTranOnHDD)
		{
			pTran->IsWritable = false;

			if (!onlyDelete)
			{
				addTranLogWithCommit('u', json, docID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity, indexes);
			}
			else
			{
				addTranLogWithCommit('d', json, docID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity, indexes);
			}
		}
		
		commitTran(tranID);
	}

	return i;
}