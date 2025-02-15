// 
// AYA version 5
//
// ディレクトリ内列挙　CDirEnum
// 

#ifndef	DIR_ENUM_H
#define	DIR_ENUM_H

//----

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
#endif

#include <stdio.h>
#include <list>

#include "globaldef.h"

#if defined(POSIX)
# include <dirent.h>
#endif


//----

class CDirEnumEntry
{
public:
	aya::string_t name;
	bool isdir;

	CDirEnumEntry() { isdir = false; }
};

class CDirEnum {
private:

#if defined(WIN32)
	HANDLE dh;
#elif defined(POSIX)
	DIR* dh;
#endif
	bool is_init;
	aya::string_t enumpath;

public:
	CDirEnum(const aya::string_t &enumpath);
	~CDirEnum();

	bool next(CDirEnumEntry &entry);
};

#endif // DIR_ENUM_H
