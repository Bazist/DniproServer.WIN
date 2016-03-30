#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <atomic>
#include <windows.h>
#include "HArrayVarRAM.h"
#include "DniproDB.h"
#include "DniproQuery.h"
#include "DniproInterpreter.h"
#include "Server.h"
#include "Storage.h"
#include "Test.h"

using namespace std;

const uint COUNT_KEYS = 10000000;

/*
int test()
{
	HArrayFixHDD sentence;
	//20/21/22/23/24 /25 /26 /
	//19/14/9 /6 /3.7/2.7/2.4/
	clock_t start, finish;

	FILE* file = fopen("c:\\avi_in_zip.zip", "rb");

	keys = new ulong[COUNT_KEYS];
	
	for(uint i=1; i<COUNT_KEYS; i++)
	{
		fread(&keys[i], 4, 1, file);
	}
	
	fclose(file);

	sentence.init(24, 0);

	uint* keys = new uint[COUNT_KEYS];
	uint* values = new uint[COUNT_KEYS];
	
	for(uint i=0; i<COUNT_KEYS; i++)
	{
		keys[i] = i;
		values[i] = i;
		count1 += i;
	}
	
	//map<uint, uint> m;

	start = clock();

	//368 mb //average for 15
	//372 mb //average for 10
	//383 mb //average for 5
	//423 mb //average for 1

	//( array(4 bytes value + 2 bytes key) + block(4bytes value + 2 bytes key)
	//12 bytes

	//init(keys, values, COUNT_KEYS);

	for(uint i=0; i<COUNT_KEYS; i++)
	{
		//if(i==4688209)
		//	printf( "%2.1d\n", i);
		
		sentence.insert(keys[i], keys[i]);
		//insert(i, i);
		//m[keys[i]] = keys[i];
		
		//4688210
		
		if(i>9918351)
		for(uint j=0; j<=i; j++)
		{
			if(getValueByKey(keys[j]) != keys[j])
			{
				printf( "Error: %2.1d %2.1d\n", getValueByKey(j), j);
				return 0;
			}
		}
	}

	finish = clock();

	//delete[] values;

	printf( "Filled: %2.1d msec\n", (finish - start) );

	start = clock();

	for(uint i=0; i<COUNT_KEYS; i++)
	{
		if(sentence.getValueByKey(keys[i]) != keys[i])
		{
			printf( "Error: %2.1d %2.1d\n", sentence.getValueByKey(keys[i]), keys[i]);
		}
	}
	
	finish = clock();

	delete[] keys;
	
	printf( "Searches: %2.1d msec\n", (finish - start) );
	printf( "CountDoublePage: %2.1d\n", sentence.CountDoublePage );
	printf( "CountMultiplyPage: %2.1d\n", sentence.CountMultiplyPage );
	printf( "Double size pages: %2.1d\n", sentence.CountDoublePage * sizeof(DoublePage));
	printf( "Multiply size pages: %2.1d\n", sentence.CountMultiplyPage * sizeof(MultiplyPage));
	printf( "Size pages: %2.1d\n", sentence.CountDoublePage * sizeof(DoublePage) + sentence.CountMultiplyPage * sizeof(MultiplyPage));
	
	uint* buff = new uint[COUNT_KEYS];

	start = clock();

	uint count = sentence.getValuesByRange(buff, COUNT_KEYS, 1, 0xFFFFFFFF);
	
	finish = clock();

	printf( "Range: %2.1d msec\n", (finish - start) );

	delete[] buff;

	sentence.destroy();

	//2.8 1.3

	start = clock();

	uint* buffer = new uint[COUNT_KEYS];

	int idx = getValuesByRange(buffer, COUNT_KEYS, 0, COUNT_KEYS - 1);
	
	delete[] buffer;

	finish = clock();

	printf( "Search by range: %2.1d msec\n", (finish - start) );

	destroy();

	printf( "List: %2.1d bytes\n", COUNT_KEYS*4*2);


	return 0;
}
*/

