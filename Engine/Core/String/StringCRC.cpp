/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "StringCRC.h"

namespace usg {

string_crc::string_crc(const char* str)
{
	checksum = utl::CRC32(str);
}

string_crc::string_crc(U8String& str)
{
	checksum = utl::CRC32(str.CStr());
}

}


