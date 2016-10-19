/*
# Copyright(C) 2010-2016 Vyacheslav Makoveychuk (email: slv709@gmail.com, skype: vyacheslavm81)
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

#ifndef _DNIPRO_DB		 // Allow use of features specific to Windows XP or later.                   
#define _DNIPRO_DB 0x777 // Change this to the appropriate value to target other versions of Windows.

#endif
#include <time.h>
#include "stdlib.h"
#include "stdafx.h"
#include "DniproError.h"
#include "DniproInfo.h"
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

	uint Operand1Len;
	uint Operand2Len;

	uint Operand1;
	uint Operand2;

	uint KeyLen;
	uint DocID;

	ValueListPool* pValueListPool;
	IndexesPool* pIndexesPool;

	HArrayTran* pTran;

	/*uint Scans;
	char* Bitmap;*/
};

struct ScanShrinkKeyData
{
public:
	HArrayVarRAM* pNewHA1;
	HArrayVarRAM* pNewHA2;

	HArrayVarRAM* pOldHA2;
};

class DniproDB
{

public:

	std::atomic<uint> lastDocID;

	std::atomic_bool blockReaders;
	std::atomic_bool blockWriters;
	static std::atomic_bool blockCheckDeadlock;

	static std::atomic<uint> amountReaders;
	static std::atomic<uint> amountMarkIsReadedTrans;
	static std::atomic<uint> amountShapshotTrans;

	std::atomic<uint> amountWritedTranPages;

	CRITICAL_SECTION writeTranLock;
	CRITICAL_SECTION writeBlobLock;

	BinaryFile* pTranLogFile;
	BinaryFile* pBlobLogFile;

	char dbTranLogFileName[256];
	char dbBlobLogFileName[256];

	char* tranLog;
	uint currTranLogPos;
	uint tranIdentity;

	bool writeTranOnHDD;
	static ulong tranLogSize;
	static ulong blobLogSize;
	
	HArrayVarRAM* has1[MAX_CHAR];
	HArrayVarRAM* has2[MAX_CHAR];

	uint countColls;
	uint currTime;

	HArrayTranItemsPool* pHArrayTranItemsPool;

	static HArrayTran _trans[MAX_TRANS]; //all trans in pool

	static DWORD WINAPI writeTrans(LPVOID lpParam);

	bool readTrans(char* filePath, uint maxDate);
	bool readBlobs(char* filePath);

	//static bool shrinkKeyHA1(uint* key,
	//						uint keyLen,
	//						uint value,
	//						uchar valueType,
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

	//			for (uint i = 0; i < pValueList->Count; i++)
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

	static bool shrinkKeyHA(uint* key,
							uint keyLen,
							uint value,
							uchar valueType,
							void* pData);

	uint shrink();

	void read(BinaryFile* pFile)
	{
		uint val;
		pFile->readInt(&val);

		lastDocID.store(val);
	}

	bool save(BinaryFile* pFile)
	{
		uint val = lastDocID.load();

		return pFile->writeInt(&val);
	}

	void init(char* dbFolder = 0,
			  uint onDate = 0)
	{
		pHArrayTranItemsPool = new HArrayTranItemsPool();

		lastDocID = 0;
		currTranLogPos = 0;
		countColls = 0;

		for (uchar i = 1; i < MAX_TRANS; i++)
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
		InitializeCriticalSection(&writeTranLock);

		InitializeCriticalSection(&writeBlobLock);

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

			CreateThread(NULL,                   // default security attributes
				0,                      // use default stack size  
				DniproDB::writeTrans,       // thread function name
				this,					 // argument to thread function 
				0,                      // use default creation flags 
				0);					 // returns the thread identifier 
		}

