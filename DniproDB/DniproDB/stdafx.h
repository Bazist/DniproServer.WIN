// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <windows.h>

#define uint unsigned int
#define ulong unsigned __int64
#define ushort unsigned short
#define uchar unsigned char
#define ucode unsigned char

const uint MAX_SHORT = 65536;
const uint MAX_CHAR = 256;
const uint INIT_MAX_PAGES = 1024;
const uint PAGE_SIZE = MAX_SHORT*2;
const uchar BLOCK_SIZE = 16;
const uchar ROW_LEN = 3;

const uint SERVER_BUFFER_SIZE = 50000000;
const uint WRITE_TRANS_BUFFER_SIZE = 1024 * 1024;
const uint MAX_POOL_PAGES = 50000;
// TODO: reference additional headers your program requires here

const uint MAX_TRANS = 16; //max trans in the same time
const uint MAX_ATTR_VALUES_PAGES = 4096;

const uint READONLY_TRAN = 0;
const uint READ_COMMITED_TRAN = 1;
const uint REPEATABLE_READ_TRAN = 2;
const uint SNAPSHOT_TRAN = 3;

const uint CONNECTION_ERROR = 1;
const uint DEADLOCK_ERROR = 2;
const uint COLL_ALREADY_EXISTS_ERROR = 3;
const uint COLL_IS_NOT_FOUND_ERROR = 4;

const uint TESTS_COUNT_KEYS = 1000000;