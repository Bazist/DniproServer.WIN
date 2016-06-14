#pragma once

#ifndef _DNIPRO_INTERPRETER		 // Allow use of features specific to Windows XP or later.                   
#define _DNIPRO_INTERPRETER 0x712 // Change this to the appropriate value to target other versions of Windows.

#endif	

#include "stdafx.h"
#include "DniproQuery.h"

typedef void STOP_SERVER_FUNC();

struct MethodParam
{
	char Value[1024];
	uint ValueInt;

	uchar Type; //1 - json
				//2 - int
				//3 - literal
				//4 - name
};

struct Method
{
	Method()
	{
		ParamCount = 0;
	}

	char MethodName[128];

	MethodParam Params[8];
	uint ParamCount;
};

class DniproInterpreter
{
public:
	DniproInterpreter(DniproDB* pDB,
		char* jsonResult = 0,
		STOP_SERVER_FUNC* stopServer = 0)
	{
		this->pDB = pDB;

		this->jsonResult = jsonResult;
		jsonResultLen = 0;

		this->stopServer = stopServer;

		TranID = 0;
		CollID = 0;
	}

	STOP_SERVER_FUNC* stopServer;

	DniproDB* pDB;

	Method methods[16];
	uint methodCount;

	char* jsonResult;
	uint jsonResultLen;

	uint CurrLine;

	uint TranID;
	uint CollID;

	static char CurrPath[512];
	static bool IsProfilerActive;

	static void restartServer(DniproDB* pDB = 0,
		STOP_SERVER_FUNC* stopServer = 0,
		char* withParam = 0)
	{
		char exePath[512];

		strcpy(exePath, CurrPath);

		if (withParam)
		{
			strcat(exePath, withParam);
		}

		if (stopServer)
		{
			(*stopServer)();
		}

		if (pDB)
		{
			pDB->destroy();

			delete pDB;
		}

		system(exePath);

		exit(0);

		//Sleep(5000);
	}

	void printResult(char* json, uint intParam)
	{
		char jsonParam[10];
		itoa(intParam, jsonParam, 10);

		printResult(json, jsonParam);
	}

	void printResult(char* json, char* jsonParam = 0)
	{
		if (jsonResult) //Exec mode
		{
			if (!jsonParam)
			{
				jsonResultLen += (uint)sprintf(jsonResult + jsonResultLen, json);
			}
			else
			{
				jsonResultLen += (uint)sprintf(jsonResult + jsonResultLen, json, jsonParam);
			}

			jsonResult[jsonResultLen++] = ';';
			jsonResult[jsonResultLen] = 0;
		}
		else //Console mode
		{
			if (!jsonParam)
			{
				printf(json);
			}
			else
			{
				printf(json, jsonParam);

			}

			printf("\n");
		}
	}

	void printError(char* json, uint intParam)
	{
		char jsonParam[10];
		itoa(intParam, jsonParam, 10);

		printError(json, jsonParam);
	}

	void printError(char* json, char* jsonParam = 0)
	{
		if (jsonResult) //Exec mode
		{
			if (!jsonParam)
			{
				jsonResultLen += (uint)sprintf(jsonResult + jsonResultLen, json);
			}
			else
			{
				jsonResultLen += (uint)sprintf(jsonResult + jsonResultLen, json, jsonParam);
			}

			char currLine[64];

			jsonResultLen += sprintf(currLine, " [Line %d]", CurrLine);

			strcat(jsonResult, currLine);

			jsonResult[jsonResultLen++] = ';';
			jsonResult[jsonResultLen] = 0;
		}
		else //Console mode
		{
			if (!jsonParam)
			{
				printf(json);
			}
			else
			{
				printf(json, jsonParam);
			}

			printf(" [Line %d]\n", CurrLine);
		}
	}

