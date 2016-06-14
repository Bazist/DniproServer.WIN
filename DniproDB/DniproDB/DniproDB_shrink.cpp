#include "DniproDB.h"

uint DniproDB::getAmountKeyIndexes(HArrayVarRAM* pHA,
									uint* key,
									uint keyLen,
									uint& fullKeyLen,
									uint* indexes)
{
	uint amountKeyIndexes = 0;

	for (uint i = keyLen - 1; i >= 0; i--, amountKeyIndexes++, fullKeyLen -= 4)
	{
		indexes[amountKeyIndexes] = key[i];

		key[i] = 0;

		uchar valueType;

		//this key should contain count of array items
		uint arrayCount = pHA->getValueByKey(key, fullKeyLen, valueType);

		if (arrayCount == 0)
		{
			key[i] = indexes[amountKeyIndexes]; //it's docID, repair key

			break;
		}
	}

	return amountKeyIndexes;
}

bool DniproDB::shrinkKeyHA(uint* key,
							uint keyLen,
							uint value,
							uchar valueType,
							void* pData)
{
	if (value)
	{
		ScanShrinkKeyData* pScanKeyData = (ScanShrinkKeyData*)pData;

		uint fullKeyLen = (keyLen << 2); //keyLen * 4

		char* attrValue = pScanKeyData->pOldHA2->attrValuesPool.fromSerPointer(value);

		//get free page
		AttrValuesPage* pAttrValuesPage = pScanKeyData->pNewHA2->attrValuesPool.checkPage(0);

		//get start new string
		char* charNewValue = pAttrValuesPage->Values + pAttrValuesPage->CurrPos;
		uint newValue = pScanKeyData->pNewHA2->attrValuesPool.toSerPointer(pAttrValuesPage, charNewValue);

		if (key[keyLen - 1]) //it's normal key
		{
			char* charKey = (char*)key;

			//reservce first 4 bytes for index in ha1 KeyValueList
			pAttrValuesPage->CurrPos += 4;

			//insert value in ha2
			pScanKeyData->pNewHA2->insert(key, fullKeyLen, newValue);

			uint indexes[MAX_DOC_DEPTH];

			uint amountKeyIndexes = getAmountKeyIndexes(pScanKeyData->pOldHA2, key, keyLen, fullKeyLen, indexes);

			uint docIDKeyPos = keyLen - amountKeyIndexes - 1;

			uint docID = key[docIDKeyPos];

			fullKeyLen -= 4; //override docID

			//copy string
			for (uint i = 4; attrValue[i]; i++, pAttrValuesPage->CurrPos++, fullKeyLen++)
			{
				pAttrValuesPage->Values[pAttrValuesPage->CurrPos] = attrValue[i];

				charKey[fullKeyLen] = attrValue[i];
			}

			pAttrValuesPage->Values[pAttrValuesPage->CurrPos++] = 0;

			//alingment
			while (fullKeyLen & 0x3)
			{
				charKey[fullKeyLen++] = 0;
			}

			//add indexes
			keyLen = (fullKeyLen >> 2);

			for (uint i = 0; i < amountKeyIndexes; i++, keyLen++, fullKeyLen += 4)
			{
				key[keyLen] = indexes[i];
			}

			//insert value in ha1
			*(uint*)charNewValue = pScanKeyData->pNewHA1->insert(key, fullKeyLen, docID);

			//restore key
			key[docIDKeyPos] = docID;

			for (uint i = 0; i < amountKeyIndexes; i++)
			{
				key[++docIDKeyPos] = indexes[i];
			}
		}
		else //if it's count of array items, just insert as is
		{
			*(uint*)(pAttrValuesPage->Values + pAttrValuesPage->CurrPos) = *(uint*)attrValue;

			//reservce first 4 bytes for index in ha1 KeyValueList
			pAttrValuesPage->CurrPos += 4;

			pScanKeyData->pNewHA2->insert(key, fullKeyLen, newValue);
		}

		//free page
		pAttrValuesPage->Type.store(0);
	}

	return true;
}

uint DniproDB::shrink()
{
	ulong beforeMemory = getUsedMemory();

	uint key[32];

	for (uint i = 0; i < countColls; i++)
	{
		ScanShrinkKeyData scanKeyData;

		//init
		scanKeyData.pNewHA1 = createHA1(has1[i]->Name);
		scanKeyData.pNewHA2 = createHA2(has2[i]->Name);

		scanKeyData.pOldHA2 = has2[i];

		//scan
		has2[i]->scanKeysAndValues(key, &shrinkKeyHA, &scanKeyData);

		//destroy
		has1[i]->destroy();
		has2[i]->destroy();

		delete has1[i];
		delete has2[i];

		//copy
		has1[i] = scanKeyData.pNewHA1;
		has2[i] = scanKeyData.pNewHA2;
	}

	ulong afterMemory = getUsedMemory();

	return (100 - (afterMemory * 100 / beforeMemory));
}