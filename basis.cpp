﻿//
// AYA version 5
//
// 主な制御を行なうクラス　CBasis
// written by umeici. 2004
//

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
#endif

#include <string.h>

#include <iostream>
#include <string>
#include <vector>
#include <stack>
//#include <filesystem>

#include "fix_unistd.h"

#if defined(POSIX)
# include <dirent.h>
# include <sys/stat.h>
#endif
/*
#  include <cstdlib>
#  include <sys/types.h>
*/
#include "basis.h"

#include "aya5.h"
#include "ccct.h"
#include "comment.h"
#include "file.h"
#include "function.h"
#include "lib.h"
#include "log.h"
#include "messages.h"
#include "misc.h"
#include "parser0.h"
#if defined(POSIX)
# include "posix_utils.h"
#endif
#include "globaldef.h"
#include "wsex.h"
#include "ayavm.h"
#include "dir_enum.h"

//////////DEBUG/////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
////////////////////////////////////////


/* -----------------------------------------------------------------------
 * CBasisコンストラクタ
 * -----------------------------------------------------------------------
 */

CBasis::CBasis(CAyaVM &vmr) : vm(vmr)
{
	ResetSuppress();

	iolog       = 1;

	dic_charset       = CHARSET_SJIS;
	setting_charset   = CHARSET_SJIS;
	output_charset    = CHARSET_UTF8;
	file_charset      = CHARSET_SJIS;
	save_charset      = CHARSET_UTF8;
	save_old_charset  = CHARSET_SJIS;
	extension_charset = CHARSET_SJIS;
	log_charset       = CHARSET_UTF8;

	encode_savefile = false;
	auto_save = true;

#if defined(WIN32)
	hlogrcvWnd  = NULL;
#endif
	run         = 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::SetModuleHandle
 *  機能概要：  モジュールハンドルを取得します
 *
 *  ついでにモジュールの主ファイル名取得も行います
 * -----------------------------------------------------------------------
 */
void	CBasis::SetModuleName(const aya::string_t &s,const aya::char_t *trailer,const aya::char_t *mode)
{
	modulename = s;
	modename = mode;

	if(trailer) {
		config_file_name_trailer = trailer;
	}
	else {
		config_file_name_trailer.erase(config_file_name_trailer.begin(), config_file_name_trailer.end());
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::SetPath
 *  機能概要：  HGLOBALに格納されたファイルパスを取得します HGLOBALは開放しません
 * -----------------------------------------------------------------------
 */
#if defined(WIN32) || defined(_WIN32_WCE)
void	CBasis::SetPath(aya::global_t h, int len)
{
	// 取得と領域開放
	std::string	mbpath;
	mbpath.assign((char *)h, 0, len);
	//GlobalFree(h); //load側で開放
	h = NULL;

	// 文字コードをUCS-2へ変換（ここでのマルチバイト文字コードはOSデフォルト）
	Ccct::MbcsToUcs2Buf(base_path, mbpath, CHARSET_DEFAULT);

	//最後が\でも/でもなければ足す
	if(!base_path.length() || ((base_path[base_path.length()-1] != L'/') && (base_path[base_path.length()-1] != L'\\'))) {
		base_path += L"\\";
	}

	load_path = base_path;
}
#elif defined(POSIX)
void	CBasis::SetPath(aya::global_t h, int len)
{
	// 取得と領域開放
	path = widen(std::string(h, static_cast<std::string::size_type>(len)));
	//free(h); //load側で開放
	h = NULL;
	// スラッシュで終わってなければ付ける。
	if(!path.length() || path[path.length() - 1] != L'/') {
		path += L'/';
	}
	// モジュールハンドルの取得は出来ないので、力技で位置を知る。
	// このディレクトリにある全ての*.dll(case insensitive)を探し、
	// 中身にaya.dllという文字列を含んでいたら、それを選ぶ。
	// ただし対応する*.txtが無ければdllの中身は見ずに次へ行く。
	modulename = L"aya";
	DIR* dh = opendir(narrow(path).c_str());
	if(dh == NULL) {
		std::cerr << narrow(path) << "is not a directory!" << std::endl;
	exit(1);
	}
	while (true) {
	struct dirent* ent = readdir(dh);
	if(ent == NULL) {
		break; // もう無い
	}
	std::string fname(ent->d_name, strlen(ent->d_name)/*ent->d_namlen*/);	// by umeici. 2005/1/16 5.6.0.232
	if(lc(get_extension(fname)) == "dll") {
		std::string txt_file = narrow(path) + change_extension(fname, "txt");
		struct stat sb;
		if(!::stat(txt_file.c_str(), &sb)) {
		// txtファイルがあるので、中身を見てみる。
		if(file_content_search(narrow(path) + fname, "aya.dll") != std::string::npos) {
			// これはAYAのDLLである。
			modulename = widen(drop_extension(fname));
			break;
		}
		}
	}
	}
	closedir(dh);
	load_path = base_path;
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::SetLogRcvWnd
 *  機能概要：  チェックツールから渡されたhWndを保持します
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
void	CBasis::SetLogRcvWnd(long hwnd)
{
	hlogrcvWnd = (HWND)hwnd;
	vm.logger().Start(logpath, log_charset, hlogrcvWnd, iolog);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::SetLogger
 *  機能概要：  ログ機能を初期化 / 再設定します
 * -----------------------------------------------------------------------
 */
void CBasis::SetLogger(void)
{
	vm.logger().Start(logpath, log_charset, hlogrcvWnd, iolog);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::Configure
 *  機能概要：  load時に行う初期設定処理
 * -----------------------------------------------------------------------
 */
void	CBasis::Configure(void)
{
	// 基礎設定ファイル（例えばaya.txt）を読み取り
	std::vector<CDic1>	dics;
	LoadBaseConfigureFile(dics);
	// 基礎設定ファイル読み取りで重篤なエラーが発生した場合はここで終了
	if(suppress)
		return;

	// ロギングを開始
	SetLogger();

	// 辞書読み込みと構文解析
	if(vm.parser0().Parse(dic_charset, dics))
		SetSuppress();

	// 前回終了時に保存した変数を復元
	RestoreVariable();

	// ここまでの処理で重篤なエラーが発生した場合はここで終了
	if(suppress)
		return;

	// 外部ライブラリとファイルの文字コードを初期化
	vm.libs().SetCharset(extension_charset);
	vm.files().SetCharset(file_charset);

	run = 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::Termination
 *  機能概要：  unload時に行う終了処理
 * -----------------------------------------------------------------------
 */
void	CBasis::Termination(void)
{
	// 動作抑止されていなければ終了時の処理を実行
	if(!suppress) {
		// unload
		ExecuteUnload();
		// ロードしているすべてのライブラリをunload
		vm.libs().DeleteAll();
		// 開いているすべてのファイルを閉じる
		vm.files().DeleteAll();
		// 変数の保存
		if(auto_save) {
			SaveVariable();
		}
	}

	// ロギングを終了
	vm.logger().Termination();

	//
	loadindex.Init();
	unloadindex.Init();
	requestindex.Init();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::IsSuppress
 *  機能概要：  現在の自律抑止状態を返します
 *
 *  返値　　：  0/1=非抑止/抑止
 *
 *  基礎設定ファイルの読み取りや辞書ファイルの解析中に、動作継続困難なエラーが発生すると
 *  SetSuppress()によって抑止設定されます。抑止設定されると、load/request/unloadでの動作が
 *  すべてマスクされます。この時、requestの返値は常に空文字列になります。（HGLOBAL=NULL、
 *  len=0で応答します）
 * -----------------------------------------------------------------------
 */
bool CBasis::IsSuppress(void) {
	return suppress;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ToFullPath
 *  機能概要：  渡された文字列が相対パス表記なら絶対パスに書き換えます
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)
aya::string_t	CBasis::ToFullPath(const aya::string_t& str)
{
	aya::char_t	drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_wsplitpath(str.c_str(), drive, dir, fname, ext);
	aya::string_t aret = str;

	if (!::wcslen(drive))
		aret = vm.basis().base_path + str;

	aya::ws_replace(aret,L"/",L"\\");
	size_t index;
	while((index = aret.find(L"\\\\")) != aya::string_t::npos)
		aret.replace(index,2,L"\\");

	return aret;
}
#elif defined(POSIX)
aya::string_t CBasis::ToFullPath(const aya::string_t& str)
{
	aya::string_t aret = str;
	if (!(str.length() > 0 && str[0] == L'/')) {
		aret = vm.basis().path + str;
	}

	aya::ws_replace(aret, L"/", L"\\");
	size_t index;
	while ((index = aret.find(L"\\\\")) != aya::string_t::npos)
		aret.replace(index, 2, L"\\");

	return aret;
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::SetSuppress
 *  機能概要：  自律動作抑止を設定します
 * -----------------------------------------------------------------------
 */
void	CBasis::SetSuppress(void)
{
	suppress = 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::ResetSuppress
 *  機能概要：  自律動作抑止機能をリセットします
 * -----------------------------------------------------------------------
 */
void	CBasis::ResetSuppress(void)
{
	suppress = 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::LoadBaseConfigureFile
 *  機能概要：  基礎設定ファイルを読み取り、各種パラメータをセットします
 *
 *  基礎設定ファイルはDLLと同階層に存在する名前が"DLL主ファイル名.txt"のファイルです。
 *
 *  辞書ファイルの文字コードはShift_JIS以外にもUTF-8とOSデフォルトのコードに対応できますが、
 *  この基礎設定ファイルはOSデフォルトのコードで読み取られることに注意してください。
 *  国際化に関して考慮する場合は、このファイル内の記述にマルチバイト文字を使用するべきでは
 *  ありません（文字コード0x7F以下のASCII文字のみで記述すべきです）。
 * -----------------------------------------------------------------------
 */
class CBasisFileStack {
public:
	FILE *fp;
	aya::string_t filename;
	int line;

	CBasisFileStack(FILE *fparg,aya::string_t &fn,int larg) : fp(fparg) , filename(fn) , line(larg) {
	}
};

void	CBasis::LoadBaseConfigureFile(std::vector<CDic1> &dics)
{
	// 設定ファイル("name".txt)読み取り

	// ファイルを開く
	aya::string_t	filename = load_path + modulename + config_file_name_trailer + L".txt";

	// 読み込み当初は文字コードが定義されていないので、CHARSET_UNDEFにする
	LoadBaseConfigureFile_Base(filename,dics,CHARSET_UNDEF);
}

void	CBasis::LoadBaseConfigureFile_Base(aya::string_t filename,std::vector<CDic1> &dics,char cset)
{
	// ファイルを開く
	FILE	*fp = aya::w_fopen(filename.c_str(), L"r");
	if(fp == NULL) {
		vm.logger().Error(E_E, 5, filename);
		SetSuppress();
		return;
	}

	// 読み取り処理
	CComment	comment;
	aya::string_t	cmd, param;
	size_t line=0;
	
	aya::string_t	readline;
	readline.reserve(1000);

	char cset_real;
	std::string buf;
	buf.reserve(1000);

	while ( true ) {
		line += 1;

		// 1行読み込み
		cset_real = cset;

		if ( cset == CHARSET_UNDEF ) {
			//後の設定で変更されている可能性があるので、毎回上書きすること
			//always overwrite cset_real because setting_charset may be modified in SetParameter function
			cset_real = setting_charset;
		}

		if (aya::ws_fgets(buf, readline, fp, cset_real, 0, line) == aya::WS_EOF) {
			// ファイルを閉じる
			fclose(fp);
			break;
		}

		// 改行は消去
		CutCrLf(readline);

		// コメントアウト処理
		comment.Process_Top(readline);
		comment.Process(readline);
		comment.Process_Tail(readline);

		// 空行、もしくは全体がコメント行だった場合は次の行へ
		if(!readline.size()) {
			continue;
		}

		// パラメータを設定
		if(Split(readline, cmd, param, L",")) {
			SetParameter(cmd, param, &dics);
		}
		else {
			vm.logger().Error(E_W, 0, filename, line);
		}
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::SetParameter
 *  機能概要：  LoadBaseConfigureFileから呼ばれます。各種パラメータを設定します
 * -----------------------------------------------------------------------
 */
bool CBasis::SetParameter(const aya::string_t &cmd, const aya::string_t &param, std::vector<CDic1> *dics)
{
	//include
	if(cmd == L"include") {
		aya::string_t param1, param2;
		Split(param, param1, param2, L",");

		aya::string_t filename = load_path + param1;

		char cset = setting_charset;
		if ( param2.size() ) {
			char cx = Ccct::CharsetTextToID(param2.c_str());
			if(cx != CHARSET_DEFAULT) {
				cset = cx;
			}
		}
		LoadBaseConfigureFile_Base(filename,*dics,cset);
		return true;
	}
	//includeEX
	else if(cmd == L"includeEX") {
		aya::string_t param1, param2;
		Split(param, param1, param2, L",");

		aya::string_t filename = load_path + param1;

		char cset = setting_charset;
		if ( param2.size() ) {
			char cx = Ccct::CharsetTextToID(param2.c_str());
			if(cx != CHARSET_DEFAULT) {
				cset = cx;
			}
		}

		aya::string_t load_path_bak = load_path;
		load_path = filename.substr(0,std::max(filename.rfind('/'),filename.rfind('\\')))+L'/';

		aya::string_t base_path_bak = base_path;
		base_path = load_path;

		LoadBaseConfigureFile_Base(filename,*dics,cset);

		load_path = load_path_bak;
		base_path = base_path_bak;

		return true;
	}
	// dic
	else if ( (cmd == L"dic" || cmd == L"dicif") && dics) {
		aya::string_t param1,param2;
		Split(param, param1, param2, L",");

		aya::string_t	filename = base_path + param1;

		char cset = dic_charset;
		if(param2.size()) {
			char cx = Ccct::CharsetTextToID(param2.c_str());
			if(cx != CHARSET_DEFAULT) {
				cset = cx;
			}
		}

		if ( cmd == L"dicif" ) {
			FILE *fp = aya::w_fopen(filename.c_str(), L"rb");
			if ( !fp ) {
				return true; //skip loading if file not exist
			}
			fclose(fp);
		}

		dics->emplace_back(filename,cset);
		return true;
	}
	// dicdir
	else if(cmd == L"dicdir" && dics) {
		aya::string_t param1,param2;
		Split(param, param1, param2, L",");

		//if the target folder has _loading_order.txt & not has param2 in this line, then includeEX this file
		if(param2.empty()) {
			aya::string_t file = param1 + L"/_loading_order.txt";
			aya::string_t filename = load_path + file;
			//_waccess is not use as it does not support mixed forward and back slashes
			if(FILE*tmp=aya::w_fopen(filename.c_str(), L"rb")) {
				fclose(tmp);
				return SetParameter(L"includeEX", file, dics);
			}
		}

		//else (loading_order not exist | param2 exist) include this folder
		{
			aya::string_t dirname = base_path + param1;
			CDirEnum	  ef(dirname);
			CDirEnumEntry entry;
			bool		  aret = true;
			
			while(ef.next(entry)) {
				//If the file suffix is bak or tmp, skip it.
				auto extbegpos=entry.name.rfind('.');
				if(extbegpos!=entry.name.npos) {
					auto ext=entry.name.substr(extbegpos+1);
					if(ext==L"bak" || ext==L"tmp") {
						continue;
					}
				}

				aya::string_t relpath_and_cs = param1 + L"\\" + entry.name + L',' + param2;

				if(entry.isdir) {
					aret &= SetParameter(L"dicdir", relpath_and_cs, dics);
				}
				else {
					aret &= SetParameter(L"dic", relpath_and_cs, dics);
				}
			}
			return aret;
		}
	}
	// messagetxt
	else if(cmd == L"messagetxt") {//本土化
		aya::string_t param1,param2;
		Split(param, param1, param2, L",");

		char cset = CHARSET_UTF8; //UTF8固定

		if(param2.size()) {
			char cx = Ccct::CharsetTextToID(param2.c_str());
			if(cx != CHARSET_DEFAULT) {
				cset = cx;
			}
		}
		if(ayamsg::LoadMessageFromTxt(load_path + param1,cset)) {
			messagetxt_path = load_path + param1;
		}
		return true;
	}
	// log
	else if(cmd == L"log") {
		if(param.empty()) {
			logpath.erase();
		}
		else {
			logpath = base_path + param;
		}
		return true;
	}
	// basepath
	else if(cmd == L"basepath") {
		CDirEnum dirCheck(param);
		CDirEnumEntry dirCheckTmp;

		if(dirCheck.next(dirCheckTmp)) { //something exist in directory
			#if defined(WIN32) || defined(_WIN32_WCE)
			if(param[1]==L':')
			#elif defined(POSIX)
			if(param[0]==L'/')
			#endif
				base_path = param;
			else
				base_path += param;

			//最後が\でも/でもなければ足す
			if(!base_path.length() || ((base_path[base_path.length()-1] != L'/') && (base_path[base_path.length()-1] != L'\\'))) {
				#if defined(WIN32) || defined(_WIN32_WCE)
				base_path += L"\\";
				#elif defined(POSIX)
				base_path += L'/';
				#endif
			}
			return true;
		}
		else{
			return false;
		}
	}
	// iolog
	else if(cmd == L"iolog") {
		iolog = param != L"off";
		return true;
	}
	// セーブデータ暗号化
	else if(cmd == L"save.encode") {
		encode_savefile = param == L"on";
		return true;
	}
	// 自動セーブ
	else if(cmd == L"save.auto") {
		auto_save = param != L"off";
		return true;
	}
	// charset
	else if(cmd == L"charset") {
		dic_charset       = Ccct::CharsetTextToID(param.c_str());
		setting_charset   = dic_charset;
		output_charset    = dic_charset;
		file_charset      = dic_charset;
		save_charset      = dic_charset;
		save_old_charset  = dic_charset;
		extension_charset = dic_charset;
		log_charset       = dic_charset;
		return true;
	}
	// charset
	else if(cmd == L"charset.dic") {
		dic_charset       = Ccct::CharsetTextToID(param.c_str());
		return true;
	}
	else if (cmd == L"charset.setting") {
		setting_charset   = Ccct::CharsetTextToID(param.c_str());
		return true;
	}
	else if (cmd == L"charset.output") {
		output_charset    = Ccct::CharsetTextToID(param.c_str());
		return true;
	}
	else if(cmd == L"charset.file") {
		file_charset      = Ccct::CharsetTextToID(param.c_str());
		return true;
	}
	else if(cmd == L"charset.save") {
		save_charset      = Ccct::CharsetTextToID(param.c_str());
		return true;
	}
	else if(cmd == L"charset.save.old") {
		save_old_charset  = Ccct::CharsetTextToID(param.c_str());
		return true;
	}
	else if(cmd == L"charset.extension") {
		extension_charset = Ccct::CharsetTextToID(param.c_str());
		return true;
	}
	else if ( cmd == L"charset.log" ) {
		log_charset = Ccct::CharsetTextToID(param.c_str());
		return true;
	}
	// fncdepth
	else if ( cmd == L"fncdepth" ) {
		size_t f_depth = (size_t)aya::ws_atoll(param, 10);
		vm.call_limit().SetMaxDepth((f_depth < 2 && f_depth != 0) ? 2 : f_depth);
		return true;
	}
	else if ( cmd == L"looplimit" ) {
		size_t loop_max = (size_t)aya::ws_atoll(param, 10);
		vm.call_limit().SetMaxLoop(loop_max);
		return true;
	}
	else if ( cmd == L"maxlognum" ) {
		size_t maxlognum = (size_t)aya::ws_atoll(param, 10);
		vm.logger().SetMaxLogNum(maxlognum);
		return true;
	}
	// iolog.filter.keyword (old syntax : ignoreiolog)
	else if(cmd == L"iolog.filter.keyword"){
		vm.logger().AddIologFilterKeyword(param);
		return true;
	}
	// iolog.filter.keyword.regex
	else if(cmd == L"iolog.filter.keyword.regex"){
		vm.logger().AddIologFilterKeywordRegex(param);
		return true;
	}
	// iolog.filter.keyword.delete (for SETSETTING)
	else if(cmd == L"iolog.filter.keyword.delete"){
		vm.logger().DeleteIologFilterKeyword(param);
		return true;
	}
	// iolog.filter.keyword.regex.delete (for SETSETTING)
	else if(cmd == L"iolog.filter.keyword.regex.delete"){
		vm.logger().DeleteIologFilterKeywordRegex(param);
		return true;
	}
	// iolog.filter.mode
	else if(cmd == L"iolog.filter.mode"){
		vm.logger().SetIologFilterMode(
			(param.find(L"white") != aya::string_t::npos) || (param.find(L"allow") != aya::string_t::npos));
		return true;
	}

	return false;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::GetParameter
 *  機能概要：  各種パラメータを文字列で返します
 * -----------------------------------------------------------------------
 */
static void CBasis_ConvertStringArray(const std::vector<aya::string_t> &array,CValue &var)
{
	var.array().clear();

	std::vector<aya::string_t>::const_iterator itr = array.begin();

	while (itr != array.end()) {
		var.array().emplace_back(*itr);
		++itr;
	}
}

CValue CBasis::GetParameter(const aya::string_t &cmd)
{
	// log
	if(cmd == L"log") {
		return logpath;
	}
	// iolog
	else if(cmd == L"iolog") {
		return aya::string_t(iolog ? L"on" : L"off");
	}
	// save.encode
	else if(cmd == L"save.encode") {
		return aya::string_t(encode_savefile ? L"on" : L"off");
	}
	// save.auto
	else if(cmd == L"save.auto") {
		return aya::string_t(auto_save ? L"on" : L"off");
	}
	// messagetxt
	else if(cmd == L"messagetxt") {
		return messagetxt_path;
	}
	// charset
	else if(cmd == L"charset") {
		return Ccct::CharsetIDToTextW(dic_charset);
	}
	// charset
	else if(cmd == L"charset.dic") {
		return Ccct::CharsetIDToTextW(dic_charset);
	}
	else if(cmd == L"charset.setting") {
		return Ccct::CharsetIDToTextW(setting_charset);
	}
	else if(cmd == L"charset.output") {
		return Ccct::CharsetIDToTextW(output_charset);
	}
	else if(cmd == L"charset.file") {
		return Ccct::CharsetIDToTextW(file_charset);
	}
	else if(cmd == L"charset.save") {
		return Ccct::CharsetIDToTextW(save_charset);
	}
	else if(cmd == L"charset.save.old") {
		return Ccct::CharsetIDToTextW(save_old_charset);
	}
	else if(cmd == L"charset.extension") {
		return Ccct::CharsetIDToTextW(extension_charset);
	}
	else if ( cmd == L"charset.log" ) {
		return Ccct::CharsetIDToTextW(log_charset);
	}
	// fncdepth
	else if ( cmd == L"fncdepth" ) {
		return CValue((aya::int_t)vm.call_limit().GetMaxDepth());
	}
	// looplimit
	else if ( cmd == L"looplimit" ) {
		return CValue((aya::int_t)vm.call_limit().GetMaxLoop());
	}
	// maxlognum
	else if ( cmd == L"maxlognum" ) {
		return CValue((aya::int_t)vm.logger().GetMaxLogNum());
	}
	// iolog.filter.keyword
	else if(cmd == L"iolog.filter.keyword"){
		CValue value(F_TAG_ARRAY, 0/*dmy*/);
		CBasis_ConvertStringArray(vm.logger().GetIologFilterKeyword(),value);
		return value;
	}
	// iolog.filter.keyword.regex
	else if(cmd == L"iolog.filter.keyword.regex"){
		CValue value(F_TAG_ARRAY, 0/*dmy*/);
		CBasis_ConvertStringArray(vm.logger().GetIologFilterKeywordRegex(),value);
		return value;
	}
	// iolog.filter.keyword.delete (for SETSETTING only)
	else if(cmd == L"iolog.filter.keyword.delete"){
		return aya::string_t(); //NOOP
	}
	// iolog.filter.keyword.regex.delete (for SETSETTING only)
	else if(cmd == L"iolog.filter.keyword.regex.delete"){
		return aya::string_t(); //NOOP
	}
	// iolog.filter.mode
	else if(cmd == L"iolog.filter.mode"){
		return vm.logger().GetIologFilterMode() ? L"allowlist" : L"denylist";
	}
	return aya::string_t();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::SaveVariable
 *  機能概要：  変数値をファイルに保存します
 *
 *  ファイル名は"DLL主ファイル名_variable.cfg"です。
 *  ファイルフォーマットは1行1変数、デリミタ半角カンマで、
 *
 *  変数名,内容,デリミタ
 *
 *  の形式で保存されます。内容は整数/実数の場合はそのまま、文字列ではダブルクォートされます。
 *  配列の場合は各要素間がコロンで分割されます。以下に要素数3、デリミタ"@"での例を示します。
 *
 *  var,1:"TEST":0.3,@
 *
 *  デリミタはダブルクォートされません。
 *
 *  基礎設定ファイルで設定した文字コードで保存されます。
 * -----------------------------------------------------------------------
 */
void	CBasis::SaveVariable(const aya::char_t* pName)
{
	// 変数の保存
	std::string old_locale = aya::get_safe_str(setlocale(LC_NUMERIC,NULL));
	setlocale(LC_NUMERIC,"English"); //小数点問題回避

	bool ayc = encode_savefile;

	// ファイルを開く
	aya::string_t	filename;
	if(! pName || ! *pName) {
		filename = GetSavefilePath();
	}
	else {
		filename = base_path + pName;
	}

	if(ayc) {
		char *s_filestr = Ccct::Ucs2ToMbcs(filename,CHARSET_DEFAULT);
#if defined(WIN32)
		DeleteFileA(s_filestr);
#else
	std::remove(s_filestr);
#endif
		free(s_filestr);
		s_filestr=0;

		filename += L".ays"; //aycだとかぶるので…
	}
	else {
		filename += L".ays"; //aycだとかぶるので…

		char *s_filestr = Ccct::Ucs2ToMbcs(filename,CHARSET_DEFAULT);
#if defined(WIN32)
		DeleteFileA(s_filestr);
#else
	std::remove(s_filestr);
#endif
		free(s_filestr);
		s_filestr=0;

		filename.erase(filename.size()-4,4);
	}

	vm.logger().Message(7);
	vm.logger().Filename(filename);
	FILE	*fp = aya::w_fopen(filename.c_str(), L"w");
	if(fp == NULL) {
		vm.logger().Error(E_E, 57, filename);
		return;
	}
/*
#if defined(WIN32)
	// UTF-8の場合は先頭にBOMを保存
	if(charset == CHARSET_UTF8)
		write_utf8bom(fp);
#endif
	// UTF-8なのにBOMを付けるのはやめた方が宜しいかと…
	// トラブルの原因になるので。

	// 了解です。外してしまいます。
	// メモ
	// UTF-8にはバイトオーダーによるバリエーションが存在しないのでBOMは必要ない。
	// 付与することは出来る。しかし対応していないソフトで読めなくなるので付けないほうが
	// 良い。
*/

	// 文字コード
	aya::string_t str;
	aya::string_t wstr;
	str.reserve(1000);

	str = L"//savefile_charset,";
	str += Ccct::CharsetIDToTextW(save_charset);
	str += L"\n";

	aya::ws_fputs(str,fp,save_charset,ayc);

	// 順次保存
	size_t	var_num = vm.variable().GetNumber();

	for(size_t i = 0; i < var_num; i++) {
		CVariable	*var = vm.variable().GetPtr(i);
		// Voidは放置
		if(var->value_const().IsVoid())
			continue;
		// 内容が空文字列の変数は保存しない
		//if(var->value_const().IsStringReal() && !var->value_const().s_value.size())
		//	continue;
		// 内容が空汎用配列の変数は保存しない
		//if(var->value_const().GetType() == F_TAG_ARRAY && !var->value_const().array_size())
		//	continue;
		// 消去フラグが立っている変数は保存しない
		if(var->IsErased())
			continue;

		// 名前の保存
		str = var->name;
		str += L',';

		// 値の保存
		switch(var->value_const().GetType()) {
		case F_TAG_INT:
			str += aya::ws_lltoa(var->value_const().i_value);
			str += L',';
			break;
		case F_TAG_DOUBLE:
			str += aya::ws_ftoa(var->value_const().d_value);
			str += L',';
			break;
		case F_TAG_STRING:
			wstr = var->value_const().s_value;
			EscapeString(wstr);
			str += L"\"";
			str += wstr;
			str += L"\",";
			break;
		case F_TAG_ARRAY:
			if(!var->value_const().array_size()) {
				str += ESC_IARRAY L":" ESC_IARRAY;
			}
			else {
				CValueArray::const_iterator	itv;
				CValueArray::const_iterator	itvbegin = var->value_const().array().begin();

				for(itv = itvbegin; itv != var->value_const().array().end(); itv++) {
					if(itv != itvbegin)
						str += L':';
					wstr = itv->GetValueString();
					EscapeString(wstr);

					if(itv->GetType() == F_TAG_STRING) {
						str += L"\"";
						str += wstr;
						str += L"\"";
					}
					else if(itv->GetType() == F_TAG_VOID) {
						str += ESC_IVOID;
					}
					else {
						str += wstr;
					}
				}
				if(var->value_const().array_size() == 1) {
					str += L":" ESC_IARRAY;
				}
			}
			str += L',';
			break;
		case F_TAG_HASH:
			if (!var->value_const().hash_size()) {
				str += ESC_IHASH L"=" ESC_IHASH L":" ESC_IHASH L"=" ESC_IHASH;
			}
			else {
				CValueHash::const_iterator	itv;
				CValueHash::const_iterator	itvbegin = var->value_const().hash().begin();

				for(itv = itvbegin; itv != var->value_const().hash().end(); itv++) {
					if (itv != itvbegin)
						str += L":";

					wstr = itv->first.GetValueString();
					EscapeString(wstr);

					if (itv->first.GetType() == F_TAG_STRING) {
						str += L"\"";
						str += wstr;
						str += L"\"";
					}
					else if (itv->first.GetType() == F_TAG_VOID) {
						str += ESC_IVOID;
					}
					else {
						str += wstr;
					}

					str += L"=";

					wstr = itv->second.GetValueString();
					EscapeString(wstr);

					if (itv->second.GetType() == F_TAG_STRING) {
						str += L"\"";
						str += wstr;
						str += L"\"";
					}
					else if (itv->second.GetType() == F_TAG_VOID) {
						str += ESC_IVOID;
					}
					else {
						str += wstr;
					}
				}
				if (var->value_const().hash_size() == 1) {
					str += L":" ESC_IHASH L"=" ESC_IHASH;
				}
			}
			str += L",";
			break;
		default:
			vm.logger().Error(E_W, 7, var->name);
			break;
		};
		// デリミタの保存
		wstr = var->delimiter;
		EscapeString(wstr);
		str += L"\"";
		str += wstr;
		str += L"\",";
		wstr = var->watcher+L'|'+var->setter+L'|'+var->destorier;
		if(wstr!=L"||")
			str += wstr;

		str += L"\n";

		aya::ws_fputs(str,fp,save_charset,ayc);
	}

	// ファイルを閉じる
	fclose(fp);

	// 小数点問題修正を戻す
	setlocale(LC_NUMERIC,old_locale.c_str());

	vm.logger().Message(8);
}
/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::RestoreVariable
 *  機能概要：  前回保存した変数内容を復元します
 * -----------------------------------------------------------------------
 */
void	CBasis::RestoreVariable(const aya::char_t* pName)
{
	std::string old_locale = aya::get_safe_str(setlocale(LC_NUMERIC,NULL));
	setlocale(LC_NUMERIC,"English"); //小数点問題回避

	bool ayc = encode_savefile;

	// ファイルを開く
	aya::string_t	filename;
	if(! pName || ! *pName) {
		filename = GetSavefilePath();
	}
	else {
		filename = base_path + pName;
	}

	vm.logger().Message(6);
	vm.logger().Filename(filename);

	FILE *fp = NULL;

	//暗号化セーブファイル対応
	if(ayc) {
		filename += L".ays";
		fp = aya::w_fopen(filename.c_str(), L"r");
		if(!fp) {
			filename.erase(filename.size()-4,4);
			fp = aya::w_fopen(filename.c_str(), L"r");
			if(!fp) {
				vm.logger().Error(E_N, 0);
				return;
			}
			else {
				ayc = false;
			}
		}
	}
	else {
		fp = aya::w_fopen(filename.c_str(), L"r");
		if(!fp) {
			filename += L".ays";
			fp = aya::w_fopen(filename.c_str(), L"r");
			if(!fp) {
				vm.logger().Error(E_N, 0);
				return;
			}
			else {
				ayc = true;
				encode_savefile = true; //簡単に戻されてしまうのを防止
			}
		}
	}

	// 内容を読み取り、順次復元していく
	aya::string_t	linebuffer;
	linebuffer.reserve(2000);

	aya::string_t	readline;
	readline.reserve(1000);
	
	aya::string_t	parseline;
	aya::string_t	varname, value, delimiter, watcher, setter, destorier;

	char savefile_charset = save_old_charset;
	std::string buf;
	buf.reserve(1000);

	for (int i = 1; ; i++) {
		watcher.clear(), setter.clear(), destorier.clear();
		// 1行読み込み
		if (aya::ws_fgets(buf, readline, fp, savefile_charset, ayc, i, false) == aya::WS_EOF)
			break;
		// 改行は消去
		CutCrLf(readline);
		// 空行なら次の行へ
		if(!readline.size()) {
			linebuffer.erase();
			continue;
		}
		// 既に読み取り済の文字列と結合
		linebuffer += readline;
		// ダブルクォーテーションが閉じているか確認する。閉じていない場合は、
		// 次の行へ値が続いていると思われるので次の行の読み取りへ
		if(IsInDQ(linebuffer, 0, linebuffer.size() - 1) != IsInDQ_notindq)
			continue;

		parseline = linebuffer;
		linebuffer.erase();

		// 変数名を取得
		if(!Split_IgnoreDQ(parseline, varname, value, L",")) {
			vm.logger().Error(E_W, 1, filename, i);
			continue;
		}
		// Charset
		if(varname == L"//savefile_charset") {
			savefile_charset = Ccct::CharsetTextToID(value.c_str());
			continue;
		}
		// 変数名の正当性を検査
		if(IsLegalVariableName(varname)) {
			vm.logger().Error(E_W, 2, filename, i);
			continue;
		}
		// 値とデリミタを取り出す
		parseline = value;
		if(!Split_IgnoreDQ(parseline, value, delimiter, L",")) {
			vm.logger().Error(E_W, 3, filename, i);
			continue;
		}
		//
		parseline = delimiter;
		Split_IgnoreDQ(parseline, delimiter, aya::string_t(), L",");
		if (!IsLegalStrLiteral(delimiter)){
			delimiter = parseline;
			if (Split_IgnoreDQ(parseline, delimiter, watcher, L",")) {
				parseline = watcher;
				if (Split_IgnoreDQ(parseline, watcher, aya::string_t(), L","))//将来のバージョンで得られる可能性のある追加情報の破棄
					vm.logger().Error(E_W,23);
				parseline = watcher;
				Split_IgnoreDQ(parseline, watcher, setter, L"|");
				parseline = setter;
				Split_IgnoreDQ(parseline, setter, destorier, L"|");
			}
			CutDoubleQuote(delimiter);
			UnescapeString(delimiter);
		}
		else
			delimiter = parseline;
		// 値をチェックして型を判定
		int	type;

		if(IsIntString(value)) {
			type = F_TAG_INT;
		}
		else if(IsDoubleButNotIntString(value)) {
			type = F_TAG_DOUBLE;
		}
		else if(!IsLegalStrLiteral(value)) {
			type = F_TAG_STRING;
		}
		else {
			if(Find_IgnoreDQ(value,L":") == aya::string_t::npos) {
				vm.logger().Error(E_W, 4, filename, i);
				continue;
			}
			else {
				type = F_TAG_ARRAY;
				if (Find_IgnoreDQ(value,L"=") != aya::string_t::npos) {
					type = F_TAG_HASH;
				}
			}
		}
		// デリミタの正当性を検査
		if(!delimiter.size()) {
			vm.logger().Error(E_W, 5, filename, i);
			continue;
		}
		// 変数を作成
		int	index = vm.variable().Make(varname, 0);
		vm.variable().SetType(index, type);

		if(type == F_TAG_INT) {
			// 整数型
			vm.variable().SetValue(index, aya::ws_atoll(value));
		}
		else if(type == F_TAG_DOUBLE) {
			// 実数型
			vm.variable().SetValue(index, aya::ws_atof(value));
		}
		else if(type == F_TAG_STRING) {
			// 文字列型
			CutDoubleQuote(value);
			UnescapeString(value);
			vm.variable().SetValue(index, value);
		}
		else if(type == F_TAG_ARRAY) {
			// 配列型
			RestoreArrayVariable(*(vm.variable().GetValuePtr(index)), value);
		}
		else if (type == F_TAG_HASH) {
			// �A�z�z��^
			RestoreHashVariable(*(vm.variable().GetValuePtr(index)), value);
		}
		else {
			vm.logger().Error(E_W, 6, filename, i);
			continue;
		}
		vm.variable().SetDelimiter(index, delimiter);
		CVariable& v = *vm.variable().GetPtr(index);
		v.set_watcher(watcher);
		v.set_destorier(destorier);
		v.set_setter(setter);
	}

	// ファイルを閉じる
	fclose(fp);

	setlocale(LC_NUMERIC,old_locale.c_str());

	vm.logger().Message(8);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::RestoreArrayVariable
 *  機能概要：  RestoreVariableから呼ばれます。配列変数の内容を復元します
 * -----------------------------------------------------------------------
 */
void	CBasis::RestoreArrayVariable(CValue &var, aya::string_t &value)
{
	var.array().clear();

	aya::string_t	par, remain;
	char splitResult;

	for(; ;) {
		splitResult = Split_IgnoreDQ(value, par, remain, L":");
		if(!splitResult) {
			par = value;
		}

		if(par != ESC_IARRAY) {
			if(par == ESC_IVOID) {
				var.array().emplace_back();
			}
			else if(IsIntString(par)) {
				var.array().emplace_back(aya::ws_atoll(par, 10));
			}
			else if(IsDoubleButNotIntString(par)) {
				var.array().emplace_back(aya::ws_atof(par));
			}
			else {
				CutDoubleQuote(par);
				UnescapeString(par);
				var.array().emplace_back(par);
			}
		}

		if(!splitResult) {
			break;
		}
		value = remain;
	}
}


/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::RestoreHashVariable
 *  �@�\�T�v�F  RestoreVariable����Ă΂�܂��B�z��ϐ��̓�e�𕜌����܂�
 * -----------------------------------------------------------------------
 */
void	CBasis::RestoreHashVariable(CValue &var, aya::string_t &value)
{
	var.hash().clear();

	aya::string_t	par, remain, key, key_value;
	char splitResult;

	for( ; ; ) {
		splitResult = Split_IgnoreDQ(value, par, remain, L":");
		if (!splitResult) {
			par = value;
		}

		if ( Split_IgnoreDQ(par, key, key_value, L"=") ) {
			if (key.compare(ESC_IHASH) != 0) {
				std::pair<CValue,CValue> kv;

				if (key.compare(ESC_IVOID) == 0) {
					kv.first = CValue();
				}
				else if (IsIntString(key)) {
					kv.first = CValue( aya::ws_atoi(key, 10) );
				}
				else if (IsDoubleButNotIntString(key)) {
					kv.first = CValue( aya::ws_atof(key) );
				}
				else {
					CutDoubleQuote(key);
					UnescapeString(key);
					kv.first = CValue(key);
				}

				if (key_value.compare(ESC_IVOID) == 0) {
					kv.second = CValue();
				}
				if (IsIntString(key_value)) {
					kv.second = CValue( aya::ws_atoi(key_value, 10) );
				}
				else if (IsDoubleButNotIntString(key_value)) {
					kv.second = CValue( aya::ws_atof(key_value) );
				}
				else {
					CutDoubleQuote(key_value);
					UnescapeString(key_value);
					kv.second = CValue(key_value);
				}

				var.hash().insert(kv);
			}
		}

		if (!splitResult) {
			break;
		}
		value = remain;
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::ExecuteLoad
 *  機能概要：  load関数を実行します
 * -----------------------------------------------------------------------
 */
void	CBasis::ExecuteLoad(void)
{
	if(IsSuppress() || loadindex.IsNotFound()) {
		return;
	}

	ptrdiff_t funcpos = loadindex.Find(vm,L"load");
	if(funcpos < 0) {
		return;
	}

	// 第一引数（dllのパス）を作成
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
	arg.array().emplace_back(base_path);

	// 実行　結果は使用しないのでそのまま捨てる
	vm.call_limit().InitCall();
	CLocalVariable	lvar(vm);
	vm.logger().Io(0, base_path);

	vm.function_exec().func[funcpos].Execute(arg, lvar);
	aya::string_t empty;
	vm.logger().Io(1, empty);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::ExecuteRequest
 *  機能概要：  request関数を実行します
 * -----------------------------------------------------------------------
 */
#if defined(WIN32) || defined(_WIN32_WCE)
aya::global_t	CBasis::ExecuteRequest(aya::global_t h, long *len, bool is_debug)
{
	if(IsSuppress() || requestindex.IsNotFound()) {
		GlobalFree(h);
		h = NULL;
		*len = 0;
		return NULL;
	}

	ptrdiff_t funcpos = requestindex.Find(vm,L"request");

	if(funcpos < 0) {
		GlobalFree(h);
		h = NULL;
		*len = 0;
		return NULL;
	}

	// 入力文字列を取得
	std::string	istr;
	istr.assign((char *)h, 0, (size_t)*len);

	// 第一引数（入力文字列）を作成　ここで文字コードをUCS-2へ変換
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
	wchar_t	*wistr = Ccct::MbcsToUcs2(istr, output_charset);

	if (wistr != NULL) {
		vm.logger().Io(0, wistr);
		arg.array().emplace_back(wistr);
		free(wistr);
		wistr = NULL;
	}
	else {
		vm.logger().Io(0, L"");
	}

	// 実行
	vm.call_limit().InitCall();
	CValue	result;
	try{
		CLocalVariable	lvar(vm);
		result = vm.function_exec().func[funcpos].Execute(arg, lvar);
	}
	catch (const aya::memory_error&) {
		if(vm.call_limit().StackCall().size()>512)
			CallOnMemoryLimit();
		else
			CallOnMemoryError();
	}
	catch (const std::bad_alloc&) {
		CallOnMemoryLimit();
	}

	// 結果を文字列として取得し、文字コードをMBCSに変換
	aya::string_t	res = result.GetValueString();
	vm.logger().Io(1, res);
	char	*mostr = Ccct::Ucs2ToMbcs(res, output_charset);
	if(mostr == NULL) {
		// 文字コード変換失敗、NULLを返す
		*len = 0;
		GlobalFree(h);
		h = NULL;
		return NULL;
	}

	// 文字コード変換が成功したので、結果をGMEMへコピーして返す
	unsigned int oldlen = *len;
	*len = (long)strlen(mostr);
	unsigned int copylen = *len + 1;

	//ゼロ終端も含む……互換処理のため
	HGLOBAL r_h = NULL;
	if(oldlen >= copylen) {
		r_h = h;
	}
	else {
		GlobalFree(h);
		h = NULL;
		r_h = ::GlobalAlloc(GMEM_FIXED,copylen);
	}

	if(r_h) {
		memcpy(r_h, mostr, copylen);
	}
	free(mostr);
	mostr = NULL;
	return r_h;
}
void CBasis::CallOnMemoryLimit()
{
	CBasisFuncPos shiori_OnMemoryLimit;
	ptrdiff_t funcpos = shiori_OnMemoryLimit.Find(vm, L"shiori.OnMemoryLimit");
	size_t lock = vm.call_limit().temp_unlock();

	if (funcpos >= 0) {
		vm.function_exec().func[funcpos].Execute();//get info from GETCALLSTACK
	}
	else {
		vm.logger().Error(E_E,99);
	}

	vm.call_limit().reset_lock(lock);
}
void CBasis::CallOnMemoryError()
{
	CBasisFuncPos shiori_OnMemoryError;
	ptrdiff_t funcpos = shiori_OnMemoryError.Find(vm, L"shiori.OnMemoryError");
	size_t lock = vm.call_limit().temp_unlock();

	if (funcpos >= 0) {
		vm.function_exec().func[funcpos].Execute();//get info from GETCALLSTACK
	}
	else {
		vm.logger().Error(E_E,100);
	}

	vm.call_limit().reset_lock(lock);
}
#elif defined(POSIX)
aya::global_t	CBasis::ExecuteRequest(aya::global_t h, long *len, bool is_debug)
{
	if(IsSuppress() || requestindex.IsNotFound()) {
		free(h);
		h = NULL;
		*len = 0;
		return NULL;
	}

	int funcpos = requestindex.Find(vm,L"request");

	if(funcpos < 0) {
		free(h);
		h = NULL;
		*len = 0;
		return NULL;
	}

	// 入力文字列を取得
	std::string istr(h, *len);
	// 第一引数（入力文字列）を作成　ここで文字コードをUCS-2へ変換
	CValue arg(F_TAG_ARRAY, 0/*dmy*/);
	wchar_t *wistr = Ccct::MbcsToUcs2(istr, output_charset);

	if (wistr != NULL) {
		vm.logger().Io(0, wistr);
		arg.array().emplace_back(wistr);
		free(wistr);
		wistr = NULL;
	}
	else {
		aya::string_t empty;
		vm.logger().Io(0, empty);
	}

	// 実行
	vm.call_limit().InitCall();
	CLocalVariable	lvar;

	CValue	result;
	vm.function()[funcpos].Execute(result, arg, lvar);

	// 結果を文字列として取得し、文字コードをMBCSに変換
	aya::string_t	res = result.GetValueString();
	vm.logger().Io(1, res);
	char *mostr = Ccct::Ucs2ToMbcs(res, output_charset);

	if(mostr == NULL) {
		// 文字コード変換失敗、NULLを返す
		*len = 0;
		return NULL;
	}

	// 文字コード変換が成功したので、結果をGMEMへコピーして返す
	*len = (long)strlen(mostr);
	char* r_h = static_cast<char*>(malloc(*len));
	memcpy(r_h, mostr, *len);
	free(mostr);
	mostr = NULL;
	return r_h;
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::ExecuteUnload
 *  機能概要：  unload関数を実行します
 * -----------------------------------------------------------------------
 */
void	CBasis::ExecuteUnload(void)
{
	if(IsSuppress() || unloadindex.IsNotFound()) {
		return;
	}

	ptrdiff_t funcpos = unloadindex.Find(vm,L"unload");
	if(funcpos < 0) {
		return;
	}

	// 実行　引数無し　結果は使用しないのでそのまま捨てる
	vm.call_limit().InitCall();
	aya::string_t empty;
	vm.logger().Io(0, empty);
	vm.function_exec().func[funcpos].Execute();
	vm.logger().Io(1, empty);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasisFuncPos::CBasisFuncPos
 *  機能概要：  関数位置を探し、位置と「探したかどうか」をキャッシュします
 * -----------------------------------------------------------------------
 */
ptrdiff_t CBasisFuncPos::Find(CAyaVM &vm,const aya::char_t *name)
{
	if(is_try_find) {
		return pos_saved;
	}

	pos_saved = vm.function_exec().GetFunctionIndexFromName(name);
	is_try_find = true;

	if(pos_saved < 0) {
		vm.logger().Error(E_E, 32, name);
	}

	return pos_saved;
}

