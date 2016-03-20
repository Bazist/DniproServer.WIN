#pragma once

#ifndef _INDEXES_POOL		 // Allow use of features specific to Windows XP or later.                   
#define _INDEXES_POOL 0x781 // Change this to the appropriate value to target other versions of Windows.

#endif
#include "stdafx.h"

class IndexesPage
{
public:
	IndexesPage()
	{
		pNext = 0;
	}

	uint Values[MAX_CHAR];

	IndexesPage* pNext;
};

class IndexesPool
{
public:
	IndexesPool()
	{
	}

	void init()
	{
		memset(this, 0, sizeof(IndexesPool));

		pIndexesPage = pLastIndexesPage = new IndexesPage();
	}

	IndexesPage* pIndexesPage;
	
	IndexesPage* pLastIndexesPage;
	uint currIndexesPos;

	uint* newObject(uint size)
	{
		if(currIndexesPos + size > MAX_CHAR)
		{
			if (pLastIndexesPage->pNext)
			{
				pLastIndexesPage = pLastIndexesPage->pNext;
			}
			else
			{
				pLastIndexesPage = pLastIndexesPage->pNext = new IndexesPage();
			}

			currIndexesPos = 0;
		}

		uint* pObject = pLastIndexesPage->Values + currIndexesPos;

		currIndexesPos += size;

		return pObject;
	}

	void clear()
	{
		pLastIndexesPage = pIndexesPage;
		currIndexesPos = 0;
	}

	void destroy()
	{
		IndexesPage* pCurrIndexesPage = pIndexesPage;

		while (pCurrIndexesPage)
		{
			IndexesPage* pNextPage = pCurrIndexesPage->pNext;

			delete pCurrIndexesPage;

			pCurrIndexesPage = pNextPage;
		}
	}
};