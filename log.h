﻿// 
// AYA version 5
//
// ロギング用クラス　CLog
// written by umeici. 2004
// 

#ifndef	LOGGERH
#define	LOGGERH

//----

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
#endif

#include <vector>
#include <deque>

#include "globaldef.h"
#include "manifest.h"
#include "timer.h"

#define	CLASSNAME_CHECKTOOL	"TamaWndClass"	/* チェックツールのウィンドウクラス名 */

//----

class	CLog
{
protected:
	aya::string_t		path;		// ログファイルのパス
	int			charset;	// 文字コードセット
#if defined(WIN32)
	HWND		hWnd;		// チェックツールのHWND
#endif
	void (*loghandler)(const aya::char_t *str, int mode, int id) = NULL;
	aya::timer timer;

	size_t logmaxnum;

	bool		enable;		  // ロギング有効フラグ
	bool		open;		  // ロギング開始フラグ
	bool		fileen;		  // ファイルへのロギング有効フラグ
	bool		iolog;		  // 入出力ロギング有効フラグ
	bool		locking;

	//入力でこの文字列があったらログ出力しないリスト
	std::vector<aya::string_t> iolog_filter_keyword;
	std::vector<aya::string_t> iolog_filter_keyword_regex;

	//allowlist = 1 / denylist = 0
	char iolog_filter_mode;

	volatile char skip_next_log_output;//↑の入力後に出力も抑制するためのフラグ

	std::deque<aya::string_t> error_log_history;

public:
	CLog(void)
	{
		charset = CHARSET_UTF8;
#if defined(WIN32)
		hWnd = NULL;
#endif
		enable = 1;
		open = 0;
		fileen = 1;
		iolog  = 1;
		skip_next_log_output=0;
		iolog_filter_mode = 0;
		logmaxnum = 256;
		locking = 0;
	}

#if defined(POSIX)
	typedef void* HWND;
#endif
	void	Start(const aya::string_t &p, int cs, HWND hw, char il);
	void	Termination(void);

	void	Write(const aya::string_t &str, int mode = 0, int id = 0);
	void	Write(const aya::char_t *str, int mode = 0, int id = 0);

	void	Message(int id, int mode = 0);
	void	Filename(const aya::string_t &filename);

	void	Error(int mode, int id, const aya::char_t *ref, const aya::string_t &dicfilename, ptrdiff_t linecount);
	void	Error(int mode, int id, const aya::string_t &ref, const aya::string_t &dicfilename, ptrdiff_t linecount);
	void	Error(int mode, int id, const aya::char_t *ref);
	void	Error(int mode, int id, const aya::string_t &ref);
	void	Error(int mode, int id, const aya::string_t &dicfilename, ptrdiff_t linecount);
	void	Error(int mode, int id);

	void Io(bool io, const aya::char_t *str);
	void Io(bool io, const aya::string_t &str);

	void IoLib(bool io, const aya::string_t &str, const aya::string_t &name);

	void	Call_loghandler(const aya::string_t& str, int mode, int id=0);
	void	Call_loghandler(const aya::char_t* str, int mode, int id=0);
	void	Set_loghandler(void(*loghandler_v)(const aya::char_t* str, int mode, int id));

	void	SendLogToWnd(const aya::char_t *str, int mode);

	void	AddIologFilterKeyword(const aya::string_t &ignorestr);
	void	AddIologFilterKeywordRegex(const aya::string_t &ignorestr);

	const std::vector<aya::string_t>& GetIologFilterKeyword(void) { return iolog_filter_keyword; }
	const std::vector<aya::string_t>& GetIologFilterKeywordRegex(void) { return iolog_filter_keyword_regex; }

	void	DeleteIologFilterKeyword(const aya::string_t &ignorestr);
	void	DeleteIologFilterKeywordRegex(const aya::string_t &ignorestr);

	void	ClearIologFilterKeyword();
	void	SetIologFilterMode(char mode);
	char	GetIologFilterMode(void) { return iolog_filter_mode; }

	std::deque<aya::string_t> & GetErrorLogHistory(void);
	void AppendErrorLogHistoryToBegin(std::deque<aya::string_t> &log);
	void AppendErrorLogHistoryToBegin(std::deque<aya::string_t>&&log);

	void SetMaxLogNum(size_t num);
	size_t GetMaxLogNum();

	void lock() { locking = 1; }
	void unlock() { locking = 0; }
protected:
#if defined(WIN32)
	HWND	GetCheckerWnd(void);
#endif

	void	AddErrorLogHistory(const aya::string_t &err);
};

//----

#endif
