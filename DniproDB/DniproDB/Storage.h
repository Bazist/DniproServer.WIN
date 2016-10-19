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

#include "AttrValuesPool.h"
#include "DniproDB.h"

class Storage
{
public:
	/*
	Storage(DniproDB* pDB, char* folder)
	{
		this->pDB = pDB;
		//this->pDB->tranStarted = true;

		sprintf(logFileName, "%s\\log.pg", folder);
		sprintf(prevLogFileName, "%s\\log_prev.pg", folder);

		sprintf(backupCurrFileName, "%s\\db_backup.pg", folder);
		sprintf(currFileName, "%s\\db.pg", folder);
	}

	DniproDB* pDB;
	
	char logFileName[1024];
	char prevLogFileName[1024];

	char currFileName[1024];
	char backupCurrFileName[1024];
	
	void createBinaryFiles(BinaryFile* pFile, bool isWritable)
	{
		//db
		char path[1024];

		sprintf(path, "%s\\db.pg", currentFolder);
		pFiles[0] = new BinaryFile(path, isWritable, isWritable);

		//ha1
		sprintf(path, "%s\\ha1_info.pg", currentFolder);
		pFiles[1] = new BinaryFile(path, isWritable, isWritable);
		
		sprintf(path, "%s\\ha1_header.pg", currentFolder);
		pFiles[2] = new BinaryFile(path, isWritable, isWritable);
		
		sprintf(path, "%s\\ha1_content.pg", currentFolder);
		pFiles[3] = new BinaryFile(path, isWritable, isWritable);
		
		sprintf(path, "%s\\ha1_var.pg", currentFolder);
		pFiles[4] = new BinaryFile(path, isWritable, isWritable);
		
		sprintf(path, "%s\\ha1_branch.pg", currentFolder);
		pFiles[5] = new BinaryFile(path, isWritable, isWritable);
		
		sprintf(path, "%s\\ha1_block.pg", currentFolder);
		pFiles[6] = new BinaryFile(path, isWritable, isWritable);

		//ha2
		sprintf(path, "%s\\ha2_info.pg", currentFolder);
		pFiles[7] = new BinaryFile(path, isWritable, isWritable);
		
		sprintf(path, "%s\\ha2_header.pg", currentFolder);
		pFiles[8] = new BinaryFile(path, isWritable, isWritable);
		
		sprintf(path, "%s\\ha2_content.pg", currentFolder);
		pFiles[9] = new BinaryFile(path, isWritable, isWritable);
		
		sprintf(path, "%s\\ha2_var.pg", currentFolder);
		pFiles[10] = new BinaryFile(path, isWritable, isWritable);
		
		sprintf(path, "%s\\ha2_branch.pg", currentFolder);
		pFiles[11] = new BinaryFile(path, isWritable, isWritable);
		
		sprintf(path, "%s\\ha2_block.pg", currentFolder);
		pFiles[12] = new BinaryFile(path, isWritable, isWritable);

		//attrValues
		sprintf(path, "%s\\attrValuesPath.pg", currentFolder);
		pFiles[13] = new BinaryFile(path, isWritable, isWritable);

		//valueLists
		sprintf(path, "%s\\valueLists.pg", currentFolder);
		pFiles[14] = new BinaryFile(path, isWritable, isWritable);
	}
	

	static void error()
	{
		printf("Error\n");
	}
	
	static DWORD WINAPI syncJob( LPVOID lpParam ) 
	{
		Storage* pStorage = (Storage*)lpParam;

		char* saveBuffer1 = new char[10000000];
		char* saveBuffer2 = new char[10000000];

		pStorage->pDB->tranLog = saveBuffer1;

		BinaryFile* pTranLog;
		
		if(BinaryFile::existsFile(pStorage->logFileName))
		{
			pTranLog = new BinaryFile(pStorage->logFileName, true, false);
		}
		else
		{
			pTranLog = new BinaryFile(pStorage->logFileName, true, true);
		}

		if(!pTranLog->open())
		{
			error();

			return 0;
		}

		uint logSize = pTranLog->getFileSize();

		while(true)
		{
			char* saveBuffer;
			uint saveLength;

			//sync with hdd
			if(pStorage->pDB->currTranLogPos)
			{
				//swap buffer
				EnterCriticalSection(&pStorage->pDB->writeTranLock);

				saveBuffer = pStorage->pDB->tranLog;
				saveLength = pStorage->pDB->currTranLogPos;

				if(saveBuffer == saveBuffer1)
				{
					pStorage->pDB->tranLog = saveBuffer2;
					pStorage->pDB->currTranLogPos = 0;
				}
				else
				{
					pStorage->pDB->tranLog = saveBuffer1;
					pStorage->pDB->currTranLogPos = 0;
				}
			
				LeaveCriticalSection(&pStorage->pDB->writeTranLock);

				//save buffer
				pTranLog->write(saveBuffer, saveLength);
				pTranLog->flush();

				logSize += saveLength;

				////do not wait
				//if(pStorage->pDB->currTranLogPos)
				//{
				//	Sleep(100);

				//	continue;
				//}

				printf("Log saved %d.\n", saveLength);
			}

			//shrink log
			//if(logSize > 200 * 1024 * 1024) //on 100 mb shrink log
			//{
			//	EnterCriticalSection(&pStorage->pDB->Lock);

			//	uint tm1 = pStorage->pDB->ha1.getTotalMemory();
			//	uint tm2 = pStorage->pDB->ha2.getTotalMemory();
			//	uint tm3 = pStorage->pDB->attrValuesPool.getTotalMemory();

			//	BinaryFile* pFile1 = new BinaryFile(pStorage->currFileName,
			//									    true,
			//									    true,
			//									    128);

			//	BinaryFile* pFile2 = new BinaryFile(pStorage->currFileName,
			//									    true,
			//									    false,
			//									    tm1);

			//	BinaryFile* pFile3 = new BinaryFile(pStorage->currFileName,
			//									    true,
			//									    false,
			//									    tm2);

			//	BinaryFile* pFile4 = new BinaryFile(pStorage->currFileName,
			//									    true,
			//									    false,
			//									    tm3);

			//	if(!pStorage->save(pFile1,
			//					   pFile2,
			//					   pFile3,
			//					   pFile4))
			//	{
			//		delete pFile1;

			//		delete pFile2;

			//		delete pFile3;

			//		delete pFile4;

			//		printf("not enough place\n");

			//		LeaveCriticalSection(&pStorage->pDB->Lock);

			//		Sleep(200);

			//		//not enough place to save, repeat
			//		continue;
			//	}

			//	if(!pStorage->pDB->currTranLogPos) //equals
			//	{
			//		logSize = 0;

			//		//version is ok
			//		//make backups
			//		if(BinaryFile::existsFile(pStorage->currFileName))
			//		{
			//			BinaryFile::renameFile(pStorage->currFileName,
			//								   pStorage->backupCurrFileName);
			//		}
			//		else //create empty file, for identify failed state
			//		{
			//			BinaryFile* pTempFile = new BinaryFile(pStorage->backupCurrFileName, true, true);
			//			if(pTempFile->open())
			//			{
			//				pTempFile->close();
			//			}
			//			else
			//			{
			//				error();
			//				
			//				return 0;
			//			}
			//		}

			//		//close tranlog
			//		pTranLog->close();

			//		delete pTranLog;

			//		//rename tranlog
			//		BinaryFile::renameFile(pStorage->logFileName,
			//							   pStorage->prevLogFileName);

			//		//create new tranlog
			//		pTranLog = new BinaryFile(pStorage->logFileName, true, true);

			//		if(!pTranLog->open())
			//		{
			//			error();
			//			
			//			return 0;
			//		}

			//		LeaveCriticalSection(&pStorage->pDB->Lock);

			//		//save current state
			//		if(pFile1->save() &&
			//		   pFile2->save() &&
			//		   pFile3->save() &&
			//		   pFile4->save())
			//		{
			//			printf("Log shrinked.\n");

			//			pFile1->close();
			//			delete pFile1;

			//			pFile2->close();
			//			delete pFile2;

			//			pFile3->close();
			//			delete pFile3;

			//			pFile4->close();
			//			delete pFile4;
			//		}
			//		else
			//		{
			//			error();
			//			
			//			return 0;
			//		}

			//		//delete old log
			//		BinaryFile::deleteFile(pStorage->backupCurrFileName);

			//		BinaryFile::deleteFile(pStorage->prevLogFileName);
			//		
			//		logSize = 0;
			//	}
			//	else
			//	{
			//		LeaveCriticalSection(&pStorage->pDB->Lock);

			//		delete pFile1;

			//		delete pFile2;

			//		delete pFile3;

			//		delete pFile4;
			//	}
			//}

			Sleep(200);
		}

		delete[] saveBuffer1;
		delete[] saveBuffer2;

		pTranLog->close();

		return 0;
	}

	bool load()
	{
		//BinaryFile::deleteFile(logFileName);

		if(BinaryFile::existsFile(backupCurrFileName))
		{
			//restore after fail
			if(BinaryFile::existsFile(prevLogFileName))
			{
				//load from backups
				if(!read(backupCurrFileName))
				{
					error();

					return false;
				}

				if(!readLog(prevLogFileName))
				{
					error();

					return false;
				}

				//save
				if(!save(currFileName))
				{
					error();

					return false;
				}

				//delete backups
				BinaryFile::deleteFile(backupCurrFileName);

				BinaryFile::deleteFile(prevLogFileName);

				//load data
				if(!readLog(logFileName))
				{
					error();

					return false;
				}
			}
			else			
			{
				//return to backup
				BinaryFile::deleteFile(currFileName);
				
				BinaryFile::renameFile(backupCurrFileName, currFileName);

				if(!read(currFileName))
				{
					error();

					return false;
				}

				//load data
				if(!readLog(logFileName))
				{
					error();

					return false;
				}
			}
		}
		else
		{
			if(BinaryFile::existsFile(logFileName))
			{
				if(BinaryFile::existsFile(prevLogFileName))
				{
					BinaryFile::deleteFile(prevLogFileName);
				}

				if(BinaryFile::existsFile(currFileName))
				{
					if(!read(currFileName))
					{
						error();

						return false;
					}
				}

				//load data
				if(!readLog(logFileName))
				{
					error();

					return false;
				}
			}
			else
			{
				if(BinaryFile::existsFile(currFileName))
				{
					if(!read(currFileName))
					{
						error();

						return false;
					}
				}

				if(BinaryFile::existsFile(prevLogFileName))
				{
					BinaryFile::renameFile(prevLogFileName, logFileName);

					//load data
					if(!readLog(logFileName))
					{
						error();
							
						return false;
					}
				}
			}
		}

		return true;
	}

	bool readLog(char* filePath)
	{
		const uint buffSize = 10000000;

		BinaryFile* pFile = new BinaryFile(filePath, false, false);

		if(pFile->open())
		{
			printf("Read log ...\n");

			char* buff = new char[buffSize];

			uint safeLen = buffSize - 1000000;

			uint pos = 0;

			//avoid log
			//pDB->tranStarted = false;

			while(true)
			{
				uint len = pFile->read(buff, pos, buffSize);
			
				uint i=0;
				for(; i < safeLen && i < len; i++)
				{
					char commandType = buff[i];
					i++;
					
					uint docID = *(uint*)(buff + i);
					i += 4;

					switch(commandType)
					{
						case 'i':
							{
								i += this->pDB->insPartDoc(buff + i, docID);

								if(docID > this->pDB->lastDocID)
								{
									this->pDB->lastDocID = docID;
								}

								break;
							}
						case 'u':
							{
								i += this->pDB->updPartDoc(buff + i, docID);
								break;
							}
						case 'd':
							{
								i += this->pDB->delPartDoc(buff + i, docID);
								break;
							}
						default:
							{
								error();

								break;
							}
					};
				}

				if(len == buffSize) //not all readed
				{
					pos += i;
				}
				else
				{
					break;
				}
			}

			//pDB->tranStarted = true;

			pFile->close();

			delete pFile;

			delete[] buff;

			printf("Log readed.\n");

			return true;
		}
		else
		{
			delete pFile;

			return false;
		}
	}

	void runSyncJob()
	{
		CreateThread(NULL,                   // default security attributes
					 0,                      // use default stack size  
					 Storage::syncJob,       // thread function name
					 this,					 // argument to thread function 
					 0,                      // use default creation flags 
					 0);					 // returns the thread identifier 
	}

	bool save(BinaryFile* pFile1,
			  BinaryFile* pFile2,
			  BinaryFile* pFile3,
			  BinaryFile* pFile4)
	{
		if(!pDB->save(pFile1))
			return false;
		
		if(!pDB->ha1.save(pFile2, pFile2, pFile2, pFile2, pFile2, pFile2, pFile2))
			return false;
		
		if(!pDB->ha2.save(pFile3, pFile3, pFile3, pFile3, pFile3, pFile3, pFile3))
			return false;
		
		if(!pDB->attrValuesPool.save(pFile4))
			return false;
	
		return true;
	}

	void read(BinaryFile* pFile)
	{
		pDB->read(pFile);

		pDB->ha1.read(pFile, pFile, pFile, pFile, pFile, pFile, pFile);

		pDB->ha2.read(pFile, pFile, pFile, pFile, pFile, pFile, pFile);

		//pDB->attrValuesPool.read(pFile);
	}

	uint getTotalMemory()
	{
		return sizeof(DniproDB) +
			pDB->ha1.getTotalMemory() +
			pDB->ha2.getTotalMemory();// +
			   //pDB->attrValuesPool.getTotalMemory();
	}

	bool read(char* fileName)
	{
		BinaryFile* pFile = new BinaryFile(fileName,
										   false,
										   false);

		if(pFile->open())
		{
			read(pFile);

			pFile->close();

			return true;
		}
		else
		{
			printf("Cant save file !");
			return false;
		}
	}

	bool save(char* fileName)
	{
		BinaryFile* pFile1 = new BinaryFile(fileName,
										   true,
										   true);

		BinaryFile* pFile2 = new BinaryFile(fileName,
										   true,
										   false);

		BinaryFile* pFile3 = new BinaryFile(fileName,
										   true,
										   false);

		BinaryFile* pFile4 = new BinaryFile(fileName,
										   true,
										   false);
		
		if(!save(pFile1, pFile2, pFile3, pFile4))
		{
			printf("Cant save file !");

			return false;
		}

		pFile1->close();
		pFile2->close();
		pFile3->close();
		pFile4->close();
		
		delete pFile1;
		delete pFile2;
		delete pFile3;
		delete pFile4;

		return true;
	}
	*/
};