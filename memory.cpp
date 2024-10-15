#include "memory.h"
#include "api.h"

VOID
memory::Copy(PVOID pDst, CONST PVOID pSrc, size_t size)
{
	void* tmp = pDst;
	size_t wordsize = sizeof(size_t);
	unsigned char* _src = (unsigned char*)pSrc;
	unsigned char* _dst = (unsigned char*)pDst;
	size_t len;
	for (len = size / wordsize; len--; _src += wordsize, _dst += wordsize)
		*(size_t*)_dst = *(size_t*)_src;

	len = size % wordsize;
	while (len--)
		*_dst++ = *_src++;
}


void* memory::m_memset(void* szBuffer, DWORD dwSym, DWORD dwLen)
{
	if (!szBuffer)
	{
		return NULL;
	}

	__asm
	{
		pushad
		mov		edi, [szBuffer]
		mov		ecx, [dwLen]
		mov		eax, [dwSym]
		rep		stosb
		popad
	}

	return NULL;
}

void* memory::m_memcpy(void* szBuf, const void* szStr, int nLen)
{
	if (!szBuf || !szStr)
	{
		return NULL;
	}

	__asm
	{
		pushad
		mov		esi, [szStr]
		mov		edi, [szBuf]
		mov		ecx, [nLen]
		rep		movsb
		popad
	}

	return NULL;
}

PVOID memory::m_new(size_t Size)
{
	void* buf = new void*[Size];
	if (buf)
	{
		m_memset(buf, 0, Size);
	}
	return buf;
}

VOID memory::m_delete(PVOID Memory)
{
	delete[]Memory;
}
PWCHAR memory::m_wchar(const char* arr, size_t size)
{	
	PWCHAR ret_wch = (PWCHAR)memory::m_new(size);
	pMultiByteToWideChar(0, 0, arr, size, ret_wch, size);
	//mbstowcs(ret_wch, arr, size);
	return ret_wch;
}


INT
memory::StrLen(__in LPCSTR Str)
{
	INT Length = 0;
	while (*Str)
	{

		Length++;
		Str++;

	}

	return Length;
}


INT
memory::StrLen(__in LPCWSTR Str)
{
	INT Length = 0;
	while (*Str)
	{

		Length++;
		Str++;

	}

	return Length;
}

LPSTR memory::FindChar(LPSTR Str, CHAR Ch)
{
	while (*Str)
	{

		if (*Str == Ch) {
			return Str;
		}

		Str++;

	}

	return NULL;
}

INT memory::FindCharWI(LPWSTR Str, WCHAR Ch)
{
	INT Length = 0;
	while (*Str)
	{
		++Length;
		if (*Str == Ch)
		{
			return Length;
		}
		++Str;
	}
	return NULL;
}


BOOL memory::StrStrC(LPCSTR Str, LPCSTR StrEq)
{
	if (StrLen(Str) != StrLen(StrEq))
	{
		return FALSE;
	}
	while (*Str)
	{
		if (*Str != *StrEq)
		{
			return FALSE;
		}
		++Str;
		++StrEq;
	}

	return TRUE;
}

INT memory::FindCharI(LPCSTR Str, CHAR ch)
{
	INT i = 0;
	while (*Str)
	{
		++i;
		if (*Str == ch)
		{
			return i;
		}		
		++Str;
	}
	return NULL;
}

//template<typename Type>
//inline Type* memory::m_new(size_t Size)
//{
//	Type* buf = new Type[Size];
//	if (buf)
//	{
//		memset(buf, 0, Size);
//	}
//	return buf;
//}
//template<typename Type>
//inline VOID memory::m_delete(Type Memory)
//{
//	delete[]Memory;
//}

