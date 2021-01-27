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

#include "BigDoc.h"
#include "BinFile.h"

HArrayTran BigDoc::_trans[MAX_TRANS];
std::atomic_bool BigDoc::blockCheckDeadlock;// = 0;

std::atomic<uint32> BigDoc::amountReaders;// = 0;
std::atomic<uint32> BigDoc::amountMarkIsReadedTrans;// = 0;
std::atomic<uint32> BigDoc::amountShapshotTrans;// = 0;

ulong64 BigDoc::tranLogSize = 0;
ulong64 BigDoc::blobLogSize = 0;

#ifdef _WIN32
CRITICAL_SECTION BigDoc::writeTranLock;
CRITICAL_SECTION BigDoc::writeBlobLock;
#endif

#ifdef __linux__
pthread_mutex_t BigDoc::writeTranLock;
pthread_mutex_t BigDoc::writeBlobLock;
#endif

std::atomic_int HArrayTran::testVal;// = 0;

void BigDoc::addTranLog(char type,
	char* json,
	uint32 docID,
	uchar8 tranID,
	uchar8 collID,
	uchar8 parentTranID,
	uint32 tranIdentity,
	uint32* indexes)
{
	//if (tranStarted)
	//{
WRITE_TRAN_LOG:

	#ifdef _WIN32
	EnterCriticalSection(&writeTranLock);
	#endif

	#ifdef __linux__
	pthread_mutex_lock( &writeTranLock );
	#endif

	ulong64 oldCurrTranLogPos = currTranLogPos;

	//packet len
	uint32* pLen = (uint32*)&tranLog[currTranLogPos];
	currTranLogPos += 4;

	//header
	tranLog[currTranLogPos++] = type;

	//docid
	*(uint32*)&tranLog[currTranLogPos] = docID;

	currTranLogPos += 4;

	//tranID
	tranLog[currTranLogPos++] = tranID;

	//collID
	tranLog[currTranLogPos++] = collID;

	//parent tran ID
	tranLog[currTranLogPos++] = parentTranID;

	//tranIdentity
	*(uint32*)&tranLog[currTranLogPos] = tranIdentity;

	currTranLogPos += 4;

	//indexes
	if (!indexes)
	{
		tranLog[currTranLogPos++] = 0;
	}
	else
	{
		uchar8* countIndexes = (uchar8*)&tranLog[currTranLogPos++];

		uchar8 i = 0;

		for (; indexes[i]; i++, currTranLogPos += 4)
		{
			*(uint32*)&tranLog[currTranLogPos] = indexes[i];
		}

		*countIndexes = i;
	}

	//json
	for (uint32 i = 0; json[i]; currTranLogPos++, i++)
	{
		if (currTranLogPos < WRITE_TRANS_BUFFER_SIZE - 2)
		{
			tranLog[currTranLogPos] = json[i];
		}
		else //buffer is full, wait and start all again
		{
			currTranLogPos = oldCurrTranLogPos;

			#ifdef _WIN32
			LeaveCriticalSection(&writeTranLock);
			#endif

			#ifdef __linux__
			pthread_mutex_unlock( &writeTranLock );
			#endif

			msleep(10);

			goto WRITE_TRAN_LOG;
		}
	}

	//zero terminate
	tranLog[currTranLogPos++] = 0;

	//len of packet
	*pLen = currTranLogPos - oldCurrTranLogPos;

	#ifdef _WIN32
	LeaveCriticalSection(&writeTranLock);
	#endif

	#ifdef __linux__
	pthread_mutex_unlock( &writeTranLock );
	#endif
	//}
}

