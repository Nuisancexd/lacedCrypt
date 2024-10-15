#ifndef _PATH_SYSTEM_HPP_
#define _PATH_SYSTEM_HPP_

#include "queue.h"
#include "memory.h"
		  
namespace pathsystem
{

	DWORD WINAPI StartLocalSearch(const char* arg[]);

	typedef struct file_system_
	{
		int err;
		int dir;
		int oth;
		int fle;
	}FILE_INFO, *PFILE_INFO;
	

	STATIC PFILE_INFO fs = new FILE_INFO();
}

#endif