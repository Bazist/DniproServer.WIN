
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

	std::atomic<uchar>* Values[MAX_CHAR];

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

	uchar BlockedByTranID;
	uint BlockedOnValue;

	ReadedPage* pReadedPage;

	ReadedPage* pLastReadedPage;
	uint currReadedPos;

	void addValue(std::atomic<uchar>* pValue)
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
				for (uint i = 0; i < currReadedPos; i++)
				{
					pCurrReadedPage->Values[i]->store(0); //release
				}
							
				break;
			}
			else
			{
				for (uint i = 0; i < MAX_CHAR; i++)
				{
					pCurrReadedPage->Values[i]->store(0); //release
				}

				pCurrReadedPage = pCurrReadedPage->pNext;
			}
		}
	}

	//contentCell was moved !
	//need replace address
	void replaceAddress(std::atomic<uchar>* oldAddress,
						std::atomic<uchar>* newAddress)
	{
		ReadedPage* pCurrReadedPage = pReadedPage;

		while (true)
		{
			if (pCurrReadedPage == pLastReadedPage) //last page
			{
				for (uint i = 0; i < currReadedPos; i++)
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
				for (uint i = 0; i < MAX_CHAR; i++)
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

	//bool hasValue(std::atomic<uchar>* pValue)
	//{
	//	ReadedPage* pCurrReadedPage = pReadedPage;

	//	while (true)
	//	{
	//		if (pCurrReadedPage == pLastReadedPage) //last page
	//		{
	//			for (uint i = 0; i < currReadedPos; i++)
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
	//			for (uint i = 0; i < MAX_CHAR; i++)
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
