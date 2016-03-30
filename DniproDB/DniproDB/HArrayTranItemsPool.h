#include "stdafx.h"
#include "HArrayTran.h"

class HArrayTranItemsPool
{
public:
	HArrayTranItemsPool()
	{
		flag = false;
		Count = 0;
		Size = 0;
	}

	HArrayTranItems** pFreeHArrayTranItemsLists[MAX_POOL_PAGES];
	HArrayTranItems* pHArrayTranItemsLists[MAX_POOL_PAGES];

	std::atomic_bool flag;
	std::atomic<uint> Count;
	uint Size;

	HArrayTranItems* newObject()
	{
		uint count = Count.fetch_add(1);

		uint page = count >> 8;
		uint index = count & 0xFF;

		bool val = false;

		while (!flag.compare_exchange_weak(val, true))
		{
			val = false;
		}

		if (count >= Size)
		{
			if (page >= MAX_POOL_PAGES)
			{
				printf("HArrayTranItemsPool is full.");

				return 0;
			}

			HArrayTranItems* pHArrayTranItems = new HArrayTranItems[MAX_CHAR];
			pHArrayTranItemsLists[page] = pHArrayTranItems;

			pFreeHArrayTranItemsLists[page] = new HArrayTranItems*[MAX_CHAR];

			for (uint i = 0; i < MAX_CHAR; i++)
			{
				pFreeHArrayTranItemsLists[page][i] = pHArrayTranItems + i;
			}

			Size += MAX_CHAR;
		}

		flag.store(false);

		HArrayTranItems* pHArrayTranItems = pFreeHArrayTranItemsLists[page][index];
		pHArrayTranItems->pNextItems = 0;

		//Count++;

		return pHArrayTranItems;
	}

	void releaseObject(HArrayTranItems* pHArrayTranItems)
	{
		uint count = --Count;

		pFreeHArrayTranItemsLists[count >> 8][count & 0xFF] = pHArrayTranItems;
	}

	void releaseObjects()
	{
		Count = 0;
	}

	uint getUsedMemory()
	{
		return Count * sizeof(HArrayTranItems) + sizeof(HArrayTranItemsPool);
	}

	uint getTotalMemory()
	{
		return Size * sizeof(HArrayTranItems) + sizeof(HArrayTranItemsPool);
	}

	void destroy()
	{
		uint page = Size >> 8;

		for (uint i = 0; i < page; i++)
		{
			delete[] pFreeHArrayTranItemsLists[i];
		}

		for (uint i = 0; i < page; i++)
		{
			delete[] pHArrayTranItemsLists[i];
		}

		Count = 0;
		Size = 0;
	}

	~HArrayTranItemsPool()
	{
		destroy();
	}
};
