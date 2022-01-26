﻿// 
// AYA version 5
//
// written by umeici. 2004
// 

#if defined(WIN32) || defined(_WIN32_WCE)
#include "stdafx.h"
#include <io.h>
#include <fcntl.h>
#endif

#include <vector>
#include <ctime>
#include <locale>
#include <clocale>
#include <locale>
#include <stdio.h>

#include "aya5.h"
#include "basis.h"
#include "ayavm.h"
#include "ccct.h"
#include "manifest.h"
#include "messages.h"

class CAyaVMWrapper;

static CAyaVMWrapper* vm=NULL;
static aya::string_t modulename;
static void (*loghandler)(const aya::char_t *str, int mode, int id)=NULL;
static long logsend_hwnd = 0;

//////////DEBUG/////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
////////////////////////////////////////

class CAyaVMWrapper {
private:
	CAyaVM *vm;

public:
	CAyaVMWrapper(const aya::string_t &path, aya::global_t h, long len) {
		vm = new CAyaVM();

		vm->logger().Set_loghandler(loghandler);
		if (logsend_hwnd != 0) {
			SetLogRcvWnd(logsend_hwnd);
			logsend_hwnd = 0;
		}

		vm->basis().SetModuleName(modulename,L"",L"normal");

		vm->load();

		vm->basis().SetPath(h, len);
		vm->basis().Configure();

		if( vm->basis().IsSuppress() ) {
			vm->logger().Message(10,E_E);

			CAyaVM *vme = new CAyaVM();

			vm->logger().Set_loghandler(loghandler);

			vme->basis().SetModuleName(modulename,L"_emerg",L"emergency");

			vme->load();

			vme->basis().SetPath(h, len);
			vme->basis().Configure();

			vme->logger().Message(11,E_E);

			if( ! vme->basis().IsSuppress() ) {
				vme->logger().AppendErrorLogHistoryToBegin(std::move(vm->logger().GetErrorLogHistory())); //エラーログを引き継ぐ

				std::swap(vm, vme);
			}
			delete vme;
		}
		vm->basis().ExecuteLoad();
	}
	virtual ~CAyaVMWrapper() {
		vm->basis().Termination();

		vm->unload();

		delete vm;
	}

	void Set_loghandler(void (*loghandler)(const aya::char_t *str, int mode, int id)){
		vm->logger().Set_loghandler(loghandler);
	}

	bool IsSuppress(void) {
		if( ! vm ) { return true; }
		return vm->basis().IsSuppress() != 0;
	}
	bool IsEmergency(void) {
		if( ! vm ) { return false; }
		return !wcscmp(vm->basis().GetModeName(),L"emergency");
	}

	aya::global_t ExecuteRequest(aya::global_t h, long *len, bool is_debug)
	{
		if( ! vm ) { return NULL; }
		
		vm->request_before();

		aya::global_t r = vm->basis().ExecuteRequest(h,len,is_debug);

		vm->request_after();

		return r;
	}

	void SetLogRcvWnd(long hwnd)
	{
		if( ! vm ) { return; }
		vm->basis().SetLogRcvWnd(hwnd);
	}

};

/* -----------------------------------------------------------------------
 *  DllMain
 * -----------------------------------------------------------------------
 */
#if defined(WIN32)

static void AYA_InitModule(HMODULE hModule)
{
	char path[MAX_PATH] = "";
	GetModuleFileName(hModule, path, sizeof(path));
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_splitpath(path, drive, dir, fname, ext);
	std::string	mbmodulename = fname;

	wchar_t	*wcmodulename = Ccct::MbcsToUcs2(mbmodulename, CHARSET_DEFAULT);
	modulename = wcmodulename;

	free(wcmodulename);
	wcmodulename = NULL;

	Ccct::sys_setlocale(LC_ALL);
}

#endif //win32

#if !defined(AYA_MAKE_EXE)
#if defined(WIN32)

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/)
{
	// モジュールの主ファイル名を取得
	// NT系ではいきなりUNICODEで取得できるが、9x系を考慮してMBCSで取得してからUCS-2へ変換
	if(ul_reason_for_call == DLL_PROCESS_ATTACH) {
		AYA_InitModule(hModule);
	}

	return TRUE;
}

#endif //win32
#endif //aya_make_exe

/* -----------------------------------------------------------------------
 *  load
 * -----------------------------------------------------------------------
 */
extern "C" DLLEXPORT BOOL_TYPE FUNCATTRIB load(aya::global_t h, long len)
{
	if( vm ) { delete vm; }

	vm = new CAyaVMWrapper(modulename,h,len);

#if defined(WIN32) || defined(_WIN32_WCE)
	::GlobalFree(h);
#elif defined(POSIX)
	free(h);
#endif

	return 1;
}

