#pragma once
#ifndef _COMMON_
#define _COMMON_

#include <WinSock2.h>
#include <iostream>

#define STATIC static


inline PVOID m_malloc(SIZE_T Size)
{
	PVOID buf = malloc(Size);
	if (buf) {
		memset(buf, 0, Size);
	}

	return buf;
}

inline VOID m_free(PVOID Memory)
{
	free(Memory);
}



#endif