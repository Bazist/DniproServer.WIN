/*
# Copyright(C) 2010-2021 Viacheslav Makoveichuk (email: BigDoc@gmail.com, skype: vyacheslavm81)
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

#include "stdafx.h"
#include "Server.h"

#ifdef _WIN32
HANDLE Server::hServer = 0;
#endif

#ifdef __linux__
pthread_t Server::hServer;
#endif

char* Server::pBuffer = 0;
char* Server::pResBuffer = 0;

BigDoc* Server::pDB = 0;
BigDocQuery* Server::pDQ = 0;

ushort16 Server::Port = 0;
