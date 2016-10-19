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
#include "DniproDB.h"

class Test
{
public:

	Test(DniproDB* pDB)
	{
		this->pDB = pDB;
		this->totalTime = 0;
		this->totalQueries = 0;
	}

	DniproDB* pDB;

	time_t start;

	uint totalTime;
	uint totalQueries;
	
	char jsons[TESTS_COUNT_KEYS][64];
	
	void startTest(uint number, char* name)
	{
		start = clock();

		printf("[Test: %u] %s...\n", number, name);
	}

	void endTest(uint amountQueries = 0,
				 uint controlValue = 0)
	{
		uint time = (uint)(clock() - start) + 1;

		printf("=> Time: %u ms.", time);

		if (amountQueries)
		{
			printf(" Queries/sec: %u", (uint)(amountQueries * 1000 / time));
		}

		if (controlValue)
		{
			printf(" Control value: %u", controlValue);
		}

		printf("\n");

		totalTime += time;
		totalQueries += amountQueries;

		DniproInfo::PrintLine();
	}

	void total()
	{
		printf("=> Total Time: %u ms. Total Queries: %u\n", totalTime, totalQueries);
		printf("=> [iCore 5, Dnipro v1_0_3]: 7994 ms\n");
				
		DniproInfo::PrintLine();
	}

	void init()
	{
		startTest(1, "Generate 1M jsons. Each with 3 attributes\nTemplate:{'attr1':'%d','attr2':'%d','obj':{'attr3':'%d'}}");

		for (uint i = 0; i<TESTS_COUNT_KEYS; i++)
		{
			sprintf(jsons[i], "{'attr1':'%d','attr2':'%d','obj':{'attr3':'%d'}}",
				i, i%100, i%1000);
		}

		endTest(TESTS_COUNT_KEYS);
	}

	void test2()
	{
		startTest(2, "Insert 1M jsons to database whithin 1M transactions.\nEach with 3 attributes (insert 3M attributes)");

		for (uint i = 0; i<TESTS_COUNT_KEYS; i++)
		{
			pDB->addDoc(jsons[i]);
		}

		endTest(TESTS_COUNT_KEYS);
	}

	void test3()
	{
		startTest(3, "Insert 1M nested attributes to database within 1 transaction \n(for each document one new attribute)");

		uint tranID = pDB->beginTran();

		for (uint i = 1; i<=TESTS_COUNT_KEYS; i++)
		{
			pDB->insPartDoc("{'obj':{'attr4':'x1'}}", i, tranID);
		}

		pDB->commitTran(tranID);

		endTest(TESTS_COUNT_KEYS);
	}

	void test4()
	{
		startTest(4, "Lookup 1M attributes in documents.\n(for each document one lookup attribute)");

		uint temp = 0;
		char jsonResult[128];

		for (uint i = 1; i <= TESTS_COUNT_KEYS; i++)
		{
			pDB->getPartDoc("{'attr1':$}", jsonResult, i);

			temp += jsonResult[0]; //avoid optimization of compilator
		}

		endTest(TESTS_COUNT_KEYS, temp);
	}

	void test5()
	{
		char query[100][128];

		for (uint i = 0; i < 100; i++)
		{
			sprintf(query[i], "{'attr2':'%d'}", i);
		}

		startTest(5, "1M queries like:\nFind all docs where {'attr2':'1'}, {'attr2':'2'}, {'attr2':'3'}");

		uint temp = 0;
		
		for (uint i = 1; i <= TESTS_COUNT_KEYS; i++)
		{
			temp += (uint)pDB->getDocsByAttr(query[i%100]);
		}

		endTest(TESTS_COUNT_KEYS, temp);
	}

	void test6()
	{
		char query[100][128];

		for (uint i = 0; i < 100; i++)
		{
			sprintf(query[i], "{'attr1':'%d','attr2':'%d'}", i, i);
		}

		startTest(6, "1M queries like:\nFind all docs where {'attr1':'1','attr2':'1'}, {'attr1':'2','attr2':'2'}");

		uint temp = 0;

		for (uint i = 1; i <= TESTS_COUNT_KEYS; i++)
		{
			temp += (uint)pDB->getDocsByAttr(query[i % 100]);
		}

		endTest(TESTS_COUNT_KEYS, temp);
	}

