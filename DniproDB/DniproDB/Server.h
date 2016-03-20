#pragma once

#ifndef _SEREVER_DB		 // Allow use of features specific to Windows XP or later.                   
#define _SERVER_DB 0x787 // Change this to the appropriate value to target other versions of Windows.

#endif

#include "stdafx.h"

class DniproPacket
{
public:
	uchar MethodType;
	uint TranID;
	uint Tag1;
	uint Tag2;
	uint Tag3;
	uint QuerySize;
	//rest of packet is query
};

class Server
{
	public:

		Server(DniproDB* pDB, ushort port)
		{
			this->pDB = pDB;
			this->pDQ = new DniproQuery(pDB);

			this->Port = port;
			this->pBuffer = new char[SERVER_BUFFER_SIZE];
			this->pResBuffer = new char[SERVER_BUFFER_SIZE];
		}

		DniproDB* pDB;
		DniproQuery* pDQ;

		ushort Port;
		char* pBuffer;
		char* pResBuffer;
		
		// initialize to empty data...
		int Run() 
		{
			// Initialize Winsock
			WSADATA wsaData;
			int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != NO_ERROR)
			{
				DniproError::Print(CONNECTION_ERROR, "Error at WSAStartup().\n");

				return 0;
			}
			
			// Create a SOCKET for listening for
			// incoming connection requests
			SOCKET ListenSocket;
			ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (ListenSocket == INVALID_SOCKET)
			{
				DniproError::Print(CONNECTION_ERROR, "Error at socket(): %ld.\n", WSAGetLastError());

				WSACleanup();

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

			// Bind the socket.
			if (bind(ListenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
			{
				DniproError::Print(CONNECTION_ERROR, "bind() failed.\n");

				closesocket(ListenSocket);

				return 0;
			}

			// Listen for incoming connection requests on the created socket
			if (listen(ListenSocket, 10) == SOCKET_ERROR)
			{
				DniproError::Print(CONNECTION_ERROR, "Server: Error listening on socket.\n");
				
				return 0;
			}
			
			// Create a SOCKET for accepting incoming requests.
			SOCKET AcceptSocket;
			DniproInfo::Print("Server started (port 4477)...\n");

			bool useExistingConnection = false;
			uint amountQueries[3];
			
			// Accept the connection if any...
			while (1)
			{
				AcceptSocket = SOCKET_ERROR;

				//ACCEPT CONNECTION ============================================================
				AcceptSocket = accept(ListenSocket, NULL, NULL);

				if(AcceptSocket == SOCKET_ERROR)
				{
					DniproError::Print(CONNECTION_ERROR, "ListenSocket error: %ld.\n", WSAGetLastError());

					continue;
				}

			NEXT_PACKET:
			
				//RECEIVE PACKET ==============================================================
				int bytesRecv = SOCKET_ERROR;
				bytesRecv = recv(AcceptSocket, pBuffer, SERVER_BUFFER_SIZE, 0);
				
				uint* pSendBufferSize = (uint*)pResBuffer;

				char* sendBuffer = pResBuffer; //reserve first 4 bytes for header
				int sendBufferSize = 4;
				
				if (bytesRecv != SOCKET_ERROR)
				{
					try
					{
						//Read rest of the data
						uint fullBytesRecv = *(uint*)pBuffer;

						while (fullBytesRecv > bytesRecv)
						{
							uint len = recv(AcceptSocket, pBuffer + bytesRecv, SERVER_BUFFER_SIZE - bytesRecv, 0);

							if (len == SOCKET_ERROR)
							{
								DniproError::Print(CONNECTION_ERROR, "AcceptSocket error: %ld.\n", WSAGetLastError());

								return 0;
							}

							bytesRecv += len;
						}

						//pDB->clearState();
						pDQ->clearState();

						for (uint i = 4; i < bytesRecv;)
						{
							DniproPacket* pPacket = (DniproPacket*)(pBuffer + i);
							char* json = (pBuffer + i + sizeof(DniproPacket));

							pDQ->TranID = pPacket->TranID;

							switch (pPacket->MethodType)
							{
							case 1: //addDoc
							{
								uint* pDocID = (uint*)(sendBuffer + sendBufferSize);

								*pDocID = pDB->addDoc(json, pPacket->TranID);

								sendBufferSize += 4;

								break;
							}
							case 2: //getDocsByAttr
							{
								ValueList* pValueList = pDB->getDocsByAttr(json, 0, pPacket->TranID);

								uint len = pValueList->Count * sizeof(uint);

								memcpy(sendBuffer + sendBufferSize, (char*)pValueList->pValues, len);

								sendBufferSize += len;

								break;
							}
							case 3: //insPartDoc
							{
								pDB->insPartDoc(json, pPacket->Tag1, pPacket->TranID);

								break;
							}
							case 4: //updPartDoc
							{
								pDB->updPartDoc(json, pPacket->Tag1, pPacket->TranID);

								break;
							}
							case 5: //delPartDoc
							{
								pDB->delPartDoc(json, pPacket->Tag1, pPacket->TranID);

								break;
							}
							case 6: //getPartDoc
							{
								//format |len|string|len|string...

								ushort* pLen = (ushort*)(sendBuffer + sendBufferSize);

								sendBufferSize += 2;

								uint len = pDB->getPartDoc(json,
									sendBuffer + sendBufferSize,
									pPacket->Tag1,
									pPacket->TranID,
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
							case 8: //getWhereElems
							{
								pDQ->getWhereElems(json, pPacket->Tag1);

								break;
							}
							case 9: //andWhere
							{
								pDQ->andWhere(json);

								break;
							}
							case 10: //andWhereElems
							{
								pDQ->andWhereElems(json, pPacket->Tag1);

								break;
							}
							case 11: //orWhere
							{
								pDQ->orWhere(json);

								break;
							}
							case 12: //orWhereElems
							{
								pDQ->orWhereElems(json, pPacket->Tag1);

								break;
							}
							case 14: //select
							{
								sendBufferSize += pDQ->select(json, sendBuffer + sendBufferSize, pPacket->Tag1, false);

								break;
							}
							case 15: //count
							{
								uint* pCount = (uint*)(sendBuffer + sendBufferSize);

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
								uint* pCount = (uint*)(sendBuffer + sendBufferSize);

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

								pDQ->join(json, json + pPacket->Tag1 + 1);

								break;
							}
							case 22: //sort
							{
								pDQ->sort(json, pPacket->Tag1);

								break;
							}
							case 23: //drop
							{
								pDQ->drop(json);

								break;
							}
							case 24: //update
							{
								pDQ->update(json);

								break;
							}
							case 25: //getAll
							{
								pDQ->getAll();

								break;
							}
							case 26: //insert
							{
								pDQ->insert(json);

								break;
							}
							case 27: //begintran
							{
								uint* tranID = (uint*)(sendBuffer + sendBufferSize);

								*tranID = pDB->beginTran(pPacket->Tag1);

								sendBufferSize += 4;

								useExistingConnection = true;

								break;
							}
							case 28: //rollbackTran
							{
								pDB->rollbackTran(pPacket->TranID);

								useExistingConnection = false;

								break;
							}
							case 29: //commitTran
							{
								pDB->commitTran(pPacket->TranID);

								useExistingConnection = false;

								break;
							}
							default:
							{
								DniproError::Print(CONNECTION_ERROR, "Server: Method type is undefined.\n");

								break;
							}
							}

							i += (sizeof(DniproPacket) + pPacket->QuerySize);
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
					DniproError::Print(CONNECTION_ERROR, "Server: recv() error %ld.\n", WSAGetLastError());

					break;
				}
				
				//SEND BUFFER ============================================================
				if(sendBuffer)
				{
					//set length in header
					*pSendBufferSize = sendBufferSize;

					uint bytesSent = send(AcceptSocket, sendBuffer, sendBufferSize, 0);

					if (bytesSent == SOCKET_ERROR)
					{
						DniproError::Print(CONNECTION_ERROR, "Server: send() error %ld.\n", WSAGetLastError());

						break;
					}
				}

				if (useExistingConnection)
				{
					goto NEXT_PACKET;
				}

				closesocket(AcceptSocket);
			}

			WSACleanup();

			return 0;
		}

	~Server()
	{
		delete pDQ;
		delete[] pBuffer;
	}
};