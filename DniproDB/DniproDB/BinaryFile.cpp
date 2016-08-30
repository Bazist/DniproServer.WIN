#include "StdAfx.h"
#include "BinaryFile.h"

BinaryFile::BinaryFile(const char* fileName, 
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

BinaryFile::BinaryFile(const char* fileName, 
					   bool isWritable, 
					   bool isOverwrite,
					   uint memoryBufferSize)
{
	strcpy(m_fileName, fileName);

	m_isWritable = isWritable;
	m_isOverwrite = isOverwrite;
	m_file = 0;

	m_data = new char[memoryBufferSize];
	m_dataPos = 0;
	m_dataSize = memoryBufferSize;
}

bool BinaryFile::open()
{
	errno_t r;

	if(!m_isWritable)
		r = fopen_s(&m_file, m_fileName, "rb");
	else if(!m_isOverwrite)
		r = fopen_s(&m_file, m_fileName, "r+b");
	else
		r = fopen_s(&m_file, m_fileName, "w+b");
	
	return (r==0);
}

bool BinaryFile::clear()
{
	close();
	
	BinaryFile::deleteFile(m_fileName);

	m_isWritable = true;
	m_isOverwrite = true;

	if(m_data)
	{
		delete[] m_data;
		m_data = 0;
	}

	return open();
}

void BinaryFile::close()
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

uint BinaryFile::read(void* pData, uint position, uint length)
{
	setPosition(position);
	uint n = fread(pData, 1, length, m_file);
	return n;
}

uint BinaryFile::read(void* pData, uint length)
{
	uint n = fread(pData, 1, length, m_file);
	return n;
}

void BinaryFile::write(const void* pData, uint position, uint length)
{
	uint n = fseek (m_file, position, SEEK_SET);
	n = fwrite(pData, 1, length, m_file);
}

bool BinaryFile::write(const void* pData, uint length)
{
	if(m_data)
	{
		if((m_dataPos + length) <= m_dataSize)
		{
			for(uint i=0; i<length; i++)
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

bool BinaryFile::readInt(uint* pValue, 
						 ulong position)
{
	fseek (m_file , position , SEEK_SET);
	size_t n = fread (pValue, 4, 1, m_file);
	return (n > 0);
}

bool BinaryFile::readLong(ulong* pValue, 
						  ulong position)
{
	fseek (m_file , position , SEEK_SET);
	size_t n = fread (pValue, 8, 1, m_file);
	return (n > 0);
}

bool BinaryFile::readInt(uint* pValue)
{
	size_t n = fread (pValue, 4, 1, m_file);
	return (n > 0);
}

bool BinaryFile::readLong(ulong* pValue)
{
	size_t n = fread (pValue, 8, 1, m_file);
	return (n > 0);
}

bool BinaryFile::readInts(uint* pValues, 
						  uint length)
{
	size_t n = fread (pValues, 4, length, m_file);
	return (n > 0);
}

bool BinaryFile::writeInt(const uint* pValue)
{
	if(m_data)
	{
		if((m_dataPos + 4) <= m_dataSize)
		{
			*((uint*)(m_data + m_dataPos)) = *pValue;
		
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

void BinaryFile::writeLong(const ulong* pValue)
{
	fwrite (pValue, 8, 1, m_file);
}

void BinaryFile::writeInt(const uint* pValue, 
						  const ulong position)
{
	setPosition(position);
	fwrite (pValue, 4, 1, m_file);
}

void BinaryFile::writeLong(const ulong* pValue, 
						   const ulong position)
{
	setPosition(position);
	fwrite (pValue, 8, 1, m_file);
}

void BinaryFile::writeChar(const uchar* pValue)
{
	fwrite(pValue, 1, 1, m_file);
}

void BinaryFile::writeChar(const uchar* pValue,
						   const ulong position)
{
	setPosition(position);
	fwrite(pValue, 1, 1, m_file);
}


void BinaryFile::writeInts(const uint* pValues, 
						   const uint length)
{
	fwrite (pValues, 4, length, m_file);
}

void BinaryFile::setPosition(ulong position)
{
	fseek (m_file, position, SEEK_SET);
}

uint BinaryFile::getPosition()
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

const char* BinaryFile::getFilePath()
{
	return m_fileName;
}

BinaryFile::~BinaryFile()
{
	if(m_data)
	{
		delete[] m_data;
		m_data = 0;
	}
}
