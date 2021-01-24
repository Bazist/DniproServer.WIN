/*
# Copyright(C) 2010-2016 Vyacheslav Makoveychuk (email: BigDoc@gmail.com, skype: vyacheslavm81)
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

#ifndef _BigDoc_DB		 // Allow use of features specific to Windows XP or later.                   
#define _BigDoc_DB 0x777 // Change this to the appropriate value to target other versions of Windows.

#endif
#include <time.h>
#include "stdlib.h"
#include "stdafx.h"
#include "BigDocError.h"
#include "BigDocInfo.h"
#include "AttrValuesPool.h"
#include "IndexesPool.h"
#include "HArrayVarRAM.h"
#include "HArrayTran.h"

struct ScanKeyData
{
public:
	ValueList* pValueList;
	ValueList* pIndexes;
	char MethodType;
	char Operand1Str[128];
	char Operand2Str[128];

	uint32 Operand1Len;
	uint32 Operand2Len;

	uint32 Operand1;
	uint32 Operand2;

	uint32 KeyLen;
	uint32 DocID;

	ValueListPool* pValueListPool;
	IndexesPool* pIndexesPool;

	HArrayTran* pTran;

	/*uint32 Scans;
	char* Bitmap;*/
};

struct ScanShrinkKeyData
{
public:
	HArrayVarRAM* pNewHA1;
	HArrayVarRAM* pNewHA2;

	HArrayVarRAM* pOldHA2;
};

class BigDoc
{

public:

	std::atomic<uint32> lastDocID;

	std::atomic_bool blockReaders;
	std::atomic_bool blockWriters;
	static std::atomic_bool blockCheckDeadlock;

	static std::atomic<uint32> amountReaders;
	static std::atomic<uint32> amountMarkIsReadedTrans;
	static std::atomic<uint32> amountShapshotTrans;

	std::atomic<uint32> amountWritedTranPages;

	#ifdef _WIN32
	static CRITICAL_SECTION writeTranLock;
	static CRITICAL_SECTION writeBlobLock;
	#endif

	#ifdef __linux__
	static pthread_mutex_t writeTranLock;
	static pthread_mutex_t writeBlobLock;
	#endif

	BinFile* pTranLogFile;
	BinFile* pBlobLogFile;

	char dbTranLogFileName[256];
	char dbBlobLogFileName[256];

	char* tranLog;
	uint32 currTranLogPos;
	uint32 tranIdentity;

	bool writeTranOnHDD;
	static ulong64 tranLogSize;
	static ulong64 blobLogSize;
	
	HArrayVarRAM* has1[MAX_CHAR];
	HArrayVarRAM* has2[MAX_CHAR];

	uint32 countColls;
	uint32 currTime;

	HArrayTranItemsPool* pHArrayTranItemsPool;

	static HArrayTran _trans[MAX_TRANS]; //all trans in pool

	#ifdef _WIN32
	static DWORD WINAPI writeTrans(LPVOID lpParam);
	#endif

	#ifdef __linux__
	static void* writeTrans(void* lpParam);
	#endif

	bool readTrans(char* filePath, uint32 maxDate);
	bool readBlobs(char* filePath);

	//static bool shrinkKeyHA1(uint32* key,
	//						uint32 keyLen,
	//						uint32 value,
	//						uchar8 valueType,
	//						void* pData)
	//{
	//	if (value)
	//	{
	//		keyLen <<= 2; //keyLen * 4

	//		ScanShrinkKeyData* pScanKeyData = (ScanShrinkKeyData*)pData;

	//		if (valueType == VALUE_TYPE)
	//		{
	//			pScanKeyData->pHA->insert(key, keyLen, value);
	//		}
	//		else if (valueType == VALUE_LIST_TYPE)
	//		{
	//			ValueList* pValueList = pScanKeyData->pValueListPool->fromSerPointer(value);

	//			for (uint32 i = 0; i < pValueList->Count; i++)
	//			{
	//				if (pValueList->pValues[i])
	//				{
	//					pScanKeyData->pHA->insert(key, keyLen, pValueList->pValues[i]);
	//				}
	//			}
	//		}
	//	}

	//	return true;
	//}

