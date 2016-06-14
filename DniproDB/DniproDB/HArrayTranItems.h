#include "stdafx.h"

enum HArrayTranItemType: uchar
{
	Inserted = 1,
	Deleted = 2,
	Rollbacked = 3
};

class HArrayTranItem
{
public:
	HArrayTranItemType Type; //1 - Inserted, 2 - Deleted, 3 - Rollbacked
	uint* pIndexInVL; //index in Value List (need for fast fast delete item from ha1.ValueList with big Count)
	uchar TranID;
	uchar CollID;
	uint Key[32]; //128 bytes by default
	uint KeyLen;
	uint Value;
};

class HArrayTranItems
{
public:
	HArrayTranItems()
	{
		pNextItems = 0;
	}

	HArrayTranItem Items[128];
	HArrayTranItems* pNextItems;
};