HArrayVarRAM ha;

struct Uint4
{
   Uint4(){}
   
   uint data[4];
};

bool operator<(const Uint4& a, const Uint4& b) 
{ 
   for(uint i=0; i<4; i++)
   {
	   if(b.data[i]==a.data[i])
		   continue;
	   else
		   return b.data[i] < a.data[i];
   }

   return false;
}

Uint4 keys[COUNT_KEYS];

void fill()
{
	FILE* file = fopen("c:\\rand.avi", "rb");

	fseek(file, 0, 0);

	for(uint i=0; i<COUNT_KEYS; i++)
	{
		fread(&keys[i].data, 4, 4, file);
	}
	
	fclose(file);
}

char jsons[100000][128];

void initJsons()
{
	for(uint i=0; i<100000; i++)
	{
		sprintf(jsons[i], "{\"name\":\"%d\",\"surname\":\"%d\",\"company\":{\"name\":\"%d\",\"year\":\"%d\",\"price\":\"%d\"},\"age\":\"%d\"}",
						  i, i, i, i, i, i);

		//sprintf(jsons[i], "{'Name':'Bill','Age':'32','Languages':['English','Germany']}");
	}
}

void initJsonsTCPH()
{
	for(uint i=0; i<10000; i++)
	{
		sprintf(jsons[i], "{'category':'%d','name':'%d','country':'%d','quantity':'0','price':'%d'}",
						  i%100, i, i%50, i%10000 + 1);
	}
}

uint benchTCPH_Distrib(DniproDB& db,
					   uint itemName,
					   char* company,
					   uint quantity,
					   uint date)
{
	//db.beginTran();

	//char query[100];
	//sprintf(query, "{\"name\":\"%d\"}", itemName);

	////get doc parameters
	//uint docID = db.getDocsByAttr(query);
	//uint totalQuantity = db.getAttrValueByDocID("{'quantity':$}", docID);

	////add distrib section
	//sprintf(query, "{'distribLog':[Add,{'company':'%s','quantity':'%d','date':'%d'}]", company, quantity, date);
	//db.insPartDoc(query, docID);

	//sprintf(query, "{'quantity':'%d'}", totalQuantity + quantity);
	//db.updPartDoc(query, docID);

	//db.commitTran();
	//
	return 70;
}

void benchTCPH()
{
	DniproDB db;
	db.init();

	clock_t start, finish;

	initJsonsTCPH();

	start = clock();

	for(uint i=1; i<100000; i++)
	{
		db.addDoc(jsons[i], 0);
	}

	finish = clock();

	uint size = 0;

	for(uint i=1; i<100000; i++)
	{
		size += strlen(jsons[i]);
	}

	printf( "addDoc: %d msec\n", (finish - start) );

	printf( "addDoc: %d mb/sec\n", (size >> 20) * CLOCKS_PER_SEC / (finish - start));

	uint control = 0;

	/*start = clock();

	for(uint i=1; i<50000; i++)
	{
		size += benchTCPH_Distrib(db, i, "MotorSich", i%10, i);
	}
	
	finish = clock();
	*/

	char query[100];
	
	start = clock();

	for(uint i=1; i<5000; i++)
	{
		sprintf(query, "{'country':'%d'}", i%50);

		ValueList* pValueList = db.getDocsByAttr(query);
		for(uint j=1; j < pValueList->Count; j++)
		{
			control += 0; // db.getAttrValueByDocID("{'quantity':$}", pValueList->pValues[j]);
		}
	}
	
	finish = clock();

	printf( "Doc Size: %d mb\n", size / 1024 / 1024);
	printf( "DB Size: %d mb\n", db.getTotalMemory() / 1024 / 1024);
	printf( "Distrib: %d msec\n", (finish - start) );
	printf( "Distrib: %d op/sec\n", 5000 * CLOCKS_PER_SEC  / (finish - start) );

	printf( "Control: %d\n", control );

}

