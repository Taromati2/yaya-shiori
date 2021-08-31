// 
// AYA version 5
//
// ログメッセージ
// written by umeici. 2004
// 

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
#endif

#include "messages.h"

#include "globaldef.h"
#include "wsex.h"
#include "misc.h"

//////////DEBUG/////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
////////////////////////////////////////

static void ClearMessageArrays()
{
	ayamsg::msgf.clear();
	ayamsg::msge.clear();
	ayamsg::msgw.clear();
	ayamsg::msgn.clear();
	ayamsg::msgj.clear();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  ayamsg::IsEmpty
 *  機能概要：  エラーメッセージテーブルが空かどうかを判定します
 * -----------------------------------------------------------------------
 */
bool ayamsg::IsEmpty(void)
{
	return ayamsg::msgf.empty() && ayamsg::msge.empty() && ayamsg::msgw.empty();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  ayamsg::LoadMessageFromTxt
 *  機能概要：  テキストファイルからエラーメッセージテーブルに読み込みます
 * -----------------------------------------------------------------------
 */
bool ayamsg::LoadMessageFromTxt(const aya::string_t &file,char cset)
{
	FILE *fp = aya::w_fopen(file.c_str(), L"rb");

	if(fp == NULL) {
		return false;
	}

	MessageArray *ptr = NULL;
	aya::string_t line;

	ClearMessageArrays();

	while ( true )
	{
		if(aya::ws_fgets(line, fp, cset, 0 /*no_enc*/, 1 /*skip_bom*/, 1 /*cut_heading_space*/) == aya::WS_EOF) {
			break;
		}

		CutCrLf(line);

		if( line.substr(0,3)==L"!!!" ) {
			aya::string_t&type = line.substr(3);

			if( type == L"msgf" ) {
				ptr = &msgf;
			}
			else if( type == L"msge" ) {
				ptr = &msge;
			}
			else if( type == L"msgw" ) {
				ptr = &msgw;
			}
			else if( type == L"msgn" ) {
				ptr = &msgn;
			}
			else if( type == L"msgj" ) {
				ptr = &msgj;
			}
			else {
				ptr = NULL;
			}
			continue;
		}
		
		if( line.substr(0,2)==L"//" ) {
			continue;
		}
		
		if( line.substr(0,1)==L"*" ) {
			if( ptr ) {
				line=line.substr(1);
				aya::ws_replace(line,L"\\n", L"\r\n");
				ptr->push_back(line);
			}
			continue;
		}
	}

	fclose(fp);

	return true;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  ayamsg::GetTextFromTable
 *  機能概要：  エラーメッセージテーブルから文字列を抜き出します。
 * -----------------------------------------------------------------------
 */
const aya::string_t ayamsg::GetTextFromTable(int mode,int id)
{
	ayamsg::MessageArray *ptr;
	aya::char_t *emsg;

	if(mode == E_F) {
		ptr = &ayamsg::msgf;
		emsg = L"fatal F";
	}
	else if(mode == E_E) {
		ptr = &ayamsg::msge;
		emsg = L"error E";
	}
	else if(mode == E_W) {
		ptr = &ayamsg::msgw;
		emsg = L"warning W";
	}
	else if(mode == E_N) {
		ptr = &ayamsg::msgn;
		emsg = L"note N";
	}
	else {
		ptr = &ayamsg::msgj;
		emsg = L"//msg M";
	}

	if( id < 0 || ptr->size() <= static_cast<size_t>(id) ) { //catch overflow
		aya::char_t buf[64] = L"";
		aya::snprintf(buf,63,L"%04d",id);

		return aya::string_t(emsg) + buf + L" : (please specify messagetxt)\r\n";
	}
	else {
		return (*ptr)[id];
	}
}

namespace ayamsg {

	// fatal
	MessageArray msgf;

	// error
	MessageArray msge;

	// warning
	MessageArray msgw;

	// note
	MessageArray msgn;

	// other
	MessageArray msgj;

}
