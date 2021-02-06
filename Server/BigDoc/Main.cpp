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

#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "HArray.h"
#include "BigDoc.h"
#include "BigDocQuery.h"
#include "BigDocInterpreter.h"
#include "Server.h"
#include "Test.h"

using namespace std;

static void msleep(uint32 ms)
	{	
		#ifdef _WIN32
		Sleep(ms);
		#endif

		#ifdef __linux__
		usleep(ms);
		#endif
	}

int main(int argc, char** argv)
{
	strcpy(BigDocInterpreter::CurrPath, argv[0]);

	//=====================================================

	#ifdef _WIN32
	system("cls");
	#endif

	if (argc == 2 && !strcmp(argv[1], "-selftest")) //test mode
	{
		//http://www.computerhope.com/color.htm
		BigDocInfo::PrintLine();
		BigDocInfo::Print("= BigDoc ");
		BigDocInfo::Print(BigDoc_VERSION);
		BigDocInfo::Print(" [SELFTEST] (c)Booben.Com =\n");
		BigDocInfo::PrintLine();

		BigDoc* pDB = new BigDoc();
		pDB->init();

		Test* pTests = new Test(pDB);
		pTests->run();

		delete pTests;

		pDB->destroy();

		delete pDB;

		printf("Restart server in normal mode ? (y/n): ");

		if (fgetc(stdin) == 'y')
		{
			BigDocInterpreter::restartServer();
		}
		else
		{
			exit(0);
		}

		return 0;
	}

	//=====================================================

	uint32 onDate = 0;

	if (argc == 8 && !strcmp(argv[1], "-restoreondate"))
	{
		struct tm when;

		when.tm_year = atoi(argv[2]) - 1900;
		when.tm_mon = atoi(argv[3]) - 1;
		when.tm_mday = atoi(argv[4]);
		when.tm_hour = atoi(argv[5]);
		when.tm_min = atoi(argv[6]);
		when.tm_sec = atoi(argv[7]);;

		time_t tm;
		tm = mktime(&when);

		if (tm != -1)
		{
			onDate = (uint32)tm;
		}
		else
		{
			BigDocError::Print(0, "Date is not correct.");

			system("pause");

			return 0;
		}
	}

	//=====================================================

	//http://www.computerhope.com/color.htm
	BigDocInfo::PrintLine();
	BigDocInfo::Print("=      BigDoc ");
	BigDocInfo::Print(BigDoc_VERSION);
	BigDocInfo::Print(" (c)Booben.Com      =\n");

	BigDocInfo::PrintLine();

	//create db
	BigDoc* pDB = new BigDoc();
	pDB->init("", onDate); //current folder

	//run server
	Server::start(pDB, 4477);
	
	//run console
	char query[1024];

	BigDocInterpreter di(pDB, 0, &Server::stop);

	//wait, server is not started
	msleep(500);

	while (true)
	{
		printf("\nEnter query >> ");
		fflush(stdout);

		fgets(query, 1024, stdin);

		di.run(query);
	}

	#ifdef _WIN32
	system("pause");
	#endif

	return 0;
};