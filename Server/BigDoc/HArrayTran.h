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

#pragma once
#include <atomic>
#include "HArrayVarRAM.h"
#include "HArrayTranItems.h"
#include "HArrayTranItemsPool.h"
#include "ReadedList.h"

class HArrayTran
{
public:

	//0 = ReadOnly
	//1 = ReadCommited
	//2 = RepeatableRead
	//3 = Snapshot

	uint32 CollID;

	uint32 TranIdentity;
	uchar8 TranID;
	uchar8 ParentTranID;
	HArrayTran* pGrandParentTran;

	uchar8 TranType;
	bool IsWritable;
	bool HasChilds;

	//bool HasCommitedLabel;
	uint32 LastWritedOnTranPage; //need for sync with parallel thread. We will wait until our pages is not saved.

	char* LastJson;

	std::atomic_bool InUse;

	ON_CONTENT_CELL_MOVED_FUNC* onContentCellMovedFunc;

	HArrayTranItemsPool* pHArrayTranItemsPool;
	
	HArrayVarRAM** has1;
	HArrayVarRAM** has2;

	IndexesPool indexesPool;
	AttrValuesPage* pAttrValuesPage;
	
	HArrayTranItemKeysPool tranItemKeysPool;
	ValueListPool valueListPool;

	ReadedList readedList;

	uint32 tempInt;

	HArrayTranItems* pHArrayTranItems1;
	HArrayTranItems* pHArrayTranItems2;

	HArrayTranItems* pLastHArrayTranItems1;
	HArrayTranItems* pLastHArrayTranItems2;

	uint32 Count1;
	uint32 Count2;

	HArrayTran()
	{
	}

	void init(uchar8 tranID,
		HArrayTranItemsPool* pHArrayTranItemsPool,
		HArrayVarRAM** has1,
		HArrayVarRAM** has2,
		ON_CONTENT_CELL_MOVED_FUNC* onContentCellMovedFunc)
	{
		CollID = 0; //default;

		TranID = tranID;
		ParentTranID = 0;
		pGrandParentTran = 0;

		this->pHArrayTranItemsPool = pHArrayTranItemsPool;
		
		this->has1 = has1;
		this->has2 = has2;

		pHArrayTranItems1 = pHArrayTranItemsPool->newObject();
		pHArrayTranItems2 = pHArrayTranItemsPool->newObject();

		pLastHArrayTranItems1 = pHArrayTranItems1;
		pLastHArrayTranItems2 = pHArrayTranItems2;

		Count1 = 0;
		Count2 = 0;

		this->onContentCellMovedFunc = onContentCellMovedFunc;

		indexesPool.init();
		//attrValuesPool.init();
		//valueListPool.init();
		readedList.init();

		pAttrValuesPage = 0;

		InUse.store(false);

		TranType = 0; //readonly by default

		IsWritable = false;
		HasChilds = false;

		//HasCommitedLabel = false;
		LastWritedOnTranPage = 0;

		LastJson = 0;
	}

