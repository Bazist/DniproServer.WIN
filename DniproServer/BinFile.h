/*
# Copyright(C) 2010-2016 Viacheslav Makoveichuk (email: slv709@gmail.com, skype: vyacheslavm81)
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

#pragma once

#include "stdafx.h"

class BinFile
{
private:
	char m_fileName[1024];

	bool m_isWritable;
	bool m_isOverwrite;

	FILE* m_file;

	//save inmemory
	char* m_data;
	uint32 m_dataPos;
	uint32 m_dataSize;

public:
	BinFile(const char* fileName, 
					   bool isWritable, 
					   bool isOverwrite)
{
	strcpy(m_fileName, fileName);

	m_isWritable = isWritable;
	m_isOverwrite = isOverwrite;
	m_file = 0;

	m_data = 0;
	m_dataPos = 0;
	m_dataSize = 0;
}

BinFile(const char* fileName, 
					   bool isWritable, 
					   bool isOverwrite,
					   uint32 memoryBufferSize)
{
	strcpy(m_fileName, fileName);

	m_isWritable = isWritable;
	m_isOverwrite = isOverwrite;
	m_file = 0;

	m_data = new char[memoryBufferSize];
	m_dataPos = 0;
	m_dataSize = memoryBufferSize;
}

	bool save()
	{
		if(open())
		{
			if(!m_isOverwrite)
			{
				fseek(m_file, 0L, SEEK_END);
			}

			fwrite(m_data, 1, m_dataPos, m_file);

			fflush(m_file);

			fclose(m_file);

			return true;
		}
		else
		{
			return false;
		}
	}

	bool open()
{
	//errno_t r;

	if(!m_isWritable)
		m_file = fopen(m_fileName, "rb");
	else if(!m_isOverwrite)
		m_file = fopen(m_fileName, "r+b");
	else
		m_file = fopen(m_fileName, "w+b");

	return m_file; //(r==0);
}

bool clear()
{
	close();
	
	deleteFile(m_fileName);

	m_isWritable = true;
	m_isOverwrite = true;

	if(m_data)
	{
		delete[] m_data;
		m_data = 0;
	}

	return open();
}

void close()
{
	if(m_file)
	{
		fclose(m_file);
	}

	if(m_data)
	{
		delete[] m_data;
		m_data = 0;
	}
}

uint32 read(void* pData, uint32 position, uint32 length)
{
	setPosition(position);
	uint32 n = fread(pData, 1, length, m_file);
	return n;
}

uint32 read(void* pData, uint32 length)
{
	uint32 n = fread(pData, 1, length, m_file);
	return n;
}

void write(const void* pData, uint32 position, uint32 length)
{
	uint32 n = fseek (m_file, position, SEEK_SET);
	n = fwrite(pData, 1, length, m_file);
}

bool readInt(uint32* pValue, 
						 ulong64 position)
{
	fseek (m_file , position , SEEK_SET);
	size_t n = fread (pValue, 4, 1, m_file);
	return (n > 0);
}

bool readLong(ulong64* pValue, 
						  ulong64 position)
{
	fseek (m_file , position , SEEK_SET);
	size_t n = fread (pValue, 8, 1, m_file);
	return (n > 0);
}

bool readInt(uint32* pValue)
{
	size_t n = fread (pValue, 4, 1, m_file);
	return (n > 0);
}

bool readLong(ulong64* pValue)
{
	size_t n = fread (pValue, 8, 1, m_file);
	return (n > 0);
}

bool readInts(uint32* pValues, 
						  uint32 length)
{
	size_t n = fread (pValues, 4, length, m_file);
	return (n > 0);
}

bool writeInt(const uint32* pValue)
{
	if(m_data)
	{
		if((m_dataPos + 4) <= m_dataSize)
		{
			*((uint32*)(m_data + m_dataPos)) = *pValue;
		
			m_dataPos += 4;

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		fwrite (pValue, 4, 1, m_file);

		return true;
	}
}

void writeLong(const ulong64* pValue)
{
	fwrite (pValue, 8, 1, m_file);
}

void writeInt(const uint32* pValue, 
						  const ulong64 position)
{
	setPosition(position);
	fwrite (pValue, 4, 1, m_file);
}

void writeLong(const ulong64* pValue, 
						   const ulong64 position)
{
	setPosition(position);
	fwrite (pValue, 8, 1, m_file);
}

void writeChar(const uchar8* pValue)
{
	fwrite(pValue, 1, 1, m_file);
}

void writeChar(const uchar8* pValue,
						   const ulong64 position)
{
	setPosition(position);
	fwrite(pValue, 1, 1, m_file);
}


void writeInts(const uint32* pValues, 
						   const uint32 length)
{
	fwrite (pValues, 4, length, m_file);
}

void setPosition(ulong64 position)
{
	fseek (m_file, position, SEEK_SET);
}

uint32 getPosition()
{
	if(m_data)
	{
		return m_dataPos;
	}
	else
	{
		return ftell(m_file);
	}
}

const char* getFilePath()
{
	return m_fileName;
}

~BinFile()
{
	if(m_data)
	{
		delete[] m_data;
		m_data = 0;
	}
}

	bool write(const void* pData, uint32 length)
{
	if(m_data)
	{
		if((m_dataPos + length) <= m_dataSize)
		{
			for(uint32 i=0; i<length; i++)
			{
				m_data[m_dataPos++] = ((char*)pData)[i];
			}

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		fwrite(pData, 1, length, m_file);

		return true;
	}
}
	void flush()
	{
		fflush(m_file);
	}

	void allocate(uint32 size, ulong64 fromPosition)
	{
		size /= MAX_SHORT;

		uchar8* buff = new uchar8[MAX_SHORT];
		memset(buff, 0, MAX_SHORT);
		
		setPosition(fromPosition);

		for(uint32 i=0; i<size; i++)
		{
			write(buff, MAX_SHORT);
		}

		//setPosition(0);

		delete[] buff;
	}

	ulong64 getFileSize()
	{
		fseek(m_file, 0L, SEEK_END);

		ulong64 len = ftell(m_file);

		fseek(m_file, 0L, SEEK_SET);

		return len;
	}

	static bool existsFile(const char* fileName)
	{
		if (FILE * file = fopen(fileName, "r"))
		{
			fclose(file);

			return true;
		}

		return false;
	}

	static int deleteFile(const char* fileName)
	{
		return remove(fileName);
	}

	static void copyFile(BinFile* pSourceFile,
						 BinFile* pDestFile)
	{
		char buffer[4096];

		pSourceFile->setPosition(0);
		pDestFile->setPosition(0);

		uint32 size;
		while (size = pSourceFile->read(buffer, 4096)) 
		{
			pDestFile->write(buffer, size);
		}
	}
	
	static int renameFile(const char* fileName1, const char* fileName2)
	{
		return rename(fileName1, fileName2);
	}


};