void BigDoc::addTranLogWithCommit(char type,
	char* json,
	uint32 docID,
	uchar8 tranID,
	uchar8 collID,
	uchar8 parentTranID,
	uint32 tranIdentity,
	uint32* indexes)
{
WRITE_TRAN_LOG:

	//if (tranStarted)
	//{
	#ifdef _WIN32
	EnterCriticalSection(&writeTranLock);
	#endif

	#ifdef __linux__
	pthread_mutex_lock( &writeTranLock );
	#endif

	ulong64 newCurrTranLogPos = currTranLogPos;

	//len
	uint32* pLen = (uint32*)&tranLog[currTranLogPos];
	newCurrTranLogPos += 4;

	//header
	tranLog[newCurrTranLogPos++] = type;

	//docid
	*(uint32*)&tranLog[newCurrTranLogPos] = docID;

	newCurrTranLogPos += 4;

	//tranID
	tranLog[newCurrTranLogPos++] = tranID;

	//collID
	tranLog[newCurrTranLogPos++] = collID;

	//parent tran
	tranLog[newCurrTranLogPos++] = parentTranID;	  //time when tran is commited

	//unique identifier tran
	*(uint32*)&tranLog[newCurrTranLogPos] = tranIdentity;
	newCurrTranLogPos += 4;

	//indexes
	if (!indexes)
	{
		tranLog[newCurrTranLogPos++] = 0;
	}
	else
	{
		uchar8* countIndexes = (uchar8*)&tranLog[newCurrTranLogPos++];

		uchar8 i = 0;

		for (; indexes[i]; i++, newCurrTranLogPos += 4)
		{
			*(uint32*)&tranLog[newCurrTranLogPos] = indexes[i];
		}

		*countIndexes = i;
	}

	//json
	for (uint32 i = 0; json[i]; newCurrTranLogPos++, i++)
	{
		if (newCurrTranLogPos < WRITE_TRANS_BUFFER_SIZE - 2)
		{
			tranLog[newCurrTranLogPos] = json[i];
		}
		else //buffer is full, wait and start all again
		{
			#ifdef _WIN32
			LeaveCriticalSection(&writeTranLock);
			#endif

			#ifdef __linux__
			pthread_mutex_unlock( &writeTranLock );
			#endif

			msleep(10);

			goto WRITE_TRAN_LOG;
		}
	}

	//zero terminated
	tranLog[newCurrTranLogPos++] = 0;

	//len of packet
	*pLen = newCurrTranLogPos - currTranLogPos;

	//len
	*(uint32*)&tranLog[newCurrTranLogPos] = 14;
	newCurrTranLogPos += 4;

	//commit label
	tranLog[newCurrTranLogPos++] = '#';
	tranLog[newCurrTranLogPos++] = tranID; //tran identity

	*(uint32*)&tranLog[newCurrTranLogPos] = tranIdentity;	  //time when tran is commited
	newCurrTranLogPos += 4;

	*(uint32*)&tranLog[newCurrTranLogPos] = currTime;	  //time when tran is commited
	newCurrTranLogPos += 4;

	//write
	pTranLogFile->write(tranLog + currTranLogPos, newCurrTranLogPos - currTranLogPos);
	pTranLogFile->flush();

	#ifdef _WIN32
	LeaveCriticalSection(&writeTranLock);
	#endif

	#ifdef __linux__
	pthread_mutex_unlock( &writeTranLock );
	#endif
	//}
}

#ifdef _WIN32
DWORD WINAPI BigDoc::writeTrans(LPVOID lpParam)
#endif
#ifdef __linux__
void* BigDoc::writeTrans(void* lpParam)
#endif
{
	BigDoc* pDB = (BigDoc*)lpParam;

	char* saveBuffer1 = new char[WRITE_TRANS_BUFFER_SIZE];
	char* saveBuffer2 = new char[WRITE_TRANS_BUFFER_SIZE];

	pDB->tranLog = saveBuffer1;
	pDB->currTranLogPos = 0;

	if (BinFile::existsFile(pDB->dbTranLogFileName))
	{
		pDB->pTranLogFile = new BinFile(pDB->dbTranLogFileName, true, false);
	}
	else
	{
		pDB->pTranLogFile = new BinFile(pDB->dbTranLogFileName, true, true);
	}

	if (!pDB->pTranLogFile->open())
	{
		printf("DB file is not opened");

		return 0;
	}

	time_t tm;

	//move to end
	pDB->pTranLogFile->setPosition(tranLogSize);

	while (pDB->writeTranOnHDD)
	{
		char* saveBuffer;
		uint32 saveLength;

		//swap buffer
		#ifdef _WIN32
		EnterCriticalSection(&pDB->writeTranLock);
		#endif

		#ifdef __linux__
		pthread_mutex_lock( &pDB->writeTranLock );
		#endif

		//update time
		time(&tm);
		pDB->currTime = (uint32)tm;

		if (pDB->currTranLogPos)
		{
			saveBuffer = pDB->tranLog;
			saveLength = pDB->currTranLogPos;

			if (saveBuffer == saveBuffer1)
			{
				pDB->tranLog = saveBuffer2;
				pDB->currTranLogPos = 0;
			}
			else
			{
				pDB->tranLog = saveBuffer1;
				pDB->currTranLogPos = 0;
			}

			#ifdef _WIN32
			LeaveCriticalSection(&pDB->writeTranLock);
			#endif

			#ifdef __linux__
			pthread_mutex_unlock( &pDB->writeTranLock );
			#endif

		}
		else
		{
			#ifdef _WIN32
			LeaveCriticalSection(&pDB->writeTranLock);
			#endif

			#ifdef __linux__
			pthread_mutex_unlock( &pDB->writeTranLock );
			#endif

			pDB->msleep(1);

			continue;
		}

		//save buffer
		pDB->pTranLogFile->write(saveBuffer, saveLength);

		pDB->amountWritedTranPages++;

		//pTranLog->flush();

		//logSize += saveLength;

		////do not wait
		//if(pStorage->pDB->currTranLogPos)
		//{
		//	msleep(100);

		//	continue;
		//}

		//printf("Log saved %d.\n", saveLength);
	}

	delete[] saveBuffer1;
	delete[] saveBuffer2;

	pDB->pTranLogFile->close();

	return 0;
}