	bool validateJson(char* query)
	{
		if (query[0] != '{')
		{
			printError("Json should be started from '{' symbol.");

			return false;
		}

		uint bracket1 = 0;
		uint bracket2 = 0;
		uint quotes = 0;

		uint i;

		for (i = 0; query[i]; i++)
		{
			switch (query[i])
			{
			case '{':
				bracket1++;
				break;
			case '}':
				bracket1--;
				break;
			case '[':
				bracket2++;
				break;
			case ']':
				bracket2--;
				break;
			case '"':
			case '\'':
			{
				quotes++;

				for (++i; i < query[i]; i++)
				{
					if (query[i] == '\\') //ecrane symbol
					{
						i++;
					}

					if (query[i] == '"' || query[i] == '\'')
					{
						quotes--;

						break;
					}
				}

				break;
			}
			default:
				break;
			};
		}

		if (i >= 1 && query[i - 1] != '}') //} \n 0 three symbols
		{
			printError("Json should be finished with '}' symbol.");

			return false;
		}

		if (quotes != 0)
		{
			printError("Quotes don't match to each other.");

			return false;
		}

		if (bracket1 != 0)
		{
			printError("Brackets '{' and '}' don't match to each other.");

			return false;
		}

		if (bracket2 != 0)
		{
			printError("Brackets '[' and ']' don't match to each other.");

			return false;
		}

		return true;
	}

	bool parseQuery(char* query, uint& i)
	{
		methodCount = 0;

		//query should be starts from "db"
		if (query[i++] != 'd' ||
			query[i++] != 'b')
		{
			printError("Query should be started from db. For example: db.GetWhere(...).Print(...)");

			return false; //error
		}

		//parse method
		while (query[i])
		{
			//skip spaces
			if (query[i] == ' ' ||
				query[i] == '\r')
			{
				i++;

				continue;
			}

			//next query
			if (query[i] == '\n')
			{
				i++;

				break;
			}

			if (query[i++] != '.')
			{
				printError("Expected symbol '.'.");

				return false; //error
			}

			Method& method = methods[methodCount];

			uint j = 0;

			//read method
			while (query[i] != '(')
			{
				if (!query[i])
				{
					printError("Unexpectable end of query.");

					return false; //error
				}

				method.MethodName[j++] = query[i++];
			}

			method.MethodName[j] = 0;
			i++;

			method.ParamCount = 0;

			//read params
			while (query[i] != ')')
			{
				if (!query[i])
				{
					printError("Unexpectable end of query.");

					return false; //error
				}

				//skip space
				if (query[i] == ' ' ||
					query[i] == ',' ||
					query[i] == '\r')
				{
					i++;

					continue;
				}

				if (!query[i])
				{
					printError("Unexpectable end of query.");

					return false; //error
				}

				//read string param
				if (query[i] == '"')
				{
					char* value = method.Params[method.ParamCount].Value;

					for (j = 0, i++; query[i] != '"'; i++, j++)
					{
						if (!query[i])
						{
							printError("Unexpectable end of query.");

							return false; //error
						}

						if (query[i] == '\\')
						{
							i++;
						}

						value[j] = query[i];
					}

					value[j] = 0;

					if (value[0] == '{') //json
					{
						method.Params[method.ParamCount].Type = 1;

						if (!validateJson(value))
						{
							return false;
						}
					}
					else //name
					{
						method.Params[method.ParamCount].Type = 4; //name
					}

					i++;

				} //read int parameter
				else if (query[i] >= '0' && query[i] <= '9')
				{
					method.Params[method.ParamCount].Type = 2;

					for (j = 0; query[i] != ',' && query[i] != ')'; i++, j++)
					{
						if (!query[i])
						{
							printError("Unexpectable end of query.");

							return false; //error
						}

						if (query[i] >= '0' && query[i] <= '9')
						{
							method.Params[method.ParamCount].Value[j] = query[i];
						}
						else
						{
							printError("Unexpectable symbols in Number.");

							return false; //error
						}
					}

					method.Params[method.ParamCount].Value[j] = 0;

					//try parse
					method.Params[method.ParamCount].ValueInt = atoi(method.Params[method.ParamCount].Value);

				} //read literal
				else
				{
					method.Params[method.ParamCount].Type = 3;

					for (j = 0; query[i] != ',' && query[i] != ')'; i++, j++)
					{
						if (!query[i])
						{
							printError("Unexpectable end of query.");

							return false; //error
						}

						method.Params[method.ParamCount].Value[j] = query[i];
					}

					method.Params[method.ParamCount].Value[j] = 0;
				}

				method.ParamCount++;
			}

			methodCount++;

			i++; //next method
		}

		return true;
	}

