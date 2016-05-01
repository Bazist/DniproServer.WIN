#include "DniproDB.h"

void DniproDB::procAlias(char* json,
						char* key,
						uint currPos,
						uint* arrayPos,
						uint** arrayMaxPos,
						uint* repeatPos,
						uint level,
						uint& i,
						uint docID,
						uchar& typeCommand,
						uint* indexes,
						HArrayTran* pTran)
{
	arrayPos[level] = 1; //by default

	if (arrayMaxPos)
	{
		//set count in array, format: path + docID + indexes (0) =========================================
		//add docID
		*(uint*)&key[currPos] = docID;

		//add indexes
		currPos += 4;

		setPathIndexes(arrayPos, indexes, key, level, currPos);

		*(uint*)&key[currPos - 4] = 0; //last index is zero (Array.Count)

		//get reference
		uchar valueType;

		//uint serPointer = ha2.getValueByKey((uint*)key, currPos, valueType);
		uint serPointer;
		
		while (blockReaders.load());

		amountReaders++;

		if (!pTran->TranType) //READ_ONLY_TRAN
		{
			serPointer = has2[pTran->CollID]->getValueByKey((uint*)key, currPos, valueType, 1); //1. Read data with check blocking (ha2)
		}
		else 
		{
			if (pTran->TranType == READ_COMMITED_TRAN)
			{
				serPointer = has2[pTran->CollID]->getValueByKey((uint*)key, currPos, valueType, 1); //1. Read data with check blocking (ha2)
			}
			else //pTran->TranType == REPEATABLE_READ_TRAN || pTran->TranType == SNAPSHOT_TRAN
			{
				serPointer = has2[pTran->CollID]->getValueByKey((uint*)key,
																currPos,
																valueType,
																2,
																pTran->TranID,
																&pTran->readedList); //2. Read data and check blocking and block with put to array blocked cell (ha2)
			}
		}

		amountReaders--;

		print(key, currPos, level);

		uint* pValue = 0;

		if (!serPointer)
		{
			if (typeCommand) //ins part doc, create array
			{
				pValue = (uint*)(pTran->pAttrValuesPage->Values + pTran->pAttrValuesPage->CurrPos);

				pTran->pAttrValuesPage->CurrPos += 4;

				*pValue = 0;

				pTran->insertOrDelete2((uint*)key,
									   currPos,
									   attrValuesPool.toSerPointer(pTran->pAttrValuesPage, (char*)pValue),
									   1,
									   pTran->TranID);
			}
		}
		else
		{
			if (typeCommand) //ins part doc, amount of array items will be modified
			{
				//need clone Arr.Count value, to create new version of value
				pValue = (uint*)(pTran->pAttrValuesPage->Values + pTran->pAttrValuesPage->CurrPos);

				pTran->pAttrValuesPage->CurrPos += 4;

				//clone
				*pValue = *(uint*)attrValuesPool.fromSerPointer(serPointer);

				pTran->insertOrDelete2((uint*)key,
										currPos,
										attrValuesPool.toSerPointer(pTran->pAttrValuesPage, (char*)pValue),
										1,
										pTran->TranID);

			}
			else //get part doc
			{
				pValue = (uint*)attrValuesPool.fromSerPointer(serPointer);
			}
		}

		arrayMaxPos[level] = pValue;
	}

	//==========================================================================================
	char nc = json[i + 1];
	if (nc == '@' &&
		('0' <= json[i + 2] && json[i + 2] <= '9'))
	{
		uint num = 0;

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
	else if (nc == 'C') //Count
	{
		arrayPos[level] = 0;

		i += 6;
	}
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

		if (arrayMaxPos[level]) //possible only if there is array
		{
			repeatPos[level] = i;
		}
	}
	else if (nc == 'P') //Portion
	{
		i += 3; //skip P,

		//read numbers
		uint j = 0;
		char num[2][10];

		for (uint k = 0; k < 2; k++)
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

void DniproDB::setPathIndexes(uint* arrayPos,
								uint* indexes,
								char* key,
								uint level,
								uint& currPos)
{
	uint currIndex = 0;

	for (uint i = 1; i <= level; i++)
	{
		if (arrayPos[i])
		{
			if (indexes && indexes[currIndex])
			{
				*(uint*)&key[currPos] = indexes[currIndex];

				currIndex++;
			}
			else
			{
				*(uint*)&key[currPos] = arrayPos[i];
			}

			currPos += 4;
		}
	}
}

ValueList* DniproDB::getDocsByAttr(char* json,
									uint docID ,
									uint tranID,
									uint collID,
									ValueList** pIndexes)
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

	uint level = 0;
	uint depthPos[10];
	depthPos[0] = 0;

	uint arrayPos[10];
	arrayPos[0] = 0;

	uint arrayLevel = 0;

	uint* arrayMaxPos[10];

	uint currPos = 0;

	bool hasValue = false;

	uchar typeCommand = 0;

	ValueList* pValueListCommand[16];
	ValueList* pIndexesListCommand[16];

	uint valueListCommandCount = 0;

	ScanKeyData scanKeyData;
	if (pIndexes)
	{
		//*pIndexes = valueListPool.newObject();

		//scanKeyData.pValueList = valueListPool.newObject();
										//scanKeyData.pIndexes = *pIndexes;

		scanKeyData.DocID = docID;
		scanKeyData.pValueListPool = &has1[pTran->CollID]->valueListPool;
		scanKeyData.pIndexesPool = &pTran->indexesPool;
		scanKeyData.pTran = pTran;
	}

	/*scanKeyData.Scans = 0;
									scanKeyData.Bitmap = new char[100000];
									memset(scanKeyData.Bitmap, 0, 100000);*/

	uint* arrayIndex = 0;

	char key[1000];

	for (uint i = 0; json[i];)
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

		//		for(uint j=0; j < pValueList->Count; j++)
										//		{
										//			if(pValueList->pValues[j])
										//			{
										//				uint newCurrPos = currPos + getAttrValueByDocID("{'pk':$}",
										//								  								key + currPos,
										//																pValueList->pValues[j]);

		//				//alingment
										//				while(newCurrPos & 0x3)
										//				{
										//					key[newCurrPos++] = 0;
										//				}

		//				uchar valueType;
										//				uint value = ha1.getValueByKey((uint*)key, newCurrPos, valueType);

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
			uint k1 = 0;
			uint k2 = 0;

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
			has1[pTran->CollID]->scanKeysAndValues((uint*)key,
														currPos,
														&getDocsByAttr_scanCond,
														&scanKeyData);

			amountReaders--;

			if (pTran->TranType) //only for not readonly trans
			{
				//scan tran
				pTran->scanKeysAndValues1((uint*)key,
					currPos,
					&getDocsByAttr_scanCond,
					&scanKeyData);
			}

			/*for(uint j=0; j<100000; j++)
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
			uint len = (c == 't' || c == 'n' ? 4 : 5);

			for (uint j = 0; j < len; i++, j++)
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

						uchar valueType;
						//uint value = ha1.getValueByKey((uint*)key, currPos, valueType);

						uint value;

						while (blockReaders.load());

						amountReaders++;
						
						if (pTran->TranType) //only for not readonly trans
						{
							value = pTran->getValueByKey1((uint*)key, currPos, valueType);
						}
						else
						{
							value = has1[pTran->CollID]->getValueByKey((uint*)key, currPos, valueType);
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
								pValueList->addValue((uint)value);

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
						has1[pTran->CollID]->scanKeysAndValues((uint*)key,
																	currPos,
																	getDocsByAttr_scanIn,
																	&scanKeyData);

						amountReaders--;

						if (pTran->TranType) //only for not readonly trans
						{
							//scan tran
							pTran->scanKeysAndValues1((uint*)key,
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
					has1[pTran->CollID]->scanKeysAndValues((uint*)key,
															currPos,
															getDocsByAttr_scanKey,
															&scanKeyData);

					amountReaders--;

					if (pTran->TranType) //only for not readonly trans
					{
						//scan store
						pTran->scanKeysAndValues1((uint*)key,
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
				for (uint i = 0; i < pValueListCommand[0]->Count; i++)
				{
					for (uint j = 1; j < valueListCommandCount; j++)
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
				uint lastIdx = valueListCommandCount - 1; //last because it is deepest level, and it should be much all previous levels

				for (uint i = 0; i < pValueListCommand[lastIdx]->Count; i++) //each doc
				{
					for (uint j = 0; j < lastIdx; j++) //and each doc
					{
						bool bFind = false;

						for (uint k = 0; k < pValueListCommand[j]->Count; k++) //any value
						{
							if (pValueListCommand[j]->pValues[k] == pValueListCommand[lastIdx]->pValues[i]) //check DocID
							{
								//check indexes
								uint* indexes1 = (uint*)pIndexesListCommand[lastIdx]->pValues[i];
								uint* indexes2 = (uint*)pIndexesListCommand[j]->pValues[k];

								for (uint l = 0; indexes2[l]; l++)
								{
									if (indexes1[l] != indexes2[l])
									{
										goto NOT_FOUND;
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
			for (uint i = 1; i < valueListCommandCount; i++)
			{
				for (uint j = 0; j < pValueListCommand[i]->Count; j++)
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

uint DniproDB::getPartDoc(char* jsonTemplate,
						  char* jsonResult,
						  uint rowNumOrDocID,
						  uint tranID,
						  uint collID,
						  bool onlyValue,
						  ValueList** pDocIDs,
						  uint* indexes,
						  uchar* collIDs)
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

	uint docID = 0;
	uint tableIndex = 0;

	uint level = 0;

	uint depthPos[10];
	depthPos[0] = 0;

	uint repeatPos[10];
	repeatPos[0] = 0;

	uint arrayPos[10];
	arrayPos[0] = 0;

	uint* arrayMaxPos[10];

	uint currPos = 0;

	uint extractFromLevel = 0;

	bool hasValue = false;

	uchar typeCommand = 0;

	char key[1024];

	uint j = 0;
	for (uint i = 0; jsonTemplate[i];)
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
			if (!arrayMaxPos[level])
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

					jsonResult[j++] = 'n';
					jsonResult[j++] = 'u';
					jsonResult[j++] = 'l';
					jsonResult[j++] = 'l';

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

			hasValue = true;

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

			hasValue = true;

			continue;
		}
		case 't':
		case 'f':
		case 'n':
		{
			uint len = (c == 't' || c == 'n' ? 4 : 5);

			for (uint j = 0; j < len; i++, j++)
			{
				key[currPos++] = jsonTemplate[i];

				if (level >= extractFromLevel)
				{
					jsonResult[j++] = jsonTemplate[i];
				}
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
		case '$':
		{
			*(uint*)&key[currPos] = docID;

			currPos += 4;

			setPathIndexes(arrayPos, indexes, key, level, currPos);

			print(key, currPos, level);

			bool needQuotas = true;

			//get tail
			uchar valueType;
			
			//char* attrValue = attrValuesPool.fromSerPointer(ha2.getValueByKey((uint*)key, currPos, valueType));
			char* attrValue;

			while (blockReaders.load());

			amountReaders++;
			
			if (!pTran->TranType) //READONLY_TRAN
			{
				//1. Read data check blocking (ha2)
				attrValue = attrValuesPool.fromSerPointer(has2[pTran->CollID]->getValueByKey((uint*)key, currPos, valueType, 1, pTran->TranID)); 
			}
			else
			{
				if (pTran->TranType == READ_COMMITED_TRAN)
				{
					attrValue = attrValuesPool.fromSerPointer(pTran->getValueByKey2((uint*)key, currPos, valueType, 1)); //1. Read data with check blocking (ha2)
				}
				else //pTran->TranType == REPEATABLE_READ_TRAN || pTran->TranType == SNAPSHOT_TRAN
				{
					//2. Read data and check blocking and block with put to array blocked cell (ha2)
					attrValue = attrValuesPool.fromSerPointer(pTran->getValueByKey2((uint*)key, currPos, valueType, 2, &pTran->readedList));
				}
			}

			amountReaders--;

			//copy value
			if (attrValue)
			{
				attrValue += 4; //skip ha1DocIndex, 4 bytes

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

			uint k = 0;

			if (attrValue)
			{
				while (attrValue[k])
				{
					jsonResult[j++] = attrValue[k++];
				}
			}

			if (needQuotas)
			{
				jsonResult[j++] = '"';
			}

			hasValue = true;

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
			hasValue = false;

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

uint DniproDB::addDocFromFile(char* fileName)
{
	BinaryFile* pFile = new BinaryFile(fileName, false, false);

	if (pFile->open())
	{
		char* pData = new char[1024];

		uint len = pFile->read(pData, 1024);
		pData[len] = 0;

		addDoc(pData, 0);

		delete[] pData;

		pFile->close();
	}

	return 0;
}

uint DniproDB::insPartDoc(char* json,
						  uint rowNumOrDocID,
						  uint tranID,
						  uint collID,
						  ValueList** pDocIDs,
						  uint* indexes)
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
			addTranLog('i', json, rowNumOrDocID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity);

			pTran->LastWritedOnTranPage = amountWritedTranPages;
		}
	}
	
	pTran->CollID = collID;
	pTran->LastJson = json;
	
	//allocate new page
	pTran->pAttrValuesPage = attrValuesPool.checkPage(pTran->pAttrValuesPage);

	//method
	uint docID = 0;
	uint tableIndex = 0;

	uint level = 0;

	uint depthPos[10];
	depthPos[0] = 0;

	uint arrayPos[10];
	arrayPos[0] = 0;

	uint* arrayMaxPos[10];

	uint currPos = 0;

	char* attrValue = 0;
	uint attrCurrPos = 0;

	bool hasValue = false;

	uchar typeCommand = 1;

	char key[1024];

	uint i = 0;

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

			pTran->pAttrValuesPage->CurrPos += 4; //reserve 4 bytes for ha1DocIndex

			break;
		}
		case '{':
		{
			//next join table
			if (!level)
			{
				if (!pDocIDs)
				{
					docID = rowNumOrDocID;
				}
				else
				{
					docID = pDocIDs[tableIndex++]->pValues[rowNumOrDocID]; //row number
				}
			}

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
			uint len = (c == 't' || c == 'n' ? 4 : 5);

			for (uint j = 0; j < len; i++, j++)
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
			 
			pTran->pAttrValuesPage->CurrPos += 4; //reserve 4 bytes for ha1DocIndex

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
					
				pTran->insertOrDelete1((uint*)key,
									   currPos,
									   docID,
									   (uint*)attrValue,
									   1,
									   pTran->TranID); //first 4 bytes will update later
				
				//}
				//else
				//{
				//	ha1.insert((uint*)key, currPos, docID);
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
				*(uint*)&key[attrCurrPos] = docID;

				attrCurrPos += 4;

				setPathIndexes(arrayPos, indexes, key, level, attrCurrPos);

				print(key, attrCurrPos, level);

				pTran->pAttrValuesPage->Values[pTran->pAttrValuesPage->CurrPos++] = 0;

				//if (pTran)
				//{
				
				pTran->insertOrDelete2((uint*)key,
										attrCurrPos,
										attrValuesPool.toSerPointer(pTran->pAttrValuesPage, attrValue),
										1,
										pTran->TranID);
				//}
				//else
				//{
				//	ha2.insert((uint*)key,
				//		attrCurrPos,
				//		attrValuesPool.toSerPointer(attrValue));
				//}

				print(key, attrCurrPos, level);

				pTran->pAttrValuesPage = attrValuesPool.checkPage(pTran->pAttrValuesPage);

				attrCurrPos = 0;

				hasValue = false;
			}

			//update level
			switch (c)
			{
			case ']':
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

					pTran->pAttrValuesPage->CurrPos += 4; //reserve 4 bytes for ha1DocIndex

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
			addTranLogWithCommit('i', json, rowNumOrDocID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity);
		}

		commitTran(tranID);
	}

	return i;
}

uint DniproDB::updPartDoc(char* json,
						uint rowNumOrDocID,
						uint tranID,
						uint collID,
						ValueList** pDocIDs,
						uint* indexes,
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
				addTranLog('u', json, rowNumOrDocID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity);
			}
			else
			{
				addTranLog('d', json, rowNumOrDocID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity);
			}

			pTran->LastWritedOnTranPage = amountWritedTranPages;
		}
	}

	pTran->CollID = collID;
	
	pTran->LastJson = json;
		
	//allocate new page
	pTran->pAttrValuesPage = attrValuesPool.checkPage(pTran->pAttrValuesPage);

	//method
	uint docID = 0;
	uint tableIndex = 0;

	uint level = 0;
	uint depthPos[10];
	depthPos[0] = 0;

	uint arrayPos[10];
	arrayPos[0] = 0;

	uint currPos = 0;

	char* attrValue = 0;
	uint attrCurrPos = 0;

	bool hasValue = false;

	uchar typeCommand;

	char key[1024];

	uint i = 0;

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
				//next join table
				if (!level)
				{
					if (!pDocIDs)
					{
						docID = rowNumOrDocID;
					}
					else
					{
						docID = pDocIDs[tableIndex++]->pValues[rowNumOrDocID]; //row number
					}
				}

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
				uint len = (c == 't' || c == 'n' ? 4 : 5);

				for (uint j = 0; j < len; i++, j++)
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

				pTran->pAttrValuesPage->CurrPos += 4; //reserve 4 bytes for ha1DocIndex

				//get value
				*(uint*)&key[currPos] = docID;

				currPos += 4;

				//add indexes
				setPathIndexes(arrayPos, indexes, key, level, currPos);

				print(key, currPos, level);

				uchar valueType;

				//char* attrVal = attrValuesPool.fromSerPointer(ha2.getValueByKey((uint*)key, currPos, valueType));
				char* delAttrVal;
				
				/*while (blockReaders.load());

				amountReaders++;*/

				//if (pTran->TranType)
				//{
				
				//attrVal = attrValuesPool.fromSerPointer(pTran->getValueByKey(false, (uint*)key, currPos, valueType));
				//delAttrVal = attrValuesPool.fromSerPointer(has2[pTran->CollID]->getValueByKey((uint*)key, currPos, valueType, 0, pTran->TranID));

				//delAttrVal = attrValuesPool.fromSerPointer(pTran->getValueByKey2((uint*)key, currPos, valueType, 0));

				//optimization for SELFTEST
				delAttrVal = attrValuesPool.fromSerPointer(has2[pTran->CollID]->getValueByKey((uint*)key, currPos, valueType, 0, pTran->TranID));

				//}
				//else
				//{
				//	attrVal = attrValuesPool.fromSerPointer(ha2.getValueByKey((uint*)key, currPos, valueType));
				//}

				//amountReaders--;

				if (delAttrVal)
				{
					char* delAttrValStr = delAttrVal;
					delAttrValStr += 4; //skip ha1DocIndex
					
					if (delAttrValStr[0])
					{
						currPos = attrCurrPos;

						//copy value
						for (uint k = 0; delAttrValStr[k]; k++, currPos++)
						{
							key[currPos] = delAttrValStr[k];
						}

						//delete
						pTran->insertOrDelete2((uint*)key, currPos, 0, 2, pTran->TranID);
												
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

						pTran->insertOrDelete1((uint*)key,
												currPos,
												docID,
												(uint*)delAttrVal,
												2,
												pTran->TranID);

						//}
						//else
						//{
						//	ha1.detValueByKey((uint*)key, currPos, docID);
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
					
					pTran->insertOrDelete1((uint*)key,
											currPos,
											docID,
											(uint*)attrValue,
											1,
											pTran->TranID);
					
					//}
					//else
					//{
					//	ha1.insert((uint*)key, currPos, docID);
					//}

					pTran->pAttrValuesPage->Values[pTran->pAttrValuesPage->CurrPos++] = 0;

					//invert index
					*(uint*)&key[attrCurrPos] = docID;

					attrCurrPos += 4;

					setPathIndexes(arrayPos, indexes, key, level, attrCurrPos);

					print(key, currPos, level);

					//if (pTran)
					//{
						
					pTran->insertOrDelete2((uint*)key,
											attrCurrPos,
											attrValuesPool.toSerPointer(pTran->pAttrValuesPage, attrValue),
											1,
											pTran->TranID);

					//}
					//else
					//{
					//	ha2.insert((uint*)key,
					//		attrCurrPos,
					//		attrValuesPool.toSerPointer(attrValue));
					//}

					currPos = depthPos[level];

					attrCurrPos = 0;

					hasValue = false;

					pTran->pAttrValuesPage = attrValuesPool.checkPage(pTran->pAttrValuesPage);
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

							pTran->pAttrValuesPage->CurrPos += 4; //reserve 4 bytes for ha1DocIndex

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
				addTranLogWithCommit('u', json, rowNumOrDocID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity);
			}
			else
			{
				addTranLogWithCommit('d', json, rowNumOrDocID, pTran->TranID, collID, pTran->ParentTranID, pTran->TranIdentity);
			}
		}
		
		commitTran(tranID);
	}

	return i;
}