bool BigDoc::readTrans(char* filePath,
					   uint32 onDate = 0)
{
	const uint32 buffSize = 10 * WRITE_TRANS_BUFFER_SIZE;

	BinFile* pFile = new BinFile(filePath, false, false);

	if (pFile->open())
	{
		bool oldWriteTranOnHDD = writeTranOnHDD;

		writeTranOnHDD = false;

		BigDocInfo::Print("Read log ...\n");

		ulong64 fileSize = pFile->getFileSize(); //avoid zero

		char* buff = new char[buffSize];

		uint32 indexes[256];

		uint32 safeLen = buffSize - WRITE_TRANS_BUFFER_SIZE;

		uint32 pos = 0;
		uint32 startTranPos = 0;

		while (true)
		{
			if (fileSize)
			{
				uint32 perc = pos * 100 / fileSize;

				if (perc)
				{
					BigDocInfo::Print("\rProgress: %u%%", perc);
				}
			}

			uint32 len = pFile->read(buff, pos, buffSize);

			uint32 i = 0;
			for (; i < safeLen && i < len;)
			{
				//is not corrupted packet ?
				if (i + 4 < len)
				{
					uint32* pLen = (uint32*)&buff[i];

					if (i + *pLen > len ||
						*pLen == 0) //empty file
					{
						break; //corrupted
					}
				}
				else
				{
					break; //corrupted
				}

				i += 4;

				//read packet
				char commandType = buff[i];
				i++;

				if (commandType == '#') //commit
				{
					uchar8 tranID = *(uint32*)(buff + i);
					i++;

					uint32 tranIdentity = *(uint32*)(buff + i);
					i += 4;

					if (tranIdentity > this->tranIdentity)
					{
						this->tranIdentity = tranIdentity;
					}

					uint32 time = *(uint32*)(buff + i);
					i += 4;

					if (onDate && time > onDate)
					{
						break; //do not read other trans
					}

					if (_trans[tranID].InUse && 
						_trans[tranID].TranIdentity == tranIdentity)
					{
						commitTran(tranID);
					}
					else
					{
						rollbackTran(tranID);
					}

					//start next tran pos
					startTranPos = i + 1;
				}
				else if (commandType == '*') //rollback
				{
					uchar8 tranID = *(uint32*)(buff + i);
					i++;

					uint32 tranIdentity = *(uint32*)(buff + i);
					i += 4;

					if (tranIdentity > this->tranIdentity)
					{
						this->tranIdentity = tranIdentity;
					}

					if (_trans[tranID].InUse &&
						_trans[tranID].TranIdentity == tranIdentity)
					{
						rollbackTran(tranID);
					}
				}
				else //query
				{
					uint32 docID = *(uint32*)(buff + i);
					i += 4;

					uchar8 tranID = *(uint32*)(buff + i);
					i++;

					uchar8 collID = *(uint32*)(buff + i);
					i++;

					uchar8 parentTranID = *(uint32*)(buff + i);
					i++;

					uint32 tranIdentity = *(uint32*)(buff + i);
					i += 4;

					//format countIndexes|indexes
					uchar8 countIndexes = *(uchar8*)(buff + i);
					i++;

					uchar8 j = 0;

					for (; j < countIndexes; j++)
					{
						indexes[j] = *(uint32*)(buff + i);

						i += 4;
					}

					indexes[j] = 0;
					
					if (tranIdentity > this->tranIdentity)
					{
						this->tranIdentity = tranIdentity;
					}

					if (tranID) //for transaction commands
					{
						if (!_trans[tranID].InUse)
						{
							//begin tran
							_trans[tranID].TranType = READ_COMMITED_TRAN;
							_trans[tranID].TranIdentity = tranIdentity;
							_trans[tranID].ParentTranID = parentTranID;

							if (parentTranID)
							{
								if (!_trans[parentTranID].pGrandParentTran)
								{
									_trans[tranID].pGrandParentTran = &_trans[parentTranID];
								}
								else
								{
									_trans[tranID].pGrandParentTran = _trans[parentTranID].pGrandParentTran;
								}

								_trans[tranID].pGrandParentTran->HasChilds = true;
							}
							
							_trans[tranID].InUse = true;
						}
						else
						{
							// not finished tran, rollback and start new
							if (_trans[tranID].InUse && _trans[tranID].TranIdentity != tranIdentity)
							{
								rollbackTran(tranID);

								//begin tran
								_trans[tranID].TranType = READ_COMMITED_TRAN;
								_trans[tranID].TranIdentity = tranIdentity;
								_trans[tranID].ParentTranID = parentTranID;

								if (parentTranID)
								{
									if (!_trans[parentTranID].pGrandParentTran)
									{
										_trans[tranID].pGrandParentTran = &_trans[parentTranID];
									}
									else
									{
										_trans[tranID].pGrandParentTran = _trans[parentTranID].pGrandParentTran;
									}

									_trans[tranID].pGrandParentTran->HasChilds = true;
								}

								_trans[tranID].InUse = true;
							}
						}
					}

					switch (commandType)
					{
					case 'i':
					{
						i += insPartDoc(buff + i, docID, tranID, collID, indexes);

						if (docID > lastDocID)
						{
							lastDocID = docID;
						}

						i++; //zero terminated

						break;
					}
					case 'u':
					{
						i += updPartDoc(buff + i, docID, tranID, collID, indexes);

						i++; //zero terminated

						break;
					}
					case 'd':
					{
						i += delPartDoc(buff + i, docID, tranID, collID, indexes);

						i++; //zero terminated

						break;
					}
					case 'c':
					{
						i += addColl(buff + i);

						i++; //zero terminated

						break;
					}
					case 'r':
					{
						i += delColl(buff + i);

						i++; //zero terminated

						break;
					}
					default:
					{
						BigDocError::Print(0, "DB File corrupted.\n");

						break;
					}
					}
				}
			}

			tranLogSize = i;

			if (len == buffSize) //not all readed
			{
				pos += i;
			}
			else
			{
				break;
			}
		}

		BigDocInfo::Print("\rProgress: 100%%\n");

		//pDB->tranStarted = true;

		pFile->close();

		delete pFile;

		delete[] buff;

		//rollback unfinished trans
		for (uint32 i = 1; i < MAX_TRANS; i++)
		{
			if (_trans[i].InUse)
			{
				rollbackTran(_trans[i].TranID);
			}
		}

		//need shrink file
		if (onDate)
		{
			BinFile* pFile = new BinFile(filePath, true, false);

			if (pFile->open())
			{
				pFile->setPosition(startTranPos);

				uchar8 blank = 0;

				for (uint32 i = startTranPos; i < fileSize; i++)
				{
					pFile->writeChar(&blank);
				}

				//pFile->flush();
				pFile->close();
			}

			delete pFile;

			time_t tm = (time_t)onDate;
			BigDocInfo::Print("Log restored on date: %s\n", ctime(&tm));
		}

		BigDocInfo::Print("Log read successful.\n");

		writeTranOnHDD = oldWriteTranOnHDD;

		return true;
	}
	else
	{
		delete pFile;

		return false;
	}
}

