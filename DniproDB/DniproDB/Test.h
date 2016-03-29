#include "stdafx.h"
#include "DniproDB.h"

class Test
{
public:

	Test(DniproDB* pDB)
	{
		this->pDB = pDB;
	}

	DniproDB* pDB;

	time_t start;
	
	char jsons[TESTS_COUNT_KEYS][64];
	
	void startTest(uint number, char* name)
	{
		start = clock();

		printf("[Test: %u] %s...\n", number, name);
	}

	void endTest(uint ops = 0)
	{
		uint time = (uint)(clock() - start);

		printf("=> Time: %u ms. Queries/sec: %u\n", time, (uint)(ops * 1000 / time));

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
		startTest(4, "Update 1M nested attributes in database within 1 transaction \n(for each document one updated attribute)");

		uint tranID = pDB->beginTran();

		for (uint i = 1; i <= TESTS_COUNT_KEYS; i++)
		{
			pDB->updPartDoc("{'obj':{'attr4':'x2'}}", i, tranID);
		}

		pDB->commitTran(tranID);

		endTest(TESTS_COUNT_KEYS);
	}

	void run()
	{
		init();

		test2();

		test3();

		//test4();
	}

	
};
