#include "DniproDB.h"

bool DniproDB::getDocsByAttr_scanKey(uint* key,
									 uint keyLen,
									 uint value,
									 uchar valueType,
									 void* pData)
{
	ScanKeyData* pScanKeyData = (ScanKeyData*)pData;

	if (pScanKeyData->pTran && pScanKeyData->pTran->hasKeyAndValue1(key, keyLen, value, Deleted)) //is deleted in tran
	{
		return true; 
	}

	uint* indexes = pScanKeyData->pIndexesPool->newObject(keyLen - pScanKeyData->KeyLen + 1);

	uint j=0;
	for(uint i=pScanKeyData->KeyLen; i<keyLen; i++, j++)
	{
		indexes[j] = key[i];
	}

	indexes[j] = 0;

	if(valueType == VALUE_LIST_TYPE)
	{
		ValueList* pValueList = pScanKeyData->pValueListPool->fromSerPointer(value);

		for(uint i=0; i<pValueList->Count; i++)
		{
			uint currDocID = pValueList->pValues[i];
			if(currDocID && (pScanKeyData->DocID == 0 || pScanKeyData->DocID == currDocID))
			{
				pScanKeyData->pValueList->addValue(currDocID, false);
				pScanKeyData->pIndexes->addValue((uint)indexes, false);
			}
		}
	}
	else
	{
		uint currDocID = value;
		if(currDocID && (pScanKeyData->DocID == 0 || pScanKeyData->DocID == currDocID))
		{
			pScanKeyData->pValueList->addValue(currDocID, false);
			pScanKeyData->pIndexes->addValue((uint)indexes, false);
		}
	}

	return true;
}

bool DniproDB::getDocsByAttr_scanIn(uint* key,
									uint keyLen,
									uint value,
									uchar valueType,
									void* pData)
{
	ScanKeyData* pScanKeyData = (ScanKeyData*)pData;

	if (pScanKeyData->pTran && pScanKeyData->pTran->hasKeyAndValue1(key, keyLen, value, Deleted)) //is deleted in tran
	{
		return true;
	}

	if(valueType == VALUE_LIST_TYPE)
	{
		ValueList* pValueList = pScanKeyData->pValueListPool->fromSerPointer(value);

		for(uint i=0; i<pValueList->Count; i++)
		{
			uint currDocID = pValueList->pValues[i];
			
			pScanKeyData->pValueList->addUniqueValue(currDocID);
		}
	}
	else
	{
		uint currDocID = value;

		pScanKeyData->pValueList->addUniqueValue(currDocID);
	}

	return true;
}

bool DniproDB::getDocsByAttr_scanCond(uint* key,
									  uint keyLen,
									  uint value,
									  uchar valueType,
									  void* pData)
{
	ScanKeyData* pScanKeyData = (ScanKeyData*)pData;

	if (pScanKeyData->pTran && pScanKeyData->pTran->hasKeyAndValue1(key, keyLen, value, Deleted)) //is deleted in tran
	{
		return true;
	}
	
	//null terminated
	key[keyLen] = 0;

	//get current value
	char* operandStr = (char*)&key[pScanKeyData->KeyLen]; //rest of the key is value
	uint operandLen = strlen(operandStr);

	//apply condition
	switch(pScanKeyData->MethodType)
	{
		case 1: //<
		{
			if(operandLen < pScanKeyData->Operand1Len)
			{
				break;
			}
			else if(operandLen > pScanKeyData->Operand1Len)
			{
				return true;
			}
			else if(atoi(operandStr) < pScanKeyData->Operand1)
			{
				break;
			}
			else
			{
				return true;
			}
		}
		case 2: //<=
		{
			if(operandLen < pScanKeyData->Operand1Len)
			{
				break;
			}
			else if(operandLen > pScanKeyData->Operand1Len)
			{
				return true;
			}
			else if(atoi(operandStr) <= pScanKeyData->Operand1)
			{
				break;
			}
			else
			{
				return true;
			}
		}
		case 3: //>
		{
			if(operandLen < pScanKeyData->Operand1Len)
			{
				return true;
			}
			else if(operandLen > pScanKeyData->Operand1Len)
			{
				break;
			}
			else if(atoi(operandStr) > pScanKeyData->Operand1)
			{
				break;
			}
			else
			{
				return true;
			}
		}
		case 4: //>=
		{
			if(operandLen < pScanKeyData->Operand1Len)
			{
				return true;
			}
			else if(operandLen > pScanKeyData->Operand1Len)
			{
				break;
			}
			else if(atoi(operandStr) >= pScanKeyData->Operand1)
			{
				break;
			}
			else
			{
				return true;
			}
		}
		case 5: //template
		{
			for(uint i=0; pScanKeyData->Operand1Str[i] != '*' && operandStr[i] != 0; i++)
			{
				if(pScanKeyData->Operand1Str[i] != operandStr[i])
					return true;
			}

			break;
		}
		case 6: //range
		{
			if(pScanKeyData->Operand1Len <= operandLen && operandLen <= pScanKeyData->Operand2Len)
			{
				uint operand = atoi(operandStr);
			
				if(pScanKeyData->Operand1 <= operand && operand <= pScanKeyData->Operand2)
				{
					break;
				}
				else
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}
		case 7: //not
		{
			if(strcmp(pScanKeyData->Operand1Str, operandStr) != 0)
			{
				break;
			}
			else
			{
				return true;
			}
		}
	}

	//add docs
	if(valueType == VALUE_LIST_TYPE)
	{
		ValueList* pValueList = pScanKeyData->pValueListPool->fromSerPointer(value);

		for(uint i=0; i<pValueList->Count; i++)
		{
			pScanKeyData->pValueList->addValue(pValueList->pValues[i]);
		}
	}
	else
	{
		pScanKeyData->pValueList->addValue(value);
	}

	return true;
}