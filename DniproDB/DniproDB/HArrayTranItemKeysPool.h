#include "stdafx.h"

#define HARRAY_TRAN_ITEM_KEYS_PAGE_SIZE 1024

class HArrayTranItemKeysPage
{
public:
	uint Keys[HARRAY_TRAN_ITEM_KEYS_PAGE_SIZE];
	HArrayTranItemKeysPage* pHArrayTranItemKeyPageNext;
};

class HArrayTranItemKeysPool
{
public:
	HArrayTranItemKeysPool()
	{
		pHArrayTranItemKeyPageFirst = pHArrayTranItemKeyPageLast = new HArrayTranItemKeysPage();
		pHArrayTranItemKeyPageLast->pHArrayTranItemKeyPageNext = 0;

		Position = 0;
	}

	uint* newKey(uint sizeKey)
	{
		if (Position + sizeKey > HARRAY_TRAN_ITEM_KEYS_PAGE_SIZE)
		{
			pHArrayTranItemKeyPageLast = pHArrayTranItemKeyPageLast->pHArrayTranItemKeyPageNext = new HArrayTranItemKeysPage();
			pHArrayTranItemKeyPageLast->pHArrayTranItemKeyPageNext = 0;

			Position = sizeKey;

			return pHArrayTranItemKeyPageLast->Keys;
		}

		uint* key = pHArrayTranItemKeyPageLast->Keys + Position;

		Position += sizeKey;

		return key;
	}

	uint Position;

	HArrayTranItemKeysPage* pHArrayTranItemKeyPageFirst;
	HArrayTranItemKeysPage* pHArrayTranItemKeyPageLast;

	void clear()
	{
		HArrayTranItemKeysPage* pPage = pHArrayTranItemKeyPageFirst->pHArrayTranItemKeyPageNext;

		while (pPage)
		{
			HArrayTranItemKeysPage* pNextPage = pPage->pHArrayTranItemKeyPageNext;

			delete pPage;

			pPage = pNextPage;
		}

		pHArrayTranItemKeyPageLast = pHArrayTranItemKeyPageFirst;

		pHArrayTranItemKeyPageFirst->pHArrayTranItemKeyPageNext = 0;
	}

	void destroy()
	{
		clear();

		delete pHArrayTranItemKeyPageFirst;
	}
};
