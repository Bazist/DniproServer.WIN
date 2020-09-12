/*
# Copyright(C) 2010-2016 Viacheslav Makoveichuk (email: dniprodb@gmail.com, skype: vyacheslavm81)
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

#include "stdafx.h"

struct DniproInfo
{
public:
	static void Print(const char* message, uint32 param = 0)
	{
		#ifdef _WIN32
		system("Color 0E");
		#endif

		if (!param)
		{
			printf(message);
		}
		else
		{
			printf(message, param);
		}
	}

	static void Print(const char* message, char* param)
	{
		#ifdef _WIN32
		system("Color 0E");
		#endif

		if (!param)
		{
			printf(message);
		}
		else
		{
			printf(message, param);
		}
	}

	static void PrintLine()
	{
		printf("===========================================\n");
	}
};
