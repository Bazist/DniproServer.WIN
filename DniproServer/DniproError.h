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

struct DniproError
{
public:
	uint32 Code;
	char Message[256];

	void Print()
	{
		#ifdef _WIN32
		system("Color 0C");
		#endif

		printf("[%d] %s", Code, Message);
	}

	static void Print(uint32 code, char* message, int innerCode = 0)
	{
		DniproError de;
		de.Code = code;

		if (innerCode)
		{
			sprintf(de.Message, message);
		}
		else
		{
			sprintf(de.Message, message, innerCode);
		}
		
		de.Print();
	}

	void clear()
	{
		Code = 0;
		Message[0] = 0;
	}
};
