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

	uint TranIdentity;
	uchar TranID;
	uchar TranType;
	bool IsWritable;
	//bool HasCommitedLabel;
	uint LasWritedOnTranPage;

	char* LastJson;

	std::atomic_bool InUse;

	ON_CONTENT_CELL_MOVED_FUNC* onContentCellMovedFunc;

	HArrayTranItemsPool* pHArrayTranItemsPool;
	HArrayVarRAM* pHA1;
	HArrayVarRAM* pHA2;

	IndexesPool indexesPool;
	AttrValuesPage* pAttrValuesPage;
	ValueListPool valueListPool;
	ReadedList readedList;

	uint tempInt;

	HArrayTranItems* pHArrayTranItems1;
	HArrayTranItems* pHArrayTranItems2;

	HArrayTranItems* pLastHArrayTranItems1;
	HArrayTranItems* pLastHArrayTranItems2;

	uint Count1;
	uint Count2;

	HArrayTran()
	{
	}

	void init(uchar tranID,
			  HArrayTranItemsPool* pHArrayTranItemsPool,
			  HArrayVarRAM* pHA1,
			  HArrayVarRAM* pHA2,
			  ON_CONTENT_CELL_MOVED_FUNC* onContentCellMovedFunc)
	{
		TranID = tranID;

		this->pHArrayTranItemsPool = pHArrayTranItemsPool;
		
		this->pHA1 = pHA1;
		this->pHA2 = pHA2;

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
		//HasCommitedLabel = false;
		LasWritedOnTranPage = 0;

		LastJson = 0;
	}

	//insert in ha1
	bool insertOrDelete1(uint* key,
						 uint keyLen,
						 uint value,
						 bool isInserted)
	{
		if (Count1 < 128)
		{
			if (!isInserted) //if delete, just mark
			{
				for (uint i = 0; i < Count1; i++)
				{
					HArrayTranItem& item = pHArrayTranItems1->Items[i];

					if (item.KeyLen == keyLen && item.Value == value)
					{
						if (!memcmp(item.Key, key, item.KeyLen))
						{
							item.IsInserted = false;

							return true;
						}
					}
				}
			}

			HArrayTranItem& item = pHArrayTranItems1->Items[Count1];
			
			item.IsInserted = isInserted;
			memcpy(item.Key, key, keyLen);
			item.KeyLen = keyLen;
			item.Value = value;
		}
		else
		{
			if (!isInserted)
			{
				HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems1;
				uint count = 128;

				while (pCurrHArrayTranItems)
				{
					for (uint i = 0; i < count; i++)
					{
						HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

						if (item.KeyLen == keyLen && item.Value == value)
						{
							if (!memcmp(item.Key, key, item.KeyLen))
							{
								item.IsInserted = false;

								return false;
							}
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
						count = Count1 & 0x7F; //last
					}
				}
			}

			//find block
			//HArrayTranItems* pCurrHArrayTranItems = pLastHArrayTranItems1;

			/*while (pCurrHArrayTranItems->pNextItems)
			{
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;
			}*/

			uint count1 = Count1 & 0x7F;

			if (!count1)
			{
				pLastHArrayTranItems1 = pLastHArrayTranItems1->pNextItems = pHArrayTranItemsPool->newObject();
			}
			
			HArrayTranItem& item = pLastHArrayTranItems1->Items[count1];

			item.IsInserted = isInserted;
			memcpy(item.Key, key, keyLen);
			item.KeyLen = keyLen;
			item.Value = value;
		}

		Count1++;

		return true;
	}

	//insert in ha2
	bool insertOrDelete2(uint* key,
						 uint keyLen,
						 uint value,
						 bool isInserted)
	{
		if (Count2 < 128)
		{
			if (!isInserted) //if delete, just mark
			{
				for (uint i = 0; i < Count2; i++)
				{
					HArrayTranItem& item = pHArrayTranItems2->Items[i];

					if (item.KeyLen == keyLen && item.Value == value)
					{
						if (!memcmp(item.Key, key, item.KeyLen))
						{
							item.IsInserted = false;

							return true;
						}
					}
				}
			}

			HArrayTranItem& item = pHArrayTranItems2->Items[Count2];

			memcpy(item.Key, key, keyLen);
			item.IsInserted = isInserted;
			item.KeyLen = keyLen;
			item.Value = value;
		}
		else
		{
			if (!isInserted)
			{
				HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems2;
				uint count = 128;

				while (pCurrHArrayTranItems)
				{
					for (uint i = 0; i < count; i++)
					{
						HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

						if (item.KeyLen == keyLen && item.Value == value)
						{
							if (!memcmp(item.Key, key, item.KeyLen))
							{
								item.IsInserted = false;

								return true;
							}
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

			//find block
			uint count2 = Count2 & 0x7F;

			if (!count2)
			{
				pLastHArrayTranItems2 = pLastHArrayTranItems2->pNextItems = pHArrayTranItemsPool->newObject();
			}

			HArrayTranItem& item = pLastHArrayTranItems2->Items[count2];

			item.IsInserted = isInserted;
			memcpy(item.Key, key, keyLen);
			item.KeyLen = keyLen;
			item.Value = value;
		}

		Count2++;

		return true;
	}

	bool hasKeyAndValue1(uint* key,
						 uint keyLen,
						 uint value,
						 bool isInserted)
	{
		if (Count1 < 128)
		{
			for (uint i = 0; i < Count1; i++)
			{
				HArrayTranItem& item = pHArrayTranItems1->Items[i];

				if (item.KeyLen == keyLen &&
					item.Value == value &&
					item.IsInserted == isInserted)
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
			if (!isInserted)
			{
				HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems1;
				uint count = 128;

				while (pCurrHArrayTranItems)
				{
					for (uint i = 0; i < count; i++)
					{
						HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

						if (item.KeyLen == keyLen &&
							item.Value == value &&
							item.IsInserted == isInserted)
						{
							if (!memcmp(item.Key, key, item.KeyLen))
							{
								return true;
							}
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
						count = Count1 & 0x7F; //last
					}
				}
			}
		}

		return false;
	}
	
	//scanKeysAndValues
	uint scanKeysAndValues1(uint* key,
						    uint keyLen,
						    HARRAY_ITEM_VISIT_FUNC visitor,
						    void* pData)
	{
		if (Count1 <= 128) //in one block
		{
			for (uint i = 0; i < Count1; i++)
			{
				HArrayTranItem& item = pHArrayTranItems1->Items[i];

				if (item.IsInserted)
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
			uint count = 128;

			while (pCurrHArrayTranItems)
			{
				for (uint i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					if (item.IsInserted)
					{
						if (!memcmp(key, item.Key, keyLen))
						{
							visitor(item.Key, item.KeyLen, item.Value, VALUE_TYPE, pData);
						}
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
					count = Count1 & 0x7F; //last
				}
			}
		}

		return 0;
	}

	//getValueByKey
	void getValueByKey1(uint* key,
						uint keyLen,
						uint& resValueInserted,
						uchar& resValueTypeInserted,
						uint& resValueDeleted,
						uchar& resValueTypeDeleted)
	{
		uint valueInserted = 0;
		ValueList* pValuesInserted = 0;

		uint valueDeleted = 0;
		ValueList* pValuesDeleted = 0;

		if (Count1 <= 128)
		{
			for (uint i = 0; i < Count1; i++)
			{
				HArrayTranItem& item = pHArrayTranItems1->Items[i];

				if (item.KeyLen == keyLen)
				{
					if (!memcmp(item.Key, key, item.KeyLen))
					{
						if (item.IsInserted)
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
						else
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
			uint count = 128;

			while (pCurrHArrayTranItems)
			{
				for (uint i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					if (item.KeyLen == keyLen && item.IsInserted)
					{
						if (!memcmp(item.Key, key, item.KeyLen))
						{
							if (item.IsInserted)
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
							else
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

			resValueInserted = (uint)pValuesInserted;
		}

		if (!pValuesDeleted)
		{
			resValueTypeDeleted = VALUE_TYPE;

			resValueDeleted = valueDeleted;
		}
		else
		{
			resValueTypeDeleted = VALUE_LIST_TYPE;

			resValueDeleted = (uint)pValuesDeleted;
		}
	}

	void getValueByKey2(uint* key,
						uint keyLen,
						uint& resValueInserted,
						uchar& resValueTypeInserted,
						uint& resValueDeleted,
						uchar& resValueTypeDeleted)
	{
		uint valueInserted = 0;
		ValueList* pValuesInserted = 0;

		uint valueDeleted = 0;
		ValueList* pValuesDeleted = 0;

		if (Count2 <= 128)
		{
			for (uint i = 0; i < Count2; i++)
			{
				HArrayTranItem& item = pHArrayTranItems2->Items[i];

				if (item.KeyLen == keyLen)
				{
					if (!memcmp(item.Key, key, item.KeyLen))
					{
						if (item.IsInserted)
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
						else
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
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems2;
			uint count = 128;

			while (pCurrHArrayTranItems)
			{
				for (uint i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					if (item.KeyLen == keyLen && item.IsInserted)
					{
						if (!memcmp(item.Key, key, item.KeyLen))
						{
							if (item.IsInserted)
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
							else
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

		if (!pValuesInserted)
		{
			resValueTypeInserted = VALUE_TYPE;

			resValueInserted = valueInserted;
		}
		else
		{
			resValueTypeInserted = VALUE_LIST_TYPE;

			resValueInserted = (uint)pValuesInserted;
		}

		if (!pValuesDeleted)
		{
			resValueTypeDeleted = VALUE_TYPE;

			resValueDeleted = valueDeleted;
		}
		else
		{
			resValueTypeDeleted = VALUE_LIST_TYPE;

			resValueDeleted = (uint)pValuesDeleted;
		}
	}

	uint getValueByKey(bool isHA1,
					   uint* key,
					   uint keyLen,
					   uchar& valueType,
					   uchar readModeType = 0,
					   uchar tranID = 0,
					   ReadedList* pReadedList = 0)
	{
		uchar tranValueTypeInserted;
		uint tranValueInserted;

		uchar tranValueTypeDeleted;
		uint tranValueDeleted;

		uchar storeValueType;
		uint storeValue;

		if (isHA1)
		{
			getValueByKey1(key,
						   keyLen,
						   tranValueInserted,
						   tranValueTypeInserted,
						   tranValueDeleted,
						   tranValueTypeDeleted);

			storeValue = pHA1->getValueByKey(key,
											 keyLen,
											 storeValueType);
		}
		else
		{
			getValueByKey2(key,
						   keyLen,
						   tranValueInserted,
						   tranValueTypeInserted,
						   tranValueDeleted,
						   tranValueTypeDeleted);

			storeValue = pHA2->getValueByKey(key,
											 keyLen,
											 storeValueType,
											 readModeType,
											 TranID,
											 &readedList);
		}

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

							return (uint)pValues;
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

							return (uint)pValues;
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

				return (uint)pValues;
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

					return (uint)pValues;
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

					return (uint)pValues;
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

				return (uint)pValues;
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

					return (uint)pValues;
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

					return (uint)pValues;
				}
			}
		}
	}

	bool isModifyCellsOfOtherTrans()
	{
		if (Count2 < 128)
		{
			for (uint i = 0; i < Count2; i++)
			{
				HArrayTranItem& item = pHArrayTranItems2->Items[i];

				uchar valueType;
				if (pHA2->getValueByKey(item.Key, item.KeyLen, valueType, 3)) //3. Check if key blocked (ha2 for commit)
				{
					return true;
				}
			}
		}
		else
		{
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems2;
			uint count = 128;

			while (true)
			{
				for (uint i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					uchar valueType;
					if (pHA2->getValueByKey(item.Key, item.KeyLen, valueType, 3)) //3. Check if key blocked (ha2 for commit)
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
			for (uint i = 0; i < Count2; i++)
			{
				HArrayTranItem& item = pHArrayTranItems2->Items[i];

				uchar valueType;
				if (pHA2->getValueByKey(item.Key,
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
			uint count = 128;

			while (true)
			{
				for (uint i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					uchar valueType;
					if (pHA2->getValueByKey(item.Key,
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
		if (Count1 <= 128) //in one block
		{
			for (uint i = 0; i < Count1; i++)
			{
				HArrayTranItem& item = pHArrayTranItems1->Items[i];

				if (item.IsInserted)
				{
					pHA1->insert(item.Key, item.KeyLen, item.Value);
				}
				else
				{
					pHA1->detValueByKey(item.Key, item.KeyLen, item.Value);
				}
			}
		}
		else
		{
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems1;
			uint count = 128;

			while (pCurrHArrayTranItems)
			{
				for (uint i = 0; i < count; i++)
				{
					HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

					if (item.IsInserted)
					{
						pHA1->insert(item.Key, item.KeyLen, item.Value);
					}
					else
					{
						pHA1->detValueByKey(item.Key, item.KeyLen, item.Value);
					}
				}

				//next block
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

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

		if (Count2 <= 128) //in one block
		{
			if (!hasReadedCells)
			{
				for (uint i = 0; i < Count2; i++)
				{
					HArrayTranItem& item = pHArrayTranItems2->Items[i];

					if (item.IsInserted)
					{
						pHA2->insert(item.Key, item.KeyLen, item.Value);
					}
					else
					{
						pHA2->detValueByKey(item.Key, item.KeyLen, item.Value);
					}
				}
			}
			else
			{
				for (uint i = 0; i < Count2; i++)
				{
					HArrayTranItem& item = pHArrayTranItems2->Items[i];

					if (item.IsInserted)
					{
						pHA2->insert(item.Key, item.KeyLen, item.Value);
					}
					else
					{
						pHA2->detValueByKey(item.Key, item.KeyLen, item.Value);
					}
				}
			}
		}
		else
		{
			HArrayTranItems* pCurrHArrayTranItems = pHArrayTranItems2;
			uint count = 128;

			while (pCurrHArrayTranItems)
			{
				if (!hasReadedCells)
				{
					for (uint i = 0; i < count; i++)
					{
						HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

						if (item.IsInserted)
						{
							pHA2->insert(item.Key, item.KeyLen, item.Value);
						}
						else
						{
							pHA2->detValueByKey(item.Key, item.KeyLen, item.Value);
						}
					}
				}
				else
				{
					for (uint i = 0; i < count; i++)
					{
						HArrayTranItem& item = pCurrHArrayTranItems->Items[i];

						if (item.IsInserted)
						{
							pHA2->insert(item.Key, item.KeyLen, item.Value);
						}
						else
						{
							pHA2->detValueByKey(item.Key, item.KeyLen, item.Value);
						}
					}
				}

				//next block
				pCurrHArrayTranItems = pCurrHArrayTranItems->pNextItems;

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

			LasWritedOnTranPage = 0;
		}

		indexesPool.clear();
		//attrValuesPool.clear();
		valueListPool.clear();

		if (pAttrValuesPage)
		{
			pAttrValuesPage->Type.store(0); //page is free
			pAttrValuesPage = 0;
		}

		InUse.store(false);

		LastJson = 0;

		TranIdentity = 0;
	}

	void destroy()
	{
		indexesPool.destroy();
		valueListPool.destroy();
	}
};