void testDniproDB()
{
	char jsonResult[1024];
	
	//инициилизируем СУБД
	DniproDB db;
	db.init();

	//db.addDoc("{'array':['1',{'x':'2'},'3']}");
	//db.addDoc("{'array':['1','2','3']}");
	//db.addDoc("{'array':['1',['2'],'3']}");
	//db.addDoc("{'array':['1',['2',{'key':'123'}],'4','5',[{'key':['12']}],'7']}");

	//db.addDoc("{'array':['1']}");

	//создали массив из одного элемента
	db.addDoc("{'array':[[Add,'1']]}");

	db.insPartDoc("{'array':[[Add, '2']]}", 1);
	db.insPartDoc("{'array':[[Add, '3']]}", 1);

	db.getPartDoc("{'array':[[$, $, $]]}", jsonResult, 1);

	//вернули последний элемент из массива
	//db.getPartDoc("{'array':[2, {'login':$,'age':$}]}", jsonResult, 1);

	//db.getPartDoc("{'array':[Add, [1, $]]}", jsonResult, 1);

	uint docID;// = db.getDocsByAttr("{'array':['1']}");

	return;

	//добавляем документ
	db.addDoc("{\"name\":\"DniproDB\",\"author\":\"Bazist\",\"version\":\"v1.0\"}", 0);

	//возвращаем айди документа по атрибуту
	//docID = db.getDocsByAttr("{\"author\":\"Bazist\"}");

	//получаем интересующие поля из джисона по айди документа
	db.getPartDoc("{\"name\":$,\"version\":$}", jsonResult, docID);

	printf(jsonResult);

	//добавляем новый аттрибут в документ
	db.insPartDoc("{\"created\":\"2015\"}", docID);

	//возвращаем новый атрибут в документе по айди документа
	db.getPartDoc("{\"name\":$,\"created\":$}", jsonResult, docID);

	printf(jsonResult);

	//обновляем старый атрибут в документе по айди документа
	db.updPartDoc("{\"version\":\"v1.1\"}", docID);

	//читаем обновленный атрибут документа
	db.getPartDoc("{\"name\":$,\"version\":$}", jsonResult, docID);

	printf(jsonResult);

	//удаляем старый атрибут из документа по айди документа
	db.delPartDoc("{\"version\":$}", docID);

	//получаем интересующие поля из джисона по айди документа
	db.getPartDoc("{\"name\":$,\"version\":$}", jsonResult, docID);

	printf(jsonResult);
}

