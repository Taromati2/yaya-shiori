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

class	CDefine
{
public:
	aya::string_t	before;
	aya::string_t	after;
	aya::string_t	dicfilename;
	aya::string_t	dicfilename_fullpath;
public:
	CDefine(CAyaVM& vm, const aya::string_t& bef, const aya::string_t& aft, const aya::string_t& df);

	CDefine(void) {}
	~CDefine(void) {}
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