	void test7()
	{
		char query[100][128];

		for (uint i = 0; i < 100; i++)
		{
			sprintf(query[i], "{'attr1':'%d','attr2':'%d','obj':{'attr3':'%d'}}", i, i, i);
		}

		startTest(7, "1M queries like:\nFind all docs where {'attr1':'1','attr2':'1','obj':{'attr3':'1'}}, {'attr1':'2','attr2':'2', 'obj':{'attr3':'2'}}");

		uint temp = 0;

		for (uint i = 1; i <= TESTS_COUNT_KEYS; i++)
		{
			temp += (uint)pDB->getDocsByAttr(query[i % 100]);
		}

		endTest(TESTS_COUNT_KEYS, temp);
	}

	void test8()
	{
		startTest(8, "Update 1M nested attributes in database within 1 transaction \n(for each document one updated attribute)");

		ValueList* vl = pDB->getDocsByAttr("{'obj':{'attr4':'x1'}}");

		uint tranID = pDB->beginTran();

		for (uint i = 1; i <= TESTS_COUNT_KEYS; i++)
		{
			pDB->updPartDoc("{'obj':{'attr4':'x2'}}", i, tranID);
		}

		pDB->commitTran(tranID);

		endTest(TESTS_COUNT_KEYS);

		vl = pDB->getDocsByAttr("{'obj':{'attr4':'x2'}}");
	}

	void test9()
	{
		return;

		char query[100][128];

		for (uint i = 0; i < 100; i++)
		{
			sprintf(query[i], "{'attr2':@'%u-%u'}", i * 10, i * 10 + 10);
		}

		startTest(9, "100K queries (scan) like:\nFind all docs where {'attr1':@'1-10'}, {'attr1':@'10-20'}, {'attr1':@'20-30'}");

		uint temp = 0;

		for (uint i = 1; i <= TESTS_COUNT_KEYS/1000; i++)
		{
			temp += (uint)pDB->getDocsByAttr(query[i % 100]);

			printf("%d\n", i);
		}

		endTest(TESTS_COUNT_KEYS/1000, temp);
	}

	void test10()
	{
		//join
	}

	void test11()
	{
		//aggregation
	}

	void test12()
	{
		startTest(12, "Delete 1M nested attributes in database within 1 transaction \n(for each document delete one nested attribute)");

		ValueList* vl = pDB->getDocsByAttr("{'obj':{'attr4':'x2'}}");

		uint tranID = pDB->beginTran();

		for (uint i = 1; i <= TESTS_COUNT_KEYS; i++)
		{
			pDB->delPartDoc("{'obj':{'attr4':'x2'}}", i, tranID);
		}

		pDB->commitTran(tranID);

		endTest(TESTS_COUNT_KEYS);
	}

	void run()
	{
		init();

		test2();

		test3();

		test4();

		test5();

		test6();

		test7();

		test8();

		test9();

		test10();

		test11();

		test12();

		total();
	}

	void testArrays()
	{
		/*
		//test 1
		uint id;

		id = db->addDoc("{'Arr':[1]}");

		char buff[128];
		db->getPartDoc("{'Arr':[Count,$]}", buff, id);

		//test 2
		id = db->addDoc("{'Arr':[]}");

		char buff[128];
		db->getPartDoc("{'Arr':[Count,$]}", buff, id);

		//test 3
		id = db->addDoc("{'Arr':[1,2]}");

		char buff[128];
		db->getPartDoc("{'Arr':[Count,$]}", buff, id);

		//test 4
		id = db->addDoc("{'Arr':[{'a':1}]}");

		char buff[128];
		db->getPartDoc("{'Arr':[Count,$]}", buff, id);

		//test 5
		id = db->addDoc("{'Arr':[{'a':1},{'b':1}]}");

		char buff[128];
		db->getPartDoc("{'Arr':[Count,$]}", buff, id);

		db->addDoc("{'Arr':[]}");
		db->addDoc("{'Arr':[{'a':1}]}");
		db->addDoc("{'Arr':[{'a':1},{'a':1}]}");
		db->addDoc("{'Arr':[1]}");
		db->addDoc("{'Arr':[1,2]}");

		char buff[256];
		db->getPartDoc("{'Arr':[R,{'a':$}]}", buff, 1);
		db->getPartDoc("{'Arr':[R,{'a':$}]}", buff, 2);
		db->getPartDoc("{'Arr':[R,{'a':$}]}", buff, 3);
		db->getPartDoc("{'Arr':[R,$]}", buff, 4);
		db->getPartDoc("{'Arr':[R,$]}", buff, 5);
		*/

		return;
	}
	
};