	//insert in ha1
	bool insertOrDelete1(uint32* key,
		uint32 keyLen,
		uint32 value,
		uint32* pIndexInVL,
		HArrayTranItemType itemType,
		uchar8 tranID)
	{
		if (ParentTranID)
		{
			//insert data to grand parent tran
			return pGrandParentTran->insertOrDelete1(key,
				keyLen,
				value,
				pIndexInVL,
				itemType,
				tranID);
		}

		if (Count1 < 128)
		{
			//if (!isInserted) //if delete, just mark
			//{
			//	for (uint32 i = 0; i < Count1; i++)
			//	{
			//		HArrayTranItem& item = pHArrayTranItems1->Items[i];

			//		if (item.KeyLen == keyLen && item.Value == value)
			//		{
			//			if (!memcmp(item.Key, key, item.KeyLen))
			//			{
			//				item.IsInserted = false;

			//				return true;
			//			}
			//		}
			//	}
			//}

			HArrayTranItem& item = pHArrayTranItems1->Items[Count1];
			item.TranID = tranID;
			item.CollID = CollID;
			item.Type = itemType;
			item.pIndexInVL = pIndexInVL;
			item.setKey(tranItemKeysPool, key, keyLen);
			item.KeyLen = keyLen;
			item.Value = value;
		}
		else
		{
			//if (!isInserted)
			//{
			//	HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems1;
			//	uint32 count = 128;

			//	while (true)
			//	{
			//		for (uint32 i = 0; i < count; i++)
			//		{
			//			HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

			//			if (item.KeyLen == keyLen && item.Value == value)
			//			{
			//				if (!memcmp(item.Key, key, item.KeyLen))
			//				{
			//					item.IsInserted = false;

			//					return false;
			//				}
			//			}
			//		}

			//		//next block
			//		pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

			//		if (!pCurrHArrayTranItems)
			//		{
			//			break;
			//		}

			//		if (pCurrHArrayTranItems->pNextItems)
			//		{
			//			count = 128; //not last
			//		}
			//		else
			//		{
			//			count = Count1 & 0x7F; //last
			//		}
			//	}
			//}

			//find block
			//HArrayTranItems* pCurrHArrayTranItems = pLastHArrayTranItems1;

			/*while (pCurrHArrayTranItems->pNextItems)
			{
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;
			}*/

			uint32 count1 = Count1 & 0x7F;

			if (!count1)
			{
				pLastHArrayTranItems1 = pLastHArrayTranItems1->pNextItems = pHArrayTranItemsPool->newObject();
			}

			HArrayTranItem& item = pLastHArrayTranItems1->Items[count1];

			item.TranID = tranID;
			item.CollID = CollID;
			item.Type = itemType;
			item.pIndexInVL = pIndexInVL;
			item.setKey(tranItemKeysPool, key, keyLen);
			item.KeyLen = keyLen;
			item.Value = value;
		}

		Count1++;

		return true;
	}

	//insert in ha2
	bool insertOrDelete2(uint32* key,
		uint32 keyLen,
		uint32 value,
		HArrayTranItemType itemType,
		uchar8 tranID)
	{
		if (ParentTranID)
		{
			//insert data to grand parent tran
			return pGrandParentTran->insertOrDelete2(key,
				keyLen,
				value,
				itemType,
				tranID);
		}

		if (Count2 < 128)
		{
			//if (!isInserted) //if delete, just mark
			//{
			//	for (uint32 i = 0; i < Count2; i++)
			//	{
			//		HArrayTranItem& item = pHArrayTranItems2->Items[i];

			//		if (item.KeyLen == keyLen && item.Value == value)
			//		{
			//			if (!memcmp(item.Key, key, item.KeyLen))
			//			{
			//				item.IsInserted = false;

			//				return true;
			//			}
			//		}
			//	}
			//}

			HArrayTranItem& item = pHArrayTranItems2->Items[Count2];

			item.setKey(tranItemKeysPool, key, keyLen);
			item.TranID = tranID;
			item.CollID = CollID;
			item.Type = itemType;
			item.KeyLen = keyLen;
			item.Value = value;
		}
		else
		{
			//if (!isInserted)
			//{
			//	HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems2;
			//	uint32 count = 128;

			//	while (true)
			//	{
			//		for (uint32 i = 0; i < count; i++)
			//		{
			//			HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

			//			if (item.KeyLen == keyLen && item.Value == value)
			//			{
			//				if (!memcmp(item.Key, key, item.KeyLen))
			//				{
			//					item.IsInserted = false;

			//					return true;
			//				}
			//			}
			//		}

			//		//next block
			//		pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

			//		if (!pCurrHArrayTranItems)
			//		{
			//			break;
			//		}

			//		if (pCurrHArrayTranItems->pNextItems)
			//		{
			//			count = 128; //not last
			//		}
			//		else
			//		{
			//			count = Count2 & 0x7F; //last
			//		}
			//	}
			//}

			//find block
			uint32 count2 = Count2 & 0x7F;

			if (!count2)
			{
				pLastHArrayTranItems2 = pLastHArrayTranItems2->pNextItems = pHArrayTranItemsPool->newObject();
			}

			HArrayTranItem& item = pLastHArrayTranItems2->Items[count2];

			item.Type = itemType;
			item.TranID = tranID;
			item.CollID = CollID;
			item.setKey(tranItemKeysPool, key, keyLen);
			item.KeyLen = keyLen;
			item.Value = value;
		}

		Count2++;

		return true;
	}

