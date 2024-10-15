#pragma once
#include "common.h"

namespace memory 
{	
	VOID Copy(PVOID pDst, CONST PVOID pSrc, size_t size);
	PVOID m_new(size_t Size);
	VOID m_delete(PVOID Memory);
	PWCHAR m_wchar(const char* arr, size_t size);
	INT StrLen(__in LPCSTR Str);
	INT StrLen(__in LPCWSTR Str);
	void* m_memset(void* szBuffer, DWORD dwSym, DWORD dwLen);
	void* m_memcpy(void* szBuf, const void* szStr, int nLen);
	LPSTR FindChar(LPSTR Str, CHAR Ch);
	INT FindCharI(LPCSTR Str, CHAR ch);
	BOOL StrStrC(LPCSTR Str, LPCSTR StrEq);
	INT FindCharWI(LPWSTR Str, WCHAR Ch);
}
