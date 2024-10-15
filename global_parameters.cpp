#include "global_parameters.h"
#include "api.h"

STATIC INT g_EncryptMode = FULL_ENCRYPT;
STATIC INT g_Percent = 20;
STATIC BOOL g_Status = FALSE;

VOID global::SetEncryptMode(INT EncryptMode)
{
	g_EncryptMode = EncryptMode;
}

INT global::GetEncryptMode()
{
	return g_EncryptMode;
}

VOID global::SetPercent(INT Percent)
{
	g_Percent = Percent;
}

INT global::GetPercent()
{
	return g_Percent;
}

VOID global::SetStatus(BOOL Status)
{
	g_Status = Status;
}

BOOL global::GetSatus()
{
	return g_Status;
}