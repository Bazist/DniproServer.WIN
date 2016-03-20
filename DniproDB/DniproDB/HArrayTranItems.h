#include "stdafx.h"

class HArrayTranItem
{
public:
	bool IsInserted; //true - Inserted, false - Deleted

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