void benchDniproDB()
{
	DniproDB db;
	db.init();

	clock_t start, finish;

	initJsons();

	start = clock();

	for(uint i=1; i<100000; i++)
	{
		db.addDoc(jsons[i], 0);
	}

	finish = clock();

	uint size = 0;

	for(uint i=1; i<100000; i++)
	{
		size += strlen(jsons[i]);
	}

	printf( "addDoc: %d msec\n", (finish - start) );

	//printf( "addDoc: %d mb/sec\n", (size >> 20) * CLOCKS_PER_SEC / (finish - start));

	printf( "Doc Size: %d mb\n", size / 1024 / 1024);
	printf( "DB Size: %d mb\n", db.getTotalMemory() / 1024 / 1024);

	db.printMemory();

	uint control = 0;

	//printf("Hash %d", db.ha.getHash());

	char* queries[50000];

	for(uint i=0; i<50000; i++)
	{
		queries[i] = new char[32];

		sprintf(queries[i], "{\"age\":\"%d\"}", i);
	}

	control = 0;

	start = clock();

	for(uint j=1; j<10; j++)
	{
		for(uint i=1; i<50000; i++)
		{
			control += (uint)db.getDocsByAttr(queries[i]);
		}
	}

	finish = clock();

	printf( "getDocsByAttr: %d op/sec\n", 10 * 50000 * CLOCKS_PER_SEC  / (finish - start) );
	
	char jsonResult[64];

	//getPartDoc("{\"age\":$}", jsonResult, 0);

	start = clock();

	for(uint j=1; j<10; j++)
	{
		for(uint i=1; i<50000; i++)
		{
			control += db.getPartDoc("{\"age\":$}", jsonResult, i);
		}
	}

	finish = clock();

	printf( "getPartDoc: %d op/sec\n", 10 * 50000 * CLOCKS_PER_SEC  / (finish - start) );

	start = clock();

	for(uint i=1; i<50000; i++)
	{
		control += db.delPartDoc("{\"age\":$}", i);
	}
	
	finish = clock();

	printf( "delPartDoc: %d op/sec\n", 50000 * CLOCKS_PER_SEC  / (finish - start) );
	
	start = clock();

	for(uint i=1; i<50000; i++)
	{
		db.updPartDoc("{\"age\":10}", 10);
	}
	
	finish = clock();

	printf( "updPartDoc: %d op/sec\n", 50000 * CLOCKS_PER_SEC  / (finish - start) );
	
	control = 0;

	start = clock();

	for(uint i=1; i<50000; i++)
	{
		//control += db.getDocsByAttr("{\"age\":#\"<50\"}");
	}
	
	finish = clock();

	//printf( "getDocsByAttr: %d op/sec\n", 50000 * CLOCKS_PER_SEC  / (finish - start) );

	//arrays
	uint docID = db.addDoc("{'array':['0']}", 0);
		
	for(uint i=0; i<50000; i++)
	{
		sprintf(queries[i], "{'array':[@%d, '%d']}", i, i);
	}
	
	start = clock();

	for(uint i=0; i<50000; i++)
	{
		db.insPartDoc(queries[i], docID, 0);
	}
	
	finish = clock();

	printf( "insPartDoc (insert item to array): %d op/sec\n", 50000 * CLOCKS_PER_SEC  / (finish - start) );
	
	printf( "Control: %d\n", control );

}

void testRandom()
{
	fill();
	
	//z & 336 == 336

	//ha.clear("c:\\fts");

	clock_t start, finish;
		
	ha.init(4, 26);
	//ha.create();

	//ha.init(8, 4, 24);

	//INSERT ===========================================

	uint ccc = 0;

	start = clock();

	//uint* val = new uint[COUNT_KEYS * 25];

	for(uint i=0; i<COUNT_KEYS; i++)
	{
		//printf("%d\n", i);
		
		/*if(i==9032)
		{
			i=i;
		}*/

		ha.insert(keys[i].data, 16, *keys[i].data);

		/*for(uint j=0; j<25; j++)
		{
			val[i * 25 + j] = i;
		}
		*/

		/*if(i==9032)
		{
			if(ha.getValueByKey(keys[i].data, 16) != keys[i].data[0])
				printf("Error %d\n", i);
		}*/
		//ha.flush();
		
		/*{
			if(keys[i].data[0] >= 1000000 && keys[i].data[0] <= 5000000)
			{
				ccc++;
			}
		}*/

		/*if( i >= 76878)
		{
			if(ha.getValueByKey(keys[76878].data) != keys[76878].data[0])
				printf("Error %d\n", i);
		}*/

		/*uint val = *keys[76878].data;
		uchar x = 24;
		uint y = 28;

		uchar s = (val<<x)>>y;*/

		//s = s;

		//if(i % 1000 == 0)
		//{
		//	for(uint j=0; j<=i; j++)
		//	{
		//		/*if(i==12686 && j==2857)
		//		{
		//			i=i;
		//		}*/

		//		if(j==275)
		//		{
		//			j = j;
		//		}

		//		//printf("\rcheck: %d", j);		

		//		if(ha.getValueByKey(keys[j].data) != keys[j].data[0])
		//		{
		//			printf("Error on %d value %d\n", i, j);
		//			return;
		//		}
		//	}
		//}

		/*if(i >= 45971 && ha.getValueByKey(keys[45971]) != keys[45971][0])
			printf("Error\n");*/
	}

	/*uint keys2[2];
	keys2[0] = 1;

	for(uint i=1; i<COUNT_KEYS; i++)
	{
		if(i==1537)
		{
			i=i;
		}

		keys2[1] = i;
		ha.insert(keys2, 2, 123);

		for(uint j=1; j<=i; j++)
		{
			keys2[1] = j;

			if(ha.getValueByKey(keys2, 2) != 123)
			{
				printf("Error\n");
				break;
			}
		}

		printf("%d\n", i);
	}*/

	/*HArrayFixPair pairs[15000];

	uint minKey[2];
	uint maxKey[2];

	minKey[0] = 1000000;
	minKey[1] = 0;

	maxKey[0] = 5000000;
	maxKey[1] = 0xFFFFFFFF;

	uint cccc = ha.getKeysAndValuesByRange(pairs, 15000, minKey, maxKey);
*/

	/*ha.Save("c:\\fts\\info.pg",
			"c:\\fts\\header.pg",
			"c:\\fts\\content.pg",
			"c:\\fts\\branch.pg",
			"c:\\fts\\block.pg");*/

	finish = clock();

	printf( "Insert: %d msec\n", (finish - start) );

	//SEARCH ===========================================
	uint control = 0;
	uchar valueType;

	start = clock();

	for(uint i=0; i<COUNT_KEYS; i++)
	{
		if(ha.getValueByKey(keys[i].data, 16, valueType) != keys[i].data[0])
			printf("Error %d\n", i);
	}

	finish = clock();

	printf( "Control: %d msec\n", control );
	
	printf( "Search: %d msec\n", (finish - start) );
}

