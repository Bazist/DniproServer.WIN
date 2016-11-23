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

#ifndef _SEREVER_DB		 // Allow use of features specific to Windows XP or later.                   
#define _SERVER_DB 0x787 // Change this to the appropriate value to target other versions of Windows.

#endif

#include "stdafx.h"
#include "DniproDB.h"
#include "DniproQuery.h"
#include "DniproInterpreter.h"

class DniproPacket
{
public:
	uchar8 MethodType;
	uint32 TranID;
	uint32 CollID;
	uint32 Tag1;
	uint32 Tag2;
	uint32 Tag3;
	uint32 QuerySize;
	//rest of packet is query
};

class Server
{
	public:

		#ifdef _WIN32
		static HANDLE hServer;
		#endif

		#ifdef __linux__
		static pthread_t hServer;
		#endif

		static char* pBuffer;
		static char* pResBuffer;

		static DniproDB* pDB;
		static DniproQuery* pDQ;

		static ushort16 Port;

		static void start(DniproDB* pDB, ushort16 port)
		{
			Server::pBuffer = new char[SERVER_BUFFER_SIZE];;
			Server::pResBuffer = new char[SERVER_BUFFER_SIZE];

			Server::pDB = pDB;
			Server::pDQ = new DniproQuery(pDB);
			Server::Port = port;

			#ifdef _WIN32
			Server::hServer = CreateThread(
										NULL,           // default security attributes
										0,              // use default stack size  
										Server::run,	// thread function name
										0,				// argument to thread function 
										0,              // use default creation flags 
										0);				// returns the thread identifier 
			#endif

			#ifdef __linux__
			int result=pthread_create(&hServer, NULL, Server::run, 0);
			#endif
		}

		// initialize to empty data...

		#ifdef _WIN32
		static DWORD WINAPI run(LPVOID pVal)
		#endif

		#ifdef __linux__
		static void* run(void* pVal)
		#endif
		{
			#ifdef _WIN32
			// Initialize Winsock
			WSADATA wsaData;
			int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != NO_ERROR)
			{
				DniproError::Print(CONNECTION_ERROR, "Error at WSAStartup().\n");

				return 0;
			}
			#endif

			// Create a SOCKET for listening for
			// incoming connection requests
			int listenSocket;
			listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (listenSocket == -1)
			{
				#ifdef _WIN32
				DniproError::Print(CONNECTION_ERROR, "Error at socket(): %ld.\n", WSAGetLastError());

				WSACleanup();
				#endif

				#ifdef __linux__
				DniproError::Print(CONNECTION_ERROR, "Error at socket().\n");
				#endif

				return 0;
			}

			// The sockaddr_in structure specifies the address family,
			// IP address, and port for the socket that is being bound.
			sockaddr_in service;
			// Address family - internet IPv4
			service.sin_family = AF_INET;
			// IP address
			service.sin_addr.s_addr = INADDR_ANY; // inet_addr("127.0.0.1");
			
			//inet_pton(AF_INET, "127.0.0.1", &(service.sin_addr.s_addr));

			//gethostbyname->getaddrinfo
			//inet_addr->inet_pron
			//inet_ntoa->inet_ntop

			// Port number
			service.sin_port = htons(Port);

