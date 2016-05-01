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