	bool hasKeyAndValue1(uint32* key,
		uint32 keyLen,
		uint32 value,
		HArrayTranItemType itemType)
	{
		if (ParentTranID)
		{
			//data in grand parent tran
			return pGrandParentTran->hasKeyAndValue1(key, keyLen, value, itemType);
		}

		if (Count1 < 128)
		{
			for (uint32 i = 0; i < Count1; i++)
			{
				HArrayTranItem& item = pHArrayTranItems1->Items[i];

				if (item.KeyLen == keyLen &&
					item.Value == value &&
					item.Type == itemType &&
					item.CollID == CollID)
				{
					if (!memcmp(item.Key, key, item.KeyLen))
					{
						return true;
					}
				}
			}
		}
		else
		{
			if (itemType == 2)
			{
				HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems1;
				uint32 count = 128;

				while (true)
				{
					for (uint32 i = 0; i < count; i++)
					{
						HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

						if (item.KeyLen == keyLen &&
							item.Value == value &&
							item.Type == itemType &&
							item.CollID == CollID)
						{
							if (!memcmp(item.Key, key, item.KeyLen))
							{
								return true;
							}
						}
					}

					//next block
					pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

					if (!pCurrHArrayTranItems)
					{
						break;
					}

					if (pCurrHArrayTranItems->pNextItems)
					{
						count = 128; //not last
					}
					else
					{
						count = Count1 & 0x7F; //last
					}
				}
			}
		}

		return false;
	}

	//scanKeysAndValues
	uint32 scanKeysAndValues1(uint32* key,
		uint32 keyLen,
		HARRAY_ITEM_VISIT_FUNC visitor,
		void* pData)
	{
		if (Count1 < 128) //in one block
		{
			for (uint32 i = 0; i < Count1; i++)
			{
				HArrayTranItem& item = pHArrayTranItems1->Items[i];

				if (item.Type == Inserted &&
					item.CollID == CollID)
				{
					if (!memcmp(key, item.Key, keyLen))
					{
						visitor(item.Key, item.KeyLen, item.Value, VALUE_TYPE, pData);
					}
				}
			}
		}
		else
		{
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems1;
			uint32 count = 128;

			while (true)
			{
				for (uint32 i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					if (item.Type == Inserted &&
						item.CollID == CollID)
					{
						if (!memcmp(key, item.Key, keyLen))
						{
							visitor(item.Key, item.KeyLen, item.Value, VALUE_TYPE, pData);
						}
					}
				}

				//next block
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

				if (!pCurrHArrayTranItems)
				{
					break;
				}

				if (pCurrHArrayTranItems->pNextItems)
				{
					count = 128; //not last
				}
				else
				{
					count = Count1 & 0x7F; //last
				}
			}
		}

		return 0;
	}