/*
class Tran
{
public:
	uint Version;
	bool IsFinished;
};

class TranVersionCell
{
public:
	Tran* pTran;
	uint Value;
};

class TranCell2
{
public:
	bool HasTran;
	uint Value; //actual
	TranVersionCell TranCells[8];

	uint getValue(Tran* pTran)
	{
		if(!HasTran)
		{
			return Value;
		}
		else if(pTran)
		{
			get my version
			for(uint i=0; i<8; i++)
			{
				if(TranCells[i].pTran == pTran)
				{
					return TranCells[i].Value;
				}
			}
		}

		//create my version
		/*while(true)
		{
			for(uint i=0; i<8; i++)
			{
				if(!TranCells[i].pTran || TranCells[i].pTran->IsFinished)
				{
					TranCells[i].pTran = pTran;
					TranCells[i].Value = Value;
					
					return Value;
				}
			}
		}

		bool isNotFinished = false;
		uint maxVersion = 0;
		
		for(uint i=0; i<8; i++)
		{
			if(!TranCells[i].pTran->IsFinished)
			{

				isNotFinished = true;

			}
		}

	}

	uint addValue(Tran* pTran)
	{
		for(uint i=0; i<8; i++)
		{
			if(TranCells[i].pTran)
			{
				if(TranCells[i].pTran->IsFinished)
				{
					TranCells[i].pTran = 0;
				}
			}
		}
	}
};

*/


