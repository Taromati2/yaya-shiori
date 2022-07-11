// 
// AYA version 5
//
// ファイルを扱うクラス　CFile/CFile1
// written by umeici. 2004
// 
// CFileが全体を管理します。個々のファイルはCFile1で管理します。
//

#ifndef	FILEH
#define	FILEH

//----

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
#endif

#include <stdio.h>
#include <list>

#include "globaldef.h"
#include "manifest.h"

class	CFile1
{
protected:
	aya::string_t	name;
	FILE	*fp;
	int		charset;
	aya::int_t size;
	aya::string_t	mode;

	int	bomcheck;

public:
	CFile1(const aya::string_t &n, int cs, aya::string_t md)
	{
		name     = n;
		charset  = cs;
		mode     = md;
		fp       = NULL;

		size = 0;
		bomcheck = 1;
	}

	CFile1(void) {
		charset = CHARSET_UTF8;
		fp = NULL;

		size = 0;
		bomcheck = 1;
	}
	~CFile1(void) { Close(); }

	inline const aya::string_t& GetName(void) const { return name; }
	inline bool operator==(const aya::string_t &n) const { return n == name; }

	bool Open(void);
	int	Close(void);

	inline aya::int_t Size(void) const { return size; }

	bool Write(const aya::string_t &istr);
	bool WriteBin(const aya::string_t &istr, const aya::char_t alt);
	bool WriteDecode(const aya::string_t &istr, const aya::string_t &type);

	int	Read(aya::string_t &ostr);
	int	ReadBin(aya::string_t &ostr, size_t len, aya::char_t alt);
	int	ReadEncode(aya::string_t &ostr, size_t len, const aya::string_t &type);

	bool	   FSeek(aya::int_t offset, int origin);
	aya::int_t FTell();
};

//----

class	CFile
{
protected:
	std::list<CFile1>	filelist;
	int			charset;

	bool    ProcessOpenMode(aya::string_t &mode);

public:
	CFile() { charset = CHARSET_UTF8; }
	~CFile(void) { DeleteAll(); }

	void	SetCharset(int cs) { charset = cs; }

	int		Add(const aya::string_t &name, const aya::string_t &mode);
	int		Delete(const aya::string_t &name);
	void	DeleteAll(void);

	bool	Write(const aya::string_t &name, const aya::string_t &istr);
	bool	WriteBin(const aya::string_t &name, const aya::string_t &istr, const aya::char_t alt);
	bool	WriteDecode(const aya::string_t &name, const aya::string_t &istr, const aya::string_t &type);

	int		Read(const aya::string_t &name, aya::string_t &ostr);
	int		ReadBin(const aya::string_t &name, aya::string_t &ostr, size_t len, aya::char_t alt);
	int		ReadEncode(const aya::string_t &name, aya::string_t &ostr, size_t len, const aya::string_t &type);

	aya::int_t Size(const aya::string_t &name);

	bool	   FSeek(const aya::string_t &name, aya::int_t offset, const aya::string_t &mode);
	aya::int_t FTell(const aya::string_t &name);
};

//----

#endif