	//getValueByKey
	void getValueByKey1(uint32* key,
		uint32 keyLen,
		uint32& resValueInserted,
		uchar8& resValueTypeInserted,
		uint32& resValueDeleted,
		uchar8& resValueTypeDeleted)
	{
		uint32 valueInserted = 0;
		ValueList* pValuesInserted = 0;

		uint32 valueDeleted = 0;
		ValueList* pValuesDeleted = 0;

		if (Count1 < 128)
		{
			for (uint32 i = 0; i < Count1; i++)
			{
				HArrayTranItem& item = pHArrayTranItems1->Items[i];

				if (item.KeyLen == keyLen &&
					item.CollID == CollID)
				{
					if (!memcmp(item.Key, key, item.KeyLen))
					{
						if (item.Type == Inserted) //inserted
						{
							if (!valueInserted)
							{
								valueInserted = item.Value;
							}
							else
							{
								if (!pValuesInserted)
								{
									pValuesInserted = valueListPool.newObject();

									pValuesInserted->addValue(valueInserted);
								}

								pValuesInserted->addValue(item.Value);
							}
						}
						else if(item.Type == Deleted) //deleted
						{
							if (!valueDeleted)
							{
								valueDeleted = item.Value;
							}
							else
							{
								if (!pValuesDeleted)
								{
									pValuesDeleted = valueListPool.newObject();

									pValuesDeleted->addValue(valueDeleted);
								}

								pValuesDeleted->addValue(item.Value);
							}
						}
					}
				}
			}
		}
		else
		{
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems1;
			uint32 count = 128;

			while (true)
			{
				for (uint32 i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					if (item.KeyLen == keyLen &&
						item.CollID == CollID)
					{
						if (!memcmp(item.Key, key, item.KeyLen))
						{
							if (item.Type == Inserted) //inserted
							{
								if (!valueInserted)
								{
									valueInserted = item.Value;
								}
								else
								{
									if (!pValuesInserted)
									{
										pValuesInserted = valueListPool.newObject();

										pValuesInserted->addValue(valueInserted);
									}

									pValuesInserted->addValue(item.Value);
								}
							}
							else if(item.Type == Deleted) //deleted
							{
								if (!valueDeleted)
								{
									valueDeleted = item.Value;
								}
								else
								{
									if (!pValuesDeleted)
									{
										pValuesDeleted = valueListPool.newObject();

										pValuesDeleted->addValue(valueDeleted);
									}

									pValuesDeleted->addValue(item.Value);
								}
							}
						}
					}
				}

				//next block
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

				if (!pCurrHArrayTranItems)
				{
					break;
				}

				if (pCurrHArrayTranItems->pNextItems)
				{
					count = 128; //not last
				}
				else
				{
					count = Count1 & 0x7F; //last
				}
			}
		}

		if (!pValuesInserted)
		{
			resValueTypeInserted = VALUE_TYPE;

			resValueInserted = valueInserted;
		}
		else
		{
			resValueTypeInserted = VALUE_LIST_TYPE;

			resValueInserted = (ulong64)pValuesInserted;
		}

		if (!pValuesDeleted)
		{
			resValueTypeDeleted = VALUE_TYPE;

			resValueDeleted = valueDeleted;
		}
		else
		{
			resValueTypeDeleted = VALUE_LIST_TYPE;

			resValueDeleted = (ulong64)pValuesDeleted;
		}
	}

	void getValueByKey2(uint32* key,
						uint32 keyLen,
						uint32& resValueInserted,
						uint32& resValueDeleted)
	{
		resValueInserted = 0;
		resValueDeleted = 0;
		
		if (Count2 <= 128)
		{
			for (int i = Count2 - 1; i >= 0; i--)
			{
				HArrayTranItem& item = pHArrayTranItems2->Items[i];

				if (item.KeyLen == keyLen &&
					item.CollID == CollID)
				{
					if (!memcmp(item.Key, key, item.KeyLen))
					{
						if (item.Type == Inserted) //inserted
						{
							resValueInserted = item.Value;
							resValueDeleted = 0;
						}
						else if(item.Type == Deleted) //deleted
						{
							resValueInserted = 0;
							resValueDeleted = item.Value;
						}

						return;
					}
				}
			}
		}
		else
		{
			//first, scan last block
			for (int i = (Count2 & 0x7F) - 1; i >= 0; i--)
			{
				HArrayTranItem& item = pLastHArrayTranItems2->Items[i];

				if (item.KeyLen == keyLen &&
					item.CollID == CollID)
				{
					if (!memcmp(item.Key, key, item.KeyLen))
					{
						if (item.Type == Inserted) //inserted
						{
							resValueInserted = item.Value;
							resValueDeleted = 0;
						}
						else if (item.Type == Deleted) //deleted
						{
							resValueInserted = 0;
							resValueDeleted = item.Value;
						}

						return;
					}
				}
			}

			//not last, find
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems2;
			uint32 count = 128;

			while (true)
			{
				for (uint32 i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					if (item.KeyLen == keyLen &&
						item.CollID == CollID)
					{
						if (!memcmp(item.Key, key, item.KeyLen))
						{
							if (item.Type == Inserted) //inserted
							{
								resValueInserted = item.Value;
								resValueDeleted = 0;
							}
							else if (item.Type == Deleted) //deleted
							{
								resValueInserted = 0;
								resValueDeleted = item.Value;
							}
						}
					}
				}

				//next block
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

				if (!pCurrHArrayTranItems)
				{
					break;
				}

				if (pCurrHArrayTranItems->pNextItems)
				{
					count = 128; //not last
				}
				else
				{
					break;
				}
			}
		}
	}

