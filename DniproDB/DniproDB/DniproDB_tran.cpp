#include "DniproDB.h"

HArrayTran DniproDB::_trans[MAX_TRANS];
std::atomic_bool DniproDB::blockCheckDeadlock = 0;

std::atomic<uint> DniproDB::amountReaders = 0;
std::atomic<uint> DniproDB::amountMarkIsReadedTrans = 0;
std::atomic<uint> DniproDB::amountShapshotTrans = 0;

ulong DniproDB::tranLogSize = 0;
ulong DniproDB::blobLogSize = 0;

std::atomic_int HArrayTran::testVal = 0;

void DniproDB::addTranLog(char type,
	char* json,
	uint docID,
	uchar tranID,
	uchar collID,
	uchar parentTranID,
	uint tranIdentity,
	uint* indexes)
{
	//if (tranStarted)
	//{
WRITE_TRAN_LOG:

	EnterCriticalSection(&writeTranLock);

	ulong oldCurrTranLogPos = currTranLogPos;

	//packet len
	uint* pLen = (uint*)&tranLog[currTranLogPos];
	currTranLogPos += 4;

	//header
	tranLog[currTranLogPos++] = type;

	//docid
	*(uint*)&tranLog[currTranLogPos] = docID;

	currTranLogPos += 4;

	//tranID
	tranLog[currTranLogPos++] = tranID;

	//collID
	tranLog[currTranLogPos++] = collID;

	//parent tran ID
	tranLog[currTranLogPos++] = parentTranID;

	//tranIdentity
	*(uint*)&tranLog[currTranLogPos] = tranIdentity;

	currTranLogPos += 4;

	//indexes
	if (!indexes)
	{
		tranLog[currTranLogPos++] = 0;
	}
	else
	{
		uchar* countIndexes = (uchar*)&tranLog[currTranLogPos++];

		uchar i = 0;

		for (; indexes[i]; i++, currTranLogPos += 4)
		{
			*(uint*)&tranLog[currTranLogPos] = indexes[i];
		}

		*countIndexes = i;
	}

	//json
	for (uint i = 0; json[i]; currTranLogPos++, i++)
	{
		if (currTranLogPos < WRITE_TRANS_BUFFER_SIZE - 2)
		{
			tranLog[currTranLogPos] = json[i];
		}
		else //buffer is full, wait and start all again
		{
			currTranLogPos = oldCurrTranLogPos;

			LeaveCriticalSection(&writeTranLock);

			Sleep(10);

			goto WRITE_TRAN_LOG;
		}
	}

	//zero terminate
	tranLog[currTranLogPos++] = 0;

	//len of packet
	*pLen = currTranLogPos - oldCurrTranLogPos;

	LeaveCriticalSection(&writeTranLock);
	//}
}

void DniproDB::addTranLogWithCommit(char type,
	char* json,
	uint docID,
	uchar tranID,
	uchar collID,
	uchar parentTranID,
	uint tranIdentity,
	uint* indexes)
{
WRITE_TRAN_LOG:

	//if (tranStarted)
	//{
	EnterCriticalSection(&writeTranLock);

	ulong newCurrTranLogPos = currTranLogPos;

	//len
	uint* pLen = (uint*)&tranLog[currTranLogPos];
	newCurrTranLogPos += 4;

	//header
	tranLog[newCurrTranLogPos++] = type;

	//docid
	*(uint*)&tranLog[newCurrTranLogPos] = docID;

	newCurrTranLogPos += 4;

	//tranID
	tranLog[newCurrTranLogPos++] = tranID;

	//collID
	tranLog[newCurrTranLogPos++] = collID;

	//parent tran
	tranLog[newCurrTranLogPos++] = parentTranID;	  //time when tran is commited

	//unique identifier tran
	*(uint*)&tranLog[newCurrTranLogPos] = tranIdentity;
	newCurrTranLogPos += 4;

	//indexes
	if (!indexes)
	{
		tranLog[newCurrTranLogPos++] = 0;
	}
	else
	{
		uchar* countIndexes = (uchar*)&tranLog[newCurrTranLogPos++];

		uchar i = 0;

		for (; indexes[i]; i++, newCurrTranLogPos += 4)
		{
			*(uint*)&tranLog[newCurrTranLogPos] = indexes[i];
		}

		*countIndexes = i;
	}

	//json
	for (uint i = 0; json[i]; newCurrTranLogPos++, i++)
	{
		if (newCurrTranLogPos < WRITE_TRANS_BUFFER_SIZE - 2)
		{
			tranLog[newCurrTranLogPos] = json[i];
		}
		else //buffer is full, wait and start all again
		{
			LeaveCriticalSection(&writeTranLock);

			Sleep(10);

			goto WRITE_TRAN_LOG;
		}
	}

	//zero terminated
	tranLog[newCurrTranLogPos++] = 0;

	//len of packet
	*pLen = newCurrTranLogPos - currTranLogPos;

	//len
	*(uint*)&tranLog[newCurrTranLogPos] = 14;
	newCurrTranLogPos += 4;

	//commit label
	tranLog[newCurrTranLogPos++] = '#';
	tranLog[newCurrTranLogPos++] = tranID; //tran identity

	*(uint*)&tranLog[newCurrTranLogPos] = tranIdentity;	  //time when tran is commited
	newCurrTranLogPos += 4;

	*(uint*)&tranLog[newCurrTranLogPos] = currTime;	  //time when tran is commited
	newCurrTranLogPos += 4;

	//write
	pTranLogFile->write(tranLog + currTranLogPos, newCurrTranLogPos - currTranLogPos);
	pTranLogFile->flush();

	LeaveCriticalSection(&writeTranLock);
	//}
}