	static bool shrinkKeyHA(uint32* key,
							uint32 keyLen,
							uint32 value,
							uchar8 valueType,
							void* pData);

	uint32 shrink();

	void read(BinFile* pFile)
	{
		uint32 val;
		pFile->readInt(&val);

		lastDocID.store(val);
	}

	bool save(BinFile* pFile)
	{
		uint32 val = lastDocID.load();

		return pFile->writeInt(&val);
	}

	void init(char* dbFolder = 0,
			  uint32 onDate = 0)
	{
		pHArrayTranItemsPool = new HArrayTranItemsPool();

		lastDocID = 0;
		currTranLogPos = 0;
		countColls = 0;

		for (uchar8 i = 1; i < MAX_TRANS; i++)
		{
			_trans[i].init(i, pHArrayTranItemsPool, has1, has2, onContentCellMoved);
		}

		blockReaders = false;
		blockWriters = false;
		blockCheckDeadlock = false;

		amountReaders = 0;
		amountMarkIsReadedTrans = 0;
		amountShapshotTrans = 0;

		writeTranOnHDD = false;

		addColl("Default");

		//run sync job
		#ifdef _WIN32
		InitializeCriticalSection(&writeTranLock);
		InitializeCriticalSection(&writeBlobLock);
		#endif

		#ifdef __linux__
		writeTranLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
		writeBlobLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
		#endif

		tranLog = 0;
		currTranLogPos = 0;
		amountWritedTranPages = 0;

		if (dbFolder)
		{
			if (strlen(dbFolder) > 0)
			{
				sprintf(dbTranLogFileName, "%s\\dbTranLog.dp", dbFolder);
				sprintf(dbBlobLogFileName, "%s\\dbBlobLog.dp", dbFolder);
			}
			else
			{
				strcpy(dbTranLogFileName, "dbTranLog.dp");
				strcpy(dbBlobLogFileName, "dbBlobLog.dp");
			}

			writeTranOnHDD = true;

			//read trans from HDD
			readTrans(dbTranLogFileName, onDate);
			readBlobs(dbBlobLogFileName);

			#ifdef _WIN32
			CreateThread(NULL,                   // default security attributes
						0,                      // use default stack size  
						BigDoc::writeTrans,       // thread function name
						this,					 // argument to thread function 
						0,                      // use default creation flags 
						0);					 // returns the thread identifier 
			#endif

			#ifdef __linux__
			pthread_t thread;
            int result=pthread_create(&thread, NULL, BigDoc::writeTrans, this);
			#endif
		}

		msleep(10); //wait until thread started
	}

	static bool checkDeadlock(uchar8 tranID);

	void clearChilds(uint32 tranID);

	void clearParts(uint32 tranID);

	uint32 beginTran(uchar8 tranType = READ_COMMITED_TRAN,
		uchar8 parentTranID = 0);

	void rollbackTran(uint32 tranID);

	void clearTran(uint32 tranID);

	void commitTran(uint32 tranID);

	void addTranLog(char type,
		char* json,
		uint32 docID,
		uchar8 tranID,
		uchar8 collID,
		uchar8 parentTranID,
		uint32 tranIdenity,
		uint32* indexes);

	void addTranLogWithCommit(char type,
		char* json,
		uint32 docID,
		uchar8 tranID,
		uchar8 collID,
		uchar8 parentTranID,
		uint32 tranIdentity,
		uint32* indexes);

	uint32 addDoc(char* json,
		uint32 tranID = 0,
		uint32 collID = 0)
	{
		insPartDoc(json, ++lastDocID, tranID, collID);

		return lastDocID.load();
	}

	static void msleep(uint32 ms)
	{	
		#ifdef _WIN32
		Sleep(ms);
		#endif

		#ifdef __linux__
		usleep(ms);
		#endif
	}

	uint32 addDocFromFile(char* fileName);

	inline void procAlias(char* json,
		char* key,
		uint32 currPos,
		uint32* arrayPos,
		uint32** arrayMaxPos,
		uint32* repeatPos,
		uint32 level,
		uint32& i,
		uint32 docID,
		uchar8& typeCommand,
		uint32* indexes,
		HArrayTran* pTran);

	inline uint32 setPathIndexes(uint32* arrayPos,
		uint32* indexes,
		char* key,
		uint32 level,
		uint32& currPos);

