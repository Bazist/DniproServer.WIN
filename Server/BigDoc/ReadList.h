/*
# Copyright(C) 2010-2021 Viacheslav Makoveichuk (email: slv709@gmail.com, skype: vyacheslavm81)
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

#ifndef _READED_LIST		 // Allow use of features specific to Windows XP or later.                   
#define _READED_LIST 0x775 // Change this to the appropriate value to target other versions of Windows.

#endif	

#include <atomic>
#include "stdafx.h"

class ReadPage
{
public:
	ReadPage()
	{
		pNext = 0;
	}

	std::atomic<uchar8>* Values[MAX_CHAR];

	ReadPage* pNext;
};

class ReadList
{
public:
	ReadList()
	{
	}

	void init()
	{
		memset(this, 0, sizeof(ReadList));

		pReadPage = pLastReadPage = new ReadPage();
	}

	uchar8 BlockedByTranID;
	uint32 BlockedOnValue;

	ReadPage* pReadPage;

	ReadPage* pLastReadPage;
	uint32 currReadPos;

	void addValue(std::atomic<uchar8>* pValue)
	{
		if (currReadPos >= MAX_CHAR)
		{
			if (pLastReadPage->pNext)
			{
				pLastReadPage = pLastReadPage->pNext;
			}
			else
			{
				pLastReadPage = pLastReadPage->pNext = new ReadPage();
			}

			currReadPos = 0;
		}

		pLastReadPage->Values[currReadPos++] = pValue;

		return;
	}

	void releaseValues()
	{
		ReadPage* pCurrReadPage = pReadPage;

		while (true)
		{
			if (pCurrReadPage == pLastReadPage) //last page
			{
				for (uint32 i = 0; i < currReadPos; i++)
				{
					pCurrReadPage->Values[i]->store(0); //release
				}
							
				break;
			}
			else
			{
				for (uint32 i = 0; i < MAX_CHAR; i++)
				{
					pCurrReadPage->Values[i]->store(0); //release
				}

				pCurrReadPage = pCurrReadPage->pNext;
			}
		}
	}

	//contentCell was moved !
	//need replace address
	void replaceAddress(std::atomic<uchar8>* oldAddress,
						std::atomic<uchar8>* newAddress)
	{
		ReadPage* pCurrReadPage = pReadPage;

		while (true)
		{
			if (pCurrReadPage == pLastReadPage) //last page
			{
				for (uint32 i = 0; i < currReadPos; i++)
				{
					if (pCurrReadPage->Values[i] == oldAddress)
					{
						pCurrReadPage->Values[i] = newAddress;
						
						return;
					}
				}

				break;
			}
			else
			{
				for (uint32 i = 0; i < MAX_CHAR; i++)
				{
					if (pCurrReadPage->Values[i] == oldAddress)
					{
						pCurrReadPage->Values[i] = newAddress;

						return;
					}
				}

				pCurrReadPage = pCurrReadPage->pNext;
			}
		}
	}

	//bool hasValue(std::atomic<uchar8>* pValue)
	//{
	//	ReadPage* pCurrReadPage = pReadPage;

	//	while (true)
	//	{
	//		if (pCurrReadPage == pLastReadPage) //last page
	//		{
	//			for (uint32 i = 0; i < currReadPos; i++)
	//			{
	//				if (pCurrReadPage->Values[i] == pValue)
	//				{
	//					return true;
	//				}
	//			}

	//			return false;
	//		}
	//		else
	//		{
	//			for (uint32 i = 0; i < MAX_CHAR; i++)
	//			{
	//				if (pCurrReadPage->Values[i] == pValue)
	//				{
	//					return true;
	//				}
	//			}

	//			pCurrReadPage = pCurrReadPage->pNext;
	//		}
	//	}
	//}

	void clear()
	{
		pLastReadPage = pReadPage;
		currReadPos = 0;

		BlockedByTranID = 0;
		BlockedOnValue = 0;
	}

	void destroy()
	{
		ReadPage* pCurrReadPage = pReadPage;

		while (pCurrReadPage)
		{
			ReadPage* pNextPage = pCurrReadPage->pNext;

			delete pCurrReadPage;

			pCurrReadPage = pNextPage;
		}
	}
};