	uint32 getValueByKey1(uint32* key,
						uint32 keyLen,
						uchar8& valueType)
	{
		if (ParentTranID)
		{
			//all data in grand parent tran
			return pGrandParentTran->getValueByKey1(key,
												keyLen,
												valueType);
		}

		uchar8 tranValueTypeInserted;
		uint32 tranValueInserted;

		uchar8 tranValueTypeDeleted;
		uint32 tranValueDeleted;

		uchar8 storeValueType;
		uint32 storeValue;

		getValueByKey1(key,
					keyLen,
					tranValueInserted,
					tranValueTypeInserted,
					tranValueDeleted,
					tranValueTypeDeleted);

		storeValue = has1[CollID]->getValueByKey(key,
												keyLen,
												storeValueType);
		
		//0. There is no value
		if (!tranValueInserted && !storeValue)
		{
			return 0;
		}

		//1. Key only in tran
		if (tranValueInserted && !storeValue) //value only in tran
		{
			valueType = tranValueTypeInserted;

			return tranValueInserted;
		}

		//2. Key only in storage
		if (storeValue && !tranValueInserted)
		{
			if (!tranValueDeleted) //the value only in store
			{
				valueType = storeValueType;

				return storeValue;
			}
			else //the value only in store, but maybe deleted in tran
			{
				if (tranValueTypeDeleted == VALUE_TYPE) //was deleted only one value
				{
					if (storeValueType == VALUE_TYPE) //in store only one value
					{
						if (storeValue == tranValueDeleted) //was deleted this value
						{
							return 0;
						}
						else //was deleted another value
						{
							valueType = storeValueType;

							return storeValue;
						}
					}
					else //if (storeValueType == VALUE_LIST_TYPE)
					{
						ValueList* pStoreValues = (ValueList*)storeValue;

						if (pStoreValues->hasValue(tranValueDeleted)) //was deleted this value
						{
							ValueList* pValues = valueListPool.newObject();
							pValues->addValues(pStoreValues);

							pValues->delValue(tranValueDeleted);

							valueType = VALUE_LIST_TYPE;

							return (ulong64)pValues;
						}
						else //was deleted another values
						{
							valueType = storeValueType;

							return storeValue;
						}
					}
				}
				else //if (tranValueTypeDeleted == VALUE_LIST_TYPE)
				{
					ValueList* pTranValuesDeleted = (ValueList*)tranValueDeleted;

					if (storeValueType == VALUE_TYPE) //in store only one value
					{
						if (pTranValuesDeleted->hasValue(storeValue)) //was deleted this value
						{
							return 0;
						}
						else //was deleted another value
						{
							valueType = storeValueType;

							return storeValue;
						}
					}
					else //if (storeValueType == VALUE_LIST_TYPE)
					{
						ValueList* pStoreValues = (ValueList*)storeValue;

						if (pStoreValues->hasValues(pTranValuesDeleted)) //was deleted this value
						{
							ValueList* pValues = valueListPool.newObject();
							pValues->addValues(pStoreValues);

							pValues->delValues(pTranValuesDeleted);

							valueType = VALUE_LIST_TYPE;

							return (ulong64)pValues;
						}
						else //was deleted another values
						{
							valueType = storeValueType;

							return storeValue;
						}
					}
				}
			}
		}

		//3. Need merge
		if (tranValueTypeInserted == VALUE_TYPE) //only one inserted tran value
		{
			if (!tranValueDeleted) //no deleted items
			{
				ValueList* pValues = valueListPool.newObject();

				pValues->addValue(tranValueInserted);

				if (storeValueType == VALUE_TYPE)
				{
					pValues->addValue(storeValue);
				}
				else
				{
					pValues->addValues((ValueList*)storeValue);
				}

				valueType = VALUE_LIST_TYPE;

				return (ulong64)pValues;
			}
			else //there are deleted items
			{
				if (tranValueTypeDeleted == VALUE_TYPE) //only one deleted value
				{
					ValueList* pValues = valueListPool.newObject();

					pValues->addValue(tranValueInserted);

					if (storeValueType == VALUE_TYPE)
					{
						pValues->addValue(storeValue);
					}
					else
					{
						pValues->addValues((ValueList*)storeValue);
					}

					//del value
					pValues->delValue(tranValueDeleted);

					valueType = VALUE_LIST_TYPE;

					return (ulong64)pValues;
				}
				else //if (tranValueTypeDeleted == VALUE_LIST_TYPE) //many deleted values
				{
					ValueList* pValues = valueListPool.newObject();

					pValues->addValue(tranValueInserted);

					if (storeValueType == VALUE_TYPE)
					{
						pValues->addValue(storeValue);
					}
					else
					{
						pValues->addValues((ValueList*)storeValue);
					}

					//del values
					pValues->delValues((ValueList*)tranValueDeleted);

					valueType = VALUE_LIST_TYPE;

					return (ulong64)pValues;
				}
			}
		}
		else //if (tranValueTypeInserted == VALUE_LIST_TYPE)
		{
			if (!tranValueDeleted) //no deleted items
			{
				ValueList* pValues = (ValueList*)tranValueInserted;

				if (storeValueType == VALUE_TYPE)
				{
					pValues->addValue(storeValue);
				}
				else
				{
					pValues->addValues((ValueList*)storeValue);
				}

				valueType = VALUE_LIST_TYPE;

				return (ulong64)pValues;
			}
			else //there are deleted items
			{
				if (tranValueTypeDeleted == VALUE_TYPE) //only one deleted value
				{
					ValueList* pValues = (ValueList*)tranValueInserted;

					if (storeValueType == VALUE_TYPE)
					{
						pValues->addValue(storeValue);
					}
					else
					{
						pValues->addValues((ValueList*)storeValue);
					}

					//del value
					pValues->delValue(tranValueDeleted);

					valueType = VALUE_LIST_TYPE;

					return (ulong64)pValues;
				}
				else //if (tranValueTypeDeleted == VALUE_LIST_TYPE) //many deleted values
				{
					ValueList* pValues = (ValueList*)tranValueInserted;

					if (storeValueType == VALUE_TYPE)
					{
						pValues->addValue(storeValue);
					}
					else
					{
						pValues->addValues((ValueList*)storeValue);
					}

					//del values
					pValues->delValues((ValueList*)tranValueDeleted);

					valueType = VALUE_LIST_TYPE;

					return (ulong64)pValues;
				}
			}
		}
	}

