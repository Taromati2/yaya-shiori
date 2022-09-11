﻿// 
// AYA version 5
//
// 構文解析/中間コードの生成を行うクラス　CParser0
// written by umeici. 2004
// 
// 構文解析時にCBasisから一度だけCParser0::Parseが実行されます。
// CParser0::ParseEmbedStringはeval系の処理で使用されます。
//

#ifndef	PARSER0H
#define	PARSER0H

//----

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
#endif

#include <vector>
#include "globaldef.h"
#include "selecter.h"
#include "wsex.h"

class	CDefine
{
public:
	aya::string_t	before;
	aya::string_t	after;
	aya::string_t	dicfilename;
	aya::string_t	dicfilename_fullpath;
	/*
	bool is_macro_function;
	aya::string_t macro_function_name;
	std::vector<aya::string_t> macro_function_args;
private:
	//IsMarcoFunction
	bool IsMarcoFunction()const {
		CRegexpT<aya::char_t> regex(L"^[^!\"#$%&'()*+,\\-/:;<=>?@\\[\\\\\\]`{|}\\s]+\\([^!\"#$%&'()*+\\-/:;<=>?@\\[\\\\\\]`{|}\\s]+\\)$", SINGLELINE);
		return regex.Match(before.c_str(), before.size());
	}
	*/
public:
	CDefine(CAyaVM& vm, const aya::string_t& bef, const aya::string_t& aft, const aya::string_t& df);

	CDefine(void) {}
	~CDefine(void) {}

	/*
	void ExecuteMarcoFunctionReplace(aya::string_t& str)const {
		//if(!is_macro_function) return;
		CRegexpT<aya::char_t> regex((macro_function_name+L"\\(([^!\"#$%&'()*+\\-/:;<=>?@\\[\\\\\\]`{|}\\s]+)\\)$").c_str(), SINGLELINE);
		int n = regex.Match(str.c_str(), str.size());
		if(n==0) return;
		//赋值函数参数，并对函数体进行替换
		//对于参数...,将其替换为剩余所有参数
		//对于函数体内的__AYA_SYSTEM_MACRO_FUNCTION_VA_ARGS__，将其替换为...对应的参数表
		std::map<aya::string_t, aya::string_t> args;
		for(int i=0; i<n; i++){
			aya::string_t arg = regex.GetMatch(str.c_str(), i+1);
			if(macro_function_args[i]==L"..."){
				aya::string_t va_args;
				for(int j=i; j<n; j++){
					va_args += regex.GetMatch(str.c_str(), j+1);
					if(j!=n-1) va_args += L",";
				}
				args[L"..."] = va_args;
			}else{
				args[macro_function_args[i]] = arg;
			}
		}
		aya::string_t func_body = after;
		for(int i=0; i<macro_function_args.size(); i++){
			aya::string_t arg = macro_function_args[i];
			aya::string_t arg_value = args[arg];
			//替换函数体内的参数
			CRegexpT<aya::char_t> regex2((L"\\b"+arg+L"\\b").c_str(), SINGLELINE);
			func_body = regex2.Replace(func_body.c_str(), arg_value.c_str());
		}
		return;
	}
	*/
	void Exec(aya::string_t& str)const{
		/*
		if(IsMarcoFunction())
			ExecuteMarcoFunctionReplace(str);
		else
		*/
			aya::ws_replace(str, before.c_str(), after.c_str());
		return;
	}
};

//----

class CAyaVM;
class CStatement;
class CCell;
class CDic1;

class	CParser0
{
private:
	CAyaVM &vm;

	CParser0(void);

	std::vector<choicetype_t> m_defaultBlockChoicetypeStack;
	std::vector<size_t>  m_BlockhHeaderOfProcessingIndexStack;

public:
	CParser0(CAyaVM &vmr) : vm(vmr) {
		; //NOOP
	}
	bool	Parse(int charset, const std::vector<CDic1>& dics);
	bool	ParseEmbedString(aya::string_t& str, CStatement& st, const aya::string_t& dicfilename, ptrdiff_t linecount);

	int		DynamicLoadDictionary(const aya::string_t& dicfilename, int charset);
	int		DynamicAppendRuntimeDictionary(const aya::string_t& codes);
	int		DynamicUnloadDictionary(aya::string_t dicfilename);
	int		DynamicUndefFunc(const aya::string_t& funcname);

	//changed to public, for processglobaldefine
	void	ExecDefinePreProcess(aya::string_t &str, const std::vector<CDefine>& defines);

protected:
	bool	ParseAfterLoad(const aya::string_t &dicfilename);
	char	LoadDictionary1(const aya::string_t& filename, std::vector<CDefine>& gdefines, int charset);
	char	GetPreProcess(aya::string_t& str, std::vector<CDefine>& defines, std::vector<CDefine>& gdefines, const aya::string_t& dicfilename,
			ptrdiff_t linecount);

	void	ExecInternalPreProcess(aya::string_t &str,const aya::string_t &file, ptrdiff_t line);

	char	IsCipheredDic(const aya::string_t& filename);
	void	SeparateFactor(std::vector<aya::string_t> &s, aya::string_t &line);
	char	DefineFunctions(std::vector<aya::string_t> &s, const aya::string_t& dicfilename, ptrdiff_t linecount, size_t&depth, ptrdiff_t&targetfunction);
	ptrdiff_t MakeFunction(const aya::string_t& name, choicetype_t chtype, const aya::string_t& dicfilename, ptrdiff_t linecount);
	bool	StoreInternalStatement(size_t targetfunc, aya::string_t& str, size_t& depth, const aya::string_t& dicfilename, ptrdiff_t linecount);
	bool	MakeStatement(int type, size_t targetfunc, aya::string_t& str, const aya::string_t& dicfilename, ptrdiff_t linecount);
	bool	StructWhen(aya::string_t &str, std::vector<CCell> &cells, const aya::string_t& dicfilename, ptrdiff_t linecount);
	bool	StructFormula(aya::string_t &str, std::vector<CCell> &cells, const aya::string_t& dicfilename, ptrdiff_t linecount);
	void	StructFormulaCell(aya::string_t &str, std::vector<CCell> &cells);

	char	AddSimpleIfBrace(const aya::string_t &dicfilename);

	char	SetCellType(const aya::string_t &dicfilename);
	bool	SetCellType1(CCell& scell, bool emb, const aya::string_t& dicfilename, ptrdiff_t linecount);

	char	MakeCompleteFormula(const aya::string_t &dicfilename);
	char	ParseEmbeddedFactor(const aya::string_t& dicfilename);
	char	ParseEmbeddedFactor1(CStatement& st, const aya::string_t& dicfilename);
	void	ConvertPlainString(const aya::string_t& dicfilename);
	void	ConvertPlainString1(CStatement& st, const aya::string_t& dicfilename);
	char	ConvertEmbedStringToFormula(aya::string_t& str, const aya::string_t& dicfilename, ptrdiff_t linecount);
	char	CheckDepthAndSerialize(const aya::string_t& dicfilename);
	char	CheckDepth1(CStatement& st, const aya::string_t& dicfilename);
	char	CheckDepthAndSerialize1(CStatement& st, const aya::string_t& dicfilename);
	char	MakeCompleteConvertionWhenToIf(const aya::string_t& dicfilename);

	bool	IsDicFileAlreadyExist(aya::string_t dicfilename);
};

//----

#endif