void testDniproQuery()
{
	DniproDB db;
	db.init();

	db.addDoc("{'x':'1'}", 0);
	db.addDoc("{'x':'2'}", 0);
	db.addDoc("{'x':'3'}", 0);
	db.addDoc("{'x':'4'}", 0);
	db.addDoc("{'x':'5'}", 0);

	ValueList* vl = db.getDocsByAttr("{'x':@'>=3'}");

	DniproQuery dq(&db);
	/*
	db.addDoc("{'array':[{'color':'green','country':'ukraine','brand':'tavria'}, {'color':'white','brand':'mercedes'}]");
	db.addDoc("{'array':[{'color':'green','brand':'citroen'}, {'color':'green','brand':'lanos'}]");
	db.addDoc("{'array':[{'color':'red','brand':'audi','country':'ukraine'}, {'color':'green','brand':'ford'}]");
	
	dq.getWhereElems("{'array':[{'color':'green'}]}")->
	   orWhereElems("{'array':[{'color':'red'}]}")->
	   andWhereElems("{'array':[{'country':'ukraine'}]}")->
	   print("{'array':[{'brand':$}]}");*/
		
	//uint idx = db.getAttrValueByDocID(, 1);

	return ;

	char res[1000];
	
	db.addDoc("{'CarName':'BMW', 'Concern':'WAG', 'Country':'Germany', 'Engine':'5.0L'}");
	db.addDoc("{'CarName':'Cayenne', 'Concern':'WAG', 'Country':'Germany', 'Engine':'3.0L'}");
	db.addDoc("{'CarName':'Reno', 'Concern':'Reno', 'Country':'France', 'Engine':'1.6L'}");

	db.addDoc("{'Login':'Bazist', 'Auto':'BMW'}");
	db.addDoc("{'Login':'dozen', 'Auto':'Reno'}");
	db.addDoc("{'Login':'vic_one', 'Auto':'BMW'}");
	db.addDoc("{'Login':'Glory', 'Auto':'Cayenne'}");
	
	dq.getWhere("{'Login':'Bazist'}")->join("{'Auto':$}",
											"{'CarName':$}")->print("{'Concern':$}");
	
	//распечатать всех пользователей у кого машины с 1,6 двигателем
	//dq.getWhere("{'Auto':@{'Engine':'1.6L'}}")->print("{'name':$}");

	return;

	//db.addDoc("{'login':'bazist', 'lang':'C++', 'age':'200'}");
	//db.addDoc("{'login':'dozen', 'lang':'Oracle', 'age':'100'}");
	//db.addDoc("{'login':'Glory', 'lang':'Basic', 'age':'100'}");
	//db.addDoc("{'login':'vic_one', 'lang':'ЯЗЧЩ', 'age':'200'}");

	//DniproQuery dq(&db);

	////распечатает две строки
	//dq.getWhere("{'age':'200'}")->print("{'login':$,'lang':$}");

	////распечатает четыре строки
	//dq.getWhere("{'age':'200'}")->orWhere("{'age':'100'}")->print("{'login':$,'lang':$}");

	////ничего не распечатает
	//dq.getWhere("{'login':'bazist'}")->andWhere("{'age':'100'}")->print("{'lang':$}");

	////агрегирующие ф-ции
	//uint count = dq.getWhere("{'age':'200'}")->orWhere("{'age':'100'}")->count();
	//uint sum = dq.getWhere("{'age':'200'}")->orWhere("{'age':'100'}")->sum("{'age':$}");
}

void testDniproTranReadCommited()
{
	DniproDB db;
	db.init();

	clock_t start, finish;

	initJsons();

	start = clock();

	uint tranID = db.beginTran(READ_COMMITED_TRAN);

	for (uint i = 0; i<100000; i++)
	{
		db.addDoc(jsons[i], tranID);
	}

	db.commitTran(tranID);

	finish = clock();

	printf("addDoc: %d msec\n", (finish - start));
}

void testDniproTranRepeatableRead()
{
	DniproDB db;
	db.init("c:\\fts\\bla.dp");

	clock_t start, finish;

	initJsons();

	start = clock();

	uint tranID = 0;
	
	//tranID = db.beginTran(REPEATABLE_READ_TRAN);
		
	//ValueList* vl = db.getDocsByAttr("{'name':'bill'}", 0, tranID);
	
	for (uint i = 0; i<1000000; i++)
	{
		db.addDoc("{'name':'bill', 'type':'person'}", tranID);
	}

	db.commitTran(tranID);

	finish = clock();

	printf("addDoc: %d msec\n", (finish - start));
}

