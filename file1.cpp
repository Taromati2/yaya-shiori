﻿// 
// AYA version 5
//
// 1つのファイルを扱うクラス　CFile1
// written by umeici. 2004
// 

#if defined(WIN32) || defined(_WIN32_WCE)
#  include "stdafx.h"
#endif

#include <string.h>

#include "ccct.h"
#include "file.h"
#include "manifest.h"
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

#ifdef POSIX
#define wcsicmp wcscasecmp
#endif

#ifdef INT64_IS_NOT_STD
extern "C" {
__int64 __cdecl _ftelli64(FILE *);
int __cdecl _fseeki64(FILE *, __int64, int);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::Open
 *  機能概要：  ファイルをオープンします
 *
 *  返値　　：　0/1=失敗/成功(既にロードされている含む)
 * -----------------------------------------------------------------------
 */
bool CFile1::Open(void) {
	if(fp != NULL)
		return 1;

	fp = aya::w_fopen(name.c_str(), (wchar_t *)mode.c_str());

	if( ! fp ) {
		size = 0;
		return 0;
	}

#ifdef POSIX
	aya::int_t cur = ftello(fp);
	fseeko(fp, 0, SEEK_SET);
	aya::int_t start = ftello(fp);
	fseeko(fp, 0, SEEK_END);
	aya::int_t end = ftello(fp);
	fseeko(fp, cur, SEEK_SET);
#else
	aya::int_t cur = _ftelli64(fp);
	_fseeki64(fp,0,SEEK_SET);
	aya::int_t start = _ftelli64(fp);
	_fseeki64(fp,0,SEEK_END);
	aya::int_t end = _ftelli64(fp);
	_fseeki64(fp,cur,SEEK_SET);
#endif

	size = end-start;
	
	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::Close
 *  機能概要：  ファイルをクローズします
 *
 *  返値　　：　1/2=成功/ロードされていない、もしくは既にunloadされている
 * -----------------------------------------------------------------------
 */
int	CFile1::Close(void)
{
	if(fp) {
		fclose(fp);
		fp = NULL;
		return 1;
	}
	else {
		return 2;
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::Write
 *  機能概要：  ファイルに文字列を書き込みます
 *
 *  返値　　：　0/1=失敗/成功
 * -----------------------------------------------------------------------
 */
bool CFile1::Write(const aya::string_t &istr) {
	if(fp == NULL)
		return 0;

	// 文字列をマルチバイト文字コードに変換
	char	*t_istr = Ccct::Ucs2ToMbcs(istr, charset);
	if(t_istr == NULL)
		return 0;

	long	len = (long)strlen(t_istr);

	// 書き込み
	fwrite(t_istr, sizeof(char), len, fp);
	free(t_istr);
	t_istr = NULL;

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::WriteBin
 *  機能概要：  ファイルにバイナリデータを書き込みます
 *
 *  返値　　：　0/1=失敗/成功
 * -----------------------------------------------------------------------
 */
bool CFile1::WriteBin(const aya::string_t &istr, const aya::char_t alt) {
	if(fp == NULL)
		return 0;

	size_t len = istr.size();

	unsigned char *t_istr = reinterpret_cast<unsigned char*>(malloc(len+1));
	if(t_istr == NULL)
		return 0;
	t_istr[len] = 0; //念のためゼロ終端（いらない）
	
	//altを0に置き換えつつデータ構築
	for ( size_t i = 0 ; i < len ; ++i ) {
		if( istr[i] == alt ) {
			t_istr[i] = 0;
		}
		else {
			t_istr[i] = static_cast<unsigned char>(istr[i]);
		}
	}

	// 書き込み
	size_t write = fwrite(t_istr, sizeof(unsigned char), len, fp);
	free(t_istr);
	t_istr = NULL;

	return write;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::WriteDecode
 *  機能概要：  ファイルにバイナリデータをデコードしながら書き込みます
 *
 *  返値　　：　0/1=失敗/成功
 * -----------------------------------------------------------------------
 */
bool CFile1::WriteDecode(const aya::string_t &istr, const aya::string_t &type) {
	if(fp == NULL)
		return 0;

	std::string out;

	if( wcsicmp(type.c_str(),L"base64") == 0 ) {
		DecodeBase64(out,istr.c_str(),istr.length());
	}
	else if( wcsicmp(type.c_str(),L"form") == 0 ) {
		DecodeURL(out,istr.c_str(),istr.length(),true);
	}
	else {
		DecodeURL(out,istr.c_str(),istr.length(),false);
	}

	// 書き込み
	size_t write = fwrite(out.c_str(), sizeof(unsigned char), out.length(), fp);

	return write;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::Read
 *  機能概要：  ファイルから文字列を1行読み取ります
 *
 *  返値　　：　-1/0/1=EOF/失敗/成功
 * -----------------------------------------------------------------------
 */
int	CFile1::Read(aya::string_t &ostr)
{
	ostr.clear();

	if(fp == NULL)
		return 0;

	if(aya::ws_fgets(ostr, fp, charset, 0, bomcheck, false) == aya::WS_EOF)
		return -1;

	bomcheck++;

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::ReadBin
 *  機能概要：  ファイルからバイナリデータを読み取ります
 *
 *  返値　　：　-1/0/1=EOF/失敗/成功
 * -----------------------------------------------------------------------
 */
int	CFile1::ReadBin(aya::string_t &ostr, size_t len, aya::char_t alt)
{
	ostr.clear();

	if(fp == NULL)
		return 0;

	if(len<1){ //0=デフォルトサイズ指定
		len = (size_t)size;
	}

	char f_buffer[1024];
	size_t read = 0;

	while ( true ) {
		size_t lenread = len - read;
		if( lenread > sizeof(f_buffer) ) {
			lenread = sizeof(f_buffer);
		}

		size_t done = fread(f_buffer,1,lenread,fp);
		if( ! done ) {
			break;
		}

		for ( size_t i = 0 ; i < done ; ++i ) {
			if( f_buffer[i] == 0 ) {
				ostr.append(1,alt);
			}
			else {
				ostr.append(1,static_cast<aya::char_t>(static_cast<unsigned char>(f_buffer[i])));
			}
		}

		read += done;
		if( done < lenread ) { break; }
	}

	return read;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::ReadEncode
 *  機能概要：  ファイルからバイナリデータをエンコードして読み取ります
 *
 *  返値　　：　-1/0/1=EOF/失敗/成功
 * -----------------------------------------------------------------------
 */
int	CFile1::ReadEncode(aya::string_t &ostr, size_t len, const aya::string_t &type)
{
	ostr.clear();

	if(fp == NULL)
		return 0;

	if(len<1){ //0=デフォルトサイズ指定
		len = (size_t)size;
	}

	char f_buffer[3*3*3*3*3*3*3]; //3の倍数にすること base64対策
	size_t read = 0;

	aya::string_t s;
	int enc_type = 0;
	if( wcsicmp(type.c_str(),L"base64") == 0 ) {
		enc_type = 1;
	}
	else if( wcsicmp(type.c_str(),L"form") == 0 ) {
		enc_type = 2;
	}

	while ( true ) {
		size_t lenread = len - read;
		if( lenread > sizeof(f_buffer) ) {
			lenread = sizeof(f_buffer);
		}

		size_t done = fread(f_buffer,1,lenread,fp);
		if( ! done ) {
			break;
		}

		s.erase();
		if( enc_type == 1 ) { //b64
			EncodeBase64(s,f_buffer,done);
		}
		else if( enc_type == 2 ) { //form
			EncodeURL(s,f_buffer,done,true);
		}
		else {
			EncodeURL(s,f_buffer,done,false);
		}
		ostr += s;

		read += done;
		if( done < lenread ) { break; }
	}

	return read;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::FSeek
 *  機能概要：  Cライブラリfseek同等
 *  返値　　：　0/1=失敗/成功
 * -----------------------------------------------------------------------
 */
bool CFile1::FSeek(aya::int_t offset, int origin) {
	if (fp == NULL)
		return 0;

#ifdef POSIX
	aya::int_t result = fseeko(fp, offset, origin);
#else
	aya::int_t result=::_fseeki64(fp,offset,origin);
#endif

	if(result!=0){
		return 0;
	}else{
		return 1;
	}
}


/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::FTell
 *  機能概要：  Cライブラリftell同等
 *  返値　　：　-1/その他=失敗/成功（ftellの結果）
 * -----------------------------------------------------------------------
 */
aya::int_t CFile1::FTell(){
	if (fp == NULL)
		return -1;

#ifdef POSIX
	aya::int_t result = ftello(fp);
#else
	aya::int_t result=::_ftelli64(fp);
#endif

	if(result<0){
		return -1;
	}else{
		return result;
	}
}