/* -----------------------------------------------------------------------
 *  unload
 * -----------------------------------------------------------------------
 */
extern "C" DLLEXPORT BOOL_TYPE FUNCATTRIB unload()
{
	if( vm ) {
		delete vm;
		vm = NULL;
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  request
 * -----------------------------------------------------------------------
 */
extern "C" DLLEXPORT aya::global_t FUNCATTRIB request(aya::global_t h, long *len)
{
	if( vm ) {
		return vm->ExecuteRequest(h, len, false);
	}
	else {
		return NULL;
	}
}

/* -----------------------------------------------------------------------
 *  CI_check_failed
 * -----------------------------------------------------------------------
 */
extern "C" DLLEXPORT BOOL_TYPE FUNCATTRIB CI_check_failed(void)
{
	if( vm ) {
		return vm->IsSuppress()||vm->IsEmergency();
	}
	else {
		return NULL;
	}
}

/* -----------------------------------------------------------------------
 *  Set_loghandler
 * -----------------------------------------------------------------------
 */
extern "C" DLLEXPORT void FUNCATTRIB Set_loghandler(void (*loghandler)(const aya::char_t *str, int mode, int id))
{
	::loghandler=loghandler;
	if( vm ) {
		vm->Set_loghandler(loghandler);
	}
}

/* -----------------------------------------------------------------------
 *  logsend（AYA固有　チェックツールから使用）
 * -----------------------------------------------------------------------
 */
#if !defined(AYA_MAKE_EXE)
#if defined(WIN32)
extern "C" DLLEXPORT BOOL_TYPE FUNCATTRIB logsend(long hwnd)
{
	if( vm ) {
		vm->SetLogRcvWnd(hwnd);
	}
	else {
		logsend_hwnd = hwnd;
	}

	return TRUE;
}
#endif //win32
#endif //aya_make_exe


/* -----------------------------------------------------------------------
 *  main (実行ファイル版のみ)
 * -----------------------------------------------------------------------
 */

#if defined(AYA_MAKE_EXE)

int main( int argc, char *argv[ ], char *envp[ ] )
{
	AYA_InitModule(NULL);

	std::string bufstr;

	_setmode( _fileno( stdin ), _O_BINARY );
	_setmode( _fileno( stdout ), _O_BINARY );

	while ( 1 ) {
		bufstr = "";

		while ( 1 ) {
			char buf[2];
			fread(buf,1,1,stdin);
			bufstr += static_cast<char>(buf[0]);

			if( bufstr.size() >= 2 ) {
				if( strcmp(bufstr.c_str() + bufstr.size() - 2,"\r\n") == 0 ) { //改行検出
					break;
				}
			}
		}

		const char* bufptr = bufstr.c_str();

		if( strncmp(bufptr,"load:",5) == 0 ) {
			bufptr += 5;
			long size = atoi(bufptr);
			if( size > 0 ) {
				char *read_ptr = (char*)::GlobalAlloc(GMEM_FIXED,size+1);
				fread(read_ptr,1,size,stdin);
				read_ptr[size] = 0;

				char *p = strstr(read_ptr,"\r\n");
				if( p ) { *p = 0; size -= 2; }
				
				load(read_ptr,size);
			}

			const char* result = "load:5\r\n1\r\n\r\n";
			fwrite(result,1,strlen(result),stdout);
			fflush(stdout);
		}
		else if( strncmp(bufptr,"unload:",7) == 0 ) {
			bufptr += 7;
			long size = atoi(bufptr);
			if( size > 0 ) {
				char *read_ptr = (char*)malloc(size);
				fread(read_ptr,1,size,stdin);
				free(read_ptr); //データまとめて破棄
			}

			unload();

			const char* result = "unload:5\r\n1\r\n\r\n";
			fwrite(result,1,strlen(result),stdout);
			fflush(stdout);
			break;
		}
		else if( (strncmp(bufptr,"request:",8) == 0) ) {
			bufptr += 8;
			
			long size = atoi(bufptr);
			if( size > 0 ) {
				char *read_ptr = (char*)::GlobalAlloc(GMEM_FIXED,size+1);
				fread(read_ptr,1,size,stdin);
				read_ptr[size] = 0;
				
				aya::global_t res = request(read_ptr,&size);

				char write_header[64];
				sprintf(write_header,"request:%d\r\n",size);
				fwrite(write_header,1,strlen(write_header),stdout);

				fwrite(res,1,size,stdout);
				fflush(stdout);

				::GlobalFree(res);
			}
			else {
				const char* w = "request:0\r\n";
				fwrite(w,1,strlen(w),stdout);
			}
		}
	}

	return 0;
}

#endif //aya_make_exe