void testDeadlock()
{
	try
	{
		DniproDB db;
		db.init();

		uint docID1 = db.addDoc("{'count1':'1'}");
		uint docID2 = db.addDoc("{'count2':'2'}");

		uint tranID1 = db.beginTran(REPEATABLE_READ_TRAN);
		uint tranID2 = db.beginTran(REPEATABLE_READ_TRAN);

		char buff[128];

		db.getPartDoc("{'count1':$}", buff, docID1, tranID1);
		db.getPartDoc("{'count2':$}", buff, docID2, tranID2);

		db.getPartDoc("{'count2':$}", buff, docID2, tranID1);
		db.getPartDoc("{'count1':$}", buff, docID1, tranID2);
		
		db.commitTran(tranID1);
		db.commitTran(tranID2);
	}
	catch (DniproError de)
	{
		printf("Error: %s\n", de.Message);
	}
}

HArrayTranItemsPool pool;

std::atomic_int val;
std::atomic_int cnt;

DniproDB db;

DWORD WINAPI MyThreadFunction1(LPVOID lpParam)
{
	val++;

	char outVal[10];
	
	for (uint i = 0; i < 1; i++)
	{
		//HArrayTranItems* pObj = pool.newObject();
		//pool.releaseObject(pObj);

		//db.getPartDoc("{'count':$}", outVal, 1, 0, true);

		//printf("%d\n", i);

		db.addDoc("{'count':'1'}");

		cnt += 1;// atoi(outVal);
	}

	val--;
	
	return 0;
}

DWORD WINAPI MyThreadFunction2(LPVOID lpParam)
{
	val++;

	for (uint i = 0; i < 1000000; i++)
	{
		//HArrayTranItems* pObj = pool.newObject();
		//pool.releaseObject(pObj);
	}

	val--;

	return 0;
}

void testThreads()
{
	cnt = 0;

	db.init("c:\\fts\\bla.dp");
	//db.addDoc("{'count':'1'}");

	for (uint i = 0; i < 1; i++)
	{
		CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			MyThreadFunction1,       // thread function name
			NULL,          // argument to thread function 
			0,                      // use default creation flags 
			0);   // returns the thread identifier 

		//CreateThread(
		//	NULL,                   // default security attributes
		//	0,                      // use default stack size  
		//	MyThreadFunction2,       // thread function name
		//	NULL,          // argument to thread function 
		//	0,                      // use default creation flags 
		//	0);   // returns the thread identifier 
	}
}