bool BigDoc::readBlobs(char* filePath)
{
	pBlobLogFile = new BinFile(filePath, true, true);

	if (pBlobLogFile->open())
	{
		blobLogSize = pBlobLogFile->getFileSize();

		return true;
	}
	/*else
	{
		delete pFile;

		return false;
	}*/

	return false;
}

bool BigDoc::checkDeadlock(uchar8 tranID)
{
	bool val = false;

	//block another check deadlocks
	while (!blockCheckDeadlock.compare_exchange_weak(val, true))
	{
		msleep(1);

		val = false;
	}

	for (uint32 i = 1; i < MAX_TRANS; i++)
	{
		if (i != tranID &&
			_trans[i].readedList.BlockedByTranID &&
			_trans[tranID].readedList.BlockedByTranID &&
			_trans[i].readedList.BlockedByTranID == _trans[tranID].TranID &&
			_trans[tranID].readedList.BlockedByTranID == _trans[i].TranID) //deadlock !!
		{
			BigDocError de;

			de.Code = DEADLOCK_ERROR;
			sprintf(de.Message,
				"Deadlock between trans %d and %d.\nBetween queries '%s' and '%s'\nBetween attr values '%s' and '%s'. First tran was killed.",
				tranID, _trans[i].TranID,
				_trans[tranID].LastJson,
				_trans[i].LastJson,
				(char*)_trans[tranID].readedList.BlockedOnValue,
				(char*)_trans[i].readedList.BlockedOnValue);

			//rollback tran
			HArrayTran& tran = _trans[tranID];

			if (tran.TranType == REPEATABLE_READ_TRAN)
			{
				amountMarkIsReadedTrans--;
			}
			else if (tran.TranType == SNAPSHOT_TRAN)
			{
				amountMarkIsReadedTrans--;

				amountShapshotTrans--;
			}

			amountReaders--;

			tran.clear();

			blockCheckDeadlock.store(false);

			//exit
			throw de;

			return true;
		}
	}

	blockCheckDeadlock.store(false);

	return false;
}