	uint32 getValueByKey2(uint32* key,
						uint32 keyLen,
						uchar8& valueType,
						uchar8 readModeType = 0,
						ReadedList* pReadedList = 0)
	{
		if (ParentTranID)
		{
			//all data in grand parent tran
			return pGrandParentTran->getValueByKey2(key,
													keyLen,
													valueType,
													readModeType,
													pReadedList);
		}

		uint32 tranValueInserted;
		uint32 tranValueDeleted;

		uchar8 storeValueType;
		uint32 storeValue;

		getValueByKey2(key,
					keyLen,
					tranValueInserted,
					tranValueDeleted);

		storeValue = has2[CollID]->getValueByKey(key,
				keyLen,
				storeValueType,
				readModeType,
				TranID,
				&readedList);
		

		//0. inserted in tran
		if (tranValueInserted)
		{
			return tranValueInserted;
		}

		//1. deleted in tran
		if (tranValueDeleted)
		{
			return tranValueDeleted;
		}

		//2. there is in store or no
		return storeValue;
	}

	bool isModifyCellsOfOtherTrans()
	{
		if (Count2 < 128)
		{
			for (uint32 i = 0; i < Count2; i++)
			{
				HArrayTranItem& item = pHArrayTranItems2->Items[i];

				uchar8 valueType;
				if (has2[item.CollID]->getValueByKey(item.Key, item.KeyLen, valueType, 3)) //3. Check if key blocked (ha2 for commit)
				{
					return true;
				}
			}
		}
		else
		{
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems2;
			uint32 count = 128;

			while (true)
			{
				for (uint32 i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					uchar8 valueType;
					if (has2[item.CollID]->getValueByKey(item.Key, item.KeyLen, valueType, 3)) //3. Check if key blocked (ha2 for commit)
					{
						return true;
					}
				}

				//next block
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

				if (pCurrHArrayTranItems->pNextItems)
				{
					count = 128; //not last
				}
				else
				{
					count = Count2 & 0x7F; //last
				}
			}
		}

		return false;
	}