		Sleep(10); //wait until thread started
	}

	static bool checkDeadlock(uchar tranID);

	void clearChilds(uint tranID);

	void clearParts(uint tranID);

	uint beginTran(uchar tranType = READ_COMMITED_TRAN,
		uchar parentTranID = 0);

	void rollbackTran(uint tranID);

	void clearTran(uint tranID);

	void commitTran(uint tranID);

	void addTranLog(char type,
		char* json,
		uint docID,
		uchar tranID,
		uchar collID,
		uchar parentTranID,
		uint tranIdenity,
		uint* indexes);

	void addTranLogWithCommit(char type,
		char* json,
		uint docID,
		uchar tranID,
		uchar collID,
		uchar parentTranID,
		uint tranIdentity,
		uint* indexes);

	uint addDoc(char* json,
		uint tranID = 0,
		uint collID = 0)
	{
		insPartDoc(json, ++lastDocID, tranID, collID);

		return lastDocID.load();
	}

	uint addDocFromFile(char* fileName);

	inline void procAlias(char* json,
		char* key,
		uint currPos,
		uint* arrayPos,
		uint** arrayMaxPos,
		uint* repeatPos,
		uint level,
		uint& i,
		uint docID,
		uchar& typeCommand,
		uint* indexes,
		HArrayTran* pTran);

	inline uint setPathIndexes(uint* arrayPos,
		uint* indexes,
		char* key,
		uint level,
		uint& currPos);

	ValueList* getDocsByAttr(char* json,
		uint docID = 0,
		uint tranID = 0,
		uint collID = 0,
		ValueList** pIndexes = 0);

	static bool getDocsByAttr_scanKey(uint* key,
		uint keyLen,
		uint value,
		uchar valueType,
		void* pData);

	static bool getDocsByAttr_scanIn(uint* key,
		uint keyLen,
		uint value,
		uchar valueType,
		void* pData);

	static bool getDocsByAttr_scanCond(uint* key,
		uint keyLen,
		uint value,
		uchar valueType,
		void* pData);

	uint getPartDoc(char* jsonTemplate,
		char* jsonResult,
		uint rowNumOrDocID,
		uint tranID = 0,
		uint collID = 0,
		bool onlyValue = false,
		ValueList** pDocIDs = 0,
		uint* indexes = 0,
		uchar* collIDs = 0);

	uint delPartDoc(char* json,
		uint docID,
		uint tranID = 0,
		uint collID = 0,
		uint* indexes = 0)
	{
		return updPartDoc(json,
			docID,
			tranID,
			collID,
			indexes,
			true);
	}

	uint updPartDoc(char* json,
		uint docID,
		uint tranID = 0,
		uint collID = 0,
		uint* indexes = 0,
		bool onlyDelete = false);

	uint insPartDoc(char* json,
		uint rowNumOrDocID = 0,
		uint tranID = 0,
		uint collID = 0,
		uint* indexes = 0);

	static void onContentCellMoved(uchar tranID,
		std::atomic<uchar>* oldAddress,
		std::atomic<uchar>* newAddress)
	{
		_trans[tranID].readedList.replaceAddress(oldAddress, newAddress);
	}

	inline void print(char* key, uint len, uint level)
	{
		return;

		uint* str = (uint*)key;
		for (int i = 0; i < len / 4; i++)
		{
			printf("%d ", str[i]);
		}

		printf("(%d)", level);

		printf("\n");
	}

	inline void print2(char* key, uint len, uint level)
	{
		uint* str = (uint*)key;
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

	uint getUsedMemory()
	{
		ulong usedMemory = sizeof(DniproDB); // +valueListPool.getTotalMemory();

		for (uint i = 0; i < countColls; i++)
		{
			usedMemory += has1[i]->getUsedMemory();
			usedMemory += has2[i]->getUsedMemory();
		}

		return usedMemory;
	}

	uint getTotalMemory()
	{
		ulong totalMemory = sizeof(DniproDB); // +valueListPool.getTotalMemory();

		for (uint i = 0; i < countColls; i++)
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

	uint addBlobValue(char* value, uint len, char* label)
	{
		EnterCriticalSection(&writeBlobLock);

		uint labelLen = sprintf(label, "#%d-%d", blobLogSize, len);

		pBlobLogFile->setPosition(blobLogSize);
		pBlobLogFile->write(value, len);
		pBlobLogFile->flush();

		blobLogSize += len;

		LeaveCriticalSection(&writeBlobLock);

		return labelLen;
	}

	bool getBlobValue(char* value, uint& len, char* label)
	{
		char* posStr = label + 1;

		for (uint i = 0; label[i]; i++)
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

	uint addColl(char* name)
	{
		for (uint i = 0; i < countColls; i++)
		{
			if (!strcmp(has1[i]->Name, name))
			{
				DniproError de;
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

	uint getCollID(char* name)
	{
		for (uint i = 0; i < countColls; i++)
		{
			if (!strcmp(has1[i]->Name, name))
			{
				return i;
			}
		}

		DniproError de;
		de.Code = COLL_IS_NOT_FOUND_ERROR;
		strcpy(de.Message, "Collection is not found.");

		throw de;
	}

	uint delColl(char* name)
	{
		//default coll can not be deleted
		for (uint i = 1; i < countColls; i++)
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

		DniproError de;
		de.Code = COLL_IS_NOT_FOUND_ERROR;
		strcpy(de.Message, "Collection is not found.");

		throw de;
	}

	void block()
	{
		//wait until all snap trans will be finished
		while (amountShapshotTrans.load())
		{
			Sleep(1);
		}

		bool val = false;

		//block writers
		while (!blockWriters.compare_exchange_weak(val, true))
		{
			Sleep(1);

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
		for (uint i = 0; i < MAX_TRANS; i++)
		{
			_trans[i].clear();
		}

		for (uint i = 0; i < countColls; i++)
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

		Sleep(10); //wait until finished

		delete pHArrayTranItemsPool;

		for (uint i = 0; i < MAX_TRANS; i++)
		{
			_trans[i].destroy();
		}

		for (uint i = 0; i < countColls; i++)
		{
			has1[i]->destroy();
			has2[i]->destroy();
		}
	}
};