void BigDoc::clearChilds(uint32 tranID)
{
	for (uint32 i = 1; i < MAX_TRANS; i++)
	{
		if (_trans[i].ParentTranID == tranID)
		{
			clearChilds(_trans[i].TranID);

			_trans[i].clear();
		}
	}
}

void BigDoc::clearParts(uint32 tranID)
{
	//for parent tran, clear data of nested trans
	for (uint32 i = 1; i < MAX_TRANS; i++)
	{
		if (_trans[i].ParentTranID == tranID)
		{
			//clear childs
			clearParts(_trans[i].TranID);
		}
	}

	//clear myself
	_trans[tranID].pGrandParentTran->clearPart(tranID);
}

uint32 BigDoc::beginTran(uchar8 tranType,
						 uchar8 parentTranID)
{
	while (true)
	{
		for (uint32 i = 1; i < MAX_TRANS; i++)
		{
			HArrayTran& tran = _trans[i];

			bool val = false;

			if (tran.InUse.compare_exchange_strong(val, true))
			{
				tran.TranType = tranType;
				tran.TranIdentity = ++tranIdentity;

				//nested tran
				if (parentTranID)
				{
					tran.ParentTranID = parentTranID;
					
					_trans[parentTranID].HasChilds = true;

					if (!_trans[parentTranID].pGrandParentTran)
					{
						tran.pGrandParentTran = &_trans[parentTranID];
					}
					else
					{
						tran.pGrandParentTran = _trans[parentTranID].pGrandParentTran;
					}

					if (tran.TranType > tran.pGrandParentTran->TranType)
					{
						tran.clear();

						BigDocError de;
						de.Code = 0;
						strcpy(de.Message, "Isolation level of nested transaction can not be more than parent transaction");

						throw de;
					}
				}
				else
				{
					if (tranType == REPEATABLE_READ_TRAN)
					{
						amountMarkIsReadedTrans++;
					}
					else if (tranType == SNAPSHOT_TRAN)
					{
						amountMarkIsReadedTrans++;

						amountShapshotTrans++;
					}
				}

				return i;
			}
		}

		msleep(10);
	}

	return 1;
}