	ValueList* getDocsByAttr(char* json,
		uint32 docID = 0,
		uint32 tranID = 0,
		uint32 collID = 0,
		ValueList** pIndexes = 0);

	static bool getDocsByAttr_scanKey(uint32* key,
		uint32 keyLen,
		uint32 value,
		uchar8 valueType,
		void* pData);

	static bool getDocsByAttr_scanIn(uint32* key,
		uint32 keyLen,
		uint32 value,
		uchar8 valueType,
		void* pData);

	static bool getDocsByAttr_scanCond(uint32* key,
		uint32 keyLen,
		uint32 value,
		uchar8 valueType,
		void* pData);

	uint32 getPartDoc(char* jsonTemplate,
		char* jsonResult,
		uint32 rowNumOrDocID,
		uint32 tranID = 0,
		uint32 collID = 0,
		bool onlyValue = false,
		ValueList** pDocIDs = 0,
		uint32* indexes = 0,
		uchar8* collIDs = 0);

	uint32 delPartDoc(char* json,
		uint32 docID,
		uint32 tranID = 0,
		uint32 collID = 0,
		uint32* indexes = 0)
	{
		return updPartDoc(json,
			docID,
			tranID,
			collID,
			indexes,
			true);
	}

	uint32 updPartDoc(char* json,
		uint32 docID,
		uint32 tranID = 0,
		uint32 collID = 0,
		uint32* indexes = 0,
		bool onlyDelete = false);

	uint32 insPartDoc(char* json,
		uint32 rowNumOrDocID = 0,
		uint32 tranID = 0,
		uint32 collID = 0,
		uint32* indexes = 0);

	static void onContentCellMoved(uchar8 tranID,
		std::atomic<uchar8>* oldAddress,
		std::atomic<uchar8>* newAddress)
	{
		_trans[tranID].readedList.replaceAddress(oldAddress, newAddress);
	}

	inline void print(char* key, uint32 len, uint32 level)
	{
		return;

		uint32* str = (uint32*)key;
		for (int i = 0; i < len / 4; i++)
		{
			printf("%d ", str[i]);
		}

		printf("(%d)", level);

		printf("\n");
	}

	inline void print2(char* key, uint32 len, uint32 level)
	{
		uint32* str = (uint32*)key;
		for (int i = 0; i < len / 4; i++)
		{
			printf("%d ", str[i]);
		}

		printf("(%d)", level);

		printf("\n");
	}

	inline void printMemory()
	{
		printf("TOTAL MEMORY: %d\n", getTotalMemory());

		//attrValuesPool.printMemory();

		//ha1.printMemory();

		//ha2.printMemory();
	}

	uint32 getUsedMemory()
	{
		ulong64 usedMemory = sizeof(BigDoc); // +valueListPool.getTotalMemory();

		for (uint32 i = 0; i < countColls; i++)
		{
			usedMemory += has1[i]->getUsedMemory();
			usedMemory += has2[i]->getUsedMemory();
		}

		return usedMemory;
	}

	uint32 getTotalMemory()
	{
		ulong64 totalMemory = sizeof(BigDoc); // +valueListPool.getTotalMemory();

		for (uint32 i = 0; i < countColls; i++)
		{
			totalMemory += has1[i]->getTotalMemory();
			totalMemory += has2[i]->getTotalMemory();
		}

		return totalMemory;
	}

	/*void clearState()
	{
	tranItemKeysPool.clear();
	valueListPool.clear();
	indexesPool.clear();
	}*/

	uint32 addBlobValue(char* value, uint32 len, char* label)
	{
		#ifdef _WIN32
		EnterCriticalSection(&writeBlobLock);
		#endif

		#ifdef __linux__
		pthread_mutex_lock( &writeBlobLock );
		#endif

		uint32 labelLen = sprintf(label, "#%d-%d", blobLogSize, len);

		pBlobLogFile->setPosition(blobLogSize);
		pBlobLogFile->write(value, len);
		pBlobLogFile->flush();

		blobLogSize += len;

		#ifdef _WIN32
		LeaveCriticalSection(&writeBlobLock);
		#endif

		#ifdef __linux__
		pthread_mutex_unlock( &writeBlobLock );
		#endif

		return labelLen;
	}