			#ifdef _WIN32
			// Bind the socket.
			if (bind(listenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
			{
				DniproError::Print(CONNECTION_ERROR, "bind() failed.\n");

				closesocket(ListenSocket);

				return 0;
			}

			// Listen for incoming connection requests on the created socket
			if (listen(listenSocket, 10) == SOCKET_ERROR)
			{
				DniproError::Print(CONNECTION_ERROR, "Server: Error listening on socket.\n");
				
				return 0;
			}

			#endif

			#ifdef __linux__
			if (bind(listenSocket, (struct sockaddr *)&service, sizeof(service)) < 0)
			{
				DniproError::Print(CONNECTION_ERROR, "bind() failed.\n");

				//closesocket(listenSocket);

				return 0;
			}

			// Listen for incoming connection requests on the created socket
			listen(listenSocket, 3);
			#endif

			// Create a SOCKET for accepting incoming requests.
			int acceptSocket;
			DniproInfo::Print("Server started (port 4477)...\n");

			bool useExistingConnection = false;
			uint32 amountQueries[3];

			// Accept the connection if any...
			while (1)
			{
			NEXT_CLIENT:
				
				acceptSocket = -1;

				//ACCEPT CONNECTION ============================================================
				acceptSocket = accept(listenSocket, NULL, NULL);

				if(acceptSocket < 0)
				{
					#ifdef _WIN32
					DniproError::Print(CONNECTION_ERROR, "ListenSocket error: %ld.\n", WSAGetLastError());
					#endif

					#ifdef __linux__
					DniproError::Print(CONNECTION_ERROR, "ListenSocket error.\n");
					#endif

					continue;
				}

			NEXT_PACKET:
			
				//RECEIVE PACKET ==============================================================
				int bytesRecv = -1;
				bytesRecv = recv(acceptSocket, pBuffer, SERVER_BUFFER_SIZE, 0);
				
				uint32* pSendBufferSize = (uint32*)pResBuffer;

				char* sendBuffer = pResBuffer; //reserve first 4 bytes for header
				int sendBufferSize = 4;
				bool isSimpleExecPacket = false;

				if (bytesRecv != -1)
				{
					try
					{
						//Read rest of the data
						uint32 fullBytesRecv = *(uint32*)pBuffer;

						while (fullBytesRecv > bytesRecv)
						{
							uint32 len = recv(acceptSocket, pBuffer + bytesRecv, SERVER_BUFFER_SIZE - bytesRecv, 0);

							if (len == 0)
							{
								goto NEXT_CLIENT;
							}

							if (len == -1)
							{
								#ifdef _WIN32
								DniproError::Print(CONNECTION_ERROR, "AcceptSocket error %ld.\n", WSAGetLastError());
								#endif

								#ifdef __linux__
								DniproError::Print(CONNECTION_ERROR, "AcceptSocket error.\n");
								#endif

								return 0;
							}

							bytesRecv += len;

							if ((uchar8)pBuffer[0] == 100) //simple Exec packet
							{
								isSimpleExecPacket = true;

								break;
							}
						}

						if (isSimpleExecPacket)
						{
							pBuffer[bytesRecv] = 0;

							DniproInterpreter di(pDB, sendBuffer);

							di.run(pBuffer);

							sendBufferSize = di.jsonResultLen;
						}
						else
						{
							//pDB->clearState();
							pDQ->clearState();
							
							for (uint32 i = 4; i < bytesRecv;)
							{
								DniproPacket* pPacket = (DniproPacket*)(pBuffer + i);
								char* json = (pBuffer + i + sizeof(DniproPacket));

								if (DniproInterpreter::IsProfilerActive)
								{
									DniproInfo::Print("%s\n", json);
									DniproInfo::PrintLine();
								}

								//init if tran exists
								if (pPacket->TranID)
								{
									pDQ->TranID = pPacket->TranID;
								}

								pDQ->DefCollID = pPacket->CollID;

								switch (pPacket->MethodType)
								{
								case 1: //addDoc
								{
									uint32* pDocID = (uint32*)(sendBuffer + sendBufferSize);

									*pDocID = pDB->addDoc(json, pPacket->TranID);

									sendBufferSize += 4;

									break;
								}
								case 2: //getDocsByAttr
								{
									ValueList* pValueList = pDB->getDocsByAttr(json, 0, pPacket->TranID, pPacket->CollID);

									uint32 len = pValueList->Count * sizeof(uint32);

									memcpy(sendBuffer + sendBufferSize, (char*)pValueList->pValues, len);

									sendBufferSize += len;

									break;
								}
								case 3: //insPartDoc
								{
									pDB->insPartDoc(json, pPacket->Tag1, pPacket->TranID, pPacket->CollID);

									break;
								}
								case 4: //updPartDoc
								{
									pDB->updPartDoc(json, pPacket->Tag1, pPacket->TranID, pPacket->CollID);

									break;
								}
								case 5: //delPartDoc
								{
									pDB->delPartDoc(json, pPacket->Tag1, pPacket->TranID, pPacket->CollID);

									break;
								}
								case 6: //getPartDoc
								{
									//format |len|string|len|string...

									ushort16* pLen = (ushort16*)(sendBuffer + sendBufferSize);

									sendBufferSize += 2;

									uint32 len = pDB->getPartDoc(json,
										sendBuffer + sendBufferSize,
										pPacket->Tag1,
										pPacket->TranID,
										pPacket->CollID,
										false);
									*pLen = len;

									sendBufferSize += len;

									break;
								}
								case 7: //getWhere
								{
									pDQ->getWhere(json);

									break;
								}
								case 9: //andWhere
								{
									pDQ->andWhere(json);

									break;
								}
								case 11: //orWhere
								{
									pDQ->orWhere(json);

									break;
								}
								case 14: //select
								{
									sendBufferSize += pDQ->select(json, sendBuffer + sendBufferSize, pPacket->Tag1, false);

									break;
								}
								case 15: //count
								{
									uint32* pCount = (uint32*)(sendBuffer + sendBufferSize);

									*pCount = pDQ->count();

									sendBufferSize += 4;

									break;
								}
								case 16: //clear
								{
									pDB->clear();

									DniproInfo::Print("Database cleared !\n");

									break;
								}
								case 17: //take
								{
									pDQ->take(pPacket->Tag1);

									break;
								}
								case 18: //skip
								{
									pDQ->skip(pPacket->Tag1);

									break;
								}
								case 19: //sum
								{
									uint32* pCount = (uint32*)(sendBuffer + sendBufferSize);

									*pCount = pDQ->sum(json);

									sendBufferSize += 4;

									break;
								}
								case 20: //value
								{
									sendBufferSize += pDQ->select(json, sendBuffer + sendBufferSize, pPacket->Tag1, true);

									break;
								}
								case 21: //join
								{
									json[pPacket->Tag1] = 0;

									pDQ->join(json, json + pPacket->Tag1 + 1, pPacket->Tag2);

									break;
								}
								case 22: //sort
								{
									pDQ->sort(json, pPacket->Tag1);

									break;
								}
								case 23: //drop
								{
									pDQ->drop(json, pPacket->Tag1);

									break;
								}
								case 24: //update
								{
									pDQ->update(json, pPacket->Tag1);

									break;
								}
								case 25: //getAll
								{
									pDQ->getAll();

									break;
								}
								case 26: //insert
								{
									pDQ->insert(json, pPacket->Tag1);

									break;
								}
								case 27: //begintran
								{
									uint32* tranID = (uint32*)(sendBuffer + sendBufferSize);

									*tranID = pDB->beginTran(pPacket->Tag1, pPacket->Tag2);

									sendBufferSize += 4;

									//useExistingConnection = true;

									break;
								}
								case 28: //rollbackTran
								{
									pDB->rollbackTran(pPacket->TranID);

									//useExistingConnection = false;

									break;
								}
								case 29: //commitTran
								{
									pDB->commitTran(pPacket->TranID);

									//useExistingConnection = false;

									break;
								}
								case 30: //Exec
								{
									DniproInterpreter di(pDB, sendBuffer + sendBufferSize);

									di.run(json);

									sendBufferSize += di.jsonResultLen;

									break;
								}
								case 31: //addBlobValue
								{
									uint32 len = pDB->addBlobValue(json, pPacket->QuerySize, sendBuffer + sendBufferSize);

									sendBufferSize += len;

									break;
								}
								case 32: //getBlobValue
								{
									uint32 len = 0;

									pDB->getBlobValue(sendBuffer + sendBufferSize, len, json);

									sendBufferSize += len;

									break;
								}
								case 33: //addCollection
								{
									pDB->addColl(json);

									break;
								}
								case 34: //getCollectionID
								{
									uint32* collID = (uint32*)(sendBuffer + sendBufferSize);

									*collID = pDB->getCollID(json);

									sendBufferSize += 4;

									break;
								}
								case 35: //delCollection
								{
									pDB->delColl(json);

									break;
								}
								case 36: //GetFirstID
								{
									uint32* id = (uint32*)(sendBuffer + sendBufferSize);

									*id = pDQ->getFirstID();

									sendBufferSize += 4;

									break;
								}
								default:
								{
									DniproError::Print(CONNECTION_ERROR, "Server Method type is undefined.\n");

									break;
								}
								}

								i += (sizeof(DniproPacket) + pPacket->QuerySize);
							}
						}
					}
					catch (DniproError de)
					{
						de.Print();

						sendBufferSize = 0; //error
					}
				}
				else
				{
					#ifdef _WIN32
					DniproError::Print(CONNECTION_ERROR, "Server recv() error %ld.\n", WSAGetLastError());
					#endif

					#ifdef __linux__
					DniproError::Print(CONNECTION_ERROR, "Server recv() error.\n");
					#endif
					break;
				}

				//SEND BUFFER ============================================================
				if(sendBuffer)
				{
					//set length in header
					if (!isSimpleExecPacket)
					{
						*pSendBufferSize = sendBufferSize;
					}

					uint32 bytesSent = send(acceptSocket, sendBuffer, sendBufferSize, 0);

					if (bytesSent == -1)
					{
						#ifdef _WIN32
						DniproError::Print(CONNECTION_ERROR, "Server send() error %ld.\n", WSAGetLastError());
						#endif

						#ifdef __linux__
						DniproError::Print(CONNECTION_ERROR, "Server send() error.\n");
						#endif

						break;
					}
				}

				if (useExistingConnection)
				{
					goto NEXT_PACKET;
				}

				#ifdef _WIN32
				closesocket(acceptSocket);
				#endif
			}

			#ifdef _WIN32
			WSACleanup();
			#endif

			return 0;
		}

		static void stop()
		{
			delete[] pBuffer;
			delete[] pResBuffer;

			delete pDQ;

			#ifdef _WIN32
			//stop myself
			TerminateThread(hServer, 0);

			CloseHandle(hServer);

			WSACleanup();

			msleep(1000);
			#endif

			#ifdef __linux__
			pthread_cancel(hServer);

			usleep(1000000);
			#endif
		}
};