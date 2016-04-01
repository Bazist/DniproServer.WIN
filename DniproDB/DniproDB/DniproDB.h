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
	static uint tranLogSize;
	static uint blobLogSize;

	HArrayVarRAM* has1[MAX_CHAR];
	HArrayVarRAM* has2[MAX_CHAR];

	uint countColls;

	HArrayTranItemsPool* pHArrayTranItemsPool;
	AttrValuesPool attrValuesPool;

	static HArrayTran _trans[MAX_TRANS]; //all trans in pool

	static DWORD WINAPI writeTrans(LPVOID lpParam);

	bool readTrans(char* filePath);
	bool readBlobs(char* filePath);

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
	
	void init(char* dbFolder = 0)
	{
		pHArrayTranItemsPool = new HArrayTranItemsPool();
		attrValuesPool.init();

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
		tranIdentity = 0;

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
			readTrans(dbTranLogFileName);
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

	uint beginTran(uchar tranType = READ_COMMITED_TRAN);

	void rollbackTran(uint tranID);

	void clearTran(uint tranID);

	void commitTran(uint tranID);
	
	void addTranLog(char type,
					char* json,
					uint docID,
					uchar tranID,
					uchar collID,
					uint tranIdentity);

	void addTranLogWithCommit(char type,
							  char* json,
							  uint docID,
							  uchar tranID,
							  uchar collID,
							  uint tranIdentity);
	
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

	inline void setPathIndexes(uint* arrayPos,
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
					uint* indexes = 0);
	
	uint delPartDoc(char* json,
					uint rowNumOrDocID,
					uint tranID = 0,
					uint collID = 0,
					ValueList** pDocIDs = 0,
					uint* indexes = 0)
	{
		return updPartDoc(json,
			  			  rowNumOrDocID,
						  tranID,
						  collID,
						  pDocIDs,
						  indexes,
						  true);
	}

	uint updPartDoc(char* json,
					uint rowNumOrDocID,
					uint tranID = 0,
					uint collID = 0,
					ValueList** pDocIDs = 0,
					uint* indexes = 0,
					bool onlyDelete = false);

	uint insPartDoc(char* json,
					uint rowNumOrDocID = 0,
					uint tranID = 0,
					uint collID = 0,
					ValueList** pDocIDs = 0,
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
		for(int i=0; i<len/4; i++)
		{
			printf("%d ", str[i]);
		}
		
		printf("(%d)", level);

		printf("\n");
	}

	inline void print2(char* key, uint len, uint level)
	{
		uint* str = (uint*)key;
		for (int i = 0; i<len / 4; i++)
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

	uint getTotalMemory()
	{
		//return sizeof(DniproDB) +
		//	   //attrValuesPool.getTotalMemory() +
		//	   //valueListPool.getTotalMemory() +
		//	   ha1.getTotalMemory() +
		//	   ha2.getTotalMemory();

		return 0;
	}

	/*void clearState()
	{
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

		has1[countColls] = new HArrayVarRAM();

		strcpy(has1[countColls]->Name, name);
		
		has1[countColls]->init(4, 16);
		has1[countColls]->AllowValueList = true;

		has2[countColls] = new HArrayVarRAM();

		strcpy(has2[countColls]->Name, name);

		has2[countColls]->init(4, 16);
		has2[countColls]->AllowValueList = false;

		has2[countColls]->onContentCellMovedFunc = onContentCellMoved;
		has2[countColls]->checkDeadlockFunc = checkDeadlock;

		if (writeTranOnHDD)
		{
			addTranLog('c', name, 0, 0, countColls, 0);
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
						addTranLog('r', name, 0, 0, countColls, 0);
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
		attrValuesPool.clear();

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
		attrValuesPool.destroy();

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

