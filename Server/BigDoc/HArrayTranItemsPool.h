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
	std::atomic<uint32> Count;
	uint32 Size;

	HArrayTranItems* newObject()
	{
		uint32 count = Count.fetch_add(1);

		uint32 page = count >> 8;
		uint32 index = count & 0xFF;

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

			for (uint32 i = 0; i < MAX_CHAR; i++)
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
		uint32 count = --Count;

		pFreeHArrayTranItemsLists[count >> 8][count & 0xFF] = pHArrayTranItems;
	}

	void releaseObjects()
	{
		Count = 0;
	}

	uint32 getUsedMemory()
	{
		return Count * sizeof(HArrayTranItems) + sizeof(HArrayTranItemsPool);
	}

	uint32 getTotalMemory()
	{
		return Size * sizeof(HArrayTranItems) + sizeof(HArrayTranItemsPool);
	}

	void destroy()
	{
		uint32 page = Size >> 8;

		for (uint32 i = 0; i < page; i++)
		{
			delete[] pFreeHArrayTranItemsLists[i];
		}

		for (uint32 i = 0; i < page; i++)
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
