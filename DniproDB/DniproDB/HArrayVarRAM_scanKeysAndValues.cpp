#include "StdAfx.h"
#include "HArrayVarRAM.h"

void HArrayVarRAM::scanKeysAndValuesFromBlock(uint* key,
											  uint contentOffset,
											  uint keyOffset,
											  uint blockOffset,
											  HARRAY_ITEM_VISIT_FUNC visitor,
											  void* pData)
{
	//Key\Value storage
}

void HArrayVarRAM::scanKeysAndValues(uint* key,
									 uint keyOffset, 
									 uint contentOffset,
									 HARRAY_ITEM_VISIT_FUNC visitor,
									 void* pData)
{
	//Key\Value storage
}

uint HArrayVarRAM::scanKeysAndValues(uint* key, 
									 uint keyLen,
									 HARRAY_ITEM_VISIT_FUNC visitor,
									 void* pData)
{
	//Key\Value storage

	return 0;
}

uint HArrayVarRAM::scanKeysAndValues(uint* key,
									HARRAY_ITEM_VISIT_FUNC visitor,
									void* pData)
{
	return 0;
}