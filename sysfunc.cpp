﻿//
// AYA version 5
//
// システム関数を実行するクラス　CSystemFunction
// written by umeici. 2004
//

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
# include <shellapi.h>
#endif

#include <algorithm>
#include <functional>
#include <exception>
#if defined(POSIX)
# include <fstream>
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define wcsnicmp wcsncasecmp
#endif
#include <string>
#include <vector>
#include <map>
#include <ctime>

#include <cstring>
#include <stdexcept>
#include <set>
#include <unordered_set>

#include "fix_unistd.h"

#include <math.h>
#include <stdio.h>
#include <locale.h>
#if defined(POSIX)
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/errno.h>
#endif

#if defined(POSIX)
# include <memory>
#endif

#include "sysfunc.h"
#include "ayavm.h"

#include "basis.h"
#include "ccct.h"
#include "cell.h"
#include "file.h"
#include "function.h"
#include "lib.h"
#include "log.h"
#include "messages.h"
#include "misc.h"
#include "parser0.h"
#if defined(POSIX)
#include "posix_utils.h"
#endif
#include "globaldef.h"
#include "value.h"
#include "variable.h"
#include "wsex.h"
#include "dir_enum.h"

extern "C" {
#define PROTOTYPES 1
#include "md5.h"
#include "sha1.h"
#include "crc32.h"
}

//////////DEBUG/////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
////////////////////////////////////////

/* -----------------------------------------------------------------------
 *  システム関数テーブル
 * -----------------------------------------------------------------------
 */

#ifdef POSIX
#define wcsicmp wcscasecmp
#ifndef TRUE
#define TRUE	1
#endif
#endif

#define SYSFUNC_HIS_NAME L"EmBeD_HiStOrY"

constexpr CSF_FUNCTABLE CSystemFunction::sysfunc[] = {
	// 型取得/変換
	{ &CSystemFunction::TOINT , L"TOINT" } ,
	{ &CSystemFunction::TOREAL , L"TOREAL" } ,
	{ &CSystemFunction::TOSTR , L"TOSTR" } ,
	{ &CSystemFunction::GETTYPE , L"GETTYPE" } ,
	{ &CSystemFunction::ISFUNC , L"ISFUNC" } ,
	{ &CSystemFunction::ISVAR , L"ISVAR" } ,
	// デバッグ
	{ &CSystemFunction::LOGGING , L"LOGGING" } ,
	{ &CSystemFunction::GETLASTERROR , L"GETLASTERROR" } ,
	// 外部ライブラリ
	{ &CSystemFunction::LOADLIB , L"LOADLIB" } ,
	{ &CSystemFunction::UNLOADLIB , L"UNLOADLIB" } ,
	{ &CSystemFunction::REQUESTLIB , L"REQUESTLIB" } ,
	{ &CSystemFunction::CHARSETLIB , L"CHARSETLIB" } ,
	// 数値
	{ &CSystemFunction::RAND , L"RAND" } ,
	{ &CSystemFunction::FLOOR , L"FLOOR" } ,
	{ &CSystemFunction::CEIL , L"CEIL" } ,
	{ &CSystemFunction::ROUND , L"ROUND" } ,
	{ &CSystemFunction::SIN , L"SIN" } ,
	{ &CSystemFunction::COS , L"COS" } ,
	{ &CSystemFunction::TAN , L"TAN" } ,
	{ &CSystemFunction::LOG , L"LOG" } ,
	{ &CSystemFunction::LOG10 , L"LOG10" } ,
	{ &CSystemFunction::POW , L"POW" } ,
	{ &CSystemFunction::SQRT , L"SQRT" } ,
	// 文字列操作
	{ &CSystemFunction::STRSTR , L"STRSTR" } ,
	{ &CSystemFunction::STRLEN , L"STRLEN" } ,
	{ &CSystemFunction::REPLACE , L"REPLACE" } ,
	{ &CSystemFunction::SUBSTR , L"SUBSTR" } ,
	{ &CSystemFunction::ERASE , L"ERASE" } ,
	{ &CSystemFunction::INSERT , L"INSERT" } ,
	{ &CSystemFunction::TOUPPER , L"TOUPPER" } ,
	{ &CSystemFunction::TOLOWER , L"TOLOWER" } ,
	{ &CSystemFunction::CUTSPACE , L"CUTSPACE" } ,
	{ &CSystemFunction::TOBINSTR , L"TOBINSTR" } ,
	{ &CSystemFunction::TOHEXSTR , L"TOHEXSTR" } ,
	{ &CSystemFunction::BINSTRTOI , L"BINSTRTOI" } ,
	{ &CSystemFunction::HEXSTRTOI , L"HEXSTRTOI" } ,
	{ &CSystemFunction::CHR , L"CHR" } ,
	// ファイル操作
	{ &CSystemFunction::FOPEN , L"FOPEN" } ,
	{ &CSystemFunction::FCLOSE , L"FCLOSE" } ,
	{ &CSystemFunction::FREAD , L"FREAD" } ,
	{ &CSystemFunction::FWRITE , L"FWRITE" } ,
	{ &CSystemFunction::FWRITE2 , L"FWRITE2" } ,
	{ &CSystemFunction::FCOPY , L"FCOPY" } ,
	{ &CSystemFunction::FMOVE , L"FMOVE" } ,
	{ &CSystemFunction::MKDIR , L"MKDIR" } ,
	{ &CSystemFunction::RMDIR , L"RMDIR" } ,
	{ &CSystemFunction::FDEL , L"FDEL" } ,
	{ &CSystemFunction::FRENAME , L"FRENAME" } ,
	{ &CSystemFunction::FSIZE , L"FSIZE" } ,
	{ &CSystemFunction::FENUM , L"FENUM" } ,
	{ &CSystemFunction::FCHARSET , L"FCHARSET" } ,
	// 配列
	{ &CSystemFunction::ArraySize , L"ARRAYSIZE" } ,
	{ &CSystemFunction::SETDELIM , L"SETDELIM" } ,
	// 特殊
	{ &CSystemFunction::EVAL , L"EVAL" } ,
	{ &CSystemFunction::ERASEVAR , L"ERASEVAR" } ,
	// システム時刻/メモリ情報
	{ &CSystemFunction::GETTIME , L"GETTIME" } ,
	{ &CSystemFunction::GETTICKCOUNT , L"GETTICKCOUNT" } ,
	{ &CSystemFunction::GETMEMINFO , L"GETMEMINFO" } ,
	// 正規表現
	{ &CSystemFunction::RE_SEARCH , L"RE_SEARCH" } ,
	{ &CSystemFunction::RE_MATCH , L"RE_MATCH" } ,
	{ &CSystemFunction::RE_GREP , L"RE_GREP" } ,
	// システムで使用
	{ &CSystemFunction::EmBeD_HiStOrY , L"EmBeD_HiStOrY" } ,	// %[n]（置換済の値の再利用）処理用
	// デバッグ用(2)
	{ &CSystemFunction::SETLASTERROR , L"SETLASTERROR" } ,
	// 正規表現(2)
	{ &CSystemFunction::RE_REPLACE , L"RE_REPLACE" } ,
	{ &CSystemFunction::RE_SPLIT , L"RE_SPLIT" } ,
	{ &CSystemFunction::RE_GETSTR , L"RE_GETSTR" } ,
	{ &CSystemFunction::RE_GETPOS , L"RE_GETPOS" } ,
	{ &CSystemFunction::RE_GETLEN , L"RE_GETLEN" } ,
	// 文字列操作(2)
	{ &CSystemFunction::CHRCODE , L"CHRCODE" } ,
	{ &CSystemFunction::ISINTSTR , L"ISINTSTR" } ,
	{ &CSystemFunction::ISREALSTR , L"ISREALSTR" } ,
	// 配列(2)
	{ &CSystemFunction::IARRAY , L"IARRAY" } ,
	// 文字列操作(3)
	{ &CSystemFunction::SPLITPATH , L"SPLITPATH" } ,
	// 型取得/変換(2)
	{ &CSystemFunction::CVINT , L"CVINT" } ,
	{ &CSystemFunction::CVSTR , L"CVSTR" } ,
	{ &CSystemFunction::CVREAL , L"CVREAL" } ,
	// 特殊(2)
	{ &CSystemFunction::LETTONAME , L"LETTONAME" } ,
	{ &CSystemFunction::LSO , L"LSO" } ,
	// 文字列操作(4)
	{ &CSystemFunction::STRFORM , L"STRFORM" } ,
	{ &CSystemFunction::ANY , L"ANY" } ,
	// 特殊(3)
	{ &CSystemFunction::SAVEVAR , L"SAVEVAR" } ,
	// 文字列操作(5)
	{ &CSystemFunction::GETSTRBYTES , L"GETSTRBYTES" } ,
	// 配列(3)
	{ &CSystemFunction::ASEARCH , L"ASEARCH" } ,
	{ &CSystemFunction::ASEARCHEX , L"ASEARCHEX" } ,
	// 配列(4)
	{ &CSystemFunction::GETDELIM , L"GETDELIM" } ,
	// 特殊(4)
	{ &CSystemFunction::GETSETTING , L"GETSETTING" } ,
	// 数値(2)
	{ &CSystemFunction::ASIN , L"ASIN" } ,
	{ &CSystemFunction::ACOS , L"ACOS" } ,
	{ &CSystemFunction::ATAN , L"ATAN" } ,
	// 文字列操作(6)
	{ &CSystemFunction::SPLIT , L"SPLIT" } ,
	{ &CSystemFunction::ARRAYDEDUP , L"ARRAYDEDUP" },
	// ファイル操作(2)
	{ &CSystemFunction::FATTRIB , L"FATTRIB" } ,
	// 型取得/変換(3)
	{ &CSystemFunction::GETFUNCLIST , L"GETFUNCLIST" } ,
	{ &CSystemFunction::GETVARLIST , L"GETVARLIST" } ,
	// 正規表現(3)
	{ &CSystemFunction::RE_REPLACEEX , L"RE_REPLACEEX" } ,
	// 外部ライブラリ(2)
	{ &CSystemFunction::CHARSETLIBEX , L"CHARSETLIBEX" } ,
	// 文字コード
	{ &CSystemFunction::CHARSETTEXTTOID , L"CHARSETTEXTTOID" } ,
	{ &CSystemFunction::CHARSETIDTOTEXT , L"CHARSETIDTOTEXT" } ,
	// ビット演算
	{ &CSystemFunction::BITWISE_AND , L"BITWISE_AND" } ,
	{ &CSystemFunction::BITWISE_OR , L"BITWISE_OR" } ,
	{ &CSystemFunction::BITWISE_XOR , L"BITWISE_XOR" } ,
	{ &CSystemFunction::BITWISE_NOT , L"BITWISE_NOT" } ,
	{ &CSystemFunction::BITWISE_SHIFT , L"BITWISE_SHIFT" } ,
	// 半角<->全角
	{ &CSystemFunction::ZEN2HAN , L"ZEN2HAN" } ,
	{ &CSystemFunction::HAN2ZEN , L"HAN2ZEN" } ,
	// 型取得/変換(3)
	{ &CSystemFunction::CVAUTO , L"CVAUTO" } ,
	{ &CSystemFunction::TOAUTO , L"TOAUTO" } ,
	{ &CSystemFunction::CVAUTOEX , L"CVAUTOEX" } ,
	{ &CSystemFunction::TOAUTOEX , L"TOAUTOEX" } ,
	// ファイル操作(3)
	{ &CSystemFunction::FREADBIN , L"FREADBIN" } ,
	{ &CSystemFunction::FWRITEBIN , L"FWRITEBIN" } ,
	// 特殊(5)
	{ &CSystemFunction::RESTOREVAR , L"RESTOREVAR" } ,
	{ &CSystemFunction::GETCALLSTACK , L"GETCALLSTACK" } ,
	// 文字列操作(7) 互換用
	{ &CSystemFunction::STRENCODE , L"GETSTRURLENCODE" } ,
	{ &CSystemFunction::STRDECODE , L"GETSTRURLDECODE" } ,
	// 数値(3)
	{ &CSystemFunction::SINH , L"SINH" } ,
	{ &CSystemFunction::COSH , L"COSH" } ,
	{ &CSystemFunction::TANH , L"TANH" } ,
	// システム時刻/メモリ情報(2)
	{ &CSystemFunction::GETSECCOUNT , L"GETSECCOUNT" } ,
	// FMO(1)
	{ &CSystemFunction::READFMO , L"READFMO" } ,
	// ファイル操作(4)
	{ &CSystemFunction::FDIGEST , L"FDIGEST" } ,
	{ &CSystemFunction::STRDIGEST , L"STRDIGEST" } ,
	// 特殊(6)
	{ &CSystemFunction::EXECUTE , L"EXECUTE" } ,
	{ &CSystemFunction::SETSETTING , L"SETSETTING" } ,
    // �n�b�V��
    { &CSystemFunction::IHASH , L"IHASH" } ,
	{ &CSystemFunction::HASH_KEYS , L"HASH_KEYS" } ,
	{ &CSystemFunction::HASH_VALUES , L"HASH_VALUES" } ,
	{ &CSystemFunction::HASH_SPLIT , L"HASH_SPLIT" } ,
	{ &CSystemFunction::HASH_EXIST , L"HASH_EXIST" } ,
	{ &CSystemFunction::HASH_SIZE , L"HASH_SIZE" } ,
	// ファイル操作(5)
	{ &CSystemFunction::FSEEK , L"FSEEK" } ,
	{ &CSystemFunction::FTELL , L"FTELL" } ,
	// 文字列操作(8)
	{ &CSystemFunction::STRENCODE , L"STRENCODE" } ,
	{ &CSystemFunction::STRDECODE , L"STRDECODE" } ,
	// 特殊(7)
	{ &CSystemFunction::EXECUTE_WAIT , L"EXECUTE_WAIT" } ,
	// 正規表現(4)
	{ &CSystemFunction::RE_OPTION , L"RE_OPTION" } ,
	// ファイル操作(6)
	{ &CSystemFunction::FREADENCODE , L"FREADENCODE" } ,
	// 型取得/変換(4)
	{ &CSystemFunction::GETTYPEEX , L"GETTYPEEX" } ,
	// 正規表現(5)
	{ &CSystemFunction::RE_ASEARCHEX , L"RE_ASEARCHEX" } ,
	{ &CSystemFunction::RE_ASEARCH , L"RE_ASEARCH" } ,
	// 配列(5)
	{ &CSystemFunction::ASORT , L"ASORT" } ,
	// 文字列操作(9)
	{ &CSystemFunction::TRANSLATE , L"TRANSLATE" } ,
	// 数値(4)
	{ &CSystemFunction::SRAND , L"SRAND" } ,
	// 特殊(8)
	{ &CSystemFunction::GETENV , L"GETENV" } ,
	// ファイル操作(7)
	{ &CSystemFunction::FWRITEDECODE , L"FWRITEDECODE" } ,
	// デバッグ用(4)
	{ &CSystemFunction::GETERRORLOG , L"GETERRORLOG" } ,
	{ &CSystemFunction::CLEARERRORLOG , L"CLEARERRORLOG" },
	// 特殊(9)
	{ &CSystemFunction::DICLOAD , L"DICLOAD" } ,
	{ &CSystemFunction::GETSYSTEMFUNCLIST , L"GETSYSTEMFUNCLIST" } ,
	{ &CSystemFunction::GETFUNCINFO , L"GETFUNCINFO" } ,
	{ &CSystemFunction::PROCESSGLOBALDEFINE , L"PROCESSGLOBALDEFINE" } ,
	{ &CSystemFunction::UNDEFFUNC , L"UNDEFFUNC" } ,
	{ &CSystemFunction::UNDEFGLOBALDEFINE , L"UNDEFGLOBALDEFINE" } ,
	{ &CSystemFunction::DICUNLOAD , L"DICUNLOAD" } ,
	{ &CSystemFunction::ISEVALUABLE , L"ISEVALUABLE" } ,
	{ &CSystemFunction::SETTAMAHWND , L"SETTAMAHWND" } ,
	{ &CSystemFunction::ISGLOBALDEFINE , L"ISGLOBALDEFINE" } ,
	{ &CSystemFunction::SETGLOBALDEFINE , L"SETGLOBALDEFINE" } ,
	{ &CSystemFunction::APPEND_RUNTIME_DIC , L"APPEND_RUNTIME_DIC" } ,
	{ &CSystemFunction::SLEEP , L"SLEEP" } ,
	{ &CSystemFunction::FUNCDECL_READ , L"FUNCDECL_READ" },
	{ &CSystemFunction::FUNCDECL_WRITE , L"FUNCDECL_WRITE" },
	{ &CSystemFunction::FUNCDECL_ERASE , L"FUNCDECL_ERASE" },
	{ &CSystemFunction::OUTPUTNUM , L"OUTPUTNUM" },
	//LINT
	{ &CSystemFunction::LINT_GetFuncUsedBy , L"LINT.GetFuncUsedBy" },
	{ &CSystemFunction::LINT_GetUserDefFuncUsedBy , L"LINT.GetUserDefFuncUsedBy" },
	{ &CSystemFunction::LINT_GetGlobalVarUsedBy , L"LINT.GetGlobalVarUsedBy" },
	{ &CSystemFunction::LINT_GetLocalVarUsedBy , L"LINT.GetLocalVarUsedBy" },
	{ &CSystemFunction::LINT_GetGlobalVarLetted , L"LINT.GetGlobalVarLetted" },
	{ &CSystemFunction::LINT_GetLocalVarLetted , L"LINT.GetLocalVarLetted" },
};

#define SYSFUNC_NUM (sizeof(CSystemFunction::sysfunc)/sizeof(CSystemFunction::sysfunc[0]))

//このグローバル変数はマルチインスタンスでも共通
class CSystemFunctionInit {
public:
	size_t sysfunc_len[SYSFUNC_NUM];
	size_t sysfunc_len_max;
	size_t sysfunc_len_min;
	size_t sysfunc_his_pos;
	aya::indexmap sysfunc_map;

	CSystemFunctionInit(void) {
		sysfunc_len_max = 0;
		sysfunc_len_min = 65536;
		sysfunc_his_pos = 0;

		for(size_t i = 0; i < SYSFUNC_NUM; i++) {
			if ( ::wcscmp(CSystemFunction::sysfunc[i].name,SYSFUNC_HIS_NAME) == 0 ) {
				sysfunc_his_pos = i;
			}
			sysfunc_len[i] = ::wcslen(CSystemFunction::sysfunc[i].name);
			sysfunc_map.insert(aya::indexmap::value_type(CSystemFunction::sysfunc[i].name,i));

			if ( sysfunc_len_max < sysfunc_len[i] ) {
				sysfunc_len_max = sysfunc_len[i];
			}
			if ( sysfunc_len_min > sysfunc_len[i] ) {
				sysfunc_len_min = sysfunc_len[i];
			}
		}
	}
};

CSystemFunctionInit sysfuncinit;

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CSystemFunction
 * -----------------------------------------------------------------------
 */