	bool getBlobValue(char* value, uint32& len, char* label)
	{
		char* posStr = label + 1;

		for (uint32 i = 0; label[i]; i++)
		{
			if (label[i] == '-')
			{
				label[i] = 0;

				char* lenStr = label + i + 1;

				len = pBlobLogFile->read(value, atoi(posStr), atoi(lenStr));

				return (len > 0);
			}
		}

		return false;
	}

	HArrayVarRAM* createHA1(char* name)
	{
		HArrayVarRAM* pHA1 = new HArrayVarRAM();

		strcpy(pHA1->Name, name);

		pHA1->init(4, 16);
		pHA1->AllowValueList = true;

		return pHA1;
	}

	HArrayVarRAM* createHA2(char* name)
	{
		HArrayVarRAM* pHA2 = new HArrayVarRAM();

		strcpy(pHA2->Name, name);

		pHA2->init(4, 16);
		pHA2->AllowValueList = false;

		pHA2->onContentCellMovedFunc = onContentCellMoved;
		pHA2->checkDeadlockFunc = checkDeadlock;

		return pHA2;
	}

	uint32 addColl(char* name)
	{
		for (uint32 i = 0; i < countColls; i++)
		{
			if (!strcmp(has1[i]->Name, name))
			{
				BigDocError de;
				de.Code = COLL_ALREADY_EXISTS_ERROR;
				strcpy(de.Message, "Collection already exists.");

				throw de;
			}
		}

		has1[countColls] = createHA1(name);

		has2[countColls] = createHA2(name);

		if (writeTranOnHDD)
		{
			addTranLog('c', name, 0, 0, countColls, 0, 0, 0);
		}

		countColls++;

		return strlen(name);
	}

	uint32 getCollID(char* name)
	{
		for (uint32 i = 0; i < countColls; i++)
		{
			if (!strcmp(has1[i]->Name, name))
			{
				return i;
			}
		}

		BigDocError de;
		de.Code = COLL_IS_NOT_FOUND_ERROR;
		strcpy(de.Message, "Collection is not found.");

		throw de;
	}

	uint32 delColl(char* name)
	{
		//default coll can not be deleted
		for (uint32 i = 1; i < countColls; i++)
		{
			if (!strcmp(has1[i]->Name, name))
			{
				if (has1[i] && has2[i])
				{
					has1[i]->destroy();
					has2[i]->destroy();

					has1[i] = 0;
					has2[i] = 0;

					if (writeTranOnHDD)
					{
						addTranLog('r', name, 0, 0, countColls, 0, 0, 0);
					}

					return strlen(name);
				}
			}
		}

		BigDocError de;
		de.Code = COLL_IS_NOT_FOUND_ERROR;
		strcpy(de.Message, "Collection is not found.");

		throw de;
	}

	void block()
	{
		//wait until all snap trans will be finished
		while (amountShapshotTrans.load())
		{
			msleep(1);
		}

		bool val = false;

		//block writers
		while (!blockWriters.compare_exchange_weak(val, true))
		{
			msleep(1);

			val = false;
		}

		//block readers
		blockReaders.store(true);

		//wait while readers finish their work
		while (amountReaders.load());
	}

	void unblock()
	{
		//allow readers
		blockReaders.store(false);

		//allow writers
		blockWriters.store(false);
	}

	void clear()
	{
		block();

		//clear all db
		for (uint32 i = 0; i < MAX_TRANS; i++)
		{
			_trans[i].clear();
		}

		for (uint32 i = 0; i < countColls; i++)
		{
			has1[i]->clear();
			has2[i]->clear();

			has1[i]->AllowValueList = true;
			has2[i]->AllowValueList = false;
		}

		lastDocID = 0;

		unblock();
	}

	void destroy()
	{
		block();

		writeTranOnHDD = false;

		msleep(10); //wait until finished

		delete pHArrayTranItemsPool;

		for (uint32 i = 0; i < MAX_TRANS; i++)
		{
			_trans[i].destroy();
		}

		for (uint32 i = 0; i < countColls; i++)
		{
			has1[i]->destroy();
			has2[i]->destroy();
		}
	}
};

