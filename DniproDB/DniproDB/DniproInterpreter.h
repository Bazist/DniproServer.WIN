#include "stdafx.h"
#include "DniproQuery.h"

struct MethodParam
{
	char Value[1024];
	uint ValueInt;

	uchar Type;
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
	DniproDB* pDB;

	Method methods[16];
	uint methodCount;

	bool validateJson(char* query)
	{
		if (query[0] != '{')
		{
			printf("Json should be started from '{' symbol.\n");

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
			printf("Json should be finished with '}' symbol.\n");

			return false;
		}

		if (quotes != 0)
		{
			printf("Quotes don't match to each other.\n");

			return false;
		}

		if (bracket1 != 0)
		{
			printf("Brackets '{' and '}' don't match to each other.\n");

			return false;
		}

		if (bracket2 != 0)
		{
			printf("Brackets '[' and ']' don't match to each other.\n");

			return false;
		}

		return true;
	}

	bool parseQuery(char* query)
	{
		methodCount = 0;

		if (query[0] != 'd' &&
			query[1] != 'b')
		{
			printf("Query should be started from db. For example: db.GetWhere(...).Print(...)");

			return false; //error
		}

		uint i = 2;

		while (query[i])
		{
			if (query[i] == ' ' ||
				query[i] == '\n')
			{
				i++;

				continue;
			}

			if (query[i++] != '.')
			{
				printf("Expected symbol '.'.\n");

				return false; //error
			}

			Method& method = methods[methodCount];

			uint j = 0;

			//read method
			while (query[i] != '(')
			{
				if (!query[i])
				{
					printf("Unexpectable end of query.\n");

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
					printf("Unexpectable end of query.\n");

					return false; //error
				}

				//skip space
				if (query[i] == ' ' ||
					query[i] == '\n' ||
					query[i] == ',')
				{
					i++;

					continue;
				}

				if (!query[i])
				{
					printf("Unexpectable end of query.\n");

					return false; //error
				}

				//read string param
				if (query[i] == '"')
				{
					method.Params[method.ParamCount].Type = 1;
					char* value = method.Params[method.ParamCount].Value;

					for (j = 0, i++; query[i] != '"'; i++, j++)
					{
						if (!query[i])
						{
							printf("Unexpectable end of query.\n");

							return false; //error
						}

						if (query[i] == '\\')
						{
							i++;
						}

						value[j] = query[i];
					}

					value[j] = 0;

					if (!validateJson(value))
					{
						return false;
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
							printf("Unexpectable end of query.\n");

							return false; //error
						}

						if (query[i] >= '0' && query[i] <= '9')
						{
							method.Params[method.ParamCount].Value[j] = query[i];
						}
						else
						{
							printf("Unexpectable symbols in Number.\n");

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
							printf("Unexpectable end of query.\n");

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
		DniproQuery* pDQ = new DniproQuery(pDB);
		uint tranID = 0;

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
						uint docID = pDB->addDoc(method.Params[0].Value, tranID);

						printf("Document %d was added.\n", docID);
					}
					else
					{
						printf("AddDoc method has format: AddDoc(json)\n");

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
						printf("AndWhere method has format: AndWhere(json)\n");

						return false;
					}
				}
				else if (!strcmp(method.MethodName, "AndWhereElems"))
				{
					if (method.ParamCount == 1 &&
						method.Params[0].Type == 1)
					{
						pDQ->andWhereElems(method.Params[0].Value);
					}
					else if (method.ParamCount == 2 &&
						method.Params[0].Type == 1 &&
						method.Params[1].Type == 2)
					{
						pDQ->andWhereElems(method.Params[0].Value,
							method.Params[1].ValueInt);
					}
					else
					{
						printf("AndWhereElems method has format: AndWhereElems(json) or AndWhereElems(json, docID)\n");

						return false;
					}
				}
				else if (!strcmp(method.MethodName, "Avg"))
				{
					if (method.ParamCount == 1 &&
						method.Params[0].Type == 1)
					{
						printf("%d\n", pDQ->sum(method.Params[0].Value));
					}
					else
					{
						printf("Avg method has format: Avg(json)\n");

						return false;
					}
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

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
						pDB->insPartDoc(method.Params[0].Value, method.Params[0].ValueInt, tranID);

						printf("Attributes %d were inserted.\n");
					}
					else
					{
						printf("InsPartDoc method has format: InsPartDoc(json, docID)\n");

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
						printf("Insert method has format: Insert(json)\n");

						return false;
					}
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

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
						pDB->updPartDoc(method.Params[0].Value, method.Params[0].ValueInt, tranID);

						printf("Attributes %d were updated.\n");
					}
					else
					{
						printf("UpdPartDoc method has format: UpdPartDoc(json, docID)\n");

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
						printf("Update method has format: Update(json)\n");

						return false;
					}
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

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
						pDB->delPartDoc(method.Params[0].Value, method.Params[0].ValueInt, tranID);

						printf("Attributes %d were deleted.\n");
					}
					else
					{
						printf("DelPartDoc method has format: DelPartDoc(json, docID)\n");

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
						printf("Delete method has format: Delete(json)\n");

						return false;
					}
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

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
						printf("GetWhere method has format: GetWhere(json)\n");

						return false;
					}
				}
				else if (!strcmp(method.MethodName, "GetWhereElems"))
				{
					if (method.ParamCount == 1 &&
						method.Params[0].Type == 1)
					{
						pDQ->getWhereElems(method.Params[0].Value);
					}
					else if (method.ParamCount == 2 &&
						method.Params[0].Type == 1 &&
						method.Params[1].Type == 2)
					{
						pDQ->getWhereElems(method.Params[0].Value,
							method.Params[1].ValueInt);
					}
					else
					{
						printf("getWhereElems method has format: GetWhereElems(json) or GetWhereElems(json, docID)\n");

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
						printf("GetAll method has format: GetAll()\n");

						return false;
					}
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

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
						printf("OrWhere method has format: OrWhere(json)\n");

						return false;
					}
				}
				else if (!strcmp(method.MethodName, "OrWhereElems"))
				{
					if (method.ParamCount == 1 &&
						method.Params[0].Type == 1)
					{
						pDQ->orWhereElems(method.Params[0].Value);
					}
					else if (method.ParamCount == 2 &&
						method.Params[0].Type == 1 &&
						method.Params[1].Type == 2)
					{
						pDQ->orWhereElems(method.Params[0].Value,
							method.Params[1].ValueInt);
					}
					else
					{
						printf("OrWhereElems method has format: OrWhereElems(json) or OrWhereElems(json, docID)\n");

						return false;
					}
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

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
							method.Params[1].Value);
					}
					else
					{
						printf("Join method has format: Join(json1, json2)\n");

						return false;
					}
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

					return false;
				}

				break;
			}

			case 'S':
			{
				if (!strcmp(method.MethodName, "Skip"))
				{
					if (method.ParamCount == 1 &&
						method.Params[0].Type == 2)
					{
						pDQ->skip(method.Params[0].ValueInt);
					}
					else
					{
						printf("Skip method has format: Skip(amount)\n");

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
						printf("Sort method has format: Sort(json) or Sort(json, isAscending)\n");

						return false;
					}
				}
				else if (!strcmp(method.MethodName, "Sum"))
				{
					if (method.ParamCount == 1 &&
						method.Params[0].Type == 1)
					{
						printf("%d\n", pDQ->sum(method.Params[0].Value));
					}
					else
					{
						printf("Sum method has format: Sum(json)\n");

						return false;
					}
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

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
						printf("Skip method has format: Take(amount)\n");

						return false;
					}
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

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
						printf("%d", pDQ->count());
					}
					else
					{
						printf("Count method has format: Count()\n");

						return false;
					}
				}
				else if (!strcmp(method.MethodName, "CommitTran"))
				{
					if (method.ParamCount == 0)
					{
						pDB->commitTran(tranID);
					}
					else
					{
						printf("CommitTran method has format CommitTran()\n");

						return false;
					}

					tranID = 0;
					pDQ->TranID = 0;

					printf("Transaction has been commited.\n");
				}
				else if (!strcmp(method.MethodName, "Clear"))
				{
					if (method.ParamCount == 0)
					{
						pDB->clear();

						printf("Database is cleared !\n");
					}
					else
					{
						printf("Clear method has format Clear()\n");

						return false;
					}
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

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
						printf("Print method has format: Print(json)\n");

						return false;
					}
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

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
						tranID = pDB->beginTran(READ_COMMITED_TRAN);
					}
					else if (method.ParamCount == 1 &&
						method.Params[0].Type == 3)
					{
						if (!strcmp(method.Params[0].Value, "TranType.ReadCommited"))
						{
							tranID = pDB->beginTran(READ_COMMITED_TRAN);
						}
						else if (!strcmp(method.Params[0].Value, "TranType.RepeatableRead"))
						{
							tranID = pDB->beginTran(REPEATABLE_READ_TRAN);
						}
						else if (!strcmp(method.Params[0].Value, "TranType.Snapshot"))
						{
							tranID = pDB->beginTran(SNAPSHOT_TRAN);
						}
						else
						{
							printf("BeginTran method has format BeginTran(TranType.ReadCommited | TranType.RepeatableRead | TranType.Snapshot)\n");

							return false;
						}
					}
					else
					{
						printf("BeginTran method has format BeginTran(TranType.ReadCommited | TranType.RepeatableRead | TranType.Snapshot)\n");

						return false;
					}

					printf("Transaction has been started.\n");

					pDQ->TranID = tranID;
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

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
						pDB->rollbackTran(tranID);
					}
					else
					{
						printf("RollbackTran method has format RollbackTran()\n");

						return false;
					}

					tranID = 0;
					pDQ->TranID = 0;

					printf("Transaction has been rollbacked.\n");
				}
				else
				{
					printf("%s method is not supported.\n", method.MethodName);

					return false;
				}

				break;
			}

			default:
			{
				printf("%s method is not supported.\n", method.MethodName);

				return false;
			}
			}
		}

		return true;
	}

	void run(char* query)
	{
		if (!parseQuery(query))
		{
			return;
		}

		if (!runQuery())
		{
			return;
		}
	}

	static DWORD WINAPI runConsole(LPVOID pVal)
	{
		char query[1024];   /* the string is stored through pointer to this buffer */

		DniproInterpreter di;
		di.pDB = (DniproDB*)pVal;

		//wait, server is not started
		Sleep(1000);

		while (true)
		{
			printf("\nEnter query >> ");
			fflush(stdout);

			fgets(query, 1024, stdin); /* buffer is sent as a pointer to fgets */

			di.run(query);
		}

	}
};