DWORD WINAPI DniproDB::writeTrans(LPVOID lpParam)
{
	DniproDB* pDB = (DniproDB*)lpParam;

	char* saveBuffer1 = new char[WRITE_TRANS_BUFFER_SIZE];
	char* saveBuffer2 = new char[WRITE_TRANS_BUFFER_SIZE];

	pDB->tranLog = saveBuffer1;
	pDB->currTranLogPos = 0;

	if (BinaryFile::existsFile(pDB->dbTranLogFileName))
	{
		pDB->pTranLogFile = new BinaryFile(pDB->dbTranLogFileName, true, false);
	}
	else
	{
		pDB->pTranLogFile = new BinaryFile(pDB->dbTranLogFileName, true, true);
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
		uint saveLength;

		//swap buffer
		EnterCriticalSection(&pDB->writeTranLock);

		//update time
		time(&tm);
		pDB->currTime = (uint)tm;

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

			LeaveCriticalSection(&pDB->writeTranLock);
		}
		else
		{
			LeaveCriticalSection(&pDB->writeTranLock);

			Sleep(1);

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
		//	Sleep(100);

		//	continue;
		//}

		//printf("Log saved %d.\n", saveLength);
	}

	delete[] saveBuffer1;
	delete[] saveBuffer2;

	pDB->pTranLogFile->close();

	return 0;
}