	bool runQuery()
	{
		try
		{
			DniproQuery* pDQ = new DniproQuery(pDB);
			pDQ->TranID = TranID;
			pDQ->DefCollID = CollID;

			for (uint i = 0; i < methodCount; i++)
			{
				Method& method = methods[i];

				switch (method.MethodName[0])
				{
				case 'A':
				{
					if (!strcmp(method.MethodName, "AddDoc"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							uint docID = pDB->addDoc(method.Params[0].Value, TranID, CollID);

							printResult("Document %s was added.", docID);
						}
						else
						{
							printError("AddDoc method has format: AddDoc(json)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "AndWhere"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							pDQ->andWhere(method.Params[0].Value);
						}
						else
						{
							printError("AndWhere method has format: AndWhere(json)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "Avg"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							printResult("%s", pDQ->sum(method.Params[0].Value));
						}
						else
						{
							printError("Avg method has format: Avg(json)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "AddColl"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 4)
						{
							pDB->addColl(method.Params[0].Value);

							printResult("%s collection is added", method.Params[0].Value);
						}
						else
						{
							printError("AddColl method has format: AddColl(name)");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				case 'I':
				{
					if (!strcmp(method.MethodName, "InsPartDoc"))
					{
						if (method.ParamCount == 2 &&
							method.Params[0].Type == 1 &&
							method.Params[0].Type == 2)
						{
							pDB->insPartDoc(method.Params[0].Value, method.Params[0].ValueInt, TranID, CollID);

							printResult("Attributes were inserted.");
						}
						else
						{
							printError("InsPartDoc method has format: InsPartDoc(json, docID)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "Insert"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							pDQ->insert(method.Params[0].Value);
						}
						else
						{
							printError("Insert method has format: Insert(json)");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				case 'U':
				{
					if (!strcmp(method.MethodName, "UpdPartDoc"))
					{
						if (method.ParamCount == 2 &&
							method.Params[0].Type == 1 &&
							method.Params[0].Type == 2)
						{
							pDB->updPartDoc(method.Params[0].Value, method.Params[0].ValueInt, TranID, CollID);

							printResult("Attributes were updated.");
						}
						else
						{
							printError("UpdPartDoc method has format: UpdPartDoc(json, docID)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "Update"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							pDQ->update(method.Params[0].Value);
						}
						else
						{
							printError("Update method has format: Update(json)");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				case 'D':
				{
					if (!strcmp(method.MethodName, "DelPartDoc"))
					{
						if (method.ParamCount == 2 &&
							method.Params[0].Type == 1 &&
							method.Params[0].Type == 2)
						{
							pDB->delPartDoc(method.Params[0].Value, method.Params[0].ValueInt, TranID, CollID);

							printResult("Attributes were deleted.");
						}
						else
						{
							printError("DelPartDoc method has format: DelPartDoc(json, docID)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "Delete"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							pDQ->drop(method.Params[0].Value);
						}
						else
						{
							printError("Delete method has format: Delete(json)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "DelColl"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 4)
						{
							pDB->delColl(method.Params[0].Value);

							printResult("%s collection is deleted", method.Params[0].Value);
						}
						else
						{
							printError("DelColl method has format: DelColl(name)");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				case 'G':
				{
					if (!strcmp(method.MethodName, "GetWhere"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							pDQ->getWhere(method.Params[0].Value);
						}
						else
						{
							printError("GetWhere method has format: GetWhere(json)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "GetAll"))
					{
						if (method.ParamCount == 0)
						{
							pDQ->getAll();
						}
						else
						{
							printError("GetAll method has format: GetAll()");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "GetInfo"))
					{
						if (method.ParamCount == 0)
						{
							DniproInfo::PrintLine();
							DniproInfo::Print("Version: %s\n", DNIPRO_VERSION);
							DniproInfo::Print("Amount documents: %u\n", pDB->lastDocID);
							DniproInfo::Print("Amount collections: %u\n", pDB->countColls);
							DniproInfo::Print("Total Memory (bytes): %u\n", pDB->getTotalMemory());
							DniproInfo::Print("Tran Log size (bytes): %u\n", pDB->tranLogSize);
							DniproInfo::Print("Tran Blob size (bytes): %u\n", pDB->blobLogSize);
							DniproInfo::PrintLine();
						}
						else
						{
							printError("GetAll method has format: GetAll()");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				case 'O':
				{
					if (!strcmp(method.MethodName, "OrWhere"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							pDQ->orWhere(method.Params[0].Value);
						}
						else
						{
							printError("OrWhere method has format: OrWhere(json)");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				case 'J':
				{
					if (!strcmp(method.MethodName, "Join"))
					{
						if (method.ParamCount == 2 &&
							method.Params[0].Type == 1 &&
							method.Params[1].Type == 1)
						{
							pDQ->join(method.Params[0].Value,
									method.Params[1].Value,
									0);
						}
						else
						{
							printError("Join method has format: Join(json1, json2)");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				case 'S':
				{
					if (!strcmp(method.MethodName, "Shrink"))
					{
						if (method.ParamCount == 0)
						{
							uint percents = pDB->shrink();

							printResult("Database is reduced on %s%%.", percents);
						}
						else
						{
							printError("Shrink method has format: Shrink()");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "SetDefColl"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 4)
						{
							CollID = pDQ->DefCollID = pDB->getCollID(method.Params[0].Value);

							printResult("%s collection setted as default", method.Params[0].Value);
						}
						else
						{
							printError("SetDefColl method has format: SetDefColl(name)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "Skip"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 2)
						{
							pDQ->skip(method.Params[0].ValueInt);
						}
						else
						{
							printError("Skip method has format: Skip(amount)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "Sort"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							pDQ->sort(method.Params[0].Value, true);
						}
						else if (method.ParamCount == 2 &&
							method.Params[0].Type == 1 &&
							method.Params[1].Type == 3)
						{
							pDQ->sort(method.Params[0].Value,
								!strcmp(method.Params[1].Value, "true"));
						}
						else
						{
							printError("Sort method has format: Sort(json) or Sort(json, isAscending)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "Sum"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							printResult("%s", pDQ->sum(method.Params[0].Value));
						}
						else
						{
							printError("Sum method has format: Sum(json)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "Select"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							if (jsonResult)
							{
								jsonResultLen += pDQ->selectStr(method.Params[0].Value, jsonResult);
							}
							else
							{
								printError("Select method is not allowed in console format. Please use Print method instead.");

							}
						}
						else
						{
							printError("Select method has format: Select(json)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "SelfTest"))
					{
						if (method.ParamCount == 0)
						{
							restartServer(pDB, stopServer, " -selftest");
						}
						else
						{
							printError("SelfTest method has format: SelfTest()");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				case 'T':
				{
					if (!strcmp(method.MethodName, "Take"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 2)
						{
							pDQ->take(method.Params[0].ValueInt);
						}
						else
						{
							printError("Skip method has format: Take(amount)");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				case 'C':
				{
					if (!strcmp(method.MethodName, "Count"))
					{
						if (method.ParamCount == 0)
						{
							printResult("%s", pDQ->count());
						}
						else
						{
							printError("Count method has format: Count()");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "CommitTran"))
					{
						if (method.ParamCount == 0)
						{
							pDB->commitTran(TranID);
						}
						else
						{
							printError("CommitTran method has format CommitTran()");

							return false;
						}

						TranID = 0;
						pDQ->TranID = 0;

						printResult("Transaction has been commited.");
					}
					else if (!strcmp(method.MethodName, "Clear"))
					{
						if (method.ParamCount == 0)
						{
							pDB->clear();

							printResult("Database is cleared !");
						}
						else
						{
							printError("Clear method has format Clear()");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}
				case 'P':
				{
					if (!strcmp(method.MethodName, "Print"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 1)
						{
							pDQ->print(method.Params[0].Value);
						}
						else
						{
							printError("Print method has format: Print(json)");

							return false;
						}
					}
					else if (!strcmp(method.MethodName, "Profiler"))
					{
						if (method.ParamCount == 1 &&
							method.Params[0].Type == 3)
						{
							if (!strcmp(method.Params[0].Value, "true"))
							{
								IsProfilerActive = true;

								printResult("Profiler turned on");
							}
							else
							{
								IsProfilerActive = false;

								printResult("Profiler turned off");
							}
						}
						else
						{
							printError("Profiler method has format: Profiler(bool)");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				case 'B':
				{
					if (!strcmp(method.MethodName, "BeginTran"))
					{
						if (method.ParamCount == 0)
						{
							TranID = pDB->beginTran(READ_COMMITED_TRAN);
						}
						else if (method.ParamCount == 1 &&
							method.Params[0].Type == 3)
						{
							if (!strcmp(method.Params[0].Value, "TranType.ReadCommited"))
							{
								TranID = pDB->beginTran(READ_COMMITED_TRAN);
							}
							else if (!strcmp(method.Params[0].Value, "TranType.RepeatableRead"))
							{
								TranID = pDB->beginTran(REPEATABLE_READ_TRAN);
							}
							else if (!strcmp(method.Params[0].Value, "TranType.Snapshot"))
							{
								TranID = pDB->beginTran(SNAPSHOT_TRAN);
							}
							else
							{
								printError("BeginTran method has format BeginTran(TranType.ReadCommited | TranType.RepeatableRead | TranType.Snapshot)");

								return false;
							}
						}
						else
						{
							printError("BeginTran method has format BeginTran(TranType.ReadCommited | TranType.RepeatableRead | TranType.Snapshot)");

							return false;
						}

						printResult("Transaction has been started.");

						pDQ->TranID = TranID;
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				case 'R':
				{
					if (!strcmp(method.MethodName, "RollbackTran"))
					{
						if (method.ParamCount == 0)
						{
							pDB->rollbackTran(TranID);
						}
						else
						{
							printError("RollbackTran method has format RollbackTran()");

							return false;
						}

						TranID = 0;
						pDQ->TranID = 0;

						printResult("Transaction has been rollbacked.");
					}
					else if (!strcmp(method.MethodName, "RestoreOnDate"))
					{
						if (method.ParamCount == 3 &&
							method.Params[0].Type == 2 &&
							method.Params[1].Type == 2 &&
							method.Params[2].Type == 2)
						{
							char param[512];
							sprintf(param, " -restoreondate %u %u %u %u %u %u",
								method.Params[0].ValueInt,
								method.Params[1].ValueInt,
								method.Params[2].ValueInt,
								0,
								0,
								0);

							restartServer(pDB, stopServer, param);
						}
						else if (method.ParamCount == 6 &&
							method.Params[0].Type == 2 &&
							method.Params[1].Type == 2 &&
							method.Params[2].Type == 2 &&
							method.Params[3].Type == 2 &&
							method.Params[4].Type == 2 &&
							method.Params[5].Type == 2)
						{
							char param[512];
							sprintf(param, " -restoreondate %u %u %u %u %u %u",
								method.Params[0].ValueInt,
								method.Params[1].ValueInt,
								method.Params[2].ValueInt,
								method.Params[3].ValueInt,
								method.Params[4].ValueInt,
								method.Params[5].ValueInt);

							restartServer(pDB, stopServer, param);
						}
						else
						{
							printError("RestoreOnDate method has format: RestoreOnDate(year, month, day) or RestoreOnDate(year, month, day, hour, minute, second)");

							return false;
						}
					}
					else
					{
						printError("%s method is not supported.", method.MethodName);

						return false;
					}

					break;
				}

				default:
				{
					printError("%s method is not supported.", method.MethodName);

					return false;
				}
				}
			}

		}
		catch (DniproError de)
		{
			printError(de.Message);
		}

		return true;
	}

	void run(char* query)
	{
		CurrLine = 1;

		uint i = 0;

		while (query[i]) //not end of query
		{
			//skip enmpty lines
			while (true)
			{
				if (query[i] == ' ' ||
					query[i] == '\r')
				{
					i++;

					continue;
				}

				if (query[i] == '\n')
				{
					i++;

					CurrLine++;

					continue;
				}

				break;
			}

			//end of file
			if (!query[i])
			{
				break;
			}

			if (!parseQuery(query, i))
			{
				return;
			}

			if (!runQuery())
			{
				return;
			}

			CurrLine++;
		}
	}
};