#include "stdafx.h"

struct DniproInfo
{
public:
	static void Print(char* message, uint param = 0)
	{
		system("Color 0E");

		if (!param)
		{
			printf(message, param);
		}
		else
		{
			printf(message);
		}
	}

	static void PrintLine()
	{
		printf("===========================================\n");
	}
};
