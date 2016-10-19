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

#include "stdafx.h"

struct DniproInfo
{
public:
	static void Print(const char* message, uint param = 0)
	{
		system("Color 0E");

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
		system("Color 0E");

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