	bool isModifyCellsOfOtherTransExcludeMine()
	{
		if (Count2 < 128)
		{
			for (uint32 i = 0; i < Count2; i++)
			{
				HArrayTranItem& item = pHArrayTranItems2->Items[i];

				uchar8 valueType;
				if (has2[CollID]->getValueByKey(item.Key,
					item.KeyLen,
					valueType,
					4,
					TranID,
					&readedList)) //4. Check if key blocked, but excluded my cells
				{
					return true;
				}
			}
		}
		else
		{
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems2;
			uint32 count = 128;

			while (true)
			{
				for (uint32 i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					uchar8 valueType;
					if (has2[CollID]->getValueByKey(item.Key,
						item.KeyLen,
						valueType,
						4,
						TranID,
						&readedList)) //4. Check if key blocked, but excluded my cells
					{
						return true;
					}
				}

				//next block
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

				if (!pCurrHArrayTranItems)
				{
					break;
				}

				if (pCurrHArrayTranItems->pNextItems)
				{
					count = 128; //not last
				}
				else
				{
					count = Count2 & 0x7F; //last
				}
			}
		}

		return false;
	}

	static std::atomic_int testVal;

	void commit(bool hasReadedCells)
	{
		//this method never calls for nested trans

		testVal = 0;

		if (Count1 < 128) //in one block
		{
			for (uint32 i = 0; i < Count1; i++)
			{
				HArrayTranItem& item = pHArrayTranItems1->Items[i];

				if (item.Type == Inserted) //inserted
				{
					//save ha1DocIndex in attrvalues
					*item.pIndexInVL = has1[item.CollID]->insert(item.Key, item.KeyLen, item.Value);
				}
				else if(item.Type == Deleted) //deleted
				{
					has1[item.CollID]->delValueByKey(item.Key, item.KeyLen, item.Value, *item.pIndexInVL);
				}
			}
		}
		else
		{
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems1;
			uint32 count = 128;

			while (true)
			{
				testVal++;

				for (uint32 i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					if (item.Type == Inserted) //inserted
					{
						*item.pIndexInVL = has1[item.CollID]->insert(item.Key, item.KeyLen, item.Value);
					}
					else if(item.Type == Deleted) //deleted
					{
						has1[item.CollID]->delValueByKey(item.Key, item.KeyLen, item.Value, *item.pIndexInVL);
					}
				}

				//next block
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

				if (!pCurrHArrayTranItems)
				{
					break;
				}

				if (pCurrHArrayTranItems)
				{
					if (pCurrHArrayTranItems->pNextItems)
					{
						count = 128; //not last
					}
					else
					{
						count = Count1 & 0x7F; //last
					}
				}
				else
				{
					break;
				}
			}
		}

		if (Count2 < 128) //in one block
		{
			if (!hasReadedCells)
			{
				for (uint32 i = 0; i < Count2; i++)
				{
					HArrayTranItem& item = pHArrayTranItems2->Items[i];

					if (item.Type == Inserted) //inserted
					{
						has2[item.CollID]->insert(item.Key, item.KeyLen, item.Value);
					}
					else if(item.Type == Deleted) //deleted
					{
						has2[item.CollID]->delValueByKey(item.Key, item.KeyLen, item.Value);
					}
				}
			}
			else
			{
				for (uint32 i = 0; i < Count2; i++)
				{
					HArrayTranItem& item = pHArrayTranItems2->Items[i];

					if (item.Type == Inserted) //inserted
					{
						has2[item.CollID]->insert(item.Key, item.KeyLen, item.Value);
					}
					else if(item.Type == Deleted) //deleted
					{
						has2[item.CollID]->delValueByKey(item.Key, item.KeyLen, item.Value);
					}
				}
			}
		}
		else
		{
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems2;
			uint32 count = 128;

			while (true)
			{
				if (!hasReadedCells)
				{
					for (uint32 i = 0; i < count; i++)
					{
						HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

						if (item.Type == Inserted) //inserted
						{
							has2[item.CollID]->insert(item.Key, item.KeyLen, item.Value);
						}
						else if(item.Type == Deleted) //deleted
						{
							has2[item.CollID]->delValueByKey(item.Key, item.KeyLen, item.Value);
						}
					}
				}
				else
				{
					for (uint32 i = 0; i < count; i++)
					{
						HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

						if (item.Type == Inserted) //inserted
						{
							has2[item.CollID]->insert(item.Key, item.KeyLen, item.Value);
						}
						else if(item.Type == Deleted) //deleted
						{
							has2[item.CollID]->delValueByKey(item.Key, item.KeyLen, item.Value);
						}
					}
				}

				//next block
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

				if (!pCurrHArrayTranItems)
				{
					break;
				}

				if (pCurrHArrayTranItems)
				{
					if (pCurrHArrayTranItems->pNextItems)
					{
						count = 128; //not last
					}
					else
					{
						count = Count2 & 0x7F; //last
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	void clearPart(uchar8 tranID)
	{
		//clear items in current parrent tran
		if (Count1 < 128) //in one block
		{
			for (uint32 i = 0; i < Count1; i++)
			{
				HArrayTranItem& item = pHArrayTranItems1->Items[i];

				if (item.TranID == tranID)
				{
					item.Type = Rollbacked; //rollbacked
				}
			}
		}
		else
		{
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems1;
			uint32 count = 128;

			while (true)
			{
				for (uint32 i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					if (item.TranID == tranID)
					{
						item.Type = Rollbacked; //rollbacked
					}
				}

				//next block
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

				if (!pCurrHArrayTranItems)
				{
					break;
				}

				if (pCurrHArrayTranItems)
				{
					if (pCurrHArrayTranItems->pNextItems)
					{
						count = 128; //not last
					}
					else
					{
						count = Count1 & 0x7F; //last
					}
				}
				else
				{
					break;
				}
			}
		}

		if (Count2 < 128) //in one block
		{
			for (uint32 i = 0; i < Count2; i++)
			{
				HArrayTranItem& item = pHArrayTranItems2->Items[i];

				if (item.TranID == tranID)
				{
					item.Type = Rollbacked; //rollbacked
				}
			}
		}
		else
		{
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems2;
			uint32 count = 128;

			while (true)
			{
				for (uint32 i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					if (item.TranID == tranID)
					{
						item.Type = Rollbacked; //rollbacked
					}
				}

				//next block
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

				if (!pCurrHArrayTranItems)
				{
					break;
				}

				if (pCurrHArrayTranItems)
				{
					if (pCurrHArrayTranItems->pNextItems)
					{
						count = 128; //not last
					}
					else
					{
						count = Count2 & 0x7F; //last
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	void clear()
	{
		if (!ParentTranID)
		{
			if (TranType) //was not read only tran
			{

				//clear ha1 blocks
				HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems1->pNextItems;

				while (pCurrHArrayTranItems)
				{
					pHArrayTranItemsPool->releaseObject(pCurrHArrayTranItems);

					//next block
					pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;
				}

				pHArrayTranItems1->pNextItems = 0;

				//clear ha2 blocks
				pCurrHArrayTranItems = pHArrayTranItems2->pNextItems;

				while (pCurrHArrayTranItems)
				{
					pHArrayTranItemsPool->releaseObject(pCurrHArrayTranItems);

					//next block
					pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;
				}

				pHArrayTranItems2->pNextItems = 0;

				//clear other
				pLastHArrayTranItems1 = pHArrayTranItems1;
				pLastHArrayTranItems2 = pHArrayTranItems2;

				Count1 = 0;
				Count2 = 0;

				readedList.clear();

				IsWritable = false;
				//HasCommitedLabel = false;

				LastWritedOnTranPage = 0;
			}

			indexesPool.clear();
			//attrValuesPool.clear();
			
			tranItemKeysPool.clear();
			valueListPool.clear();

			if (pAttrValuesPage)
			{
				pAttrValuesPage->Type.store(0); //page is free
				pAttrValuesPage = 0;
			}
		}
		
		InUse.store(false);

		LastJson = 0;

		ParentTranID = 0;
		pGrandParentTran = 0;
		HasChilds = false;
	}

	void destroy()
	{
		indexesPool.destroy();

		tranItemKeysPool.clear();
		valueListPool.destroy();
	}
};
