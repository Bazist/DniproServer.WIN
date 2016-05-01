#include "stdafx.h"

struct DniproError
{
public:
	uint Code;
	char Message[256];

	void Print()
	{
		system("Color 0C");

		printf("[%d] %s", Code, Message);
	}

	static void Print(uint code, char* message, int innerCode = 0)
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
