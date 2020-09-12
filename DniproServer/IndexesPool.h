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

	uint32 Values[MAX_CHAR];

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
	uint32 currIndexesPos;

	uint32* newObject(uint32 size)
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

		uint32* pObject = pLastIndexesPage->Values + currIndexesPos;

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