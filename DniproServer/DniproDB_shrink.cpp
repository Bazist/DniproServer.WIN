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

bool DniproDB::shrinkKeyHA(uint32* key,
							uint32 keyLen,
							uint32 value,
							uchar8 valueType,
							void* pData)
{
	if (value)
	{
		ScanShrinkKeyData* pScanKeyData = (ScanShrinkKeyData*)pData;

		uint32 fullKeyLen = (keyLen << 2); //keyLen * 4

		char* attrValue = pScanKeyData->pOldHA2->attrValuesPool.fromSerPointer(value);

		//get free page
		AttrValuesPage* pAttrValuesPage = pScanKeyData->pNewHA2->attrValuesPool.checkPage(0);

		//get start new string
		char* charNewValue = pAttrValuesPage->Values + pAttrValuesPage->CurrPos;
		uint32 newValue = pScanKeyData->pNewHA2->attrValuesPool.toSerPointer(pAttrValuesPage, charNewValue);

		if (key[keyLen - 1]) //it's normal key
		{
			char* charKey = (char*)key;

			//insert value in ha2
			pScanKeyData->pNewHA2->insert(key, fullKeyLen, newValue);

			uchar8 amountShrinkKeyIndexes = *(uchar8*)(attrValue + 4);

			//reservce first 4 bytes for index in ha1 KeyValueList
			pAttrValuesPage->CurrPos += 4;

			//reserve first byte for amount indexes
			pAttrValuesPage->Values[pAttrValuesPage->CurrPos++] = amountShrinkKeyIndexes;

			uint32 indexes[MAX_DOC_DEPTH];

			for (uint32 i = 0, currKeyLen = keyLen - amountShrinkKeyIndexes; i < amountShrinkKeyIndexes; i++, currKeyLen++)
			{
				indexes[i] = key[currKeyLen];
			}

			uint32 docIDKeyPos = keyLen - amountShrinkKeyIndexes - 1;

			uint32 docID = key[docIDKeyPos];

			fullKeyLen -= ((amountShrinkKeyIndexes * 4) + 4); //override docID and indexes

			//copy string, 5 chars skip 4 bytes ha1 KeyValueList + 1 byte amount indexes
			for (uint32 i = 5; attrValue[i]; i++, pAttrValuesPage->CurrPos++, fullKeyLen++)
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

			for (uint32 i = 0; i < amountShrinkKeyIndexes; i++, keyLen++, fullKeyLen += 4)
			{
				key[keyLen] = indexes[i];
			}

			//insert value in ha1
			*(uint32*)charNewValue = pScanKeyData->pNewHA1->insert(key, fullKeyLen, docID);

			//restore key
			key[docIDKeyPos] = docID;

			for (uint32 i = 0; i < amountShrinkKeyIndexes; i++)
			{
				key[++docIDKeyPos] = indexes[i];
			}
		}
		else //if it's count of array items, just insert as is
		{
			*(uint32*)(pAttrValuesPage->Values + pAttrValuesPage->CurrPos) = *(uint32*)attrValue;

			//reservce first 4 bytes for index in ha1 KeyValueList
			pAttrValuesPage->CurrPos += 4;

			pScanKeyData->pNewHA2->insert(key, fullKeyLen, newValue);
		}

		//free page
		pAttrValuesPage->Type.store(0);
	}

	return true;
}

uint32 DniproDB::shrink()
{
	ulong64 beforeMemory = getUsedMemory();

	uint32 key[32];

	for (uint32 i = 0; i < countColls; i++)
	{
		ScanShrinkKeyData scanKeyData;

		//destroy ha1
		has1[i]->destroy();
		delete has1[i];

		//init
		scanKeyData.pNewHA1 = createHA1(has1[i]->Name);
		scanKeyData.pNewHA2 = createHA2(has2[i]->Name);

		scanKeyData.pOldHA2 = has2[i];

		//scan
		has2[i]->scanKeysAndValues(key, &shrinkKeyHA, &scanKeyData);

		//destroy ha2
		has2[i]->destroy();
		delete has2[i];

		//replace
		has1[i] = scanKeyData.pNewHA1;
		has2[i] = scanKeyData.pNewHA2;
	}

	ulong64 afterMemory = getUsedMemory();

	return (100 - (afterMemory * 100 / beforeMemory));
}