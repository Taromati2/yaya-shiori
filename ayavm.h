﻿// 
// AYA version 5
//
// AYAの1インスタンスを保持するクラスAYAVM
// written by the Maintenance Shop/C.Ponapalt 2006
// 
// CAyaVMをたくさん作ると複数のAYAを1つのプロセス/スレッド/モジュール内で走らせることができます。
// 

#ifndef AYAVM_H
#define AYAVM_H

#include <vector>
#include <map>
#include <memory>
#include "log.h"
#include "mt19937ar.h"
#include "globaldef.h"

class CBasis;
class CFunction;
class CFunctionDef;
class CCallLimit;
class CSystemFunction;
class CGlobalVariable;
class CFile;
class CLib;
class CParser0;
class CParser1;
class CDefine;
class CAyaVM;

class CAyaVM
{
private:
	std_shared_ptr<CBasis>					m_basis;

	std_shared_ptr<CFunctionDef>	m_function_parse;
	std_shared_ptr<CFunctionDef>	m_function_exec;
	std_shared_ptr<CFunctionDef>	m_function_destruct;
	
	std_shared_ptr< std::vector<CDefine> >	m_gdefines;

	std_shared_ptr<CCallLimit>				m_call_limit;
	std_shared_ptr<CSystemFunction>			m_sysfunction;
	std_shared_ptr<CGlobalVariable>			m_variable;

	std_shared_ptr<CFile>					m_files;
	std_shared_ptr<CLib>						m_libs;

	std_shared_ptr<CParser0>					m_parser0;
	std_shared_ptr<CParser1>					m_parser1;

	CLog	m_logger;

	MersenneTwister64 rs_sysfunc64;
	MersenneTwister64 rs_internal64;

public:
	CAyaVM();
	CAyaVM(CAyaVM &vm);
	virtual ~CAyaVM() {}

	CAyaVM* get_a_deep_copy();

	void load(void);
	void unload(void);

	void request_before(void);
	void request_after(void);

	void func_parse_to_exec(void);
	void func_parse_destruct(void);
	void func_parse_new(void);

	size_t genrand_uint(size_t n);

	aya::int_t genrand_sysfunc_ll(aya::int_t n);

	void genrand_sysfunc_srand_ll(aya::int_t n);
	void genrand_sysfunc_srand_array(const std::uint64_t a[],const int n);

	// 主制御
	CBasis&					basis();

	// 関数/システム関数/グローバル変数
	CFunctionDef&	function_parse(); //パース用
	CFunctionDef&	function_exec(); //実行用

	std::vector<CDefine>&	gdefines();

	CCallLimit&				call_limit();
	CSystemFunction&		sysfunction();
	CGlobalVariable&		variable();

	// ファイルと外部ライブラリ
	CFile&					files();
	CLib&					libs();

	// ロガー
	inline CLog& logger() {
		return m_logger;
	}

	// パーサ
	CParser0&				parser0();
	CParser1&				parser1();
};

#endif //AYAVM_H