int main(int argc, char** argv)
{

	/*DniproDB* db = new DniproDB();
	db->init();

	db->addDoc("{'attr':'x1'}");

	db->updPartDoc("{'attr':'x1'}", 1);

	return 0;

	for (uint i = 0; i < 10; i++)
	{
		db->addDoc("{'attr':'x1'}");
	}

	for (uint i = 1; i <= 10; i++)
	{
		db->updPartDoc("{'attr':'x1'}", i);
	}
	
	return 0;*/

	system("cls");

	//if (argc > 1 && !strcmp(argv[1], "-selftest")) //test mode
	{
		//http://www.computerhope.com/color.htm
		DniproInfo::PrintLine();
		DniproInfo::Print("= DniproDB v1.0.3 [SELFTEST] (c)Booben.Com =\n");
		DniproInfo::PrintLine();

		DniproDB* pDB = new DniproDB();
		pDB->init();

		Test* pTests = new Test(pDB);
		pTests->run();

		delete pTests;

		pDB->destroy();

		delete pDB;

		printf("Restart server in normal mode ? (y/n): ");

		if (fgetc(stdin) == 'y')
		{
			DniproInterpreter::restartServer();
		}
		else
		{
			exit(0);
		}

		return 0;
	}

	//=====================================================

	//http://www.computerhope.com/color.htm
	DniproInfo::PrintLine();
	DniproInfo::Print("=      DniproDB v1.0.3 (c)Booben.Com      =\n");
	DniproInfo::PrintLine();

	//create db
	DniproDB* pDB = new DniproDB();
	pDB->init(""); //current folder

	//run server
	Server::start(pDB, 4477);
	
	//run console
	char query[1024];

	DniproInterpreter di(pDB, 0, &Server::stop);

	//wait, server is not started
	Sleep(500);

	while (true)
	{
		printf("\nEnter query >> ");
		fflush(stdout);

		fgets(query, 1024, stdin); /* buffer is sent as a pointer to fgets */

		di.run(query);
	}
		
	//=====================================================

	/*clock_t start, finish;

	start = clock();

	testThreads();

	Sleep(100);

	while (val.load())
	{
		Sleep(20);
	}

	finish = clock();

	printf("Queries per sec: %d msec\n", cnt / (finish - start) * 1000);*/

	////printf("%d!", pool.Count);
	//printf("%d!\d", cnt);

	//testDniproTranReadCommited();
	//testDniproTranRepeatableRead();
	//testDeadlock();

	//std::atomic_char IsReaded;
	
	//IsReaded.compare_exchange_strong()

	//int sz = sizeof(IsReaded);
	
	/*pDB->addDoc("{'type':'employee', 'name':'Mykola', 'carModel':'zaz'}");
	pDB->addDoc("{'type':'car', 'model':'zaz', 'country':'ua', 'engine':'1.2'}");

	DniproQuery* pDQ = new DniproQuery(pDB);

	pDQ->getWhere("{'type':'employee', 'name':'Mykola'}")->
		 join("{'carModel':$}", "{'type':'car', 'model':$}")->
		 print("{'name':$}{'engine':$}");*/

	/*for (uint i = 0; i < 100; i++)
	{
		pDB->addDoc("{'Company':'AnySoft', 'Country':'UK', 'Employees':[{'FirstName':'Ivan', 'LastName':'Ivanov', 'Age': 25, 'Childs':[{'FirstName':'Ivanko', 'LastName':'Ivankov', 'Age':5}]}]}");
	}

	uint sum = 0;

	clock_t start, finish;

	start = clock();

	DniproQuery dq(pDB);
	
	for (uint i = 0; i < 10000; i++)
	{
		sum += dq.getWhereElems("{'Country':'UK','Employees':[{'Age':25, 'Childs':[{'Age':5}]}]}")->count();
	}

	finish = clock();

	printf("%d\n", sum);

	printf("Selects: %d msec\n", (finish - start));*/

	/*char res[1024];
	pDB->getPartDoc("{'a':{'b':$}}", res, 1);

	int x = 1;*/

	//Storage s(pDB, "c:\\data");

	//s.load();

	//s.runSyncJob();

	//time_t start, finish;

	//start = clock();

	//for(uint i=0; i<5000000; i++)
	//{
	//	pDB->addDoc("{'arr':[{'a':'1','b':'2'},{'a':'3','b':'4'}]}");
	//}

	////pDB->printMemory();

	//finish = clock();

	//printf( "Ins: %d msec\n", (finish - start) );
	
	//char buff[100];

	//db.getPartDoc("{'arr':[$]}", buff, 1);
	//db.getPartDoc("{'arr':[{'a':$,'b':$}]}", buff, 1);

	//printf(buff);
	
	//Server::Run();

	
	//testDniproQuery();

	//testRandom();

	//benchDniproDB();
	//testDniproDB();

	//benchTCPH();

	/*
	HArrayVarRAM ha;
	ha.init(4, 24);

	uint key[3];
	key[0] = 111;
	key[1] = 222;
	key[2] = 333;

	ha.insert(key, 12, 1);
	ha.insert(key, 12, 2);
	ha.insert(key, 12, 3);
	
	uchar valueType;
	uint value = ha.getValueByKey(key, 12, valueType);

	if(valueType == VALUE_LIST_TYPE)
	{
		ValueList* pValueList = (ValueList*)value;
		pValueList->Value = pValueList->Value;
	}
	*/

	system("pause");

	return 0;
};