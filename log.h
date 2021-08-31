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

	char		enable;		// ロギング有効フラグ
	char		open;		// ロギング開始フラグ
	char		fileen;		// ファイルへのロギング有効フラグ
	char		iolog;		// 入出力ロギング有効フラグ

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
		enable = 1;
		open = 0;
		fileen = 1;
		iolog  = 1;
		skip_next_log_output=0;
		iolog_filter_mode = 0;
	}

#if defined(POSIX)
	typedef void* HWND;
#endif
	void	Start(const aya::string_t &p, int cs, HWND hw, char il);
	void	Termination(void);

	void	Write(const aya::string_t &str, int mode = 0);
	void	Write(const aya::char_t *str, int mode = 0);

	void	Message(int id, int mode = 0);
	void	Filename(const aya::string_t &filename);

	void	Error(int mode, int id, const aya::char_t *ref, const aya::string_t &dicfilename, int linecount);
	void	Error(int mode, int id, const aya::string_t &ref, const aya::string_t &dicfilename, int linecount);
	void	Error(int mode, int id, const aya::char_t *ref);
	void	Error(int mode, int id, const aya::string_t &ref);
	void	Error(int mode, int id, const aya::string_t &dicfilename, int linecount);
	void	Error(int mode, int id);

	void	Io(char io, const aya::char_t *str);
	void	Io(char io, const aya::string_t &str);

	void	IoLib(char io, const aya::string_t &str, const aya::string_t &name);

	void	SendLogToWnd(const aya::char_t *str, int mode);
	void	SendLogToWnd(const aya::string_t &str, int mode);

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
	void SetErrorLogHistory(std::deque<aya::string_t> &log);

protected:
#if defined(WIN32)
	HWND	GetCheckerWnd(void);
#endif

	void	AddErrorLogHistory(const aya::string_t &err);
};

//----

#endif
