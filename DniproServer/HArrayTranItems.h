/*
# Copyright(C) 2010-2016 Viacheslav Makoveichuk (email: dniprodb@gmail.com, skype: vyacheslavm81)
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
#include "HArrayTranItemKeysPool.h"

enum HArrayTranItemType: uchar8
{
	Inserted = 1,
	Deleted = 2,
	Rollbacked = 3
};

class HArrayTranItem
{
public:
	HArrayTranItemType Type; //1 - Inserted, 2 - Deleted, 3 - Rollbacked
	uint32* pIndexInVL; //index in Value List (need for fast fast delete item from ha1.ValueList with big Count)
	uchar8 TranID;
	uchar8 CollID;
	uint32* Key;
	uint32 KeyLen;
	uint32 Value;

	void setKey(HArrayTranItemKeysPool& keyPool, uint32* key, uint32 keyLen)
	{
		Key = keyPool.newKey(keyLen);

		memcpy(Key, key, keyLen);
	}
};

class HArrayTranItems
{
public:
	HArrayTranItems()
	{
		pNextItems = 0;
	}

	HArrayTranItem Items[128];
	HArrayTranItems* pNextItems;
};
