#pragma once

#ifndef _BINARY_FILE		// Allow use of features specific to Windows XP or later.                   
#define _BINARY_FILE 0x0503	// Change this to the appropriate value to target other versions of Windows.
#endif	

class BinaryFile
{
private:
	char m_fileName[1024];

	bool m_isWritable;
	bool m_isOverwrite;

	FILE* m_file;

	//save inmemory
	char* m_data;
	uint m_dataPos;
	uint m_dataSize;

public:
	BinaryFile(const char* fileName, bool isWritable, bool isOverwrite);
	BinaryFile(const char* fileName, bool isWritable, bool isOverwrite, uint memoryBufferSize);

	bool open();
	void close();

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

	uint read(void* pData, uint position, uint length);
	uint read(void* pData, uint length);

	void write(const void* pData, uint position, const uint length);
	bool write(const void* pData, const uint length);

	bool readInt(uint* pValue, ulong position);
	bool readInt(uint* pValue);
	
	bool readLong(ulong* pValue, ulong position);
	bool readLong(ulong* pValue);

	bool readInts(uint* pValues, uint length);

	bool writeInt(const uint* pValue);
	void writeInt(const uint* pValue, const ulong position);

	void writeLong(const ulong* pValue);
	void writeLong(const ulong* pValue, const ulong position);

	void writeChar(const uchar* pValue);
	void writeChar(const uchar* pValue, const ulong position);

	void writeInts(const uint* pValues, const uint length);

	void flush()
	{
		fflush(m_file);
	}

	void allocate(uint size, ulong fromPosition)
	{
		size /= MAX_SHORT;

		uchar* buff = new uchar[MAX_SHORT];
		memset(buff, 0, MAX_SHORT);
		
		setPosition(fromPosition);

		for(uint i=0; i<size; i++)
		{
			write(buff, MAX_SHORT);
		}

		//setPosition(0);

		delete[] buff;
	}

	ulong getFileSize()
	{
		fseek(m_file, 0L, SEEK_END);

		ulong len = ftell(m_file);

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

	static void copyFile(BinaryFile* pSourceFile,
						 BinaryFile* pDestFile)
	{
		char buffer[4096];

		pSourceFile->setPosition(0);
		pDestFile->setPosition(0);

		uint size;
		while (size = pSourceFile->read(buffer, 4096)) 
		{
			pDestFile->write(buffer, size);
		}
	}
	
	static int renameFile(const char* fileName1, const char* fileName2)
	{
		return rename(fileName1, fileName2);
	}

	bool clear();
	
	void setPosition(ulong position);

	uint getPosition();

	const char* getFilePath();

	~BinaryFile();
};

