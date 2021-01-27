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

#ifndef _READED_LIST		 // Allow use of features specific to Windows XP or later.                   
#define _READED_LIST 0x775 // Change this to the appropriate value to target other versions of Windows.

#endif	

#include <atomic>
#include "stdafx.h"

class ReadedPage
{
public:
	ReadedPage()
	{
		pNext = 0;
	}

	std::atomic<uchar8>* Values[MAX_CHAR];

	ReadedPage* pNext;
};

class ReadedList
{
public:
	ReadedList()
	{
	}

	void init()
	{
		memset(this, 0, sizeof(ReadedList));

		pReadedPage = pLastReadedPage = new ReadedPage();
	}

	uchar8 BlockedByTranID;
	uint32 BlockedOnValue;

	ReadedPage* pReadedPage;

	ReadedPage* pLastReadedPage;
	uint32 currReadedPos;

	void addValue(std::atomic<uchar8>* pValue)
	{
		if (currReadedPos >= MAX_CHAR)
		{
			if (pLastReadedPage->pNext)
			{
				pLastReadedPage = pLastReadedPage->pNext;
			}
			else
			{
				pLastReadedPage = pLastReadedPage->pNext = new ReadedPage();
			}

			currReadedPos = 0;
		}

		pLastReadedPage->Values[currReadedPos++] = pValue;

		return;
	}

	void releaseValues()
	{
		ReadedPage* pCurrReadedPage = pReadedPage;

		while (true)
		{
			if (pCurrReadedPage == pLastReadedPage) //last page
			{
				for (uint32 i = 0; i < currReadedPos; i++)
				{
					pCurrReadedPage->Values[i]->store(0); //release
				}
							
				break;
			}
			else
			{
				for (uint32 i = 0; i < MAX_CHAR; i++)
				{
					pCurrReadedPage->Values[i]->store(0); //release
				}

				pCurrReadedPage = pCurrReadedPage->pNext;
			}
		}
	}

	//contentCell was moved !
	//need replace address
	void replaceAddress(std::atomic<uchar8>* oldAddress,
						std::atomic<uchar8>* newAddress)
	{
		ReadedPage* pCurrReadedPage = pReadedPage;

		while (true)
		{
			if (pCurrReadedPage == pLastReadedPage) //last page
			{
				for (uint32 i = 0; i < currReadedPos; i++)
				{
					if (pCurrReadedPage->Values[i] == oldAddress)
					{
						pCurrReadedPage->Values[i] = newAddress;
						
						return;
					}
				}

				break;
			}
			else
			{
				for (uint32 i = 0; i < MAX_CHAR; i++)
				{
					if (pCurrReadedPage->Values[i] == oldAddress)
					{
						pCurrReadedPage->Values[i] = newAddress;

						return;
					}
				}

				pCurrReadedPage = pCurrReadedPage->pNext;
			}
		}
	}

	//bool hasValue(std::atomic<uchar8>* pValue)
	//{
	//	ReadedPage* pCurrReadedPage = pReadedPage;

	//	while (true)
	//	{
	//		if (pCurrReadedPage == pLastReadedPage) //last page
	//		{
	//			for (uint32 i = 0; i < currReadedPos; i++)
	//			{
	//				if (pCurrReadedPage->Values[i] == pValue)
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
	//				if (pCurrReadedPage->Values[i] == pValue)
	//				{
	//					return true;
	//				}
	//			}

	//			pCurrReadedPage = pCurrReadedPage->pNext;
	//		}
	//	}
	//}

	void clear()
	{
		pLastReadedPage = pReadedPage;
		currReadedPos = 0;

		BlockedByTranID = 0;
		BlockedOnValue = 0;
	}

	void destroy()
	{
		ReadedPage* pCurrReadedPage = pReadedPage;

		while (pCurrReadedPage)
		{
			ReadedPage* pNextPage = pCurrReadedPage->pNext;

			delete pCurrReadedPage;

			pCurrReadedPage = pNextPage;
		}
	}
};
