#include "stdafx.h"
#include "Server.h"

HANDLE Server::hServer = 0;

char* Server::pBuffer = 0;
char* Server::pResBuffer = 0;

DniproDB* Server::pDB = 0;
DniproQuery* Server::pDQ = 0;

ushort Server::Port = 0;