void BigDoc::rollbackTran(uint32 tranID)
{
	HArrayTran& tran = _trans[tranID];

	if (!tran.ParentTranID)
	{
		if (tran.TranType == REPEATABLE_READ_TRAN)
		{
			amountMarkIsReadedTrans--;
		}
		else if (tran.TranType == SNAPSHOT_TRAN)
		{
			amountMarkIsReadedTrans--;

			amountShapshotTrans--;
		}
	}
	else
	{
		//our data in parent grand tran
		clearParts(tran.TranID);

		//write on disc
		if (writeTranOnHDD)
		{
			while (tran.LastWritedOnTranPage >= amountWritedTranPages) //our page is not saved, wait
			{
				msleep(1);
			}

			//save rollback label
			char rollbackLabel[10];

			//len
			*(uint32*)&rollbackLabel[0] = 10;

			rollbackLabel[4] = '*'; //rollback
			rollbackLabel[5] = tran.TranID;					//tran ID
			*(uint32*)&rollbackLabel[6] = tran.TranIdentity;	//time identity

			#ifdef _WIN32
			EnterCriticalSection(&writeTranLock);
			#endif

			#ifdef __linux__
			pthread_mutex_lock( &writeTranLock );
			#endif

			pTranLogFile->write(rollbackLabel, 10);
			pTranLogFile->flush();

			#ifdef _WIN32
			LeaveCriticalSection(&writeTranLock);
			#endif

			#ifdef __linux__
			pthread_mutex_unlock( &writeTranLock );
			#endif
		}
	}

	if (tran.HasChilds)
	{
		//clear all child trans
		clearChilds(tran.TranID);
	}

	//clear tran
	tran.clear();
}

void BigDoc::clearTran(uint32 tranID)
{
	_trans[tranID].clear(); //always readonly tran
}

void BigDoc::commitTran(uint32 tranID)
{
	HArrayTran& tran = _trans[tranID];

	if (tran.TranType && !tran.ParentTranID) //not readonly tran and commit will be not in parent tran
	{
		if (tran.TranType == SNAPSHOT_TRAN)
		{
			amountShapshotTrans--;
		}

	TRY_COMMIT:

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

		bool hasReadedCells = false;

		//release readed cells
		if (tran.TranType == READ_COMMITED_TRAN)
		{
			if (amountMarkIsReadedTrans > 0)		//there is another snap tran
			{
				if (tran.isModifyCellsOfOtherTrans())	//check if we will modify cells of other trans
				{
					//allow readers
					blockReaders.store(false);

					//allow writers
					blockWriters.store(false);

					//give time for other trans
					msleep(10);

					//try again
					goto TRY_COMMIT;
				}

				hasReadedCells = true;
			}
		}
		else //if (tran.TranType == REPEATABLE_READ_TRAN || tran.TranType == SNAPSHOT_TRAN)
		{
			if (amountMarkIsReadedTrans > 1)		//first my snap tran, second another snap tran
			{
				if (tran.isModifyCellsOfOtherTransExcludeMine())	//check if we will modify cells of other trans
				{
					//allow readers
					blockReaders.store(false);

					//allow writers
					blockWriters.store(false);

					//give time for other trans
					msleep(10);

					//try again
					goto TRY_COMMIT;
				}

				hasReadedCells = true;
			}

			//release readed values
			tran.readedList.releaseValues();

			amountMarkIsReadedTrans--;
		}

		//commit tran
		tran.commit(hasReadedCells);

		//allow readers
		blockReaders.store(false);

		//allow writers
		blockWriters.store(false);

		//commit tran on disc
		if (tran.IsWritable && writeTranOnHDD)
		{
			while (tran.LastWritedOnTranPage >= amountWritedTranPages) //our page is not saved, wait
			{
				msleep(1);
			}

			//save commit label
			char commitLabel[14];

			//len
			*(uint32*)&commitLabel[0] = 14;

			commitLabel[4] = '#';
			commitLabel[5] = tran.TranID;				//tran ID
			*(uint32*)&commitLabel[6] = tran.TranIdentity;	//time identity
			*(uint32*)&commitLabel[10] = currTime;			//time when tran is commited

			#ifdef _WIN32
			EnterCriticalSection(&writeTranLock);
			#endif

			#ifdef __linux__
			pthread_mutex_lock( &writeTranLock );
			#endif

			pTranLogFile->write(commitLabel, 14);
			pTranLogFile->flush();

			#ifdef _WIN32
			LeaveCriticalSection(&writeTranLock);
			#endif

			#ifdef __linux__
			pthread_mutex_unlock( &writeTranLock );
			#endif
		}

		if (tran.HasChilds)
		{
			clearChilds(tran.TranID);
		}

		//clear tran
		tran.clear();
	}
}