CSystemFunction::CSystemFunction(CAyaVM &vmr)
	: vm(vmr), re_str(F_TAG_ARRAY, 0/*dmy*/), re_pos(F_TAG_ARRAY, 0/*dmy*/), re_len(F_TAG_ARRAY, 0/*dmy*/)
{
	lasterror   = 0;
	lso         = -1;

	re_option   = 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GetMaxNameLength
 *  機能概要：  システム関数の名前の最大値を返します
 * -----------------------------------------------------------------------
 */
size_t CSystemFunction::GetMaxNameLength(void)
{
	return sysfuncinit.sysfunc_len_max;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FindIndex
 *  機能概要：  システム関数を探索します
 * -----------------------------------------------------------------------
 */
ptrdiff_t CSystemFunction::FindIndex(const aya::string_t &str)
{
	if ( str.size() == 0 ) { return -1; }

	aya::indexmap::const_iterator it = sysfuncinit.sysfunc_map.find(str);
	if ( it == sysfuncinit.sysfunc_map.end() ) { return -1; }

	return it->second;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FindIndexLongestMatch
 *  機能概要：  いちばん長くマッチするシステム関数を探索します
 * -----------------------------------------------------------------------
 */
size_t CSystemFunction::FindIndexLongestMatch(const aya::string_t &str, size_t max_len)
{
	size_t found_len = 0;
	for(size_t i = 0; i < SYSFUNC_NUM; i++) {
		if ( sysfuncinit.sysfunc_len[i] <= max_len ) { continue; }

		if ( wcsncmp(str.c_str(),sysfunc[i].name,sysfuncinit.sysfunc_len[i]) == 0 ) {
			found_len = sysfuncinit.sysfunc_len[i];
			max_len = found_len;
		}
	}
	return found_len;
}


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GetNameFromIndex
 *  機能概要：  Index->名前
 * -----------------------------------------------------------------------
 */
const aya::char_t* CSystemFunction::GetNameFromIndex(int idx)
{
	if ( idx < 0 || idx >= SYSFUNC_NUM ) { return L""; }
	return CSystemFunction::sysfunc[idx].name;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::HistoryIndex / HistoryFunctionName
 *  機能概要：  履歴系の定数を返します
 * -----------------------------------------------------------------------
 */
size_t CSystemFunction::HistoryIndex(void)
{
	return sysfuncinit.sysfunc_his_pos;
}

const aya::char_t* CSystemFunction::HistoryFunctionName(void)
{
	return CSystemFunction::sysfunc[sysfuncinit.sysfunc_his_pos].name;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::Execute
 *  機能概要：  システム関数を実行します
 *
 *  thisfuncはこの関数を実行したCFunctionインスタンスです
 * -----------------------------------------------------------------------
 */
class CSF_FUNCPARAM
{
public:
	CSF_FUNCPARAM(ptrdiff_t pindex,const CValue &parg,const std::vector<CCell *> &ppcellarg,CValueArgArray &pvaluearg,CLocalVariable &plvar,int pline,CFunction *pthisfunc,const aya::string_t &pdicname) :
		index(pindex) , arg(parg) , pcellarg(ppcellarg), valuearg(pvaluearg), lvar(plvar), line(pline), thisfunc(pthisfunc), dicname(pdicname) {
	}

	ptrdiff_t index;
	const CValue &arg;
	const std::vector<CCell *> &pcellarg;
	CValueArgArray &valuearg;
	CLocalVariable &lvar;
	const int line;
	CFunction *thisfunc;
	const aya::string_t &dicname;
};

CValue	CSystemFunction::Execute(ptrdiff_t index, const CValue &arg, const std::vector<CCell *> &pcellarg,
			CValueArgArray &valuearg, CLocalVariable &lvar, int line, CFunction *thisfunc)
{
	CSF_FUNCPARAM p(index,arg,pcellarg,valuearg,lvar,line,thisfunc,thisfunc->dicfilename);

	if ( index >= 0 && index < SYSFUNC_NUM ) {
		return (this->*sysfunc[index].func)(p);
	}
	else {
		vm.logger().Error(E_E, 49, p.dicname, p.line);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}
}


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOINT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOINT(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"TOINT", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	return CValue(p.arg.array()[0].GetValueInt());
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOREAL
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOREAL(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"TOREAL", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	return CValue(p.arg.array()[0].GetValueDouble());
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOSTR(CSF_FUNCPARAM &p)
{
	if (p.valuearg.empty()) {
		vm.logger().Error(E_W, 8, L"TOSTR", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	aya::string_t str;
	for ( size_t i = 0; i < p.valuearg.size(); i++ ) {
		if(p.pcellarg[i]->value_GetType() == F_TAG_VARIABLE) {
			auto &var = vm.variable().GetValue(p.pcellarg[i]->index);
			if(var.GetType() == F_TAG_ARRAY) {
				aya::string_t delimiter = vm.variable().GetDelimiter(p.pcellarg[i]->index);
				if(delimiter.empty()) {
					delimiter = VAR_DELIMITER;
				}
				for(CValueArray::const_iterator it =var.array().begin(); it != var.array().end(); it++) {
					if(it != var.array().begin())
						str += delimiter;
					str += it->GetValueString();
				}
			}
			else
				str += var.GetValueString();
		}
		else if(p.pcellarg[i]->value_GetType() == F_TAG_LOCALVARIABLE) {
			auto &var = p.lvar.GetValue(p.pcellarg[i]->name);
			if(var.GetType() == F_TAG_ARRAY) {
				aya::string_t delimiter = p.lvar.GetDelimiter(p.pcellarg[i]->name);
				if(delimiter.empty()) {
					delimiter = VAR_DELIMITER;
				}
				for(CValueArray::const_iterator it =var.array().begin(); it != var.array().end(); it++) {
					if(it != var.array().begin())
						str += delimiter;
					str += it->GetValueString();
				}
			}
			else
				str += var.GetValueString();
		}	
		else
			str += p.valuearg[i].GetValueString();
	}
	return CValue(str);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOAUTO
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOAUTO(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"TOAUTO", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString()) {
		return CValue(p.arg.array()[0]);
	}

	aya::string_t str = p.arg.array()[0].GetValueString();

	if ( IsIntString(str) ) {
		return CValue(p.arg.array()[0].GetValueInt());
	}
	else if ( IsDoubleButNotIntString(str) ) {
		return CValue(p.arg.array()[0].GetValueDouble());
	}
	else {
		return CValue(str);
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOAUTOEX
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOAUTOEX(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"TOAUTOEX", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString()) {
		return CValue(p.arg.array()[0]);
	}

	aya::string_t str = p.arg.array()[0].GetValueString();
	CValue result;
	
	if ( IsIntString(str) ) {
		result = p.arg.array()[0].GetValueInt();
	}
	else if ( IsDoubleButNotIntString(str) ) {
		result = p.arg.array()[0].GetValueDouble();
	}
	else {
		return CValue(str);
	}

	if(str==result.GetValueString()) {
		return CValue(result);
	}
	else {
		return CValue(str);
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETTYPE
 *  返値　　：  0/1/2/3/4/5=エラー/整数/実数/文字列/配列/連想配列
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETTYPE(CSF_FUNCPARAM &p)
{
	if (p.valuearg.empty()) {
		vm.logger().Error(E_W, 8, L"GETTYPE", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	switch(p.valuearg[0].GetType()) {
	case F_TAG_VOID:
		return CValue(0);
	case F_TAG_INT:
		return CValue(1);
	case F_TAG_DOUBLE:
		return CValue(2);
	case F_TAG_STRING:
		return CValue(3);
	case F_TAG_ARRAY: //互換処理
		{
			if ( p.valuearg[0].array_size() == 1 ) {
				int t = p.valuearg[0].array()[0].GetType();
				if ( t == F_TAG_INT ) {
					return CValue(1);
				}
				else if ( t == F_TAG_DOUBLE ) {
					return CValue(2);
				}
				else if ( t == F_TAG_STRING ) {
					return CValue(3);
				}
				else {
					return CValue(0);
				}
			}
			else {
				return CValue(4);
			}
		}
	case F_TAG_HASH:
		return CValue(5);
	default:
		vm.logger().Error(E_E, 88, L"GETTYPE", p.dicname, p.line);
		return CValue(0);
	};
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETTYPEEX
 *  返値　　：  0/1/2/3/4=エラー/整数/実数/文字列/配列
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETTYPEEX(CSF_FUNCPARAM &p)
{
	size_t arg_size = p.arg.array_size();

	if (!arg_size) {
		vm.logger().Error(E_W, 8, L"GETTYPEEX", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	//文字列かどうかチェック - 警告は吐くが処理続行
	if ( ! p.arg.array()[0].IsString() ) {
		vm.logger().Error(E_W, 9, L"GETTYPEEX", p.dicname, p.line);
		SetError(9);
	}

	const aya::string_t &arg0 = p.arg.array()[0].GetValueString();
	if (!arg0.size()) {
		return CValue(0);
	}

	int type = 0;
	if (arg0[0] == L'_') {
		const CValue *v = p.lvar.GetValuePtr(arg0);
		if ( v ) {
			type = v->GetType();
		}
	}
	else {
		int gidx = vm.variable().GetIndex(arg0);
		if ( gidx >= 0 ) {
			const CValue &v = vm.variable().GetValue(gidx);
			type = v.GetType();
		}
	}

	switch(type) {
	case F_TAG_INT:
		return CValue(1);
	case F_TAG_DOUBLE:
		return CValue(2);
	case F_TAG_STRING:
		return CValue(3);
	case F_TAG_ARRAY:
		return CValue(4);
	case F_TAG_HASH:
		return CValue(5);
	default:
		return CValue(0);
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ISFUNC
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ISFUNC(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"ISFUNC", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"ISFUNC", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	ptrdiff_t i = vm.function_exec().GetFunctionIndexFromName(p.arg.array()[0].GetValueString());
	if(i != -1)
		return CValue(1);

	if ( FindIndex(p.arg.array()[0].GetValueString()) >= 0 ) {
		return CValue(2);
	}

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ISVAR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ISVAR(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"ISVAR", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"ISVAR", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	int	index = vm.variable().GetIndex(p.arg.array()[0].GetValueString());
	if (index >= 0)
		return CValue(1);

	int	depth = -1;
	p.lvar.GetIndex(p.arg.array()[0].GetValueString(), index, depth);
	if (index >= 0)
		return CValue(2);

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LOGGING
 *
 *  文字列の場合はダブルクォートします。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LOGGING(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size())
		vm.logger().Write(p.arg.GetValueStringForLogging());

	vm.logger().Write(L"\n");

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETERRORLOG
 *
 *  エラーログを配列で返します
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETERRORLOG(CSF_FUNCPARAM &p)
{

	CValue result(F_TAG_ARRAY, 0/*dmy*/);

	//絞りこみ文字列がない場合
	std::deque<aya::string_t> &log = vm.logger().GetErrorLogHistory();

	for(std::deque<aya::string_t>::iterator it = log.begin(); it != log.end(); it++) {
		result.array().emplace_back(*it);
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETERRORLOG
 *
 *  エラーログを配列で返します
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CLEARERRORLOG(CSF_FUNCPARAM &p)
{

	CValue result(F_TAG_ARRAY, 0/*dmy*/);

	//絞りこみ文字列がない場合
	vm.logger().GetErrorLogHistory().clear();

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LOADLIB
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LOADLIB(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"LOADLIB", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"LOADLIB", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	int excode = vm.libs().Add(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));
	if (!excode) {
		vm.logger().Error(E_W, 13, L"LOADLIB", p.dicname, p.line);
		SetError(13);
	}

	return CValue(excode);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::UNLOADLIB
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::UNLOADLIB(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"UNLOADLIB", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"UNLOADLIB", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	int result = vm.libs().Delete(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));

	if (!result) {
		vm.logger().Error(E_W, 13, L"UNLOADLIB", p.dicname, p.line);
		SetError(13);
	}
	else if (result == 2) {
		vm.logger().Error(E_W, 14, p.dicname, p.line);
		SetError(14);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::REQUESTLIB
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::REQUESTLIB(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"REQUESTLIB", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"REQUESTLIB", p.dicname, p.line);
		SetError(9);
		return CValue();
	}

	aya::string_t	result;
	if(!vm.libs().Request(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), p.arg.array()[1].GetValueString(), result)) {
		vm.logger().Error(E_W, 13, L"REQUESTLIB", p.dicname, p.line);
		SetError(13);
	}

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CHARSETTEXTTOID
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CHARSETTEXTTOID(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"CHARSETTEXTTOID", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"CHARSETTEXTTOID", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	return CValue(Ccct::CharsetTextToID(p.arg.array()[0].GetValueString().c_str()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CHARSETIDTOTEXT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CHARSETIDTOTEXT(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"CHARSETIDTOTEXT", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"CHARSETIDTOTEXT", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	return CValue(Ccct::CharsetIDToTextW((const int)p.arg.array()[0].GetValueInt()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::BITWISE_***
 *  ビット演算関連です
 * -----------------------------------------------------------------------
 */
CValue CSystemFunction::BITWISE_AND(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"BITWISE_AND", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	return CValue(p.arg.array()[0].GetValueInt() & p.arg.array()[1].GetValueInt());
}

CValue CSystemFunction::BITWISE_OR(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"BITWISE_OR", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	return CValue(p.arg.array()[0].GetValueInt() | p.arg.array()[1].GetValueInt());
}

CValue CSystemFunction::BITWISE_XOR(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"BITWISE_XOR", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	return CValue(p.arg.array()[0].GetValueInt() ^ p.arg.array()[1].GetValueInt());
}

CValue CSystemFunction::BITWISE_NOT(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"BITWISE_NOT", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	return CValue(~p.arg.array()[0].GetValueInt());
}

CValue CSystemFunction::BITWISE_SHIFT(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"BITWISE_SHIFT", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	aya::int_t shiftValue = p.arg.array()[1].GetValueInt();
	if ( shiftValue > 0 ) {
		return CValue(p.arg.array()[0].GetValueInt() << shiftValue );
	}
	else {
		return CValue(p.arg.array()[0].GetValueInt() >> abs(shiftValue) );
	}
}
/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ZEN2HAN / HAN2ZEN
 *
 *  半角<->全角
 * -----------------------------------------------------------------------
 */

static const aya::char_t zen_support_symbol[] =
	L"　！“”＃＄％＆‘’（）＝｜‘｛＋＊｝＜＞？＿ー＾￥＠；：・．［］";
static const aya::char_t han_support_symbol[] =
	L" !\"\"#$%&''()=|`{+*}<>?_-^\\@;:･.[]";

static const aya::char_t zen_support_kana[] =
	L"アイウエオカキクケコサシスセ\x30bdタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲンァィゥェォャュョッ゛゜、。";
static const aya::char_t han_support_kana[] =
	L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝｧｨｩｪｫｬｭｮｯﾞﾟ､｡";

static const aya::char_t zen_support_kana2[] =
	L"ガギグゲゴザジズゼゾダヂヅデドバビブベボヴ";
static const aya::char_t han_support_kana2[] =
	L"ｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾊﾋﾌﾍﾎｳ";

static const aya::char_t zen_support_kana3[] =
	L"パピプペポ";
static const aya::char_t han_support_kana3[] =
	L"ﾊﾋﾌﾍﾎ";


#define ZH_FLAG_NUMBER   0x1U
#define ZH_FLAG_ALPHABET 0x2U
#define ZH_FLAG_SYMBOL   0x4U
#define ZH_FLAG_KANA     0x8U

static unsigned int CSystemFunction_ZHFlag(const aya::string_t &str);

static unsigned int CSystemFunction_ZHFlag(const aya::string_t &str)
{
	unsigned int flag = 0;

	if ( str.find(L"num") != aya::string_t::npos ) {
		flag |= ZH_FLAG_NUMBER;
	}
	if ( str.find(L"alpha") != aya::string_t::npos ) {
		flag |= ZH_FLAG_ALPHABET;
	}
	if ( str.find(L"sym") != aya::string_t::npos ) {
		flag |= ZH_FLAG_SYMBOL;
	}
	if ( str.find(L"kana") != aya::string_t::npos ) {
		flag |= ZH_FLAG_KANA;
	}
	return flag;
}

static const aya::char_t char_zen_0 = 0xff10;
static const aya::char_t char_zen_9 = 0xff19;

static const aya::char_t char_zen_upper_a = 0xff21;
static const aya::char_t char_zen_upper_z = 0xff3a;

static const aya::char_t char_zen_lower_a = 0xff41;
static const aya::char_t char_zen_lower_z = 0xff5a;

static const aya::char_t char_dakuten = 0xff9e;
static const aya::char_t char_handakuten = 0xff9f;

CValue CSystemFunction::ZEN2HAN(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"ZEN2HAN", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	aya::string_t str = p.arg.array()[0].GetValueString();

	unsigned int flag = 0xFFFFFFFFU;
	if ( p.arg.array_size() >= 2 ) {
		flag = CSystemFunction_ZHFlag(p.arg.array()[1].GetValueString());
	}

	for ( aya::string_t::iterator it = str.begin() ; it < str.end(); ++it ) {
		if ( *it >= char_zen_0 && *it <= char_zen_9 ) {
			if ( flag & ZH_FLAG_NUMBER ) {
				*it = *it - char_zen_0 + L'0';
			}
		}
		else if ( *it >= char_zen_upper_a && *it <= char_zen_upper_z ) {
			if ( flag & ZH_FLAG_ALPHABET ) {
				*it = *it - char_zen_upper_a + L'A';
			}
		}
		else if ( *it >= char_zen_lower_a && *it <= char_zen_lower_z ) {
			if ( flag & ZH_FLAG_ALPHABET ) {
				*it = *it - char_zen_lower_a + L'a';
			}
		}
		else {
			if ( flag & ZH_FLAG_SYMBOL ) {
				const aya::char_t *found = wcschr(zen_support_symbol,*it);
				if ( found ) {
					*it = han_support_symbol[found - zen_support_symbol];
				}
			}
			if ( flag & ZH_FLAG_KANA ) {
				const aya::char_t *found = wcschr(zen_support_kana,*it);
				if ( found ) {
					*it = han_support_kana[found - zen_support_kana];
				}

				found = wcschr(zen_support_kana2,*it);
				if ( found ) {
					*it = han_support_kana2[found - zen_support_kana2];
					it = str.insert(it+1,char_dakuten);
				}

				found = wcschr(zen_support_kana3,*it);
				if ( found ) {
					*it = han_support_kana3[found - zen_support_kana3];
					it = str.insert(it+1,char_handakuten);
				}
			}
		}
	}

	return CValue(str);
}

CValue CSystemFunction::HAN2ZEN(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"HAN2ZEN", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	aya::string_t str = p.arg.array()[0].GetValueString();

	unsigned int flag = 0xFFFFFFFFU;
	if ( p.arg.array_size() >= 2 ) {
		flag = CSystemFunction_ZHFlag(p.arg.array()[1].GetValueString());
	}

	for ( aya::string_t::iterator it = str.begin() ; it < str.end(); ++it ) {
		if ( *it >= L'0' && *it <= L'9' ) {
			if ( flag & ZH_FLAG_NUMBER ) {
				*it = *it - L'0' + char_zen_0;
			}
		}
		else if ( *it >= L'A' && *it <= L'Z' ) {
			if ( flag & ZH_FLAG_ALPHABET ) {
				*it = *it - L'A' + char_zen_upper_a;
			}
		}
		else if ( *it >= L'a' && *it <= L'z' ) {
			if ( flag & ZH_FLAG_ALPHABET ) {
				*it = *it - L'a' + char_zen_lower_a;
			}
		}
		else {
			if ( flag & ZH_FLAG_SYMBOL ) {
				const aya::char_t *found = wcschr(han_support_symbol,*it);
				if ( found ) {
					*it = zen_support_symbol[found - han_support_symbol];
				}
			}
			if ( flag & ZH_FLAG_KANA ) {
				const aya::char_t *found = wcschr(han_support_kana,*it);
				if ( found ) {
					if ( it < str.end()-1 && ((*(it+1) == char_dakuten) || (*(it+1) == char_handakuten)) ) {
						if ( *(it+1) == char_dakuten ) {
							found = wcschr(han_support_kana2,*it);
							if ( found ) {
								*it = zen_support_kana2[found - han_support_kana2];
								it = str.erase(it+1) - 1;
							}
						}
						else /*if char_handakuten*/ {
							found = wcschr(han_support_kana3,*it);
							if ( found ) {
								*it = zen_support_kana3[found - han_support_kana3];
								it = str.erase(it+1) - 1;
							}
						}
					}
					else {
						*it = zen_support_kana[found - han_support_kana];
					}
				}
			}
		}
	}
	return CValue(str);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CHARSETLIB
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CHARSETLIB(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		return CValue(Ccct::CharsetIDToTextW(vm.libs().GetCharset()));
	}
	else {
		int	charset = GetCharset(p.arg.array()[0],L"CHARSETLIB",p.dicname,p.line);
		if ( charset < 0 ) {
			return CValue();
		}

		vm.libs().SetCharset(charset);
		return CValue();
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CHARSETLIBEX
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CHARSETLIBEX(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 1) {
		vm.logger().Error(E_W, 8, L"CHARSETLIBEX", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"CHARSETLIBEX", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if ( p.arg.array_size() >= 2 ) {
		int	charset = GetCharset(p.arg.array()[1],L"CHARSETLIBEX", p.dicname, p.line);
		if ( charset < 0 ) {
			return CValue(0);
		}
		int result = vm.libs().SetCharsetDynamic(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), charset);

		if (!result) {
			vm.logger().Error(E_W, 13, L"CHARSETLIBEX", p.dicname, p.line);
			SetError(13);
		}

		return CValue(result);
	}
	else {
		int result = vm.libs().GetCharsetDynamic(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));
		if ( result < 0 ) {
			return CValue();
		}
		return CValue(Ccct::CharsetIDToTextW(result));
	}
}


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RAND
 *
 *  引数なしはRAND(100)扱いです。
 *  引数エラーでは0を返します。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RAND(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size())
		return vm.genrand_sysfunc_ll(100);

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"RAND", p.dicname, p.line);
		SetError(9);
	}

	aya::int_t num = p.arg.array()[0].GetValueInt();

	if ( num == 0 ) {
		vm.logger().Error(E_W, 19, L"RAND", p.dicname, p.line);
		SetError(19);
		return CValue(0);
	}

	return vm.genrand_sysfunc_ll(num);
}


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SRAND
 *
 *  RANDのseed。パラメータは文字列も可。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SRAND(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 19, L"SRAND", p.dicname, p.line);
		SetError(19);
		return CValue(0);
	}

	if (p.arg.array()[0].IsInt()) {
		aya::int_t num = p.arg.array()[0].GetValueInt();
		vm.genrand_sysfunc_srand_ll(num);
	}
	else if (p.arg.array()[0].IsDouble()) {
		union {
			double d;
			std::uint64_t i[1];
		} num{};

		num.d = p.arg.array()[0].GetValueDouble();

		vm.genrand_sysfunc_srand_array(num.i,1);
	}
	else if (p.arg.array()[0].IsString()) {
		std::vector<std::uint64_t> num;

		aya::string_t str = p.arg.array()[0].GetValueString();

		int nlen = str.length();
		int n = nlen / 2;

		for ( int i = 0 ; i < n ; ++i ) {
			num.emplace_back( str[i]);
		}

		vm.genrand_sysfunc_srand_array(&(num[0]),num.size());
	}
	else {
		vm.logger().Error(E_W, 9, L"SRAND", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	return CValue(1);
}


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FLOOR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FLOOR(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"FLOOR", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"FLOOR", p.dicname, p.line);
		SetError(9);
	}
	return CValue(floor(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CEIL
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CEIL(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"CEIL", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"CEIL", p.dicname, p.line);
		SetError(9);
	}
	return CValue(ceil(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ROUND
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ROUND(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"ROUND", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (p.arg.array()[0].IsInt()) {
		return CValue((double)p.arg.array()[0].GetValueInt());
	}
	else {
		if (!p.arg.array()[0].IsDouble()) {
			vm.logger().Error(E_W, 9, L"ROUND", p.dicname, p.line);
			SetError(9);
		}
		double	value = p.arg.array()[0].GetValueDouble();
		if (value - floor(value) >= 0.5)
			return CValue(ceil(value));
		else
			return CValue(floor(value));
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SIN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SIN(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"SIN", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"SIN", p.dicname, p.line);
		SetError(9);
	}
	return CValue(sin(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::COS
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::COS(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"COS", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"COS", p.dicname, p.line);
		SetError(9);
	}
	return CValue(cos(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TAN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TAN(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"TAN", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"TAN", p.dicname, p.line);
		SetError(9);
	}
	return CValue(tan(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SINH
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SINH(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"SINH", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"SINH", p.dicname, p.line);
		SetError(9);
	}
	return CValue(sinh(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::COSH
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::COSH(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"COSH", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"COSH", p.dicname, p.line);
		SetError(9);
	}
	return CValue(cosh(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TANH
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TANH(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"TANH", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"TANH", p.dicname, p.line);
		SetError(9);
	}
	return CValue(tanh(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ASIN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ASIN(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"ASIN", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"ASIN", p.dicname, p.line);
		SetError(9);
	}
	return CValue(asin(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ACOS
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ACOS(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"ACOS", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"ACOS", p.dicname, p.line);
		SetError(9);
	}
	return CValue(acos(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ATAN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ATAN(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"ATAN", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"ATAN", p.dicname, p.line);
		SetError(9);
	}
	return CValue(atan(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LOG
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LOG(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"LOG", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"LOG", p.dicname, p.line);
		SetError(9);
	}
	return CValue(log(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LOG10
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LOG10(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"LOG10", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"LOG10", p.dicname, p.line);
		SetError(9);
	}
	return CValue(log10(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::POW
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::POW(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"POW", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum() || !p.arg.array()[1].IsNum()) {
		vm.logger().Error(E_W, 9, L"POW", p.dicname, p.line);
		SetError(9);
	}
	return CValue(pow(p.arg.array()[0].GetValueDouble(), p.arg.array()[1].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SQRT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SQRT(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"SQRT", p.dicname, p.line);
		SetError(8);
		return CValue(0.0);
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"SQRT", p.dicname, p.line);
		SetError(9);
	}

	double	value = p.arg.array()[0].GetValueDouble();

	if (value < 0.0)
		return CValue(-1);

	return CValue(sqrt(p.arg.array()[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::STRSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::STRSTR(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 3) {
		vm.logger().Error(E_W, 8, L"STRSTR", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[2].IsNum()) {
		vm.logger().Error(E_W, 9, L"STRSTR", p.dicname, p.line);
		SetError(9);
	}

	aya::string_t &str = p.arg.array()[0].GetValueString();
	aya::string_t &target = p.arg.array()[1].GetValueString();
	aya::int_t start_tmp = p.arg.array()[2].GetValueInt();
	aya::string_t::size_type start = start_tmp < 0 ? 0 : static_cast<aya::string_t::size_type>(start_tmp);

	aya::string_t::size_type found = str.find(target,start);

	return CValue(found == aya::string_t::npos ? -1 : static_cast<aya::int_t>(found) );
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::STRLEN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::STRLEN(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"STRLEN", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"STRLEN", p.dicname, p.line);
		SetError(9);
	}

	return CValue((aya::int_t)p.arg.array()[0].GetValueString().size());
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::REPLACE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::REPLACE(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 3) {
		vm.logger().Error(E_W, 8, L"REPLACE", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString() ||
		!p.arg.array()[2].IsString()) {
		vm.logger().Error(E_W, 9, L"REPLACE", p.dicname, p.line);
		SetError(9);
	}

	aya::int_t count = 0;
	if ( p.arg.array_size() >= 4 ) {
		if (!p.arg.array()[3].IsInt()) {
			vm.logger().Error(E_W, 9, L"REPLACE", p.dicname, p.line);
			SetError(9);
		}
		count = p.arg.array()[3].GetValueInt();
	}

	aya::string_t	result = p.arg.array()[0].GetValueString();
	aya::string_t  before = p.arg.array()[1].GetValueString();
	aya::string_t  after  = p.arg.array()[2].GetValueString();
	//int	sz_before = before->size();
	//int	sz_after  = after->size();

	if (!before.empty()) {
		aya::ws_replace(result, before.c_str(), after.c_str(), count);
	}

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SUBSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SUBSTR(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 3) {
		vm.logger().Error(E_W, 8, L"SUBSTR", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[1].IsNum() ||
		!p.arg.array()[2].IsNum()) {
		vm.logger().Error(E_W, 9, L"SUBSTR", p.dicname, p.line);
		SetError(9);
	}

	const aya::string_t& src = p.arg.array()[0].GetValueString();
	aya::int_t pos = p.arg.array()[1].GetValueInt();
	aya::int_t len = p.arg.array()[2].GetValueInt();

	if ( pos < 0 ) {
		pos += src.length();
		if ( pos < 0 ) { //まだ負なら強制補正
			len += pos; //負値なのでたしざんで引かれる
			pos = 0;
			if ( len <= 0 ) {
				return aya::string_t();
			}
		}
	}

	if ( pos >= src.length() || len <= 0 ) {
		return aya::string_t();
	}
	if ( pos + len >= src.length() ) {
		len = src.length() - pos;
	}

	return CValue(src.substr((size_t)pos, (size_t)len));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ERASE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ERASE(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 3) {
		vm.logger().Error(E_W, 8, L"ERASE", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsNum() ||
		!p.arg.array()[2].IsNum()) {
		vm.logger().Error(E_W, 9, L"ERASE", p.dicname, p.line);
		SetError(9);
	}

	aya::string_t src = p.arg.array()[0].GetValueString();
	aya::int_t pos = p.arg.array()[1].GetValueInt();
	aya::int_t len = p.arg.array()[2].GetValueInt();

	if ( pos < 0 ) {
		pos += src.length();
		if ( pos < 0 ) { //まだ負なら強制補正
			len += pos; //負値なのでたしざんで引かれる
			pos = 0;
			if ( len <= 0 ) {
				return aya::string_t();
			}
		}
	}

	if ( pos >= src.length() || len <= 0 ) {
		return aya::string_t();
	}
	if ( pos + len >= src.length() ) {
	    len = src.length() - pos;
	}

	return CValue(src.erase((size_t)pos, (size_t)len));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::INSERT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::INSERT(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 3) {
		vm.logger().Error(E_W, 8, L"INSERT", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsNum() ||
		!p.arg.array()[2].IsString()) {
		vm.logger().Error(E_W, 9, L"INSERT", p.dicname, p.line);
		SetError(9);
	}

	aya::string_t str = p.arg.array()[0].GetValueString();
	return CValue(str.insert(static_cast<size_t>(p.arg.array()[1].GetValueInt()), p.arg.array()[2].GetValueString()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOUPPER
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOUPPER(CSF_FUNCPARAM &p)
{
	return ToLowerOrUpper(p,L"TOUPPER",true);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOLOWER
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOLOWER(CSF_FUNCPARAM &p)
{
	return ToLowerOrUpper(p,L"TOLOWER",false);
}

/* -----------------------------------------------------------------------
 *  TOLOWER/TOUPPERの本体
 * -----------------------------------------------------------------------
 */
CValue CSystemFunction::ToLowerOrUpper(CSF_FUNCPARAM &p,const aya::char_t *funcname,bool isUpper)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, funcname, p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, funcname, p.dicname, p.line);
		SetError(9);
	}

	std::string locale("C");

	if (p.arg.array_size() >= 2) {
		if (!p.arg.array()[1].IsString()) {
			vm.logger().Error(E_W, 9, funcname, p.dicname, p.line);
			SetError(9);
		}
		char *p_locale = Ccct::Ucs2ToMbcs(p.arg.array()[1].GetValueString(), CHARSET_UTF8);
		locale = p_locale;
		free(p_locale);
	}

	aya::string_t result = p.arg.array()[0].GetValueString();
	size_t len = result.size();

	if ( locale == "C" ) {
		if ( isUpper ) {
			for ( size_t i = 0; i < len; ++i ) {
				if (result[i] >= L'a' && result[i] <= L'z') {
					result[i] -= static_cast<aya::string_t::value_type>(L'a' - L'A'); //小文字のほうがでかい
				}
			}
		}
		else {
			for ( size_t i = 0; i < len; ++i ) {
				if (result[i] >= L'A' && result[i] <= L'Z') {
					result[i] += static_cast<aya::string_t::value_type>(L'a' - L'A');
				}
			}
		}
	}
	else {
		std::string old_locale = aya::get_safe_str(setlocale(LC_CTYPE,NULL));
		setlocale(LC_CTYPE,locale.c_str());

		if ( isUpper ) {
			for ( size_t i = 0; i < len; ++i ) {
				result[i] = towupper(result[i]);
			}
		}
		else {
			for ( size_t i = 0; i < len; ++i ) {
				result[i] = towlower(result[i]);
			}
		}

		setlocale(LC_CTYPE,old_locale.c_str());
	}

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CUTSPACE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CUTSPACE(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"CUTSPACE", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"CUTSPACE", p.dicname, p.line);
		SetError(9);
	}

	aya::string_t	result = p.arg.array()[0].GetValueString();
	CutSpace(result);

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOBINSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOBINSTR(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"TOBINSTR", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"TOBINSTR", p.dicname, p.line);
		SetError(9);
	}

	return CValue(aya::ws_lltoa(p.arg.array()[0].GetValueInt(), 2));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOHEXSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOHEXSTR(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"TOHEXSTR", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsInt()) {
		vm.logger().Error(E_W, 9, L"TOHEXSTR", p.dicname, p.line);
		SetError(9);
	}

	return CValue(aya::ws_lltoa(p.arg.array()[0].GetValueInt(), 16));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::BINSTRTOI
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::BINSTRTOI(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"BINSTRTOI", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"BINSTRTOI", p.dicname, p.line);
		SetError(9);
	}

	if (!IsIntBinString(p.arg.array()[0].GetValueString(), 0)) {
		vm.logger().Error(E_W, 12, L"BINSTRTOI", p.dicname, p.line);
		SetError(12);
		return CValue(0);
	}

	return CValue(aya::ws_atoll(p.arg.array()[0].GetValueString(), 2));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::HEXSTRTOI
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::HEXSTRTOI(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"HEXSTRTOI", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"HEXSTRTOI", p.dicname, p.line);
		SetError(9);
	}

	aya::string_t str = p.arg.array()[0].GetValueString();

	if ( wcsnicmp(str.c_str(),L"0x",2) == 0 ) {
		str.erase(0,2);
	}

	if (!IsIntHexString(str, 0)) {
		vm.logger().Error(E_W, 12, L"HEXSTRTOI", p.dicname, p.line);
		SetError(12);
		return CValue(0);
	}

	return CValue(aya::ws_atoll(str, 16));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CHR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CHR(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"CHR", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsNum()) {
		vm.logger().Error(E_W, 9, L"CHR", p.dicname, p.line);
		SetError(9);
	}

	aya::string_t r_value(1, static_cast<aya::char_t>(p.arg.array()[0].GetValueInt()));

	for ( CValueArray::const_iterator i = p.arg.array().begin() + 1 ;
		i < p.arg.array().end() ; ++i ) {
		r_value.append(1, static_cast<aya::char_t>(i->GetValueInt()) );
	}

	return CValue(r_value);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FOPEN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FOPEN(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"FOPEN", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"FOPEN", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	return CValue(vm.files().Add(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), p.arg.array()[1].GetValueString()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FCLOSE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FCLOSE(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"FCLOSE", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"FCLOSE", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	int result = vm.files().Delete(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));

	if (!result) {
		vm.logger().Error(E_W, 13, L"FCLOSE", p.dicname, p.line);
		SetError(13);
	}
	else if (result == 2) {
		vm.logger().Error(E_W, 15, p.dicname, p.line);
		SetError(15);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FREAD
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FREAD(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"FREAD", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"FREAD", p.dicname, p.line);
		SetError(9);
		return CValue();
	}

	aya::string_t	r_value;
	int	result = vm.files().Read(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), r_value);
	CutCrLf(r_value);

	if (!result) {
		vm.logger().Error(E_W, 13, L"FREAD", p.dicname, p.line);
		SetError(13);
		return CValue(-1);
	}
	if (result == -1) {
		return CValue(-1);
	}

	return CValue(r_value);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FREADBIN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FREADBIN(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"FREADBIN", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString() || (p.arg.array_size() >= 2 && !p.arg.array()[1].IsInt()) ) {
		vm.logger().Error(E_W, 9, L"FREADBIN", p.dicname, p.line);
		SetError(9);
		return CValue();
	}

	size_t readsize = 0;
	if ( p.arg.array_size() >= 2 ) {
		readsize = static_cast<size_t>( p.arg.array()[1].GetValueInt() );
	}

	aya::char_t alt = L' ';
	if (p.arg.array_size() >= 3) {
		if (!p.arg.array()[2].IsString()) {
			vm.logger().Error(E_W, 9, L"FREADBIN", p.dicname, p.line);
			SetError(9);
			return CValue(F_TAG_NOP, 0/*dmy*/);
		}
		alt = p.arg.array()[2].GetValueString()[0];
	}

	aya::string_t	r_value;
	int	result = vm.files().ReadBin(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), r_value, readsize, alt);

	if (!result) {
		vm.logger().Error(E_W, 13, L"FREADBIN", p.dicname, p.line);
		SetError(13);
	}
	else if (result == -1)
		return CValue(-1);

	return CValue(r_value);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FREADENCODE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FREADENCODE(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"FREADENCODE", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString() || (p.arg.array_size() >= 2 && !p.arg.array()[1].IsInt()) ) {
		vm.logger().Error(E_W, 9, L"FREADENCODE", p.dicname, p.line);
		SetError(9);
		return CValue();
	}

	size_t readsize = 0;
	if ( p.arg.array_size() >= 2 ) {
		readsize = static_cast<size_t>( p.arg.array()[1].GetValueInt() );
	}

	aya::string_t type = L"base64";
	if ( p.arg.array_size() >= 3 ) {
		type = p.arg.array()[2].GetValueString();
	}

	aya::string_t	r_value;
	int	result = vm.files().ReadEncode(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), r_value, readsize, type);

	if (!result) {
		vm.logger().Error(E_W, 13, L"FREADENCODE", p.dicname, p.line);
		SetError(13);
	}
	else if (result == -1)
		return CValue(-1);

	return CValue(r_value);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FWRITE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FWRITE(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"FWRITE", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"FWRITE", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!vm.files().Write(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), p.arg.array()[1].GetValueString() + aya::string_t(L"\n"))) {
		vm.logger().Error(E_W, 13, L"FWRITE", p.dicname, p.line);
		SetError(13);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FWRITEBIN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FWRITEBIN(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"FWRITEBIN", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"FWRITEBIN", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	aya::char_t alt = L' ';

	if (p.arg.array_size() >= 3) {
		if (!p.arg.array()[2].IsString()) {
			vm.logger().Error(E_W, 9, L"FWRITEBIN", p.dicname, p.line);
			SetError(9);
			return CValue(F_TAG_NOP, 0/*dmy*/);
		}
		alt = p.arg.array()[2].GetValueString()[0];
	}

	if (!vm.files().WriteBin(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), p.arg.array()[1].GetValueString(), alt) ) {
		vm.logger().Error(E_W, 13, L"FWRITEBIN", p.dicname, p.line);
		SetError(13);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FWRITEDECODE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FWRITEDECODE(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"FWRITEDECODE", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"FWRITEDECODE", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	aya::string_t type = L"base64";

	if (p.arg.array_size() >= 3) {
		if (!p.arg.array()[2].IsString()) {
			vm.logger().Error(E_W, 9, L"FWRITEDECODE", p.dicname, p.line);
			SetError(9);
			return CValue(F_TAG_NOP, 0/*dmy*/);
		}
		type = p.arg.array()[2].GetValueString();
	}

	if (!vm.files().WriteDecode(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), p.arg.array()[1].GetValueString(), type) ) {
		vm.logger().Error(E_W, 13, L"FWRITEDECODE", p.dicname, p.line);
		SetError(13);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FWRITE2
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FWRITE2(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"FWRITE2", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"FWRITE2", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!vm.files().Write(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), p.arg.array()[1].GetValueString())) {
		vm.logger().Error(E_W, 13, L"FWRITE2", p.dicname, p.line);
		SetError(13);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FSEEK
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FSEEK(CSF_FUNCPARAM &p){
	if (p.arg.array_size() < 3) {
		vm.logger().Error(E_W, 8, L"FSEEK", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsInt()    ||
		!p.arg.array()[2].IsString()
		) {
		vm.logger().Error(E_W, 9, L"FSEEK", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	aya::int_t result=vm.files().FSeek(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), p.arg.array()[1].GetValueInt() ,p.arg.array()[2].GetValueString());
	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FTELL
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FTELL(CSF_FUNCPARAM &p){
	if (p.arg.array_size() < 1) {
		vm.logger().Error(E_W, 8, L"FTELL", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"FTELL", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	aya::int_t result=vm.files().FTell(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));
	return CValue(result);
}




/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FCOPY
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
CValue	CSystemFunction::FCOPY(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"FCOPY", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"FCOPY", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	// 絶対パス化
	aya::char_t	drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_wsplitpath(p.arg.array()[0].GetValueString().c_str(), drive, dir, fname, ext);
	aya::string_t	s_path = ((::wcslen(drive)) ? aya::string_t() : vm.basis().base_path) + p.arg.array()[0].GetValueString();

	aya::char_t	fname2[_MAX_FNAME], ext2[_MAX_EXT];
	_wsplitpath(p.arg.array()[1].GetValueString().c_str(), drive, dir, fname2, ext2);
	aya::string_t	d_path = ((::wcslen(drive)) ?
						aya::string_t() : vm.basis().base_path) + p.arg.array()[1].GetValueString() + L"\\" + fname + ext;

	int result;

	if ( IsUnicodeAware() ) {
		result = ::CopyFileW(s_path.c_str(),d_path.c_str(),FALSE) ? 1 : 0;
	}
	else {
		// パスをMBCSに変換
		char	*s_pstr = Ccct::Ucs2ToMbcs(s_path, CHARSET_DEFAULT);
		if (s_pstr == NULL) {
			vm.logger().Error(E_E, 89, L"FCOPY", p.dicname, p.line);
			return CValue(0);
		}
		char	*d_pstr = Ccct::Ucs2ToMbcs(d_path, CHARSET_DEFAULT);
		if (d_pstr == NULL) {
			free(s_pstr);
			s_pstr = NULL;
			vm.logger().Error(E_E, 89, L"FCOPY", p.dicname, p.line);
			return CValue(0);
		}

		// 実行
		result = (::CopyFileA(s_pstr, d_pstr, FALSE) ? 1 : 0);
		free(s_pstr);
		s_pstr = NULL;
		free(d_pstr);
		d_pstr = NULL;
	}

	return CValue(result);
}
#elif defined(POSIX)
CValue CSystemFunction::FCOPY(CSF_FUNCPARAM &p) {
	if (p.arg.array_size() < 2) {
	vm.logger().Error(E_W, 8, L"FCOPY", p.dicname, p.line);
	SetError(8);
	return CValue(0);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"FCOPY", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	// 絶対パス化
	std::string src = narrow(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));
	std::string dest = narrow(vm.basis().ToFullPath(p.arg.array()[1].GetValueString()));
	fix_filepath(src);
	fix_filepath(dest);

	// srcは通常ファイルでなければならない。
	// FCOPY("/dev/zero", "/tmp/hoge") とかやられたら嫌過ぎ。
	struct stat sb;
	if (stat(src.c_str(), &sb) != 0) {
	return CValue(0);
	}
	if ((sb.st_mode & S_IFREG) == 0) {
	return CValue(0);
	}

	// 実行
	std::remove(dest.c_str()); // コピー先がシンボリックリンクとかだと嫌。
	std::ifstream is(src.c_str());
	int result = 0;
	if (is.good()) {
		std::ofstream os(dest.c_str());
	if (os.good()) {
		std::unique_ptr<char[]> buf(new char[512]);
	    while (is.good()) {
		is.read(buf.get(), 512);
		int len = is.gcount();
		if (len == 0) {
		    break;
		}
		os.write(buf.get(), len);
	    }
	    result = 1;
	}
	}

	return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FMOVE
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
CValue	CSystemFunction::FMOVE(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"FMOVE", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"FMOVE", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	// 絶対パス化
	aya::char_t	drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_wsplitpath(p.arg.array()[0].GetValueString().c_str(), drive, dir, fname, ext);
	aya::string_t	s_path = ((::wcslen(drive)) ? aya::string_t() : vm.basis().base_path) + p.arg.array()[0].GetValueString();

	aya::char_t	fname2[_MAX_FNAME], ext2[_MAX_EXT];
	_wsplitpath(p.arg.array()[1].GetValueString().c_str(), drive, dir, fname2, ext2);
	aya::string_t	d_path = ((::wcslen(drive)) ?
						aya::string_t() : vm.basis().base_path) + p.arg.array()[1].GetValueString() + L"\\" + fname + ext;

	int result;

	if ( IsUnicodeAware() ) {
		result = ::MoveFileW(s_path.c_str(),d_path.c_str()) ? 1 : 0;
	}
	else {
		// パスをMBCSに変換
		char	*s_pstr = Ccct::Ucs2ToMbcs(s_path, CHARSET_DEFAULT);
		if (s_pstr == NULL) {
			vm.logger().Error(E_E, 89, L"FMOVE", p.dicname, p.line);
			return CValue(0);
		}
		char	*d_pstr = Ccct::Ucs2ToMbcs(d_path, CHARSET_DEFAULT);
		if (d_pstr == NULL) {
			free(s_pstr);
			s_pstr = NULL;
			vm.logger().Error(E_E, 89, L"FMOVE", p.dicname, p.line);
			return CValue(0);
		}

		// 実行
		result = (::MoveFileA(s_pstr, d_pstr) ? 1 : 0);
		free(s_pstr);
		s_pstr = NULL;
		free(d_pstr);
		d_pstr = NULL;
	}

	return CValue(result);
}
#elif defined(POSIX)
CValue CSystemFunction::FMOVE(CSF_FUNCPARAM &p) {
	if (p.arg.array_size() < 2) {
	vm.logger().Error(E_W, 8, L"FMOVE", p.dicname, p.line);
	SetError(8);
	return CValue(0);
	}

	if (!p.arg.array()[0].IsString() ||
	!p.arg.array()[1].IsString()) {
	vm.logger().Error(E_W, 9, L"FMOVE", p.dicname, p.line);
	SetError(9);
	return CValue(0);
	}

	// 絶対パス化
	std::string src = narrow(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));
	std::string dest = narrow(vm.basis().ToFullPath(p.arg.array()[1].GetValueString()));
	fix_filepath(src);
	fix_filepath(dest);

	// 実行
	int result = rename(src.c_str(), dest.c_str()) ? 0 : 1;

	return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::MKDIR
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
CValue	CSystemFunction::MKDIR(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"MKDIR", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"MKDIR", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	// パスをMBCSに変換
	char	*s_dirstr = Ccct::Ucs2ToMbcs(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), CHARSET_DEFAULT);
	if (s_dirstr == NULL) {
		vm.logger().Error(E_E, 89, L"MKDIR", p.dicname, p.line);
		return CValue(0);
	}

	// 実行
	int	result = (::CreateDirectoryA(s_dirstr,NULL) ? 1 : 0); //mkdirと論理が逆
	free(s_dirstr);
	s_dirstr = NULL;

	return CValue(result);
}
#elif defined(POSIX)
CValue CSystemFunction::MKDIR(CSF_FUNCPARAM &p) {
	if (!p.arg.array_size()) {
	vm.logger().Error(E_W, 8, L"MKDIR", p.dicname, p.line);
	SetError(8);
	return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
	vm.logger().Error(E_W, 9, L"MKDIR", p.dicname, p.line);
	SetError(9);
	return CValue(0);
	}

	std::string dirstr = narrow(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));
	fix_filepath(dirstr);

	// 実行
	int result = (mkdir(dirstr.c_str(), 0644) == 0 ? 1 : 0);

	return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RMDIR
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
CValue	CSystemFunction::RMDIR(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"RMDIR", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"RMDIR", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	// パスをMBCSに変換
	char	*s_dirstr = Ccct::Ucs2ToMbcs(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()), CHARSET_DEFAULT);
	if (s_dirstr == NULL) {
		vm.logger().Error(E_E, 89, L"RMDIR", p.dicname, p.line);
		return CValue(0);
	}

	// 実行
	int	result = (::RemoveDirectoryA(s_dirstr) == 0 ? 0 : 1);
	free(s_dirstr);
	s_dirstr = NULL;

	return CValue(result);
}
#elif defined(POSIX)
CValue CSystemFunction::RMDIR(CSF_FUNCPARAM &p) {
	if (!p.arg.array_size()) {
	vm.logger().Error(E_W, 8, L"RMDIR", p.dicname, p.line);
	SetError(8);
	return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
	vm.logger().Error(E_W, 9, L"RMDIR", p.dicname, p.line);
	SetError(9);
	return CValue(0);
	}

	std::string dirstr = narrow(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));
	fix_filepath(dirstr);

	// 実行。
	int result = (std::remove(dirstr.c_str()) ? 0 : 1);

	return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FDEL
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
CValue	CSystemFunction::FDEL(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"FDEL", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"FDEL", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	int result;
	aya::string_t fullpath = vm.basis().ToFullPath(p.arg.array()[0].GetValueString());

	if ( IsUnicodeAware() ) {
		result = (::DeleteFileW(fullpath.c_str()) ? 1 : 0);
	}
	else {
		// パスをMBCSに変換
		char	*s_filestr = Ccct::Ucs2ToMbcs(fullpath, CHARSET_DEFAULT);
		if (s_filestr == NULL) {
			vm.logger().Error(E_E, 89, L"FDEL", p.dicname, p.line);
			return CValue(0);
		}

		// 実行
		result = (::DeleteFileA(s_filestr) ? 1 : 0);
		free(s_filestr);
		s_filestr = NULL;
	}

	return CValue(result);
}
#elif defined(POSIX)
CValue CSystemFunction::FDEL(CSF_FUNCPARAM &p) {
	if (!p.arg.array_size()) {
	vm.logger().Error(E_W, 8, L"FDEL", p.dicname, p.line);
	SetError(8);
	return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
	vm.logger().Error(E_W, 9, L"FDEL", p.dicname, p.line);
	SetError(9);
	return CValue(0);
	}

	std::string filestr = narrow(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));
	fix_filepath(filestr);

	// 実行
	int result = (std::remove(filestr.c_str()) ? 0 : 1);

	return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FRENAME
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
CValue	CSystemFunction::FRENAME(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"FRENAME", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"FRENAME", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	int result;

	aya::string_t s_file = vm.basis().ToFullPath(p.arg.array()[0].GetValueString());
	aya::string_t d_file = vm.basis().ToFullPath(p.arg.array()[1].GetValueString());

	if ( IsUnicodeAware() ) {
		result = ::MoveFileW(s_file.c_str(), d_file.c_str()) ? 1 : 0;
	}
	else {
		// パスをMBCSに変換
		char	*s_filestr = Ccct::Ucs2ToMbcs(s_file, CHARSET_DEFAULT);
		if (s_filestr == NULL) {
			vm.logger().Error(E_E, 89, L"FRENAME", p.dicname, p.line);
			return CValue(0);
		}
		char	*d_filestr = Ccct::Ucs2ToMbcs(d_file, CHARSET_DEFAULT);
		if (d_filestr == NULL) {
			free(s_filestr);
			s_filestr = NULL;
			vm.logger().Error(E_E, 89, L"FRENAME", p.dicname, p.line);
			return CValue(0);
		}

		// 実行
		result = (::MoveFileA(s_filestr, d_filestr) ? 1 : 0);
		free(s_filestr);
		s_filestr = NULL;
		free(d_filestr);
		d_filestr = NULL;
	}

	return CValue(result);
}
#elif defined(POSIX)
CValue CSystemFunction::FRENAME(CSF_FUNCPARAM &p) {
	if (p.arg.array_size() < 2) {
	vm.logger().Error(E_W, 8, L"FRENAME", p.dicname, p.line);
	SetError(8);
	return CValue(0);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"FRENAME", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	// 絶対パス化
	std::string src = narrow(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));
	std::string dest = narrow(vm.basis().ToFullPath(p.arg.array()[1].GetValueString()));
	fix_filepath(src);
	fix_filepath(dest);

	// 実行
	int result = rename(src.c_str(), dest.c_str()) ? 0 : 1;

	return CValue(result);
}
#endif


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FDIGEST
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FDIGEST(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"FDIGEST", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"FDIGEST", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t digest_type = L"md5";
	if (p.arg.array_size()>=2) {
		digest_type = p.arg.array()[1].GetValueString();
	}

	FILE *pF = NULL;

	aya::string_t full_path = vm.basis().ToFullPath(p.arg.array()[0].GetValueString());

#if defined(WIN32)
	if ( IsUnicodeAware() ) {
		pF = _wfopen(full_path.c_str(),L"rb");
	}
	else {
		// パスをMBCSに変換
		const char *s_filestr = Ccct::Ucs2ToMbcs(full_path, CHARSET_DEFAULT);
		if (s_filestr == NULL) {
			vm.logger().Error(E_E, 89, L"FDIGEST", p.dicname, p.line);
			return CValue(-1);
		}

		pF = fopen(s_filestr,"rb");

		free((void*)s_filestr);
		s_filestr = NULL;
	}
#elif defined(POSIX)
	std::string path = narrow(full_path);
	fix_filepath(path);

	pF = fopen(path.c_str(),"rb");
#endif

	if ( ! pF ) { return CValue(-1); }

	unsigned char digest_result[32];
	size_t digest_len;

	const size_t buf_len = 32768;
	unsigned char* buf_ptr = new unsigned char[buf_len];
	
	if ( wcsicmp(digest_type.c_str(),L"sha1") == 0 || wcsicmp(digest_type.c_str(),L"sha-1") == 0 ) {
		SHA1Context sha1ctx;
		SHA1Reset(&sha1ctx);

		while ( ! feof(pF) ) {
			size_t readsize = fread(buf_ptr,sizeof(buf_ptr[0]),buf_len,pF);
			SHA1Input(&sha1ctx,buf_ptr,readsize);
			if ( readsize < buf_len ) { break; }
		}

		SHA1Result(&sha1ctx,digest_result);
		digest_len = SHA1HashSize;
	}
	else if ( wcsicmp(digest_type.c_str(),L"crc32") == 0 ) {
		unsigned long crc = 0;

		while ( ! feof(pF) ) {
			size_t readsize = fread(buf_ptr,sizeof(buf_ptr[0]),buf_len,pF);
			crc = update_crc32(buf_ptr,readsize,crc);;
			if ( readsize < buf_len ) { break; }
		}

		digest_result[0] = static_cast<unsigned char>(crc & 0xFFU);
		digest_result[1] = static_cast<unsigned char>((crc >> 8) & 0xFFU);
		digest_result[2] = static_cast<unsigned char>((crc >> 16) & 0xFFU);
		digest_result[3] = static_cast<unsigned char>((crc >> 24) & 0xFFU);
		digest_len = 4;
	}
	else { //md5
		MD5_CTX md5ctx;
		MD5Init(&md5ctx);

		while ( ! feof(pF) ) {
			size_t readsize = fread(buf_ptr,sizeof(buf_ptr[0]),buf_len,pF);
			MD5Update(&md5ctx,buf_ptr,readsize);
			if ( readsize < buf_len ) { break; }
		}

		MD5Final(digest_result,&md5ctx);
		digest_len = 16;
	}

	delete [] buf_ptr;
	buf_ptr = NULL;

	fclose(pF);

	aya::char_t md5str[65]{};
	md5str[digest_len*2] = 0; //ゼロ終端

	for ( unsigned int i = 0 ; i < digest_len ; ++i ) {
		aya::snprintf(md5str+i*2,sizeof(md5str)/sizeof(md5str[0]), L"%02X",digest_result[i]);
	}

	return CValue(aya::string_t(md5str));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::STRDIGEST
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::STRDIGEST(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"STRDIGEST", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"STRDIGEST", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t digest_type = L"md5";
	if (p.arg.array_size()>=2) {
		digest_type = p.arg.array()[1].GetValueString();
	}
	auto buf = p.arg.array()[0].GetValueString();

	unsigned char digest_result[32];
	size_t digest_len;

	const size_t buf_len = buf.size();
	unsigned char* buf_ptr = (unsigned char*)buf.c_str();
	
	if ( wcsicmp(digest_type.c_str(),L"sha1") == 0 || wcsicmp(digest_type.c_str(),L"sha-1") == 0 ) {
		SHA1Context sha1ctx;
		SHA1Reset(&sha1ctx);

		SHA1Input(&sha1ctx,buf_ptr,buf_len);

		SHA1Result(&sha1ctx,digest_result);
		digest_len = SHA1HashSize;
	}
	else if ( wcsicmp(digest_type.c_str(),L"crc32") == 0 ) {
		unsigned long crc = 0;

		crc = update_crc32(buf_ptr,buf_len,crc);

		digest_result[0] = static_cast<unsigned char>(crc & 0xFFU);
		digest_result[1] = static_cast<unsigned char>((crc >> 8) & 0xFFU);
		digest_result[2] = static_cast<unsigned char>((crc >> 16) & 0xFFU);
		digest_result[3] = static_cast<unsigned char>((crc >> 24) & 0xFFU);
		digest_len = 4;
	}
	else { //md5
		MD5_CTX md5ctx;
		MD5Init(&md5ctx);

		MD5Update(&md5ctx,buf_ptr,buf_len);

		MD5Final(digest_result,&md5ctx);
		digest_len = 16;
	}

	aya::char_t md5str[65]{};
	md5str[digest_len*2] = 0; //ゼロ終端

	for ( unsigned int i = 0 ; i < digest_len ; ++i ) {
		aya::snprintf(md5str+i*2,sizeof(md5str)/sizeof(md5str[0]), L"%02X",digest_result[i]);
	}

	return CValue(aya::string_t(md5str));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::DICLOAD
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::DICLOAD(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"DICLOAD", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"DICLOAD", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t fullpath = vm.basis().ToFullPath(p.arg.array()[0].GetValueString());
	char cset = vm.basis().GetDicCharset();

	if ( p.arg.array_size() >= 2 && p.arg.array()[1].GetValueString().size() ) {
		char cx = Ccct::CharsetTextToID(p.arg.array()[1].GetValueString().c_str());
		if ( cx != CHARSET_DEFAULT ) {
			cset = cx;
		}
	}

	int err = vm.parser0().DynamicLoadDictionary(fullpath,cset);

	if ( err > 1 ) {
		SetError(err);
	}

	return CValue(err != 0 ? 1 : 0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::DICUNLOAD
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::DICUNLOAD(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"DICUNLOAD", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"DICUNLOAD", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	int err = vm.parser0().DynamicUnloadDictionary(p.arg.array()[0].GetValueString());

	if ( err > 1 ) {
		SetError(err);
	}

	return CValue(err != 0 ? 1 : 0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::UNDEFFUNC
 *  引数　　：　_argv[0] = 関数名
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::UNDEFFUNC(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"UNDEFFUNC", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"UNDEFFUNC", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t funcname = p.arg.array()[0].GetValueString();

	int err = vm.parser0().DynamicUndefFunc(funcname);

	if ( err > 1 ) {
		SetError(err);
	}

	return CValue(err != 0 ? 1 : 0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::UNDEFGLOBALDEFINE
 *  引数　　：　_argv[0] = GLOBALDEFINE name
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::UNDEFGLOBALDEFINE(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"UNDEFGLOBALDEFINE", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"UNDEFGLOBALDEFINE", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t defname = p.arg.array()[0].GetValueString();

	std::vector<CDefine> &gdefines = vm.gdefines();
	std::vector<CDefine>::iterator itg = gdefines.begin();

	int delcount = 0;

	while (itg != gdefines.end()) {
		if ( itg->before == defname ) {
			itg = gdefines.erase(itg);
			delcount += 1;
		}
		else {
			++itg;
		}
	}

	return CValue(delcount);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ISGLOBALDEFINE
 *  引数　　：　_argv[0] = GLOBALDEFINE name
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ISGLOBALDEFINE(CSF_FUNCPARAM &p)
{
	if(!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"ISGLOBALDEFINE", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if(!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"ISGLOBALDEFINE", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t defname = p.arg.array()[0].GetValueString();

	std::vector<CDefine> &gdefines = vm.gdefines();
	std::vector<CDefine>::iterator itg = gdefines.begin();

	while (itg != gdefines.end()) {
		if( itg->before == defname ) {
			return CValue(1);
		}
		else {
			++itg;
		}
	}

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::APPEND_RUNTIME_DIC
 *  引数　　：　_argv[0] = temporary dictionary text
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::APPEND_RUNTIME_DIC(CSF_FUNCPARAM &p)
{
	if(!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"APPEND_RUNTIME_DIC", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if(!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"APPEND_RUNTIME_DIC", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t def = p.arg.array()[0].GetValueString();

	int err = vm.parser0().DynamicAppendRuntimeDictionary(def);

	return CValue(err ? -1 : 0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SETGLOBALDEFINE
 *  引数　　：　_argv[0] = GLOBALDEFINE name
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SETGLOBALDEFINE(CSF_FUNCPARAM &p)
{
	if(!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"SETGLOBALDEFINE", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if(!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"SETGLOBALDEFINE", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t defname = p.arg.array()[0].GetValueString();
	aya::string_t defbody = p.arg.array()[1].GetValueString();

	std::vector<CDefine> &gdefines = vm.gdefines();
	std::vector<CDefine>::iterator itg = gdefines.begin();

	while (itg != gdefines.end()) {
		if( itg->before == defname ) {
			itg->after=defbody;
			itg->dicfilename=L"_RUNTIME_DIC_";
			return CValue(1);
		}
		else {
			++itg;
		}
	}

	gdefines.emplace_back(vm, defname, defbody, L"_RUNTIME_DIC_");
	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETFUNCINFO
 * -----------------------------------------------------------------------
 */
CValue CSystemFunction::GETFUNCINFO(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"GETFUNCINFO", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"GETFUNCINFO", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t name = p.arg.array()[0].GetValueString();

	ptrdiff_t index = vm.function_exec().GetFunctionIndexFromName(name);

	if ( index < 0 ) {
		vm.logger().Error(E_W, 12, L"GETFUNCINFO", p.dicname, p.line);
		SetError(12);
		return CValue(-1);
	}

	CValue result(F_TAG_ARRAY, 0/*dmy*/);
	const CFunction *it = &vm.function_exec().func[size_t(index)];

	result.array().emplace_back(it->GetFileName());
	result.array().emplace_back((aya::int_t)it->GetLineNumBegin());
	result.array().emplace_back((aya::int_t)it->GetLineNumEnd());

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::PROCESSGLOBALDEFINE
 * -----------------------------------------------------------------------
 */
CValue CSystemFunction::PROCESSGLOBALDEFINE(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"PROCESSGLOBALDEFINE", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"PROCESSGLOBALDEFINE", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t aret = p.arg.array()[0].GetValueString();
	vm.parser0().ExecDefinePreProcess(aret,vm.gdefines());

	return CValue(aret);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FUNCDECL_READ
 *  変数読み込みフック　FUNCDECL_READ(変数名,関数名)
 * -----------------------------------------------------------------------
 */
CValue CSystemFunction::FUNCDECL_READ(CSF_FUNCPARAM& p)
{
	size_t arg_size = p.arg.array_size();

	if (arg_size < 2) {
		vm.logger().Error(E_W, 8, L"FUNCDECL_READ", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	//文字列かどうかチェック
	if ( ! p.arg.array()[0].IsString() ) {
		vm.logger().Error(E_W, 9, L"FUNCDECL_READ", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}
	if ( ! p.arg.array()[1].IsString() ) {
		vm.logger().Error(E_W, 9, L"FUNCDECL_READ", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	const aya::string_t &var_name = p.arg.array()[0].GetValueString();
	const aya::string_t &func_name = p.arg.array()[1].GetValueString();

	CVariable* pv;
	if (var_name[0] == L'_') {
		pv=p.lvar.GetPtr(var_name);
	}
	else {
		pv=vm.variable().GetPtr(var_name);
	}

	if (pv) {
		if ( func_name.empty() ) {
			pv->set_watcher(aya::string_t());
			return CValue(1);
		}
		else {
			ptrdiff_t i = vm.function_exec().GetFunctionIndexFromName(func_name);

			if(i != -1) {
				pv->set_watcher(func_name);
				return CValue(1);
			}
			else {
				vm.logger().Error(E_W, 9, L"FUNCDECL_READ", p.dicname, p.line);
			}
		}
	}

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FUNCDECL_WRITE
 *  変数書き込みフック　FUNCDECL_WRITE(変数名,関数名)
 * -----------------------------------------------------------------------
 */
CValue CSystemFunction::FUNCDECL_WRITE(CSF_FUNCPARAM& p)
{
	size_t arg_size = p.arg.array_size();

	if (arg_size < 2) {
		vm.logger().Error(E_W, 8, L"FUNCDECL_WRITE", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	//文字列かどうかチェック
	if ( ! p.arg.array()[0].IsString() ) {
		vm.logger().Error(E_W, 9, L"FUNCDECL_WRITE", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}
	if ( ! p.arg.array()[1].IsString() ) {
		vm.logger().Error(E_W, 9, L"FUNCDECL_WRITE", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	const aya::string_t &var_name = p.arg.array()[0].GetValueString();
	const aya::string_t &func_name = p.arg.array()[1].GetValueString();

	CVariable* pv;
	if (var_name[0] == L'_') {
		pv=p.lvar.GetPtr(var_name);
	}
	else {
		pv=vm.variable().GetPtr(var_name);
	}

	if (pv) {
		if ( func_name.empty() ) {
			pv->set_setter(aya::string_t());
			return CValue(1);
		}
		else {
			ptrdiff_t i = vm.function_exec().GetFunctionIndexFromName(func_name);

			if(i != -1) {
				pv->set_setter(func_name);
				return CValue(1);
			}
			else {
				vm.logger().Error(E_W, 9, L"FUNCDECL_WRITE", p.dicname, p.line);
			}
		}
	}

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FUNCDECL_ERASE
 *  変数削除フック　FUNCDECL_ERASE(変数名,関数名)
 * -----------------------------------------------------------------------
 */
CValue CSystemFunction::FUNCDECL_ERASE(CSF_FUNCPARAM& p)
{
	size_t arg_size = p.arg.array_size();

	if (arg_size < 2) {
		vm.logger().Error(E_W, 8, L"FUNCDECL_ERASE", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	//文字列かどうかチェック
	if ( ! p.arg.array()[0].IsString() ) {
		vm.logger().Error(E_W, 9, L"FUNCDECL_ERASE", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}
	if ( ! p.arg.array()[1].IsString() ) {
		vm.logger().Error(E_W, 9, L"FUNCDECL_ERASE", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	const aya::string_t &var_name = p.arg.array()[0].GetValueString();
	const aya::string_t &func_name = p.arg.array()[1].GetValueString();

	CVariable* pv;
	if (var_name[0] == L'_') {
		pv=p.lvar.GetPtr(var_name);
	}
	else {
		pv=vm.variable().GetPtr(var_name);
	}

	if (pv) {
		if ( func_name.empty() ) {
			pv->set_destorier(aya::string_t());
			return CValue(1);
		}
		else {
			ptrdiff_t i = vm.function_exec().GetFunctionIndexFromName(func_name);

			if(i != -1) {
				pv->set_destorier(func_name);
				return CValue(1);
			}
			else {
				vm.logger().Error(E_W, 9, L"FUNCDECL_ERASE", p.dicname, p.line);
			}
		}
	}

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::OUTPUTNUM
 *
 *  OUTPUTNUM("function_name") -> number of candidate element
 * -----------------------------------------------------------------------
 */
CValue CSystemFunction::OUTPUTNUM(CSF_FUNCPARAM& p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"OUTPUTNUM", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"OUTPUTNUM", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t name = p.arg.array()[0].GetValueString();

	ptrdiff_t index = vm.function_exec().GetFunctionIndexFromName(name);

	if ( index < 0 ) {
		vm.logger().Error(E_W, 12, L"OUTPUTNUM", p.dicname, p.line);
		SetError(12);
		return CValue(-1);
	}

	CFunction *it = &vm.function_exec().func[size_t(index)];

	return (aya::int_t)it->Execute().OutputNum();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FSIZE
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
CValue	CSystemFunction::FSIZE(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"FSIZE", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"FSIZE", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	//すでに開いているファイルならそっちから情報をパクる
	aya::string_t fullpath = vm.basis().ToFullPath(p.arg.array()[0].GetValueString());
	aya::int_t size = vm.files().Size(fullpath);
	if ( size >= 0 ) { return CValue((aya::int_t)size); }

	HANDLE hFile = INVALID_HANDLE_VALUE;

	if ( IsUnicodeAware() ) {
		hFile = ::CreateFileW(fullpath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	else {
		// パスをMBCSに変換
		char *s_filestr = Ccct::Ucs2ToMbcs(fullpath, CHARSET_DEFAULT);
		if (s_filestr == NULL) {
			vm.logger().Error(E_E, 89, L"FSIZE", p.dicname, p.line);
			return CValue(-1);
		}

		// 実行
		hFile = ::CreateFileA(s_filestr, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		free(s_filestr);
		s_filestr = NULL;
	}

	if (hFile == INVALID_HANDLE_VALUE) {
		return CValue(-1);
	}
	
	LARGE_INTEGER result;

	typedef BOOL (WINAPI* YGetFileSizeEx)(HANDLE hFile,PLARGE_INTEGER lpFileSize);
	static const YGetFileSizeEx pGetFileSizeEx = (YGetFileSizeEx)::GetProcAddress(::GetModuleHandleA("kernel32"),"GetFileSizeEx");
	
	if ( pGetFileSizeEx ) {
		if(!pGetFileSizeEx(hFile, &result)) {
			result.QuadPart=-1;
		}
	}
	else {
		result.LowPart = ::GetFileSize(hFile,(DWORD*)&result.HighPart);
		if ( result.LowPart == INVALID_FILE_SIZE ) {
			result.QuadPart=-1;
		}
	}

	::CloseHandle(hFile);

	return CValue((aya::int_t)result.QuadPart);
}
#elif defined(POSIX)
CValue CSystemFunction::FSIZE(CSF_FUNCPARAM &p) {
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"FSIZE", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"FSIZE", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	aya::string_t fullpath = vm.basis().ToFullPath(p.arg.array()[0].GetValueString());
	aya::int_t size = vm.files().Size(fullpath);
	if ( size >= 0 ) { return CValue((aya::int_t)size); }

	std::string path = narrow(fullpath);
	fix_filepath(path);

	struct stat sb;
	if (stat(path.c_str(), &sb) != 0) {
	return CValue(-1);
	}
	return CValue(static_cast<aya::int_t>(sb.st_size));
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FENUM
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FENUM(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();

	if (!sz) {
		vm.logger().Error(E_W, 8, L"FENUM", p.dicname, p.line);
		SetError(8);
		return CValue();
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"FENUM", p.dicname, p.line);
		SetError(9);
		return CValue();
	}

	// デリミタ取得
	aya::string_t	delimiter = VAR_DELIMITER;
	if (sz >= 2) {
		if (p.arg.array()[1].IsString() &&
			p.arg.array()[1].GetValueString().size())
			delimiter = p.arg.array()[1].GetValueString();
		else {
			vm.logger().Error(E_W, 9, L"FENUM", p.dicname, p.line);
			SetError(9);
			return CValue();
		}
	}

	CDirEnum ef(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));
	CDirEnumEntry entry;
	size_t count = 0;
	aya::string_t result;

	while ( ef.next(entry) ) {
		if(count)
			result += delimiter;
		if(entry.isdir)
			result += L'\\';
		result += entry.name;

		count += 1;
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FCHARSET
 *
 *  有効な値は、0/1/127=Shift_JIS/UTF-8/OSデフォルト　です。
 *  これ以外の値を与えた場合は無効で、warningとなります。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FCHARSET(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"FCHARSET", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	int	charset = GetCharset(p.arg.array()[0],L"FCHARSET", p.dicname, p.line);
	if ( charset < 0 ) {
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	vm.files().SetCharset(charset);

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ARRAYSIZE
 *
 *  文字列の場合は簡易配列の、汎用配列の場合はその要素数を返します。
 *
 *  winnt.h とマクロが被った。減点２。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ArraySize(CSF_FUNCPARAM &p)
{
	// 引数無しなら0
	size_t sz = p.valuearg.size();
	if (!sz) {
		return CValue(0);
	}

	if ( p.valuearg[0].IsArray() ) {
		return CValue(static_cast<aya::int_t>(p.valuearg[0].array_size()));
	}
	else if ( p.valuearg[0].IsString() ) {
		if ( p.valuearg[0].GetValueString().size() == 0 ) {
			return CValue(0);
		}
		// 引数1つで文字列なら簡易配列の要素数を返す　変数の場合はそのデリミタで分割する
		aya::string_t	delimiter = VAR_DELIMITER;
		if (p.pcellarg[0]->value_GetType() == F_TAG_VARIABLE)
			delimiter = vm.variable().GetDelimiter(p.pcellarg[0]->index);
		else if (p.pcellarg[0]->value_GetType() == F_TAG_LOCALVARIABLE)
			delimiter = p.lvar.GetDelimiter(p.pcellarg[0]->name);

		return CValue((aya::int_t)SplitToMultiString(p.valuearg[0].GetValueString(), NULL, delimiter));
	}
	else if ( p.valuearg[0].IsVoid() ) {
		return CValue(0);
	}
	else {
		return CValue(1);
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SETDELIM
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SETDELIM(CSF_FUNCPARAM &p)
{
	if (p.pcellarg.size() < 2) {
		vm.logger().Error(E_W, 8, L"SETDELIM", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	const CValue *delimiter;
	if (p.pcellarg[1]->value_GetType() >= F_TAG_ORIGIN_VALUE &&
		p.pcellarg[1]->value_GetType() <= F_TAG_STRING)
		delimiter = &(p.pcellarg[1]->value_const());
	else
		delimiter = &(p.pcellarg[1]->ansv_const());

	if (!delimiter->IsString()) {
		vm.logger().Error(E_W, 9, L"SETDELIM", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!delimiter->GetValueString().size()) {
		vm.logger().Error(E_W, 10, L"SETDELIM", p.dicname, p.line);
		SetError(10);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (p.pcellarg[0]->value_GetType() == F_TAG_VARIABLE)
		vm.variable().SetDelimiter(p.pcellarg[0]->index, delimiter->GetValueString());
	else if (p.pcellarg[0]->value_GetType() == F_TAG_LOCALVARIABLE)
		p.lvar.SetDelimiter(p.pcellarg[0]->name, delimiter->GetValueString());
	else {
		vm.logger().Error(E_W, 11, L"SETDELIM", p.dicname, p.line);
		SetError(11);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CSystemFunction::IHASH
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::IHASH(CSF_FUNCPARAM &p)
{
	if ( ! p.arg.array_size() ) {
		return CValue(F_TAG_HASH, 0);
	}

	if ( p.arg.array_size() % 2 ) {
		vm.logger().Error(E_W, 20, L"IHASH", p.dicname, p.line);
	}

	CValue result(F_TAG_HASH,0/*dmy*/);
	
	CValueArray::const_iterator itr = p.arg.array().begin();
	CValueArray::const_iterator ite = p.arg.array().end();

	while ( itr != ite ) {
		++itr;
		if ( itr != ite ) {
			result.hash().insert(std::pair<CValue,CValue>(*(itr-1),*(itr)));
			++itr;
		}
		else {
			result.hash().insert(std::pair<CValue,CValue>(*(itr-1),CValue()));
		}
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CSystemFunction::HASH_KEYS
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::HASH_KEYS(CSF_FUNCPARAM &p)
{
	if (p.valuearg.size() < 1) {
		vm.logger().Error(E_W, 8, L"HASH_KEYS", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if ( p.valuearg[0].GetType() != F_TAG_HASH ) {
		vm.logger().Error(E_W, 9, L"HASH_KEYS", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	const CValueHash &map = p.valuearg[0].hash();

	if ( map.empty() ) {
		return CValue(F_TAG_ARRAY,0/*dmy*/);
	}

	CValue result(F_TAG_ARRAY,0/*dmy*/);
	
	CValueHash::const_iterator itr = map.begin();
	CValueHash::const_iterator ite = map.end();

	for ( ; itr != ite ; ++itr ) {
		result.array().emplace_back(itr->first);
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CSystemFunction::HASH_VALUES
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::HASH_VALUES(CSF_FUNCPARAM &p)
{
	if (p.valuearg.size() < 1) {
		vm.logger().Error(E_W, 8, L"HASH_VALUES", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if ( p.valuearg[0].GetType() != F_TAG_HASH ) {
		vm.logger().Error(E_W, 9, L"HASH_VALUES", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	const CValueHash &map = p.valuearg[0].hash();

	if ( map.empty() ) {
		return CValue(F_TAG_ARRAY,0/*dmy*/);
	}

	CValue result(F_TAG_ARRAY,0/*dmy*/);
	
	CValueHash::const_iterator itr = map.begin();
	CValueHash::const_iterator ite = map.end();

	for ( ; itr != ite ; ++itr ) {
		result.array().emplace_back(itr->second);
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CSystemFunction::HASH_EXIST
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::HASH_EXIST(CSF_FUNCPARAM &p)
{
	if (p.valuearg.size() < 2) {
		vm.logger().Error(E_W, 8, L"HASH_EXIST", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if ( p.valuearg[0].GetType() == F_TAG_HASH || p.valuearg[0].GetType() == F_TAG_ARRAY ) {
		vm.logger().Error(E_W, 9, L"HASH_EXIST", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if ( p.valuearg[1].GetType() != F_TAG_HASH ) {
		vm.logger().Error(E_W, 9, L"HASH_EXIST", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	return CValue(p.valuearg[1].hash().find(CValueRef(p.valuearg[0])) != p.valuearg[1].hash().end() ? 1 : 0);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CSystemFunction::HASH_SIZE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::HASH_SIZE(CSF_FUNCPARAM &p)
{
	if (p.valuearg.size() < 1) {
		vm.logger().Error(E_W, 8, L"HASH_SIZE", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if ( p.valuearg[0].GetType() != F_TAG_HASH ) {
		vm.logger().Error(E_W, 9, L"HASH_SIZE", p.dicname, p.line);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	return CValue((int)p.valuearg[0].hash().size());
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ISEVALUABLE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ISEVALUABLE(CSF_FUNCPARAM &p)
{
	if(!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"ISEVALUABLE", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if(!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"ISEVALUABLE", p.dicname, p.line);
		SetError(9);
	}

	// 数式へ展開
	aya::string_t	str = p.arg.array()[0].GetValueString();
	CStatement	t_state(ST_FORMULA, p.line);
	bool result;
	vm.logger().lock();
	try {
		result = !vm.parser0().ParseEmbedString(str, t_state, p.dicname, p.line);
	}
	catch (...) {
		vm.logger().unlock();
		throw;
	}
	vm.logger().unlock();
	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::EVAL
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::EVAL(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"EVAL", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"EVAL", p.dicname, p.line);
		SetError(9);
	}

	// 数式へ展開
	aya::string_t	str = p.arg.array()[0].GetValueString();
	CStatement	t_state(ST_FORMULA, p.line);
	if (vm.parser0().ParseEmbedString(str, t_state, p.dicname, p.line))
		return CValue(p.arg.array()[0].GetValueString());

	// 実行して結果を返す
	CValue	result = p.thisfunc->GetFormulaAnswer(p.lvar, t_state);
	if (t_state.type == ST_FORMULA_SUBST)
		return CValue(F_TAG_NOP, 0/*dmy*/);
	else
		return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ERASEVAR
 *
 *  ローカル変数では消去フラグを立てるだけです。
 *  グローバル変数では消去フラグを立て、さらにunload時にファイルへ値を保存しなくなります。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ERASEVAR(CSF_FUNCPARAM &p)
{
	size_t arg_size = p.arg.array_size();

	if (!arg_size) {
		vm.logger().Error(E_W, 8, L"ERASEVAR", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	for ( size_t i = 0 ; i < arg_size ; ++i ) {

		//文字列かどうかチェック - 警告は吐くが処理続行
		if ( ! p.arg.array()[i].IsString() ) {
			vm.logger().Error(E_W, 9, L"ERASEVAR", p.dicname, p.line);
			SetError(9);
		}

		const aya::string_t &arg0 = p.arg.array()[i].GetValueString();
		if (!arg0.size()) {
			continue;
		}

		CVariable* pv;
		if (arg0[0] == L'_')
			pv=p.lvar.GetPtr(arg0);
		else
			pv=vm.variable().GetPtr(arg0);
		if (pv) {
			pv->call_destorier(vm);
			pv->Erase();
		}
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETTIME
 *
 *  引数なしか、Epochからの秒数(GETSECCOUNT)
 *
 *  返値　　：  year,month,day,week(0-6),hour,minute,secondの汎用配列
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETTIME(CSF_FUNCPARAM &p)
{
	aya::time_t ltime;

	if (!p.arg.array_size()) {
		ltime = GetEpochTime();
	}
	else {
		ltime = p.arg.array()[0].GetValueInt();
	}

	struct tm today = EpochTimeToLocalTime(ltime);

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);

	result.array().emplace_back(static_cast<aya::int_t>(today.tm_year) + 1900);
	result.array().emplace_back(static_cast<aya::int_t>(today.tm_mon) + 1);
	result.array().emplace_back(static_cast<aya::int_t>(today.tm_mday));
	result.array().emplace_back(static_cast<aya::int_t>(today.tm_wday));
	result.array().emplace_back(static_cast<aya::int_t>(today.tm_hour));
	result.array().emplace_back(static_cast<aya::int_t>(today.tm_min));
	result.array().emplace_back(static_cast<aya::int_t>(today.tm_sec));
	result.array().emplace_back(static_cast<aya::int_t>(today.tm_yday));
	result.array().emplace_back(static_cast<aya::int_t>(today.tm_isdst));

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETSECCOUNT
 *
 *  引数なしか、year,month,day,week(0-6),hour,minute,secondの配列、または
 *  日時を表すテキスト
 *
 *  返値　　：  EPOCHからの秒数
 * -----------------------------------------------------------------------
 */
/*-----------------------------------------------------
	HTTP Date Conversion
------------------------------------------------------*/
#define HTTP_DATE_TOKEN " \t,:-;"

static const char * const g_pWDayArray[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static const char * const g_pMonthArray[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

static unsigned int Utils_HTTPToSystemTime_MonthConv(char *pMon)
{
	static const unsigned int n = sizeof(g_pMonthArray) / sizeof(g_pMonthArray[0]);
	unsigned int i = 0;
	for ( i = 0 ; i < n ; ++i ) {
		if ( strnicmp(pMon,g_pMonthArray[i],3) == 0 ) {
			break;
		}
	}
	return i + 1;
}

static int Utils_TimeZoneConvert(char *pTZ)
{
	if ( ! pTZ || ! *pTZ ) { return 0; }

	int tzdiff = atoi(pTZ);
	if ( ! tzdiff ) {
		if ( stricmp(pTZ,"ut") == 0 ) {
			return 0;
		}
		if ( stricmp(pTZ,"gmt") == 0 ) {
			return 0;
		}
		if ( stricmp(pTZ,"est") == 0 ) {
			return -5*60;
		}
		if ( stricmp(pTZ,"edt") == 0 ) {
			return -4*60;
		}
		if ( stricmp(pTZ,"cst") == 0 ) {
			return -6*60;
		}
		if ( stricmp(pTZ,"cdt") == 0 ) {
			return -5*60;
		}
		if ( stricmp(pTZ,"mst") == 0 ) {
			return -7*60;
		}
		if ( stricmp(pTZ,"mdt") == 0 ) {
			return -6*60;
		}
		if ( stricmp(pTZ,"pst") == 0 ) {
			return -8*60;
		}
		if ( stricmp(pTZ,"pdt") == 0 ) {
			return -7*60;
		}

		if ( pTZ[1] == 0 ) { //1文字アルファベット？
			int c = pTZ[0];
			if ( c >= 'A' && c <= 'Z' ) {
				c = c - 'A' + 'a';
			}
			if ( c >= 'a' && c <= 'm' ) {
				return static_cast<int>(c - 'n' + 1)*-60;
			}
			if ( c >= 'n' && c <= 'y' ) {
				return static_cast<int>(c - 'n' + 1)*60;
			}
			if ( c == 'z' ) {
				return 0;
			}
		}
	}

	if ( labs(tzdiff) <= 24 ) { //2けた+時間
		return tzdiff * 60;
	}

	int h = tzdiff / 100;
	return (h * 60) + (tzdiff - (h*100));
}

// Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
// Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
// Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format

static bool Utils_HTTPToTM(const char *pText,struct tm &outTime)
{
	if ( ! pText ) { return false; }

	memset(&outTime,0,sizeof(outTime));

	unsigned int len = strlen(pText) + 1;
	char *pData = (char*)malloc(len);
	if( ! pData ) { return false; }
	memcpy(pData,pText,len);

	char *pTok = strtok(pData,HTTP_DATE_TOKEN);
	unsigned int num = 0;

	char *pTokArray[8];
	memset(&pTokArray,0,sizeof(pTokArray));

	while ( pTok && (num <= 7) ) {
		pTokArray[num] = pTok;
		++num;
		pTok = strtok(NULL,HTTP_DATE_TOKEN);
	}

	if ( num < 6 ) {
		free(pData);
		return FALSE;
	}

	unsigned int n,i = 0;
	bool isDayOfWeekFound = false;
	n = sizeof(g_pWDayArray) / sizeof(g_pWDayArray[0]);
	for ( i = 0 ; i < n ; ++i ) {
		if ( strnicmp(pTokArray[0],g_pWDayArray[i],3) == 0 ) {
			isDayOfWeekFound = true;
			break;
		}
	}

	if ( ! isDayOfWeekFound ) { //曜日省略形
		for ( int j = 7 ; j > 0 ; --j ) {
			pTokArray[j] = pTokArray[j-1];
		}
		pTokArray[0] = "";
		outTime.tm_wday = 0;
	}
	else {
		outTime.tm_wday = static_cast<unsigned short>(i);
	}

	if ( isdigit(pTokArray[1][0]) ) { //RFC Format
		outTime.tm_mday = static_cast<unsigned short>(strtoul(pTokArray[1],NULL,10));
		outTime.tm_mon = Utils_HTTPToSystemTime_MonthConv(pTokArray[2]) - 1;
		outTime.tm_year = static_cast<unsigned short>(strtoul(pTokArray[3],NULL,10)) - 1900;
		outTime.tm_hour = static_cast<unsigned short>(strtoul(pTokArray[4],NULL,10));
		outTime.tm_min = static_cast<unsigned short>(strtoul(pTokArray[5],NULL,10));
		outTime.tm_sec = static_cast<unsigned short>(strtoul(pTokArray[6],NULL,10));

		if ( outTime.tm_year < 100 ) { //2桁だった
			if ( outTime.tm_year < 70 ) {
				outTime.tm_year += 100;
			}
		}

		if ( pTokArray[7] ) { //補正
			int diff = Utils_TimeZoneConvert(pTokArray[7]);
			if ( diff ) {
				outTime.tm_min -= diff;
			}
		}
	}
	else { //C asctime
		outTime.tm_mon = Utils_HTTPToSystemTime_MonthConv(pTokArray[1]) - 1;
		outTime.tm_mday = static_cast<unsigned short>(strtoul(pTokArray[2],NULL,10));
		outTime.tm_hour = static_cast<unsigned short>(strtoul(pTokArray[3],NULL,10));
		outTime.tm_min = static_cast<unsigned short>(strtoul(pTokArray[4],NULL,10));
		outTime.tm_sec = static_cast<unsigned short>(strtoul(pTokArray[5],NULL,10));
		outTime.tm_year = static_cast<unsigned short>(strtoul(pTokArray[6],NULL,10)) - 1900;

		if ( outTime.tm_year < 100 ) { //2桁だった
			if ( outTime.tm_year < 70 ) {
				outTime.tm_year += 100;
			}
		}
	}

	free(pData);
	return true;
}

CValue	CSystemFunction::GETSECCOUNT(CSF_FUNCPARAM &p)
{
	aya::time_t ltime = GetEpochTime();

	if (!p.arg.array_size()) {
		return CValue(static_cast<aya::int_t>(ltime));
	}

	struct tm input_time = {0};
	struct tm today = EpochTimeToLocalTime(ltime);

	input_time = today;
	input_time.tm_yday = 0;
	input_time.tm_wday = 0;

	unsigned int asize = p.arg.array_size();
	if ( asize > 7 ) { asize = 7; }

	if ( asize == 1 && p.arg.array()[0].IsString() ) { //文字列日付の可能性
		char* text = Ccct::Ucs2ToMbcs(p.arg.array()[0].GetValueString().c_str(),CHARSET_DEFAULT);
		Utils_HTTPToTM(text,input_time);
		free(text);

		return CValue(static_cast<aya::int_t>(LocalTimeToEpochTime(input_time)));
	}
	else {
		switch ( asize ) {
		case 7:
			input_time.tm_sec = static_cast<int>(p.arg.array()[6].GetValueInt());
		case 6:
			input_time.tm_min = static_cast<int>(p.arg.array()[5].GetValueInt());
		case 5:
			input_time.tm_hour = static_cast<int>(p.arg.array()[4].GetValueInt());
		/*case 4:
			input_time.tm_wday = static_cast<int>( p.arg.array()[3].GetValueInt());*/ //代入禁止
		case 3:
			input_time.tm_mday = static_cast<int>(p.arg.array()[2].GetValueInt());
		case 2:
			input_time.tm_mon = static_cast<int>(p.arg.array()[1].GetValueInt()-1);
		case 1:
			input_time.tm_year = static_cast<int>( p.arg.array()[0].GetValueInt()-1900);
		}
		return CValue(static_cast<aya::int_t>(LocalTimeToEpochTime(input_time)));
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETTICKCOUNT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETTICKCOUNT(CSF_FUNCPARAM &p)
{
	//for compat (64bitになったのでいらなくなった)
	if (p.arg.array_size() > 0) {
		if (p.arg.array()[0].GetValueInt()) {
			return CValue(0);
		}
	}

#if defined(WIN32)
	typedef std::uint64_t (WINAPI *DefGetTickCount64)();

	static const DefGetTickCount64 pGetTickCount64 = (DefGetTickCount64)::GetProcAddress(::GetModuleHandleA("kernel32"),"GetTickCount64");

	if ( pGetTickCount64 ) {
		return CValue(static_cast<aya::int_t>(pGetTickCount64()));
	}
	else {
		#if _MSC_VER
			#pragma warning(push)
			#pragma warning(disable: 28159)
		#endif
		return CValue(static_cast<aya::int_t>(::GetTickCount()));
		#if _MSC_VER
			#pragma warning(pop)
		#endif
	}

#elif defined(POSIX)
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);

	return CValue(static_cast<aya::int_t>(tv.tv_sec) * 1000 + static_cast<aya::int_t>(tv.tv_usec) / 1000);
#endif
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETMEMINFO
 *
 *  返値　　：  memoryload,memorytotalphys,memoryavailphys,memorytotalvirtual,memoryavailvirtual
 *  　　　　　  の汎用配列
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
CValue	CSystemFunction::GETMEMINFO(CSF_FUNCPARAM &p)
{
	typedef BOOL (WINAPI *DefGlobalMemoryStatusEx)(LPMEMORYSTATUSEX lpBuffer);

	static const DefGlobalMemoryStatusEx pGlobalMemoryStatusEx = (DefGlobalMemoryStatusEx)::GetProcAddress(::GetModuleHandleA("kernel32"),"GlobalMemoryStatusEx");

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);

	if ( pGlobalMemoryStatusEx ) {
		MEMORYSTATUSEX	meminfo = {0};
		meminfo.dwLength = sizeof(meminfo);
		pGlobalMemoryStatusEx(&meminfo);

		result.array().emplace_back((aya::int_t)meminfo.dwMemoryLoad   );
		result.array().emplace_back((aya::int_t)meminfo.ullTotalPhys   );
		result.array().emplace_back((aya::int_t)meminfo.ullAvailPhys   );
		result.array().emplace_back((aya::int_t)meminfo.ullTotalVirtual);
		result.array().emplace_back((aya::int_t)meminfo.ullAvailVirtual);
	}
	else {
		MEMORYSTATUS meminfo = {0};
		meminfo.dwLength = sizeof(meminfo);
		#if _MSC_VER
			#pragma warning(push)
			#pragma warning(disable: 28159)
		#endif
		::GlobalMemoryStatus(&meminfo);
		#if _MSC_VER
			#pragma warning(pop)
		#endif

		result.array().emplace_back((aya::int_t)meminfo.dwMemoryLoad   );
		result.array().emplace_back((aya::int_t)meminfo.dwTotalPhys    );
		result.array().emplace_back((aya::int_t)meminfo.dwAvailPhys    );
		result.array().emplace_back((aya::int_t)meminfo.dwTotalVirtual );
		result.array().emplace_back((aya::int_t)meminfo.dwAvailVirtual );
	}

	return result;
}
#elif defined(POSIX)
CValue CSystemFunction::GETMEMINFO(CSF_FUNCPARAM &p) {
	// メモリの状態を取得するポータブルな方法は無いので…
	CValue result(F_TAG_ARRAY, 0/*dmy*/);
	result.array().emplace_back(0); // dwMemoryLoad
	result.array().emplace_back(0); // dwTotalPhys
	result.array().emplace_back(0); // dwAvailPhys
	result.array().emplace_back(0); // dwTotalVirtual
	result.array().emplace_back(0); // dwAvailVirtual
	return result;
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_SEARCH
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_SEARCH(CSF_FUNCPARAM &p)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"RE_SEARCH", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"RE_SEARCH", p.dicname, p.line);
		SetError(9);
	}
	const aya::string_t &arg0 = p.arg.array()[0].GetValueString();
	const aya::string_t &arg1 = p.arg.array()[1].GetValueString();

	if (!arg0.size() || !arg1.size())
		return CValue(0);

	// 実行
	MatchResult	t_result;
	try {
		CRegexpT<aya::char_t> regex(arg1.c_str(),re_option);
		t_result = regex.Match(arg0.c_str());
		if (t_result.IsMatched()) {
			StoreReResultDetails(arg0,t_result);
		}
	}
	catch(const std::runtime_error &) {
		vm.logger().Error(E_W, 16, L"RE_SEARCH", p.dicname, p.line);
		SetError(16);
	}
	catch(...) {
		vm.logger().Error(E_W, 17, L"RE_SEARCH", p.dicname, p.line);
		SetError(17);
	}

	return CValue(t_result.IsMatched() ? 1 : 0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_ASEARCH / RE_ASEARCHEX
 * -----------------------------------------------------------------------
 */

CValue	CSystemFunction::RE_ASEARCH(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();

	if (sz < 2) {
		//check real arg size : if search target is empty array , arg array is 1
		if (p.valuearg.size() < 2) {
			vm.logger().Error(E_W, 8, L"RE_ASEARCH", p.dicname, p.line);
			SetError(8);
		}
		return CValue(-1);
	}

	const CValue &key = p.arg.array()[0];

	try {
		CRegexpT<aya::char_t> regex(key.GetValueString().c_str(),re_option);

		for(int i = 1; i < sz; i++) {
			try {
				MatchResult t_result = regex.Match(p.arg.array()[i].GetValueString().c_str());
				if (t_result.IsMatched()) {
					return CValue(i-1);
				}
			}
			catch(const std::runtime_error &) {
				vm.logger().Error(E_W, 16, L"RE_ASEARCH", p.dicname, p.line);
				SetError(16);
			}
			catch(...) {
				vm.logger().Error(E_W, 17, L"RE_ASEARCH", p.dicname, p.line);
				SetError(17);
			}
		}

	}
	catch(...) {
		vm.logger().Error(E_W, 17, L"RE_ASEARCH", p.dicname, p.line);
		SetError(17);
		return CValue(F_TAG_ARRAY, 0/*dmy*/);
	}

	return CValue(-1);
}

CValue	CSystemFunction::RE_ASEARCHEX(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();

	if (sz < 2) {
		//check real arg size : if search target is empty array , arg array is 1
		if (p.valuearg.size() < 2) {
			vm.logger().Error(E_W, 8, L"RE_ASEARCHEX", p.dicname, p.line);
			SetError(8);
		}
		return CValue(F_TAG_ARRAY, 0/*dmy*/);
	}

	const CValue &key = p.arg.array()[0];
	CValue res(F_TAG_ARRAY, 0/*dmy*/);

	try {
		CRegexpT<aya::char_t> regex(key.GetValueString().c_str(),re_option);

		for(int i = 1; i < sz; i++) {
			try {
				MatchResult t_result = regex.Match(p.arg.array()[i].GetValueString().c_str());
				if (t_result.IsMatched()) {
					res.array().emplace_back(i-1);
				}
			}
			catch(const std::runtime_error &) {
				vm.logger().Error(E_W, 16, L"RE_ASEARCHEX", p.dicname, p.line);
				SetError(16);
			}
			catch(...) {
				vm.logger().Error(E_W, 17, L"RE_ASEARCHEX", p.dicname, p.line);
				SetError(17);
			}
		}

	}
	catch(...) {
		vm.logger().Error(E_W, 17, L"RE_ASEARCHEX", p.dicname, p.line);
		SetError(17);
		return CValue(F_TAG_ARRAY, 0/*dmy*/);
	}

	return res;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_MATCH
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_MATCH(CSF_FUNCPARAM &p)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"RE_MATCH", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"RE_MATCH", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	const aya::string_t &arg0 = p.arg.array()[0].GetValueString();
	const aya::string_t &arg1 = p.arg.array()[1].GetValueString();

	if (!arg0.size() || !arg1.size())
		return CValue(0);

	// 実行
	MatchResult	t_result;
	try {
		CRegexpT<aya::char_t> regex(arg1.c_str(),re_option);
		t_result = regex.MatchExact(arg0.c_str());
		if (t_result.IsMatched()) {
			StoreReResultDetails(arg0,t_result);
		}
	}
	catch(const std::runtime_error &) {
		vm.logger().Error(E_W, 16, L"RE_MATCH", p.dicname, p.line);
		SetError(16);
	}
	catch(...) {
		vm.logger().Error(E_W, 17, L"RE_MATCH", p.dicname, p.line);
		SetError(17);
	}

	return CValue(t_result.IsMatched() ? 1 : 0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_GREP
 *
 *  regex_grepは使用せず、regex_searchを繰り返し実行することで同等の機能としています。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_GREP(CSF_FUNCPARAM &p)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"RE_GREP", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"RE_GREP", p.dicname, p.line);
		SetError(9);
	}

	const aya::string_t &arg0 = p.arg.array()[0].GetValueString();
	const aya::string_t &arg1 = p.arg.array()[1].GetValueString();

	if (!arg0.size() || !arg1.size())
		return CValue(0);

	// 実行
	int	match_count = 0;

	try {
		CRegexpT<aya::char_t> regex(arg1.c_str(),re_option);
		CContext *pCtx = regex.PrepareMatch(arg0.c_str());

		for( ; ; ) {
			MatchResult result = regex.Match(pCtx);
			if ( ! result.IsMatched() ) {
				break;
			}
			match_count++;

			AppendReResultDetail(
				arg0.substr(result.GetStart(),result.GetEnd()-result.GetStart()),
				result.GetStart(),
				result.GetEnd()-result.GetStart());
		}

		regex.ReleaseContext(pCtx);
	}
	catch(const std::runtime_error &) {
		match_count = 0;
		vm.logger().Error(E_W, 16, L"RE_GREP", p.dicname, p.line);
		SetError(16);
	}
	catch(...) {
		match_count = 0;
		vm.logger().Error(E_W, 17, L"RE_GREP", p.dicname, p.line);
		SetError(17);
	}

	return CValue(match_count);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETLASTERROR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETLASTERROR(CSF_FUNCPARAM &p)
{
	return CValue(lasterror);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SETLASTERROR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SETLASTERROR(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"SETLASTERROR", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (p.arg.array()[0].IsNum()) {
		lasterror = static_cast<int>( p.arg.array()[0].GetValueInt() );
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	vm.logger().Error(E_W, 9, L"SETLASTERROR", p.dicname, p.line);
	SetError(9);
	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::EmBeD_HiStOrY
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::EmBeD_HiStOrY(CSF_FUNCPARAM &p)
{
	vm.logger().Error(E_E, 49, p.dicname, p.line);
	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_OPTION
 *
 *  引数1個：Perlスタイルの正規表現オプション
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_OPTION(CSF_FUNCPARAM &p)
{
	// 引数の数/型チェック
	if (p.arg.array_size() >= 1) {
		aya::string_t opt = p.arg.array()[0].GetValueString();

		re_option = 0;
		if ( opt.find(L"m") != aya::string_t::npos ) {
			re_option |= MULTILINE;
		}
		if ( opt.find(L"s") != aya::string_t::npos ) {
			re_option |= SINGLELINE;
		}
		if ( opt.find(L"x") != aya::string_t::npos ) {
			re_option |= EXTENDED;
		}
		if ( opt.find(L"i") != aya::string_t::npos ) {
			re_option |= IGNORECASE;
		}
	}

	aya::string_t result;

	if ( (re_option & MULTILINE) != 0 ) {
		result += L'm';
	}
	if ( (re_option & SINGLELINE) != 0 ) {
		result += L's';
	}
	if ( (re_option & EXTENDED) != 0 ) {
		result += L'x';
	}
	if ( (re_option & IGNORECASE) != 0 ) {
		result += L'i';
	}

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_SPLIT
 *
 *  regex_splitは使用せず、regex_searchを繰り返し実行することで同等の機能としています。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_SPLIT(CSF_FUNCPARAM &p)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	int sz = p.arg.array_size();
	if (sz < 2) {
		vm.logger().Error(E_W, 8, L"RE_SPLIT", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"RE_SPLIT", p.dicname, p.line);
		SetError(9);
	}

	aya::string_t::size_type nums = 0;
	if (sz > 2) {
		if (!p.arg.array()[2].IsNum()) {
			vm.logger().Error(E_W, 9, L"RE_SPLIT", p.dicname, p.line);
			SetError(9);
			return CValue(F_TAG_ARRAY, 0/*dmy*/);
		}
		nums = static_cast<aya::string_t::size_type>(p.arg.array()[2].GetValueInt());

		if ( nums <= 1 ) {
			return CValue(p.arg.array()[0]);
		}
	}

	return RE_SPLIT_CORE(p.arg, p.dicname, p.line, L"RE_SPLIT", nums);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_REPLACE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_REPLACE(CSF_FUNCPARAM &p)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	if (p.arg.array_size() < 3) {
		vm.logger().Error(E_W, 8, L"RE_REPLACE", p.dicname, p.line);
		SetError(8);
		return CValue(p.arg.array()[0].GetValueString());
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString() ||
		!p.arg.array()[2].IsString()) {
		vm.logger().Error(E_W, 9, L"RE_REPLACE", p.dicname, p.line);
		SetError(9);
	}

	size_t count = 0;
	if ( p.arg.array_size() >= 4 ) {
		if (!p.arg.array()[3].IsInt()) {
			vm.logger().Error(E_W, 9, L"RE_REPLACE", p.dicname, p.line);
			SetError(9);
		}
		count = (size_t)p.arg.array()[3].GetValueInt();
		if ( count <= 0 ) { count = 0; }
		else { count += 1; }
	}

	const aya::string_t &arg0 = p.arg.array()[0].GetValueString();
	const aya::string_t &arg1 = p.arg.array()[1].GetValueString();
	const aya::string_t &arg2 = p.arg.array()[2].GetValueString();

	if (!arg0.size() || !arg1.size())
		return CValue(arg0);

	// まずsplitする
	CValue	splits = RE_SPLIT_CORE(p.arg, p.dicname, p.line, L"RE_REPLACE", count);
	int	num = splits.array_size();
	if (!num || num == 1)
		return CValue(arg0);

	// 置換後文字列の作成
	aya::string_t	result;
	int	i = 0;
	for(i = 0; i < num; i++) {
		if (i) {
			result += arg2;
		}
		result += splits.array()[i].GetValueString();
	}

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_REPLACEEX
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_REPLACEEX(CSF_FUNCPARAM &p)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	if (p.arg.array_size() < 3) {
		vm.logger().Error(E_W, 8, L"RE_REPLACEEX", p.dicname, p.line);
		SetError(8);
		return CValue(p.arg.array()[0].GetValueString());
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString() ||
		!p.arg.array()[2].IsString()) {
		vm.logger().Error(E_W, 9, L"RE_REPLACEEX", p.dicname, p.line);
		SetError(9);
	}

	aya::int_t count = -1;
	if ( p.arg.array_size() >= 4 ) {
		if (!p.arg.array()[3].IsInt()) {
			vm.logger().Error(E_W, 9, L"RE_REPLACEEX", p.dicname, p.line);
			SetError(9);
		}
		count = p.arg.array()[3].GetValueInt();
		if ( count <= 0 ) { count = -1; }
	}

	const aya::string_t &arg0 = p.arg.array()[0].GetValueString();
	const aya::string_t &arg1 = p.arg.array()[1].GetValueString();
	const aya::string_t &arg2_orig = p.arg.array()[2].GetValueString();

	if (!arg0.size() || !arg1.size())
		return CValue(arg0);

	aya::string_t arg2 = arg2_orig;

	//最後から1文字手前まで
	if ( arg2.size() > 0 ) {
		for ( aya::string_t::iterator it = arg2.begin() ; it < (arg2.end()-1) ; ++it ) {
			if ( *it == L'\\' ) {
				aya::char_t c = *(it+1);

				if ( c == L'\\' ) {
					arg2.replace(it,it+2,L"\\");
				}
				else if ( c == L'a' ) {
					arg2.replace(it,it+2,L"\a");
				}
				else if ( c == L'e' ) {
					arg2.replace(it,it+2,L"\x1B");
				}
				else if ( c == L'f' ) {
					arg2.replace(it,it+2,L"\f");
				}
				else if ( c == L'n' ) {
					arg2.replace(it,it+2,L"\n");
				}
				else if ( c == L'r' ) {
					arg2.replace(it,it+2,L"\r");
				}
				else if ( c == L't' ) {
					arg2.replace(it,it+2,L"\t");
				}
				else if ( c == L'v' ) {
					arg2.replace(it,it+2,L"\v");
				}
				else if ( c >= L'0' && c <= L'9' ) {
					aya::char_t rep[3] = L"$0";
					rep[1] = c;
					arg2.replace(it,it+2,rep);
					it += 1; //次の文字は読み飛ばして良い
				}
			}
		}
	}

	// 実行
	aya::string_t str_result;

	try {
		CRegexpT<aya::char_t> regex(arg1.c_str(),re_option);

		MatchResult t_result;
		aya::char_t *result;

		if ( arg2.size() > 0 ) {
			result = regex.Replace(arg0.c_str(),arg2.c_str(),0,(int)count,&t_result);
		}
		else {
			result = regex.Replace(arg0.c_str(),L"",0,(int)count,&t_result);
		}

		str_result = result;

		if (t_result.IsMatched()) {
			StoreReResultDetails(str_result,t_result);
		}

		regex.ReleaseString(result);
	}
	catch(const std::runtime_error &) {
		vm.logger().Error(E_W, 16, L"RE_GREP", p.dicname, p.line);
		SetError(16);
	}
	catch(...) {
		vm.logger().Error(E_W, 17, L"RE_GREP", p.dicname, p.line);
		SetError(17);
	}
	return CValue(str_result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_GETSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_GETSTR(CSF_FUNCPARAM &p)
{
	return re_str;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_GETPOS
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_GETPOS(CSF_FUNCPARAM &p)
{
	return re_pos;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_GETSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_GETLEN(CSF_FUNCPARAM &p)
{
	return re_len;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_SPLIT_CORE
 *
 *  RE_SPLITの主処理部分です。RE_REPLACEでも使用します。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_SPLIT_CORE(const CValue &arg, const aya::string_t &d, int l, const aya::char_t *fncname, size_t num)
{
	const aya::string_t &arg0 = arg.array()[0].GetValueString();
	const aya::string_t &arg1 = arg.array()[1].GetValueString();

	int	t_pos = 0;
	size_t count = 1;
	CValue	splits(F_TAG_ARRAY, 0/*dmy*/);

	try {
		CRegexpT<aya::char_t> regex(arg1.c_str(),re_option);
		CContext *pCtx = regex.PrepareMatch(arg0.c_str());

		for( ; ; ) {
			MatchResult result = regex.Match(pCtx);

			if ( ! result.IsMatched() )
				break;

			count += 1;

			splits.array().emplace_back(arg0.substr(t_pos, result.GetStart()-t_pos));
			t_pos = result.GetEnd();

			AppendReResultDetail(
				arg0.substr(result.GetStart(),result.GetEnd()-result.GetStart()),
				result.GetStart(),
				result.GetEnd()-result.GetStart());

			if ( num != 0 && (count >= num) ) {
				break;
			}
		}

		regex.ReleaseContext(pCtx);

		int len = arg0.size() - t_pos;
		if ( len > 0 ) {
			splits.array().emplace_back(arg0.substr(t_pos, len));
		}
		else {
			splits.array().emplace_back(aya::string_t());
		}
	}
	catch(const std::runtime_error &) {
		splits = CValue(F_TAG_ARRAY, 0/*dmy*/);
		vm.logger().Error(E_W, 16, fncname, d, l);
		SetError(16);
	}
	catch(...) {
		splits = CValue(F_TAG_ARRAY, 0/*dmy*/);
		vm.logger().Error(E_W, 17, fncname, d, l);
		SetError(17);
	}

	return splits;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CHRCODE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CHRCODE(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"CHRCODE", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"CHRCODE", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	if (!p.arg.array()[0].GetValueString().size()) {
		vm.logger().Error(E_W, 10, L"CHRCODE", p.dicname, p.line);
		SetError(10);
		return CValue(0);
	}

	size_t getpos = 0;

	if (p.arg.array_size() >= 2) {
		if (!p.arg.array()[1].IsInt()) {
			vm.logger().Error(E_W, 9, L"CHRCODE", p.dicname, p.line);
			SetError(9);
			return CValue(0);
		}
		getpos = static_cast<size_t>( p.arg.array()[1].GetValueInt() );
		if ( getpos >= p.arg.array()[0].GetValueString().length() ) {
			getpos = p.arg.array()[0].GetValueString().length() - 1;
		}
	}

	return CValue(static_cast<aya::int_t>(p.arg.array()[0].GetValueString()[getpos]));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ISINTSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ISINTSTR(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"ISINTSTR", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"ISINTSTR", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	return CValue((aya::int_t)IsIntString(p.arg.array()[0].GetValueString()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ISREALSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ISREALSTR(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"ISREALSTR", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"ISREALSTR", p.dicname, p.line);
		SetError(9);
		return CValue(0);
	}

	return CValue( IsIntString(p.arg.array()[0].GetValueString()) || IsDoubleButNotIntString(p.arg.array()[0].GetValueString()) );
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SPLITPATH
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
CValue	CSystemFunction::SPLITPATH(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"SPLITPATH", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"SPLITPATH", p.dicname, p.line);
		SetError(9);
	}

	aya::char_t drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	aya::string_t path = p.arg.array()[0].GetValueString();

	_wsplitpath(path.c_str(), drive, dir, fname, ext);

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);
	result.array().emplace_back(drive);
	result.array().emplace_back(dir);
	result.array().emplace_back(fname);
	result.array().emplace_back(ext);

	return CValue(result);
}
#elif defined(POSIX)
CValue CSystemFunction::SPLITPATH(CSF_FUNCPARAM &p) {
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"SPLITPATH", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"SPLITPATH", p.dicname, p.line);
		SetError(9);
	}

	aya::string_t path = p.arg.array()[0].GetValueString();
	fix_filepath(path);

	CValue result(F_TAG_ARRAY, 0/*dmy*/);
	result.array().emplace_back(aya::string_t()); // driveは常に空文字列

	aya::string_t::size_type pos_slash = path.rfind(L'/');
	aya::string_t fname;
	if (pos_slash == aya::string_t::npos) {
		result.array().emplace_back(aya::string_t()); // dirも空
		fname = path;
	}
	else {
		result.array().emplace_back(path.substr(0, pos_slash+1));
		fname = path.substr(pos_slash+1);
	}

	aya::string_t::size_type pos_period = fname.rfind(L'.');
	if (pos_period == aya::string_t::npos) {
		result.array().emplace_back(fname);
		result.array().emplace_back(aya::string_t()); // extは空
	}
	else {
		result.array().emplace_back(fname.substr(0, pos_period));
		result.array().emplace_back(fname.substr(pos_period+1));
	}

	return CValue(result);
}
#endif


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::IARRAY
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::IARRAY(CSF_FUNCPARAM &p)
{
	return CValue(F_TAG_ARRAY, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CVINT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CVINT(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"CVINT", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (p.pcellarg[0]->value_GetType() == F_TAG_VARIABLE)
		vm.variable().SetValue(p.pcellarg[0]->index, p.arg.array()[0].GetValueInt());
	else if (p.pcellarg[0]->value_GetType() == F_TAG_LOCALVARIABLE)
		p.lvar.SetValue(p.pcellarg[0]->name, CValue(p.arg.array()[0].GetValueInt()));
	else {
		vm.logger().Error(E_W, 11, L"CVINT", p.dicname, p.line);
		SetError(11);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CVSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CVSTR(CSF_FUNCPARAM &p)
{
	if (!p.valuearg.size()) {
		vm.logger().Error(E_W, 8, L"CVSTR", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (p.pcellarg[0]->value_GetType() == F_TAG_VARIABLE)
		vm.variable().SetValue(p.pcellarg[0]->index, TOSTR(p));
	else if (p.pcellarg[0]->value_GetType() == F_TAG_LOCALVARIABLE)
		p.lvar.SetValue(p.pcellarg[0]->name, TOSTR(p));
	else {
		vm.logger().Error(E_W, 11, L"CVSTR", p.dicname, p.line);
		SetError(11);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CVREAL
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CVREAL(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"CVREAL", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (p.pcellarg[0]->value_GetType() == F_TAG_VARIABLE)
		vm.variable().SetValue(p.pcellarg[0]->index, p.arg.array()[0].GetValueDouble());
	else if (p.pcellarg[0]->value_GetType() == F_TAG_LOCALVARIABLE)
		p.lvar.SetValue(p.pcellarg[0]->name, CValue(p.arg.array()[0].GetValueDouble()));
	else {
		vm.logger().Error(E_W, 11, L"CVREAL", p.dicname, p.line);
		SetError(11);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CVAUTO
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CVAUTO(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"CVAUTO", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString()) {
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	aya::string_t str = p.arg.array()[0].GetValueString();
	CValue		  result;
	if(IsIntString(str)) {
		result = CValue(p.arg.array()[0].GetValueInt());
	}
	else if(IsDoubleButNotIntString(str)) {
		result = CValue(p.arg.array()[0].GetValueDouble());
	}
	else {
		return CValue(F_TAG_NOP, 0 /*dmy*/);
	}

	if(p.pcellarg[0]->value_GetType() == F_TAG_VARIABLE) {
		vm.variable().SetValue(p.pcellarg[0]->index, result);
	}
	else if(p.pcellarg[0]->value_GetType() == F_TAG_LOCALVARIABLE) {
		p.lvar.SetValue(p.pcellarg[0]->name, result);
	}
	else {
		vm.logger().Error(E_W, 11, L"CVAUTO", p.dicname, p.line);
		SetError(11);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CVAUTOEX
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CVAUTOEX(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"CVAUTOEX", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString()) {
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	aya::string_t str = p.arg.array()[0].GetValueString();
	CValue result;
	if(IsIntString(str)) {
		result = CValue(p.arg.array()[0].GetValueInt());
	}
	else if(IsDoubleButNotIntString(str)) {
		result = CValue(p.arg.array()[0].GetValueDouble());
	}
	else {
		return CValue(F_TAG_NOP, 0 /*dmy*/);
	}

	if(str!=result.GetValueString()) {
		return CValue(F_TAG_NOP, 0 /*dmy*/);
	}
	
	if(p.pcellarg[0]->value_GetType() == F_TAG_VARIABLE) {
		vm.variable().SetValue(p.pcellarg[0]->index, result);
	}
	else if(p.pcellarg[0]->value_GetType() == F_TAG_LOCALVARIABLE) {
		p.lvar.SetValue(p.pcellarg[0]->name, result);
	}
	else {
		vm.logger().Error(E_W, 11, L"CVAUTOEX", p.dicname, p.line);
		SetError(11);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LETTONAME
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LETTONAME(CSF_FUNCPARAM &p)
{
	int	sz = p.valuearg.size();

	if (sz < 2) {
		if ( p.valuearg[0].IsArray() && p.valuearg[0].array_size() >= 2 ) {
			aya::string_t	vname = p.valuearg[0].array()[0].GetValueString();

			if ( vname[0] == L'_' ) {
				p.lvar.SetValue(vname,p.valuearg[0].array()[1]);
			}
			else {
				int	index = vm.variable().Make(vname, 0);
				vm.variable().SetValue(index,p.valuearg[0].array()[1]);
			}

			return CValue(F_TAG_NOP, 0/*dmy*/);
		}
		else {
			vm.logger().Error(E_W, 8, L"LETTONAME", p.dicname, p.line);
			SetError(8);

			return CValue(F_TAG_NOP, 0/*dmy*/);
		}
	}

	if (!p.valuearg[0].IsString()) {
		vm.logger().Error(E_W, 9, L"LETTONAME", p.dicname, p.line);
		SetError(9);
	}

	aya::string_t	vname = p.valuearg[0].GetValueString();

	if ( vname[0] == L'_' ) {
		p.lvar.SetValue(vname,p.valuearg[1]);
	}
	else {
		int	index = vm.variable().Make(vname, 0);
		vm.variable().SetValue(index,p.valuearg[1]);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LSO
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LSO(CSF_FUNCPARAM &)
{
	return CValue((aya::int_t)lso);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::STRFORM
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::STRFORM(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();

	if (!sz)
		return CValue();

	if (sz == 1)
		return CValue(p.arg.array()[0]);

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"STRFORM", p.dicname, p.line);
		SetError(9);
		return CValue(p.arg.array()[0]);
	}

	// '$'でsplitする
	std::vector<aya::string_t>	vargs;
	int	vargs_sz = SplitToMultiString(p.arg.array()[0].GetValueString(), &vargs, aya::string_t(L"$"));
	if (!vargs_sz)
		return CValue();

	// 各要素ごとに_snwprintfで書式化して結合していく
	aya::string_t	left, right;
	aya::string_t	result = vargs[0];
	aya::char_t	t_str[128];
	aya::string_t	t_format;

	for(int i = 1; i < vargs_sz; i++) {
		t_format = L"%";

		const aya::char_t *arg_str = vargs[i].c_str();
		if ( (*arg_str == L'-') || (*arg_str == L'+') || (*arg_str == L'0') || (*arg_str == L' ') || (*arg_str == L'#') ) { //flag
			t_format += *arg_str;
			arg_str += 1;
		}
		while ( (*arg_str >= L'0') && (*arg_str <= L'9') ) { //width
			t_format += *arg_str;
			arg_str += 1;
		}
		if ( *arg_str == L'.' ) { //precision
			t_format += *arg_str;
			arg_str += 1;
			while ( (*arg_str >= L'0') && (*arg_str <= L'9') ) {
				t_format += *arg_str;
				arg_str += 1;
			}
		}
		if ( (*arg_str == L'h') || (*arg_str == L'l') || (*arg_str == L'L') ) { //extension
			//ここは読み飛ばす
			arg_str += 1;
		}
		if ( wcsncmp(arg_str,L"I64",3) == 0 ) {
			//ここは読み飛ばす
			arg_str += 3;
		}
		int type = F_TAG_VOID;
		if ( (*arg_str == L'c') || (*arg_str == L'C') || (*arg_str == L'd') || (*arg_str == L'i') || (*arg_str == L'o') || (*arg_str == L'u') || (*arg_str == L'x') || (*arg_str == L'X') ) {
			type = F_TAG_INT;
#ifdef INT64_IS_NOT_STD
			t_format += L"I64";
#else
			t_format += L"ll";
#endif
			t_format += *arg_str;
			arg_str += 1;
		}
		else if ( (*arg_str == L'e') || (*arg_str == L'E') || (*arg_str == L'f') || (*arg_str == L'g') || (*arg_str == L'G') ) {
			type = F_TAG_DOUBLE;
			t_format += *arg_str;
			arg_str += 1;
		}
		else if ( *arg_str == L's' ) {
			type = F_TAG_STRING;
			t_format += *arg_str;
			arg_str += 1;
		}
		else if ( *arg_str == L'S' ) {
			type = F_TAG_STRING;
			t_format += L's'; //ワイド文字列しか使わないので調整
			arg_str += 1;
		}
		else if ( (*arg_str == L'n') || (*arg_str == L'p') ) {
			//nとpは無視
			arg_str += 1;
		}

		aya::string_t	format = L"%" + vargs[i];
		if (i < sz) {
			switch ( type ) {
			case F_TAG_INT:
				aya::snprintf(t_str,128,t_format.c_str(),p.arg.array()[i].GetValueInt());
				break;
			case F_TAG_DOUBLE:
				aya::snprintf(t_str,128,t_format.c_str(),p.arg.array()[i].GetValueDouble());
				break;
			case F_TAG_STRING:
				aya::snprintf(t_str,128,t_format.c_str(),p.arg.array()[i].GetValueString().c_str());
				break;
			case F_TAG_VOID:
				t_str[0] = 0;
				break;
			default:
				vm.logger().Error(E_E, 91, p.dicname, p.line);
				t_str[0] = 0;
				break;
			};
			result += t_str;
			result += arg_str;
		}
		else {
			result += L'$' + vargs[i];
		}
	}

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ANY
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ANY(CSF_FUNCPARAM &p)
{
	// 引数無しなら空文字列
	int	sz = p.arg.array_size();
	if (!sz) {
		SetLso(-1);
		return CValue();
	}

	if (sz == 1) {
		// 引数1つで文字列でないなら引数がそのまま返る
		if (!p.arg.array()[0].IsString()) {
			SetLso(0);
			return CValue(p.arg.array()[0]);
		}

		// 引数1つで文字列なら簡易配列として処理　変数の場合はそのデリミタで分割する
		aya::string_t	delimiter = VAR_DELIMITER;
		if (p.pcellarg[0]->value_GetType() == F_TAG_VARIABLE)
			delimiter = vm.variable().GetDelimiter(p.pcellarg[0]->index);
		else if (p.pcellarg[0]->value_GetType() == F_TAG_LOCALVARIABLE)
			delimiter = p.lvar.GetDelimiter(p.pcellarg[0]->name);

		std::vector<aya::string_t>	s_array;
		int	a_sz = SplitToMultiString(p.arg.array()[0].GetValueString(), &s_array, delimiter);
		if (!a_sz) {
			SetLso(-1);
			return CValue();
		}

		size_t s_pos = vm.genrand_uint(a_sz);
		SetLso(s_pos);
		return CValue(s_array[s_pos]);
	}

	// 引数が複数なら汎用配列として処理
	size_t s_pos = vm.genrand_uint(sz);
	SetLso(s_pos);
	return CValue(p.arg.array()[s_pos]);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SAVEVAR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SAVEVAR(CSF_FUNCPARAM &p)
{
	if ( p.arg.array_size() ) {
		aya::string_t path = p.arg.array()[0].GetValueString();
		vm.basis().SaveVariable(path.c_str());
	}
	else {
		vm.basis().SaveVariable();
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RESTOREVAR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RESTOREVAR(CSF_FUNCPARAM &p)
{
	if ( p.arg.array_size() ) {
		aya::string_t path = p.arg.array()[0].GetValueString();
		vm.basis().RestoreVariable(path.c_str());
	}
	else {
		vm.basis().RestoreVariable();
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETSTRBYTES
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETSTRBYTES(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();

	if (!sz) {
		vm.logger().Error(E_W, 8, L"GETSTRBYTES", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	// 文字コード取得
	int	charset = CHARSET_SJIS;
	if (sz > 1) {
		charset = GetCharset(p.arg.array()[1],L"GETSTRBYTES", p.dicname, p.line);
		if ( charset < 0 ) {
			return CValue(0);
		}
	}

	// 主処理
	char	*t_str = Ccct::Ucs2ToMbcs(p.arg.array()[0].GetValueString(), charset);
	if (t_str == NULL) {
		vm.logger().Error(E_E, 89, L"GETSTRBYTES", p.dicname, p.line);
		return CValue(0);
	}
	int	result = ::strlen(t_str);
	free(t_str);
	t_str = NULL;

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::STRENCODE
 * -----------------------------------------------------------------------
 */

//std::tolowerの定義がへちょい処理系対策
struct ToLower {
	aya::char_t operator()(aya::char_t c) { return ::tolower(c); }
};

CValue	CSystemFunction::STRENCODE(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();

	if (!sz) {
		vm.logger().Error(E_W, 8, L"STRENCODE", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	// 文字コード取得
	int	charset = CHARSET_SJIS;
	if (sz > 1) {
		charset = GetCharset(p.arg.array()[1],L"STRENCODE", p.dicname, p.line);
		if ( charset < 0 ) {
			return CValue(0);
		}
	}

	//変換タイプ
	aya::string_t type = L"url";
	if ( sz > 2 ) {
		type = p.arg.array()[2].GetValueString();
		std::transform(type.begin(), type.end(), type.begin(), ToLower());
	}

	// 主処理
	char *t_str = Ccct::Ucs2ToMbcs(p.arg.array()[0].GetValueString(), charset);
	if (t_str == NULL) {
		vm.logger().Error(E_E, 89, L"STRENCODE", p.dicname, p.line);
		return CValue(0);
	}

	aya::native_signed len = strlen(t_str);

	aya::string_t result;
	result.reserve(len);

	if ( wcsicmp(type.c_str(),L"base64") == 0 ) {
		EncodeBase64(result,t_str,strlen(t_str));
	}
	else if ( wcsicmp(type.c_str(),L"form") == 0 ) {
		EncodeURL(result,t_str,strlen(t_str),true);
	}
	else {
		EncodeURL(result,t_str,strlen(t_str),false);
	}

	free(t_str);
	t_str = NULL;

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::STRDECODE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::STRDECODE(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();

	if (!sz) {
		vm.logger().Error(E_W, 8, L"STRDECODE", p.dicname, p.line);
		SetError(8);
		return CValue(0);
	}

	// 文字コード取得
	int	charset = CHARSET_SJIS;
	if (sz > 1) {
		charset = GetCharset(p.arg.array()[1],L"STRDECODE", p.dicname, p.line);
		if ( charset < 0 ) {
			return CValue(0);
		}
	}

	//変換タイプ
	aya::string_t type = L"url";
	if ( sz > 2 ) {
		type = p.arg.array()[2].GetValueString();
		std::transform(type.begin(), type.end(), type.begin(), ToLower());
	}

	// 主処理
	aya::string_t src = p.arg.array()[0].GetValueString();

	std::string str;
	str.reserve(src.size());

	if ( wcsicmp(type.c_str(),L"base64") == 0 ) {
		DecodeBase64(str,src.c_str(),src.length());
	}
	else if ( wcsicmp(type.c_str(),L"form") == 0 ) {
		DecodeURL(str,src.c_str(),src.length(),true);
	}
	else { //if url
		DecodeURL(str,src.c_str(),src.length(),false);
	}

	aya::char_t *t_str = Ccct::MbcsToUcs2(str, charset);
	if (t_str == NULL) {
		vm.logger().Error(E_E, 89, L"STRDECODE", p.dicname, p.line);
		return CValue(0);
	}

	CValue result(t_str);
	free(t_str);
	t_str = NULL;

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ASEARCH / ASEARCHEX
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ASEARCH(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();

	if (sz < 2) {
		//check real arg size : if search target is empty array , arg array is 1
		if (p.valuearg.size() < 2) {
			vm.logger().Error(E_W, 8, L"ASEARCH", p.dicname, p.line);
			SetError(8);
		}
		return CValue(-1);
	}

	const CValue &key = p.arg.array()[0];
	for (int i = 1; i < sz; i++) {
		if (key.Compare(p.arg.array()[i])) {
			return CValue(i - 1);
		}
	}

	return CValue(-1);
}

CValue	CSystemFunction::ASEARCHEX(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();

	if (sz < 2) {
		//check real arg size : if search target is empty array , arg array is 1
		if (p.valuearg.size() < 2) {
			vm.logger().Error(E_W, 8, L"ASEARCHEX", p.dicname, p.line);
			SetError(8);
		}
		return CValue(F_TAG_ARRAY, 0/*dmy*/);
	}

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);
	const CValue &key = p.arg.array()[0];
	for(int i = 1; i < sz; i++) {
		if (key.Compare(p.arg.array()[i])) {
			result.array().emplace_back(i - 1);
		}
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ASORT
 * -----------------------------------------------------------------------
 */

//string,ascent
template <class TTYPE>
class CSFSORT_StringAscent : public std::binary_function<TTYPE, TTYPE, bool> {
private:
	const CValueArray &a;

public:
	CSFSORT_StringAscent(const CValueArray &ain) : a(ain) { }
	CSFSORT_StringAscent(const CSFSORT_StringAscent &ain) : a(ain.a) { }

	inline bool operator()(const TTYPE& x, const TTYPE& y) const {
		return wcscmp(a[x].GetValueString().c_str(),a[y].GetValueString().c_str()) < 0;
	}
};

//string,descent
template <class TTYPE>
class CSFSORT_StringDescent : public std::binary_function<TTYPE, TTYPE, bool> {
private:
	const CValueArray &a;

public:
	CSFSORT_StringDescent(const CValueArray &ain) : a(ain) { }
	CSFSORT_StringDescent(const CSFSORT_StringDescent &ain) : a(ain.a) { }

	inline bool operator()(const TTYPE& x, const TTYPE& y) const {
		return wcscmp(a[x].GetValueString().c_str(),a[y].GetValueString().c_str()) > 0;
	}
};

//string,ascent,case insensitive
template <class TTYPE>
class CSFSORT_StringAscentI : public std::binary_function<TTYPE, TTYPE, bool> {
private:
	const CValueArray &a;

public:
	CSFSORT_StringAscentI(const CValueArray &ain) : a(ain) { }
	CSFSORT_StringAscentI(const CSFSORT_StringAscentI &ain) : a(ain.a) { }

	inline bool operator()(const TTYPE& x, const TTYPE& y) const {
		return wcsicmp(a[x].GetValueString().c_str(),a[y].GetValueString().c_str()) < 0;
	}
};

//string,descent,case insensitive
template <class TTYPE>
class CSFSORT_StringDescentI : public std::binary_function<TTYPE, TTYPE, bool> {
private:
	const CValueArray &a;

public:
	CSFSORT_StringDescentI(const CValueArray &ain) : a(ain) { }
	CSFSORT_StringDescentI(const CSFSORT_StringDescentI &ain) : a(ain.a) { }

	inline bool operator()(const TTYPE& x, const TTYPE& y) const {
		return wcsicmp(a[x].GetValueString().c_str(),a[y].GetValueString().c_str()) > 0;
	}
};

//string,ascent,length
template <class TTYPE>
class CSFSORT_StringAscentL : public std::binary_function<TTYPE, TTYPE, bool> {
private:
	const CValueArray &a;

public:
	CSFSORT_StringAscentL(const CValueArray &ain) : a(ain) { }
	CSFSORT_StringAscentL(const CSFSORT_StringAscentL &ain) : a(ain.a) { }

	inline bool operator()(const TTYPE& x, const TTYPE& y) const {
		return a[x].GetValueString().size() < a[y].GetValueString().size();
	}
};

//string,descent,length
template <class TTYPE>
class CSFSORT_StringDescentL : public std::binary_function<TTYPE, TTYPE, bool> {
private:
	const CValueArray &a;

public:
	CSFSORT_StringDescentL(const CValueArray &ain) : a(ain) { }
	CSFSORT_StringDescentL(const CSFSORT_StringDescentL &ain) : a(ain.a) { }

	inline bool operator()(const TTYPE& x, const TTYPE& y) const {
		return a[x].GetValueString().size() > a[y].GetValueString().size();
	}
};

//int,ascent
template <class TTYPE>
class CSFSORT_IntAscent : public std::binary_function<TTYPE, TTYPE, bool> {
private:
	const CValueArray &a;

public:
	CSFSORT_IntAscent(const CValueArray &ain) : a(ain) { }
	CSFSORT_IntAscent(const CSFSORT_IntAscent &ain) : a(ain.a) { }

	inline bool operator()(const TTYPE& x, const TTYPE& y) const {
		return a[x].GetValueInt() < a[y].GetValueInt();
	}
};

//int,descent
template <class TTYPE>
class CSFSORT_IntDescent : public std::binary_function<TTYPE, TTYPE, bool> {
private:
	const CValueArray &a;

public:
	CSFSORT_IntDescent(const CValueArray &ain) : a(ain) { }
	CSFSORT_IntDescent(const CSFSORT_IntDescent &ain) : a(ain.a) { }

	inline bool operator()(const TTYPE& x, const TTYPE& y) const {
		return a[x].GetValueInt() > a[y].GetValueInt();
	}
};

//double,ascent
template <class TTYPE>
class CSFSORT_DoubleAscent : public std::binary_function<TTYPE, TTYPE, bool> {
private:
	const CValueArray &a;

public:
	CSFSORT_DoubleAscent(const CValueArray &ain) : a(ain) { }
	CSFSORT_DoubleAscent(const CSFSORT_DoubleAscent &ain) : a(ain.a) { }

	inline bool operator()(const TTYPE& x, const TTYPE& y) const {
		return a[x].GetValueDouble() < a[y].GetValueDouble();
	}
};

//double,descent
template <class TTYPE>
class CSFSORT_DoubleDescent : public std::binary_function<TTYPE, TTYPE, bool> {
private:
	const CValueArray &a;

public:
	CSFSORT_DoubleDescent(const CValueArray &ain) : a(ain) { }
	CSFSORT_DoubleDescent(const CSFSORT_DoubleDescent &ain) : a(ain.a) { }

	inline bool operator()(const TTYPE& x, const TTYPE& y) const {
		return a[x].GetValueDouble() > a[y].GetValueDouble();
	}
};

CValue	CSystemFunction::ASORT(CSF_FUNCPARAM &p)
{
	unsigned int sz = p.arg.array_size();
	if (sz <= 0) {
		vm.logger().Error(E_W, 8, L"ASORT", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_ARRAY, 0/*dmy*/);
	}
	if (sz <= 1) {
		return CValue(F_TAG_ARRAY, 0/*dmy*/);
	}

	aya::string_t option = p.arg.array()[0].GetValueString();
	if ( ! p.arg.array()[0].IsString() || option.size() == 0 ) {
		option = L"string,ascent";
	}

	if (sz <= 2) {
		CValue rval(F_TAG_ARRAY, 0/*dmy*/);
		if ( option.find(L"index") != aya::string_t::npos ) {
			rval.array().emplace_back(0);
		}
		else {
			rval.array().emplace_back(p.arg.array()[1]);
		}
		return rval;
	}

	std::vector<unsigned int> sort_vector;

	for ( unsigned int i = 1 ; i < sz ; ++i ) {
		sort_vector.emplace_back(i);
	}

	bool isDescent = (option.find(L"des") != aya::string_t::npos);

	if ( option.find(L"int") != aya::string_t::npos ) { //int
		if ( isDescent ) {
			std::sort(sort_vector.begin(),sort_vector.end(),CSFSORT_IntDescent<unsigned int>(p.arg.array()));
		}
		else {
			std::sort(sort_vector.begin(),sort_vector.end(),CSFSORT_IntAscent<unsigned int>(p.arg.array()));
		}
	}
	else if ( option.find(L"double") != aya::string_t::npos ) { //double
		if ( isDescent ) {
			std::sort(sort_vector.begin(),sort_vector.end(),CSFSORT_DoubleDescent<unsigned int>(p.arg.array()));
		}
		else {
			std::sort(sort_vector.begin(),sort_vector.end(),CSFSORT_DoubleAscent<unsigned int>(p.arg.array()));
		}
	}
	else /*if ( option.find(L"str") != aya::string_t::npos )*/ {
		if ( option.find(L"len") != aya::string_t::npos ) { //strlen
			if ( isDescent ) {
				std::sort(sort_vector.begin(),sort_vector.end(),CSFSORT_StringDescentL<unsigned int>(p.arg.array()));
			}
			else {
				std::sort(sort_vector.begin(),sort_vector.end(),CSFSORT_StringAscentL<unsigned int>(p.arg.array()));
			}
		}
		else if ( option.find(L"case") != aya::string_t::npos ) { //string,case
			if ( isDescent ) {
				std::sort(sort_vector.begin(),sort_vector.end(),CSFSORT_StringDescent<unsigned int>(p.arg.array()));
			}
			else {
				std::sort(sort_vector.begin(),sort_vector.end(),CSFSORT_StringAscent<unsigned int>(p.arg.array()));
			}
		}
		else /*if ( option.find(L"case") != aya::string_t::npos )*/ { //string
			if ( isDescent ) {
				std::sort(sort_vector.begin(),sort_vector.end(),CSFSORT_StringDescentI<unsigned int>(p.arg.array()));
			}
			else {
				std::sort(sort_vector.begin(),sort_vector.end(),CSFSORT_StringAscentI<unsigned int>(p.arg.array()));
			}
		}
	}

	CValue rval(F_TAG_ARRAY, 0/*dmy*/);

	if ( option.find(L"index") != aya::string_t::npos ) {
		unsigned int n = sort_vector.size();
		for ( unsigned int i = 0 ; i < n ; ++i ) {
			rval.array().emplace_back((aya::int_t)sort_vector[i]-1);
		}
	}
	else {
		unsigned int n = sort_vector.size();
		for ( unsigned int i = 0 ; i < n ; ++i ) {
			rval.array().emplace_back(p.arg.array()[sort_vector[i]]);
		}
	}

	return rval;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETDELIM
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETDELIM(CSF_FUNCPARAM &p)
{
	if (!p.pcellarg.size()) {
		vm.logger().Error(E_W, 8, L"GETDELIM", p.dicname, p.line);
		SetError(8);
		return aya::string_t();
	}

	CValue	delimiter{aya::string_t()};
	if (p.pcellarg[0]->value_GetType() == F_TAG_VARIABLE)
		delimiter = vm.variable().GetDelimiter(p.pcellarg[0]->index);
	else if (p.pcellarg[0]->value_GetType() == F_TAG_LOCALVARIABLE)
		delimiter = p.lvar.GetDelimiter(p.pcellarg[0]->name);
	else {
		vm.logger().Error(E_W, 18, L"GETDELIM", p.dicname, p.line);
		SetError(18);
	}

	return delimiter;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SETSETTING
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SETSETTING(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 2) {
		vm.logger().Error(E_W, 8, L"SETSETTING", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	bool result = vm.basis().SetParameter(p.arg.array()[0].GetValueString(),p.arg.array()[1].GetValueString());
	if ( result ) {
		vm.basis().SetLogger();
		return CValue(1);
	}
	else {
		return CValue(0);
	}
}


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETSETTING
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETSETTING(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();

	if (!sz) {
		vm.logger().Error(E_W, 8, L"GETSETTING", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (p.arg.array()[0].IsString()) {
		const aya::string_t str = p.arg.array()[0].GetValueString();

		if ( str == L"coreinfo.version" ) {
			return CValue(aya::string_t(aya_version));
		}
		if ( str == L"coreinfo.path" ) {
			return CValue(vm.basis().GetRootPath());
		}
		if ( str == L"coreinfo.name" ) {
			return CValue(aya::string_t(aya_name));
		}
		if ( str == L"coreinfo.author" ) {
			return CValue(aya::string_t(aya_author));
		}
		if ( str == L"coreinfo.savefile" ) {
			return CValue(vm.basis().GetSavefilePath());
		}
		if ( str == L"coreinfo.mode" ) {
			return CValue(vm.basis().GetModeName());
		}

		return vm.basis().GetParameter(str);
	}
	else {
		switch(p.arg.array()[0].GetValueInt()) {
		case 0:	// AYAINFO_VERSION
			return CValue(aya::string_t(aya_version));
		case 1:	// AYAINFO_CHARSET
			return CValue(static_cast<aya::int_t>(vm.basis().GetDicCharset()));
		case 2:	// AYAINFO_PATH
			return CValue(vm.basis().GetRootPath());
		case 3:	// AYAINFO_NAME
			return CValue(aya::string_t(aya_name));
		case 4:	// AYAINFO_AUTHOR
			return CValue(aya::string_t(aya_author));
		default:
			break;
		};
	}

	vm.logger().Error(E_W, 12, L"GETSETTING", p.dicname, p.line);
	SetError(12);
	return CValue(F_TAG_NOP, 0/*dmy*/);
}


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ARRAYDEDUP
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ARRAYDEDUP(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size())
		return CValue(F_TAG_ARRAY, 0/*dmy*/);

	CValue result(F_TAG_ARRAY, 0/*dmy*/);
	std::set<CValue> tmpset;

	for(auto&i:p.arg.array())
		tmpset.insert(i);

	result.array() = { tmpset.begin(),tmpset.end() };
	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SPLIT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SPLIT(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();

	if (sz < 2) {
		vm.logger().Error(E_W, 8, L"SPLIT", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_ARRAY, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString()) {
		vm.logger().Error(E_W, 9, L"SPLIT", p.dicname, p.line);
		SetError(9);
	}

	aya::string_t::size_type nums = 0;
	if (sz > 2) {
		if (!p.arg.array()[2].IsNum()) {
			vm.logger().Error(E_W, 9, L"SPLIT", p.dicname, p.line);
			SetError(9);
			return CValue(F_TAG_ARRAY, 0/*dmy*/);
		}
		nums = static_cast<aya::string_t::size_type>(p.arg.array()[2].GetValueInt());
	}

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);

	const aya::string_t &tgt_str = p.arg.array()[0].GetValueString();
	const aya::string_t &sep_str = p.arg.array()[1].GetValueString();

	if (nums == 1 || sep_str.length() == 0) {
		result.array().emplace_back(p.arg.array()[0].GetValueString());
		return result;
	}

	const aya::string_t::size_type sep_strlen = sep_str.size();
	const aya::string_t::size_type tgt_strlen = tgt_str.size();
	aya::string_t::size_type seppoint = 0;
	aya::string_t::size_type spoint;

	for(aya::string_t::size_type i = 1; ; i++) {
		spoint = tgt_str.find(sep_str,seppoint);
		if (spoint == aya::string_t::npos || i == nums) {
			result.array().emplace_back(tgt_str.substr(seppoint,tgt_strlen - seppoint));
			break;
		}
		result.array().emplace_back(tgt_str.substr(seppoint, spoint-seppoint));
		seppoint = spoint + sep_strlen;
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SETTAMAHWND
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SETTAMAHWND(CSF_FUNCPARAM &p)
{
	if(!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"SETTAMAHWND", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if(p.arg.array()[0].IsNum()) {
		size_t hwnd = static_cast<size_t>( p.arg.array()[0].GetValueInt() );
		vm.basis().SetLogRcvWnd((long)hwnd);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	vm.logger().Error(E_W, 9, L"SETTAMAHWND", p.dicname, p.line);
	SetError(9);
	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CSystemFunction::HASH_SPLIT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::HASH_SPLIT(CSF_FUNCPARAM &p)
{
	int	sz = p.arg.array_size();
	if (sz < 3) {
		vm.logger().Error(E_W, 8, L"HASH_SPLIT", p.dicname, p.line);
		SetError(8);
		return CValue(F_TAG_HASH, 0/*dmy*/);
	}

	if (!p.arg.array()[0].IsString() ||
		!p.arg.array()[1].IsString() ||
		!p.arg.array()[2].IsString()) {
		vm.logger().Error(E_W, 9, L"HASH_SPLIT", p.dicname, p.line);
		SetError(9);
	}

	CValue	result(F_TAG_HASH, 0/*dmy*/);

	const aya::string_t &tgt_str  = p.arg.array()[0].GetValueString();
	const aya::string_t &sep_str1 = p.arg.array()[1].GetValueString();
	const aya::string_t &sep_str2 = p.arg.array()[2].GetValueString();

	if (sep_str1.length() == 0 || sep_str2.length() == 0) {
		vm.logger().Error(E_W, 10, L"HASH_SPLIT", p.dicname, p.line);
		SetError(10);
		return CValue(F_TAG_HASH, 0/*dmy*/);
	}
	
	const aya::string_t::size_type sep_str1len = sep_str1.size();
	const aya::string_t::size_type sep_str2len = sep_str2.size();
	const aya::string_t::size_type tgt_strlen  = tgt_str.size();
	aya::string_t::size_type seppoint = 0;
	aya::string_t::size_type spoint,spoint2;
	aya::string_t element;

	while(true) {
		if ( tgt_strlen-seppoint == 0 ) {
			break;
		}

		spoint = tgt_str.find(sep_str1,seppoint);
		if ( spoint == seppoint ) {
			seppoint += sep_str1len;
			continue;
		}

		if (spoint == aya::string_t::npos) {
			element = tgt_str.substr(seppoint,tgt_strlen - seppoint);
		}
		else {
			element = tgt_str.substr(seppoint, spoint-seppoint);
		}
		
		spoint2 = element.find(sep_str2,0);

		if ( spoint2 == aya::string_t::npos ) {
			result.hash().insert(std::pair<CValue,CValue>(element,CValue()));
		}
		else {
			if ( spoint2 != 0 ) {
				result.hash().insert(std::pair<CValue,CValue>(CValue(element.substr(0,spoint2))
					,CValue(element.substr(spoint2+sep_str2len,element.size()-spoint2-sep_str2len))));
			}
		}

		if (spoint == aya::string_t::npos) {
			break;
		}

		seppoint = spoint + sep_str1len;
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FATTRIB
 * -----------------------------------------------------------------------
 */

#if defined(WIN32)
static time_t FileTimeToUnixTime(FILETIME &filetime)
{
	FILETIME localfiletime;
	SYSTEMTIME systime;
	struct tm  utime {};
	FileTimeToLocalFileTime(&filetime, &localfiletime);
	FileTimeToSystemTime(&localfiletime, &systime);
	utime.tm_sec=systime.wSecond;
	utime.tm_min=systime.wMinute;
	utime.tm_hour=systime.wHour;
	utime.tm_mday=systime.wDay;
	utime.tm_mon=systime.wMonth-1;
	utime.tm_year=systime.wYear-1900;
	utime.tm_isdst=-1;
	return(mktime(&utime));
}
#endif

CValue	CSystemFunction::FATTRIB(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"FATTRIB", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsStringReal()) {
		vm.logger().Error(E_W, 9, L"FATTRIB", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

#if defined(WIN32)
	aya::string_t fullpath = vm.basis().ToFullPath(p.arg.array()[0].GetValueString());

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);

	if ( IsUnicodeAware() ) {
		WIN32_FIND_DATAW ffdata{};
		if ( ::GetFileAttributesExW(fullpath.c_str(),GetFileExInfoStandard,&ffdata) ) {
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE   ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN    ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes == FILE_ATTRIBUTE_NORMAL   ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE   ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_READONLY  ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM    ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ) ? 1 : 0);

			result.array().emplace_back((aya::int_t)FileTimeToUnixTime(ffdata.ftCreationTime));
			result.array().emplace_back((aya::int_t)FileTimeToUnixTime(ffdata.ftLastWriteTime));
		}
		else {
			result = CValue(-1);
		}
	}
	else {
		// パスをMBCSに変換
		char	*s_filestr = Ccct::Ucs2ToMbcs(fullpath, CHARSET_DEFAULT);
		if (s_filestr == NULL) {
			vm.logger().Error(E_E, 89, L"FATTRIB", p.dicname, p.line);
			return CValue(-1);
		}

		WIN32_FIND_DATAA ffdata{};
		if ( ::GetFileAttributesExA(s_filestr,GetFileExInfoStandard,&ffdata) ) {
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE   ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN    ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes == FILE_ATTRIBUTE_NORMAL   ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE   ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_READONLY  ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM    ) ? 1 : 0);
			result.array().emplace_back((ffdata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY ) ? 1 : 0);

			result.array().emplace_back((aya::int_t)FileTimeToUnixTime(ffdata.ftCreationTime));
			result.array().emplace_back((aya::int_t)FileTimeToUnixTime(ffdata.ftLastWriteTime));
		}
		else {
			result = CValue(-1);
		}

		free(s_filestr);
	}

#elif defined(POSIX)
	std::string path = narrow(vm.basis().ToFullPath(p.arg.array()[0].GetValueString()));
	fix_filepath(path);

	struct stat sb;
	if (stat(path.c_str(), &sb) != 0) {
		return CValue(-1);
	}

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);
	result.array().emplace_back(0);
	result.array().emplace_back(0);
	result.array().emplace_back(S_ISDIR(sb.st_mode) ? 1 : 0);
	result.array().emplace_back(0);
	result.array().emplace_back(S_ISREG(sb.st_mode) ? 1 : 0);
	result.array().emplace_back(0);
	result.array().emplace_back(0);
	result.array().emplace_back(0);
	result.array().emplace_back(0);
	result.array().emplace_back((aya::int_t)sb.st_ctime);
	result.array().emplace_back((aya::int_t)sb.st_mtime);
#endif

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETFUNCLIST
 *  引数　　：　_argv[0] = 絞りこみ文字列
 *  　　　　　　指定した文字列が頭についてるもののみ抽出して配列で返す
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETFUNCLIST(CSF_FUNCPARAM &p)
{
	aya::string_t name;

	//STRINGの場合のみ絞りこみ文字列として認識
	if ( p.arg.array_size() ) {
		if (p.arg.array()[0].IsString()) {
			name = p.arg.array()[0].GetValueString();
		}
	}

	CValue result(F_TAG_ARRAY, 0/*dmy*/);

	//絞りこみ文字列がない場合
	if ( name.empty() ) {
		for(std::vector<CFunction>::iterator it = vm.function_exec().func.begin(); it != vm.function_exec().func.end(); it++) {
			result.array().emplace_back(it->name);
		}
	}
	//ある場合
	else {
		aya::string_t::size_type len = name.length();

		for(std::vector<CFunction>::iterator it = vm.function_exec().func.begin(); it != vm.function_exec().func.end(); it++) {
			if(name.compare(0,len,it->name,0,len) == 0) {
				result.array().emplace_back(it->name);
			}
		}
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETSYSTEMFUNCLIST
 *  引数　　：　_argv[0] = 絞りこみ文字列
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETSYSTEMFUNCLIST(CSF_FUNCPARAM &p)
{
	aya::string_t name;

	//STRINGの場合のみ絞りこみ文字列として認識
	if ( p.arg.array_size() ) {
		if (p.arg.array()[0].IsString()) {
			name = p.arg.array()[0].GetValueString();
		}
	}

	CValue result(F_TAG_ARRAY, 0/*dmy*/);

	//絞りこみ文字列がない場合
	if ( name.empty() ) {
		for ( int i = 0 ; i < sizeof(sysfunc) / sizeof(sysfunc[0]) ; ++i ) {
			result.array().emplace_back(sysfunc[i].name);
		}
	}
	//ある場合
	else {
		aya::string_t::size_type len = name.length();

		for ( int i = 0 ; i < sizeof(sysfunc) / sizeof(sysfunc[0]) ; ++i ) {
			if ( name.compare(0,len,sysfunc[i].name,0,len) == 0 && sysfunc[i].name[0] ) {
				result.array().emplace_back(sysfunc[i].name);
			}
		}
	}

	return result;
}
/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETVARLIST
 *  引数　　：　_argv[0] = 絞りこみ文字列
 *  　　　　　　指定した文字列が頭についてるもののみ抽出して配列で返す
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETVARLIST(CSF_FUNCPARAM &p)
{
	aya::string_t name;

	//STRINGの場合のみ絞りこみ文字列として認識
	if ( p.arg.array_size() ) {
		if (p.arg.array()[0].IsString()) {
			name = p.arg.array()[0].GetValueString();
		}
	}

	CValue result(F_TAG_ARRAY, 0/*dmy*/);

	//絞りこみ文字列がない場合
	if ( name.empty() ) {
		//グローバル変数
		size_t n = vm.variable().GetNumber();

		for(size_t i = 0; i < n; ++i) {
			CVariable *pVal = vm.variable().GetPtr(i);
			if (pVal && !pVal->IsErased()) {
				result.array().emplace_back(pVal->name);
			}
		}

		unsigned int depthmax = p.lvar.GetDepth();

		//ローカル変数
		for(unsigned int depth = 0; depth < depthmax; ++depth) {
			size_t n = p.lvar.GetNumber(depth);
			for(size_t i = 0; i < n; ++i) {
				CVariable *pVal = p.lvar.GetPtr(depth,i);
				if (pVal && !pVal->IsErased()) {
					result.array().emplace_back(pVal->name);
				}
			}
		}
	}
	//ある場合
	else {
		aya::string_t::size_type len = name.length();

		if (name[0] != L'_') {
			//グローバル変数
			size_t n = vm.variable().GetNumber();

			for(size_t i = 0; i < n; ++i) {
				CVariable *pVal = vm.variable().GetPtr(i);
				if (pVal && !pVal->IsErased()) {
					if(name.compare(0,len,pVal->name,0,len) == 0) {
						result.array().emplace_back(pVal->name);
					}
				}
			}
		}
		else {
			//ローカル変数
			unsigned int depthmax = p.lvar.GetDepth();

			for(unsigned int depth = 0; depth < depthmax; ++depth) {
				size_t n = p.lvar.GetNumber(depth);
				for(size_t i = 0; i < n; ++i) {
					CVariable *pVal = p.lvar.GetPtr(depth,i);
					if (pVal && !pVal->IsErased()) {
						if(name.compare(0,len,pVal->name,0,len) == 0) {
							result.array().emplace_back(pVal->name);
						}
					}
				}
			}
		}
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETCALLSTACK（呼び出し履歴）
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETCALLSTACK(CSF_FUNCPARAM &p)
{
	CValue result(F_TAG_ARRAY, 0/*dmy*/);

	std::vector<aya::string_t> &stack = vm.call_limit().StackCall();

	size_t n = stack.size();

	for(size_t i = 0; i < n; ++i) {
		result.array().emplace_back(stack[i]);
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::MakeReResultDetails
 *  機能概要：  正規表現系関数の処理結果詳細を蓄積します
 * -----------------------------------------------------------------------
 */
void	CSystemFunction::StoreReResultDetails(const aya::string_t &str,MatchResult &result)
{
	int	sz = result.MaxGroupNumber();
	for(int i = 0; i <= sz; i++) {
		if ( result.GetGroupStart(i) >= 0 ) {
			AppendReResultDetail(
				str.substr(result.GetGroupStart(i),result.GetGroupEnd(i)-result.GetGroupStart(i)),
				result.GetGroupStart(i),
				result.GetGroupEnd(i)-result.GetGroupStart(i));
		}
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ClearReResultDetails
 *  機能概要：  正規表現系関数の処理結果詳細をクリアします
 * -----------------------------------------------------------------------
 */
void	CSystemFunction::ClearReResultDetails(void)
{
	re_str.array().clear();
	re_pos.array().clear();
	re_len.array().clear();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::AppendReResultDetail
 *  機能概要：  正規表現系関数の処理結果詳細を1つ蓄積します
 * -----------------------------------------------------------------------
 */
void	CSystemFunction::AppendReResultDetail(const aya::string_t &str, int pos, int len)
{
	re_str.array().emplace_back(str);
	re_pos.array().emplace_back(pos);
	re_len.array().emplace_back(len);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SetError
 *  機能概要：  GETLASTERRORが返す値を設定します
 * -----------------------------------------------------------------------
 */
void	CSystemFunction::SetError(int code)
{
	lasterror = code;
}

int CSystemFunction::GetCharset(const CValue &var,const wchar_t *fname, const aya::string_t &d, int l)
{
	if (var.IsNum()) {
		int	charset = static_cast<int>( var.GetValueInt() );
		if (Ccct::CheckInvalidCharset(charset)) {
			vm.logger().Error(E_W, 12, fname, d, l);
			SetError(12);
			return -1;
		}
		return charset;
	}

	if (var.IsString()) {
		aya::string_t cset = var.GetValueString();
		int	charset = Ccct::CharsetTextToID(cset.c_str());
		return charset;
	}

	vm.logger().Error(E_W, 9, fname, d, l);
	SetError(9);
	return -1;
}


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::READFMO
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
CValue CSystemFunction::READFMO(CSF_FUNCPARAM &p)
{
	aya::string_t fmoname = L"Sakura";

	if (p.arg.array_size() >= 1) {
		fmoname=p.arg.array()[0].GetValueString();
	}

	int charset = CHARSET_DEFAULT;

	if (p.arg.array_size() >= 2) {
		charset = GetCharset(p.arg.array()[1],L"READFMO", p.dicname, p.line);
		if ( charset < 0 ) {
			charset = CHARSET_DEFAULT;
		}
	}

	char* tmpstr=Ccct::Ucs2ToMbcs(fmoname.c_str(),CHARSET_DEFAULT);

	HANDLE hFMO = ::OpenFileMappingA(FILE_MAP_READ,false,tmpstr);
	if(hFMO == NULL){
		vm.logger().Error(E_W, 13, L"READFMO(" + fmoname + L").OpenFileMapping Failed", p.dicname, p.line);
		SetError(13);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	free(tmpstr);
	tmpstr = NULL;

	void *pData = ::MapViewOfFile(hFMO,FILE_MAP_READ,0,0,0);
	if(pData == NULL){
		::CloseHandle(hFMO);
		vm.logger().Error(E_W, 13, L"READFMO(" + fmoname + L").MapViewOfFile Failed" , p.dicname, p.line);
		SetError(13);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	unsigned int size=*(unsigned int*)(pData);
	if ( size<=4 ) {
		::UnmapViewOfFile(pData);
		::CloseHandle(hFMO);
		vm.logger().Error(E_W, 13, L"READFMO(" + fmoname + L").FMO size less than 4 bytes" , p.dicname, p.line);
		SetError(13);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}
	else if ( size > 1000000 ) {
		::UnmapViewOfFile(pData);
		::CloseHandle(hFMO);
		vm.logger().Error(E_W, 13, L"READFMO(" + fmoname + L").FMO size too big" , p.dicname, p.line);
		SetError(13);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	char* pBuf = new char[size+1];
	strncpy( pBuf , (const char*) pData+4, size-4 );
	pBuf[size] = 0;

	::UnmapViewOfFile(pData);
	::CloseHandle(hFMO);

	aya::char_t *t_str = Ccct::MbcsToUcs2(pBuf,charset);

	if (t_str == NULL) {
		vm.logger().Error(E_E, 13, L"READFMO(" + fmoname + L").MbcsToUcs2 Failed", p.dicname, p.line);
		SetError(13);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}
	
	delete[](pBuf);
	pBuf = NULL;
	CValue result = CValue(t_str);
	free(t_str);
	t_str= NULL;

	return result;
}
#else
CValue CSystemFunction::READFMO(CSF_FUNCPARAM &p)
{
	vm.logger().Error(E_W, 13, L"READFMO not implemented for non-win32 system.", p.dicname, p.line);
	return CValue(F_TAG_NOP, 0/*dmy*/);
}
#endif


/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::EXECUTE_WAIT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::EXECUTE_WAIT(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"EXECUTE_WAIT", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"EXECUTE_WAIT", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	// パスをMBCSに変換
	int result;

#if defined(WIN32)

	char *s_filestr = Ccct::Ucs2ToMbcs(p.arg.array()[0].GetValueString(), CHARSET_DEFAULT);
	if (s_filestr == NULL) {
		vm.logger().Error(E_E, 89, L"EXECUTE_WAIT", p.dicname, p.line);
		return CValue(-1);
	}

	char *s_parameter = NULL;
	if ( p.arg.array_size() >= 2 ) {
		if ( p.arg.array()[1].GetValueString().size() ) {
			s_parameter = Ccct::Ucs2ToMbcs(p.arg.array()[1].GetValueString(), CHARSET_DEFAULT);
		}
	}

	SHELLEXECUTEINFOA inf;
	ZeroMemory(&inf,sizeof(inf));
	inf.cbSize = sizeof(inf);
	inf.fMask = SEE_MASK_NOCLOSEPROCESS;
	inf.lpVerb = "open";
	inf.lpFile = s_filestr;
	inf.lpParameters = s_parameter;
	inf.nShow = SW_SHOWNORMAL;

	if ( ::ShellExecuteExA(&inf) ) {
		::WaitForSingleObject(inf.hProcess,INFINITE);
		DWORD status;
		result = ::GetExitCodeProcess(inf.hProcess,&status);
		::CloseHandle(inf.hProcess);
	}
	else {
		result = -1;
	}

	free(s_filestr);
	s_filestr = NULL;
	if ( s_parameter ) { free(s_parameter); s_parameter = NULL;}

#elif defined(POSIX)

	std::string path = narrow(p.arg.array()[0].GetValueString());
	fix_filepath(path);

	if ( p.arg.array_size() >= 2 ) {
		if ( p.arg.array()[1].GetValueString().size() ) {
			path += ' ';
			std::string tmp(p.arg.array()[1].GetValueString().begin(), p.arg.array()[1].GetValueString().end());
			path += tmp;
		}
	}

	result = system(path.c_str());

#endif

	// 実行
	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETENV
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETENV(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"GETENV", p.dicname, p.line);
		SetError(8);
		return aya::string_t();
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"GETENV", p.dicname, p.line);
		SetError(9);
		return aya::string_t();
	}

	char *s_name = Ccct::Ucs2ToMbcs(p.arg.array()[0].GetValueString(), CHARSET_DEFAULT);
	if (s_name == NULL) {
		vm.logger().Error(E_E, 89, L"GETENV", p.dicname, p.line);
		SetError(89);
		return aya::string_t();
	}

	const char *s_env = getenv(s_name);

	if (s_env == NULL) {
		vm.logger().Error(E_W, 12, L"GETENV", p.dicname, p.line);
		SetError(12);
		return aya::string_t();
	}

	aya::char_t	*t_env = Ccct::MbcsToUcs2(s_env, CHARSET_DEFAULT);
	if (t_env == NULL) {
		vm.logger().Error(E_E, 89, L"GETENV", p.dicname, p.line);
		SetError(89);
		return aya::string_t();
	}

	return CValue(t_env);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SLEEP
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SLEEP(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"SLEEP", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsIntReal()) {
		vm.logger().Error(E_W, 9, L"SLEEP", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	int time = static_cast<int>( p.arg.array()[0].GetValueInt() );

	if ( time <= 0 ) {
		vm.logger().Error(E_W, 9, L"SLEEP", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

#if defined(WIN32)
	::Sleep(time);

#elif defined(POSIX)
	struct timespec req,rem;
	req.tv_sec = time / 1000;
	req.tv_nsec = (time % 1000) * 1000 * 1000;

	while ( true ) {
		int r = nanosleep(&req,&rem);
		if ( r == 0 ) { break; }
		if ( (r != 0) && (errno != EINTL) ) { break; }
		req = rem;
	}
#endif

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::EXECUTE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::EXECUTE(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"EXECUTE", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"EXECUTE", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	// パスをMBCSに変換
#if defined(WIN32)
	int result;

	char *s_filestr = Ccct::Ucs2ToMbcs(p.arg.array()[0].GetValueString(), CHARSET_DEFAULT);
	if (s_filestr == NULL) {
		vm.logger().Error(E_E, 89, L"EXECUTE", p.dicname, p.line);
		return CValue(-1);
	}

	char *s_parameter = NULL;
	if ( p.arg.array_size() >= 2 ) {
		if ( p.arg.array()[1].GetValueString().size() ) {
			s_parameter = Ccct::Ucs2ToMbcs(p.arg.array()[1].GetValueString(), CHARSET_DEFAULT);
		}
	}

	result = (int)::ShellExecuteA(NULL,"open",s_filestr,s_parameter,NULL,SW_SHOWNORMAL);
	if ( result <= 32 ) { result = -1; }

	free(s_filestr);
	s_filestr = NULL;
	if ( s_parameter ) { free(s_parameter); s_parameter = NULL;}

	// 実行
	return CValue(result);

#elif defined(POSIX)
	//TODO: Implement
	return CValue(-1);

#endif

}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TRANSLATE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TRANSLATE(CSF_FUNCPARAM &p)
{
	if (p.arg.array_size() < 3) {
		vm.logger().Error(E_W, 8, L"TRANSLATE", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	aya::string_t str          = p.arg.array()[0].GetValueString();
	aya::string_t rep_from_str = p.arg.array()[1].GetValueString();
	aya::string_t rep_to_str   = p.arg.array()[2].GetValueString();

	std::vector<aya::char_t> rep_from;
	std::vector<aya::char_t> rep_to;

	if ( ! ProcessTranslateSyntax(rep_from,rep_from_str, p.dicname, p.line) ) {
		return CValue(-1);
	}
	if ( ! ProcessTranslateSyntax(rep_to,rep_to_str, p.dicname, p.line) ) {
		return CValue(-1);
	}

	if ( rep_from.size() > rep_to.size() ) {
		if ( rep_to.size() > 0 ) {
			//置き換え先のほうが小さいっぽい？
			vm.logger().Error(E_W, 12, L"TRANSLATE", p.dicname, p.line);
			SetError(12);
			//警告を吐いた後で、一番最後の文字で埋めておく
			aya::char_t cx = *(rep_to.end()-1);
			while ( rep_from.size() > rep_to.size() ) {
				rep_to.emplace_back(cx);
			}
		}
	}

	bool is_delete = (rep_to.size() == 0);

	//signedでカウントしないと erase時にひどいことになる
	int n = str.length();
	int rep_size = rep_from.size();

	for ( int i = 0 ; i < n ; ++i ) {
		aya::char_t cx = str[i];
		for ( int r = 0 ; r < rep_size ; ++r ) {
			if ( cx == rep_from[r] ) {
				if ( is_delete ) {
					str.erase(i,1);
					n -= 1;
					i -= 1;
				}
				else {
					str[i] = rep_to[r];
				}
				break;
			}
		}
	}

	return CValue(str);
}

bool CSystemFunction::ProcessTranslateSyntax(std::vector<aya::char_t> &array,aya::string_t &str, const aya::string_t &d, int l)
{
	size_t n = str.length();

	for ( size_t i = 0 ; i < n ; ++i ) {
		if ( str[i] == L'-' ) {
			if ( (i >= (n-1)) || (array.size() == 0) ) {
				//-が閉じてない、もしくは開いてない
				vm.logger().Error(E_W, 12, L"TRANSLATE", d, l);
				SetError(12);
				array.emplace_back(L'-');
				continue;
			}
			i += 1;
			aya::char_t start = *(array.end()-1);
			array.erase(array.end()-1,array.end());
			aya::char_t end = str[i];
			if ( start > end ) {
				//startのほうがでかい：ゼロ要素として処理可能なので続行
				vm.logger().Error(E_W, 12, L"TRANSLATE", d, l);
				SetError(12);
			}
			else if ( start == end ) {
				array.emplace_back(start);
			}
			else {
				if ( (end - start) >= 256 ) {
					//範囲がでかすぎる：ちょん切って続行
					vm.logger().Error(E_W, 12, L"TRANSLATE", d, l);
					SetError(12);
					end = start + 255;
				}

				for ( aya::char_t cx = start ; cx <= end ; ++cx ) {
					array.emplace_back(cx);
				}
			}
		}
		else if ( str[i] == L'\\' ) {
			if ( i >= (n-1) ) {
				//エスケープ後の文字がない
				vm.logger().Error(E_W, 12, L"TRANSLATE", d, l);
				SetError(12);
				array.emplace_back(L'-');
				continue;
			}
			i += 1;
			aya::char_t esc_char = str[i];

			if ( esc_char == L'a' ) {
				array.emplace_back(L'\a');
			}
			else if ( esc_char == L'b' ) {
				array.emplace_back(L'\b');
			}
			else if ( esc_char == L'e' ) {
				array.emplace_back(0x1bU);
			}
			else if ( esc_char == L'f' ) {
				array.emplace_back(L'\f');
			}
			else if ( esc_char == L'n' ) {
				array.emplace_back(L'\n');
			}
			else if ( esc_char == L'r' ) {
				array.emplace_back(L'\r');
			}
			else if ( esc_char == L't' ) {
				array.emplace_back(L'\t');
			}
			else {
				array.emplace_back(esc_char);
			}
		}
		else {
			array.emplace_back(str[i]);
		}
	}
	return true;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LINT.GetFuncUsedBy
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LINT_GetFuncUsedBy(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"LINT.GetFuncUsedBy", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"LINT.GetFuncUsedBy", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	ptrdiff_t index = vm.function_exec().GetFunctionIndexFromName(p.arg.array()[0].GetValueString());
	if ( index < 0 ) {
		vm.logger().Error(E_W, 12, L"LINT.GetFuncUsedBy", p.dicname, p.line);
		SetError(12);
		return CValue(-1);
	}

	CValue result(F_TAG_ARRAY, 0/*dmy*/);
	const CFunction *it = &vm.function_exec().func[size_t(index)];
	std::unordered_set<aya::string_t> name_set;

	for(auto&s:it->statement)
		for(auto&c:s.cell())
			if (F_TAG_ISFUNC(c.value_GetType()))
				name_set.insert(c.name);

	result.array() = { name_set.begin(),name_set.end() };
	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LINT.GetUserDefFuncUsedBy
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LINT_GetUserDefFuncUsedBy(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"LINT.GetUserDefFuncUsedBy", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"LINT.GetUserDefFuncUsedBy", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	ptrdiff_t index = vm.function_exec().GetFunctionIndexFromName(p.arg.array()[0].GetValueString());
	if ( index < 0 ) {
		vm.logger().Error(E_W, 12, L"LINT.GetUserDefFuncUsedBy", p.dicname, p.line);
		SetError(12);
		return CValue(-1);
	}

	CValue result(F_TAG_ARRAY, 0/*dmy*/);
	const CFunction *it = &vm.function_exec().func[size_t(index)];
	std::unordered_set<aya::string_t> name_set;

	for(auto&s:it->statement)
		for(auto&c:s.cell())
			if (c.value_GetType() == F_TAG_USERFUNC)
				name_set.insert(c.name);

	result.array() = { name_set.begin(),name_set.end() };
	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LINT.GetGlobalVarUsedBy
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LINT_GetGlobalVarUsedBy(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"LINT.GetGlobalVarUsedBy", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"LINT.GetGlobalVarUsedBy", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	ptrdiff_t index = vm.function_exec().GetFunctionIndexFromName(p.arg.array()[0].GetValueString());
	if ( index < 0 ) {
		vm.logger().Error(E_W, 12, L"LINT.GetGlobalVarUsedBy", p.dicname, p.line);
		SetError(12);
		return CValue(-1);
	}

	CValue result(F_TAG_ARRAY, 0/*dmy*/);
	const CFunction *it = &vm.function_exec().func[size_t(index)];
	std::unordered_set<aya::string_t> name_set;

	for(auto&s:it->statement)
		for(auto&c:s.cell())
			if (c.value_GetType() == F_TAG_VARIABLE)
				name_set.insert(vm.variable().GetName(c.index));

	result.array() = { name_set.begin(),name_set.end() };
	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LINT.GetLocalVarUsedBy
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LINT_GetLocalVarUsedBy(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"LINT.GetLocalVarUsedBy", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"LINT.GetLocalVarUsedBy", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	ptrdiff_t index = vm.function_exec().GetFunctionIndexFromName(p.arg.array()[0].GetValueString());
	if ( index < 0 ) {
		vm.logger().Error(E_W, 12, L"LINT.GetLocalVarUsedBy", p.dicname, p.line);
		SetError(12);
		return CValue(-1);
	}

	CValue result(F_TAG_ARRAY, 0/*dmy*/);
	const CFunction *it = &vm.function_exec().func[size_t(index)];
	std::vector<CValueRef>& array = result.array();
	size_t value_count = 0;

	for(auto&s:it->statement){
		if(s.type==ST_OPEN)
			array.emplace_back(L"{");
		else if(s.type==ST_CLOSE)
			array.emplace_back(L"}");
		else
			for (auto&c : s.cell())
				if ( c.value_GetType() == F_TAG_LOCALVARIABLE ) {
					array.emplace_back(c.name);
					++value_count;
				}
	}

	if ( value_count == 0 ) {
		//no local variable detected. clear all and return empty array.
		array.clear();
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LINT.GetGlobalVarLetted
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LINT_GetGlobalVarLetted(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"LINT.GetGlobalVarLetted", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"LINT.GetGlobalVarLetted", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	ptrdiff_t index = vm.function_exec().GetFunctionIndexFromName(p.arg.array()[0].GetValueString());
	if ( index < 0 ) {
		vm.logger().Error(E_W, 12, L"LINT.GetGlobalVarLetted", p.dicname, p.line);
		SetError(12);
		return CValue(-1);
	}

	CValue result(F_TAG_ARRAY, 0/*dmy*/);
	const CFunction *it = &vm.function_exec().func[size_t(index)];
	std::unordered_set<aya::string_t> name_set;
	const CCell* sid_0_cell = 0;
	size_t o_index;

	for(auto&s:it->statement)
		for(auto&se:s.serial()){
			o_index = se.tindex;
			const CCell& o_cell = s.cell()[o_index];
			if(F_TAG_ISLET(o_cell.value_GetType())){
				sid_0_cell = &(s.cell()[se.index[0]]);
				if(sid_0_cell->value_GetType()==F_TAG_VARIABLE)
					name_set.insert(vm.variable().GetName(sid_0_cell->index));
			}
		}

	result.array() = { name_set.begin(),name_set.end() };
	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LINT.GetLocalVarLetted
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LINT_GetLocalVarLetted(CSF_FUNCPARAM &p)
{
	if (!p.arg.array_size()) {
		vm.logger().Error(E_W, 8, L"LINT.GetLocalVarLetted", p.dicname, p.line);
		SetError(8);
		return CValue(-1);
	}

	if (!p.arg.array()[0].IsString()) {
		vm.logger().Error(E_W, 9, L"LINT.GetLocalVarLetted", p.dicname, p.line);
		SetError(9);
		return CValue(-1);
	}

	ptrdiff_t index = vm.function_exec().GetFunctionIndexFromName(p.arg.array()[0].GetValueString());
	if ( index < 0 ) {
		vm.logger().Error(E_W, 12, L"LINT.GetLocalVarLetted", p.dicname, p.line);
		SetError(12);
		return CValue(-1);
	}

	CValue result(F_TAG_ARRAY, 0/*dmy*/);
	const CFunction *it = &vm.function_exec().func[size_t(index)];
	auto& array = result.array();
	const CCell* sid_0_cell = 0;
	size_t o_index;
	size_t value_count = 0;

	for(auto&s:it->statement){
		if (s.type == ST_OPEN)
			array.emplace_back(L"{");
		else if (s.type == ST_CLOSE)
			array.emplace_back(L"}");
		else
			for(auto&se:s.serial()){
				o_index = se.tindex;
				const CCell& o_cell = s.cell()[o_index];
				if(F_TAG_ISLET(o_cell.value_GetType())){
					sid_0_cell = &(s.cell()[se.index[0]]);
					if(sid_0_cell->value_GetType()==F_TAG_LOCALVARIABLE){
						array.emplace_back(sid_0_cell->name);
						++value_count;
					}
				}
			}
	}

	if ( value_count == 0 ) {
		//no local variable detected. clear all and return empty array.
		array.clear();
	}

	return result;
}