bool DniproDB::readTrans(char* filePath,
						 uint onDate = 0)
{
	const uint buffSize = 10 * WRITE_TRANS_BUFFER_SIZE;

	BinaryFile* pFile = new BinaryFile(filePath, false, false);

	if (pFile->open())
	{
		bool oldWriteTranOnHDD = writeTranOnHDD;

		writeTranOnHDD = false;

		DniproInfo::Print("Read log ...\n");

		ulong fileSize = pFile->getFileSize(); //avoid zero

		char* buff = new char[buffSize];

		uint indexes[256];

		uint safeLen = buffSize - WRITE_TRANS_BUFFER_SIZE;

		uint pos = 0;
		uint startTranPos = 0;

		while (true)
		{
			if (fileSize)
			{
				uint perc = pos * 100 / fileSize;

				if (perc)
				{
					DniproInfo::Print("\rProgress: %u%%", perc);
				}
			}

			uint len = pFile->read(buff, pos, buffSize);

			uint i = 0;
			for (; i < safeLen && i < len;)
			{
				//is not corrupted packet ?
				if (i + 4 < len)
				{
					uint* pLen = (uint*)&buff[i];

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
					uchar tranID = *(uint*)(buff + i);
					i++;

					uint tranIdentity = *(uint*)(buff + i);
					i += 4;

					if (tranIdentity > this->tranIdentity)
					{
						this->tranIdentity = tranIdentity;
					}

					uint time = *(uint*)(buff + i);
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
					uchar tranID = *(uint*)(buff + i);
					i++;

					uint tranIdentity = *(uint*)(buff + i);
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
					uint docID = *(uint*)(buff + i);
					i += 4;

					uchar tranID = *(uint*)(buff + i);
					i++;

					uchar collID = *(uint*)(buff + i);
					i++;

					uchar parentTranID = *(uint*)(buff + i);
					i++;

					uint tranIdentity = *(uint*)(buff + i);
					i += 4;

					//format countIndexes|indexes
					uchar countIndexes = *(uchar*)(buff + i);
					i++;

					uchar j = 0;

					for (; j < countIndexes; j++)
					{
						indexes[j] = *(uint*)(buff + i);

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
						DniproError::Print(0, "DB File corrupted.\n");

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

		DniproInfo::Print("\rProgress: 100%%\n");

		//pDB->tranStarted = true;

		pFile->close();

		delete pFile;

		delete[] buff;

		//rollback unfinished trans
		for (uint i = 1; i < MAX_TRANS; i++)
		{
			if (_trans[i].InUse)
			{
				rollbackTran(_trans[i].TranID);
			}
		}

		//need shrink file
		if (onDate)
		{
			BinaryFile* pFile = new BinaryFile(filePath, true, false);

			if (pFile->open())
			{
				pFile->setPosition(startTranPos);

				uchar blank = 0;

				for (uint i = startTranPos; i < fileSize; i++)
				{
					pFile->writeChar(&blank);
				}

				//pFile->flush();
				pFile->close();
			}

			delete pFile;

			time_t tm = (time_t)onDate;
			DniproInfo::Print("Log restored on date: %s\n", ctime(&tm));
		}

		DniproInfo::Print("Log readed.\n");

		writeTranOnHDD = oldWriteTranOnHDD;

		return true;
	}
	else
	{
		delete pFile;

		return false;
	}
}

bool DniproDB::readBlobs(char* filePath)
{
	pBlobLogFile = new BinaryFile(filePath, true, true);

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

bool DniproDB::checkDeadlock(uchar tranID)
{
	bool val = false;

	//block another check deadlocks
	while (!blockCheckDeadlock.compare_exchange_weak(val, true))
	{
		Sleep(1);

		val = false;
	}

	for (uint i = 1; i < MAX_TRANS; i++)
	{
		if (i != tranID &&
			_trans[i].readedList.BlockedByTranID &&
			_trans[tranID].readedList.BlockedByTranID &&
			_trans[i].readedList.BlockedByTranID == _trans[tranID].TranID &&
			_trans[tranID].readedList.BlockedByTranID == _trans[i].TranID) //deadlock !!
		{
			DniproError de;

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

void DniproDB::clearChilds(uint tranID)
{
	for (uint i = 1; i < MAX_TRANS; i++)
	{
		if (_trans[i].ParentTranID == tranID)
		{
			clearChilds(_trans[i].TranID);

			_trans[i].clear();
		}
	}
}

void DniproDB::clearParts(uint tranID)
{
	//for parent tran, clear data of nested trans
	for (uint i = 1; i < MAX_TRANS; i++)
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

uint DniproDB::beginTran(uchar tranType,
						 uchar parentTranID)
{
	while (true)
	{
		for (uint i = 1; i < MAX_TRANS; i++)
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

						DniproError de;
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

		Sleep(10);
	}

	return 1;
}

void DniproDB::rollbackTran(uint tranID)
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
				Sleep(1);
			}

			//save rollback label
			char rollbackLabel[10];

			//len
			*(uint*)&rollbackLabel[0] = 10;

			rollbackLabel[4] = '*'; //rollback
			rollbackLabel[5] = tran.TranID;					//tran ID
			*(uint*)&rollbackLabel[6] = tran.TranIdentity;	//time identity

			EnterCriticalSection(&writeTranLock);

			pTranLogFile->write(rollbackLabel, 10);
			pTranLogFile->flush();

			LeaveCriticalSection(&writeTranLock);
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

void DniproDB::clearTran(uint tranID)
{
	_trans[tranID].clear(); //always readonly tran
}

void DniproDB::commitTran(uint tranID)
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
					Sleep(10);

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
					Sleep(10);

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
				Sleep(1);
			}

			//save commit label
			char commitLabel[14];

			//len
			*(uint*)&commitLabel[0] = 14;

			commitLabel[4] = '#';
			commitLabel[5] = tran.TranID;				//tran ID
			*(uint*)&commitLabel[6] = tran.TranIdentity;	//time identity
			*(uint*)&commitLabel[10] = currTime;			//time when tran is commited

			EnterCriticalSection(&writeTranLock);

			pTranLogFile->write(commitLabel, 14);
			pTranLogFile->flush();

			LeaveCriticalSection(&writeTranLock);
		}

		if (tran.HasChilds)
		{
			clearChilds(tran.TranID);
		}

		//clear tran
		tran.clear();
	}
}
