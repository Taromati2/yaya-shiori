﻿// 
// AYA version 5
//
// 関数を扱うクラス　CFunction
// - 主処理部
// written by umeici. 2004
// 

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
#endif

#include <assert.h>

#include <vector>

#include "function.h"
#include "ayavm.h"

#include "log.h"
#include "messages.h"
#include "misc.h"
#include "parser0.h"
#include "globaldef.h"
#include "sysfunc.h"

//////////DEBUG/////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "basis.h"
#endif
////////////////////////////////////////

CFunction::CFunction(CAyaVM& vmr, const aya::string_t& n, const aya::string_t& df, int lc) : pvm(&vmr), name(n), dicfilename(df), linecount(lc), dicfilename_fullpath(vmr.basis().ToFullPath(df))
{
	statelenm1 = 0;
	namelen = name.size();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::deep_copy_statement
 *  機能概要：  CStatement配列をディープコピーします
 * -----------------------------------------------------------------------
 */
void    CFunction::deep_copy_statement(CFunction &from)
{
	for ( size_t i = 0 ; i < from.statement.size() ; ++i ) {
		statement[i].deep_copy(from.statement[i]);
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::CompleteSetting
 *  機能概要：  関数の構築が完了した（≒辞書の読み込みが完了した）際に呼ばれます
 *  　　　　　  実行の際に必要な最後の初期化処理を行ないます
 * -----------------------------------------------------------------------
 */
void	CFunction::CompleteSetting(void)
{
	statelenm1 = statement.size() - 1;
}

CFunction::ExecutionResult	CFunction::Execute() {
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
	return Execute(arg);
}
CFunction::ExecutionResult	CFunction::Execute(const CValue& arg) {
	CLocalVariable	lvar(*pvm);
	return Execute(arg, lvar);
}
/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::Execute
 *  機能概要：  関数を実行します
 *
 *  引数CValue argは必ず配列型です。arrayが空であれば引数の無いコールとなります
 * -----------------------------------------------------------------------
 */
CFunction::ExecutionResult	CFunction::Execute(const CValue &arg, CLocalVariable &lvar)
{
	int exitcode = ST_NOP;

	// _argvを作成
	lvar.SetValue(L"_argv", arg);
	// _argcを作成
	CValue	t_argc((aya::int_t)arg.array_size());
	lvar.SetValue(L"_argc", t_argc);
	//_FUNC_NAME_を作成
	lvar.SetValue(L"_FUNC_NAME_", this->name);

	// 実行
	if (!pvm->call_limit().AddCall(name)) {
		CBasisFuncPos shiori_OnCallLimit;
		ptrdiff_t funcpos = shiori_OnCallLimit.Find(*pvm, L"shiori.OnCallLimit");
		size_t lock = pvm->call_limit().temp_unlock();

		if(funcpos >= 0) {
			//_argv[0] = dicname _argv[1] = linenum
			CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
			arg.array().emplace_back(dicfilename);
			arg.array().emplace_back((aya::int_t)linecount);

			pvm->function_exec().func[funcpos].Execute(arg);
		}
		else {
			pvm->logger().Error(E_E, 97, dicfilename, linecount);
		}

		pvm->call_limit().reset_lock(lock);
		pvm->call_limit().DeleteCall();
		return ExecutionResult(pvm);
	}
	ExecutionResult result(NULL);
	Execute_SEHbody(result,lvar, exitcode);
	
	pvm->call_limit().DeleteCall();

	for ( size_t i = 0 ; i < statement.size() ; ++i ) {
		statement[i].cell_cleanup();
	}

	return result;
}

void CFunction::Execute_SEHhelper(CFunction::ExecutionResult& aret, CLocalVariable& lvar, int& exitcode)
{
	aret = ExecuteInBrace(1, lvar, BRACE_DEFAULT, exitcode, NULL, 0);
}

void CFunction::Execute_SEHbody(ExecutionResult& retas, CLocalVariable& lvar, int& exitcode)
{
	__try
	{
		Execute_SEHhelper(retas, lvar, exitcode);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		throw aya::memory_error();
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::ExecuteInBrace
 *  機能概要：  {}を実行し、結果をひとつ返します
 *  引数　　　  type     この{}の種別。ただし0～の場合はswitch構文の際の候補抽出位置
 *  　　　　　  exitcode 終了コード。ST_NOP/ST_BREAK/ST_RETURN/ST_CONTINUE=通常/break/return/continue
 *
 *  "{}"内の各ステートメントを実行します。引数lineで指定される位置から実行を開始し、"}"に突き当たるまで
 *  順次実行していきます。
 *  返値は実行を終了した"}"の位置です。
 * -----------------------------------------------------------------------
 */
CFunction::ExecutionInBraceResult	CFunction::ExecuteInBrace(size_t line, CLocalVariable &lvar, aya::int_t type, int &exitcode, std::vector<CVecValue>* UpperLvCandidatePool,bool inpool)
{
	// 開始時の処理
	lvar.AddDepth();
	CDuplEvInfo* pdupl = statement[line-1].dupl_block.get();
	const bool	 inmutiarea = statement[line-1].ismutiarea;		// pool用

	// 実行
	CSelecter	output(pvm, pdupl, (ptrdiff_t)type);
	bool		exec_end	 = 0;		// この{}の実行を終了するためのフラグ 1で終了
	bool		ifflg		 = 0;		// if-elseif-else制御用。1でそのブロックを処理したことを示す
	bool		notpoolblock = 0;		// pool用
	const bool	ispoolbegin	 = !inpool;	// pool用
	bool		meltblock	 = 0;
	if(pdupl){
		if ( pdupl->GetType() & CHOICETYPE_POOL_FLAG ) {
			if(!inpool) {
				UpperLvCandidatePool = &output.values;
				inpool = 1;
			}
		}
		else {
			notpoolblock = true;
		}
		if ( pdupl->GetType() & CHOICETYPE_MELT_FLAG ) {
			meltblock = 1;
		}
	}
	if(!UpperLvCandidatePool){
		UpperLvCandidatePool = &output.values;
		meltblock = 0;
	}

	const bool inpool_to_next = (!inmutiarea ? !notpoolblock : false);

	size_t t_statelenm1 = statelenm1;

	size_t i = 0;
	for(i = line; i < t_statelenm1; i++) {
		CStatement& st = statement[i];

		switch(st.type) {
		case ST_OPEN: {					// "{"
			ExecutionInBraceResult info = ExecuteInBrace(i + 1, lvar, BRACE_DEFAULT, exitcode, UpperLvCandidatePool, inpool_to_next);
			i = info.linenum;
			output.Append(info.Output());
			break;
		}
		case ST_CLOSE:					// "}"　注　関数終端の"}"はここを通らない
			exec_end = 1;
			break;
		case ST_COMBINE:				// "--"
			output.AddArea();
			break;
		case ST_FORMULA_OUT_FORMULA:	// 出力（数式。配列、引数つき関数も含まれる）
			output.Append(GetFormulaAnswer(lvar, st));
			break;
		case ST_FORMULA_SUBST:			// 代入
			GetFormulaAnswer(lvar, st);
			break;
		case ST_IF:						// if
			ifflg = 0;
			if (GetFormulaAnswer(lvar, st).GetTruth()) {
				ExecutionInBraceResult info = ExecuteInBrace(i + 2, lvar, BRACE_DEFAULT, exitcode, UpperLvCandidatePool, inpool_to_next);
				i = info.linenum;
				output.Append(info.Output());
				ifflg = 1;
			}
			else
				i = st.jumpto;
			break;
		case ST_ELSEIF:					// elseif
			if (ifflg)
				i = st.jumpto;
			else if (GetFormulaAnswer(lvar, st).GetTruth()) {
				ExecutionInBraceResult info = ExecuteInBrace(i + 2, lvar, BRACE_DEFAULT, exitcode, UpperLvCandidatePool, inpool_to_next);
				i = info.linenum;
				output.Append(info.Output());
				ifflg = 1;
			}
			else
				i = st.jumpto;
			break;
		case ST_ELSE:					// else
			if (ifflg)
				i = st.jumpto;
			else {
				ExecutionInBraceResult info = ExecuteInBrace(i + 2, lvar, BRACE_DEFAULT, exitcode, UpperLvCandidatePool, inpool_to_next);
				i = info.linenum;
				output.Append(info.Output());
			}
			break;
		case ST_PARALLEL:				// parallel
			{
		        const CValue& val = GetFormulaAnswer(lvar, st);
				if (val.GetType() == F_TAG_ARRAY) {
					for(size_t j = 0; j < val.array_size(); ++j)
						output.Append(CValue(val.array()[j]));
				}
				else
					output.Append(val);
			}
			break;
		case ST_VOID:				// void
			{
				//実行だけして捨てる
		        GetFormulaAnswer(lvar, st);
			}
			break;
		case ST_WHILE:					// while
			{
				size_t loop_max = pvm->call_limit().GetMaxLoop();
				size_t loop_cur = 0;

				while ( (loop_max == 0) || (loop_max > loop_cur++) ) {
					if (!GetFormulaAnswer(lvar, st).GetTruth())
						break;
					CValue t_value = ExecuteInBrace(i + 2, lvar, BRACE_LOOP, exitcode, UpperLvCandidatePool, inpool_to_next);
					if(exitcode == ST_RETURN_WITH_VALUE)
						output.clear();
					output.Append(t_value);

					if(exitcode == ST_BREAK) {
						exitcode = ST_NOP;
						break;
					}
					else if(exitcode == ST_RETURN)
						break;
					else if(exitcode == ST_RETURN_WITH_VALUE)
						break;
					else if(exitcode == ST_CONTINUE)
						exitcode = ST_NOP;
				}

				if (loop_max && loop_max <= loop_cur ) {
					CBasisFuncPos shiori_OnLoopLimit;
					ptrdiff_t funcpos = shiori_OnLoopLimit.Find(*pvm, L"shiori.OnLoopLimit");

					if (funcpos >= 0) {
						//_argv[0] = dicname _argv[1] = linenum
						CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
						arg.array().emplace_back(dicfilename);
						arg.array().emplace_back(st.linecount);

						pvm->function_exec().func[funcpos].Execute(arg);
					}
					else {
						pvm->logger().Error(E_E, 98, dicfilename, st.linecount);
					}
				}

				i = st.jumpto;
				break;
			}
		case ST_FOR:					// for
			{
				GetFormulaAnswer(lvar, st); //for第一パラメータ

				size_t loop_max = pvm->call_limit().GetMaxLoop();
				size_t loop_cur = 0;

				while ( (loop_max == 0) || (loop_max > loop_cur++) ) {
					if (!GetFormulaAnswer(lvar, statement[i + 1]).GetTruth()) //for第二パラメータ
						break;
					CValue t_value = ExecuteInBrace(i + 4, lvar, BRACE_LOOP, exitcode, UpperLvCandidatePool, inpool_to_next);
					if(exitcode == ST_RETURN_WITH_VALUE)
						output.clear();
					output.Append(t_value);

					if (exitcode == ST_BREAK) {
						exitcode = ST_NOP;
						break;
					}
					else if (exitcode == ST_RETURN)
						break;
					else if (exitcode == ST_RETURN_WITH_VALUE)
						break;
					else if (exitcode == ST_CONTINUE)
						exitcode = ST_NOP;

					GetFormulaAnswer(lvar, statement[i + 2]); //for第三パラメータ
				}

				if (loop_max && loop_max <= loop_cur ) {
					CBasisFuncPos shiori_OnLoopLimit;
					ptrdiff_t funcpos = shiori_OnLoopLimit.Find(*pvm, L"shiori.OnLoopLimit");

					if (funcpos >= 0) {
						//_argv[0] = dicname _argv[1] = linenum
						CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
						arg.array().emplace_back(dicfilename);
						arg.array().emplace_back(st.linecount);

						pvm->function_exec().func[funcpos].Execute(arg);
					}
					else {
						pvm->logger().Error(E_E, 98, dicfilename, st.linecount);
					}
				}

				i = st.jumpto;
				break;
			}
		case ST_SWITCH: {				// switch
				aya::int_t sw_index = GetFormulaAnswer(lvar, st).GetValueInt();
				if (sw_index < 0)
					sw_index = BRACE_SWITCH_OUT_OF_RANGE;
				ExecutionInBraceResult info = ExecuteInBrace(i + 2, lvar, sw_index, exitcode, NULL, 0);
				i = info.linenum;
				output.Append(info.Output());
			}
			break;
		case ST_FOREACH:				// foreach
			Foreach(lvar, output, i, exitcode, UpperLvCandidatePool, inpool_to_next);
			i  = st.jumpto;
			break;
		case ST_BREAK:					// break
			exitcode = ST_BREAK;
			break;
		case ST_CONTINUE:				// continue
			exitcode = ST_CONTINUE;
			break;
		case ST_RETURN:					// return
			exitcode = ST_RETURN;
			break;
		case ST_RETURN_WITH_VALUE:
			output.clear();
			output.Append(GetFormulaAnswer(lvar, st));
			exitcode = ST_RETURN_WITH_VALUE;
			break;
		default:						// 未知のステートメント
			pvm->logger().Error(E_E, 82, dicfilename, st.linecount);
			break;
		};
		if (exec_end)
			break;

		if (exitcode != ST_NOP)
			FeedLineToTail(i);
	}

	// 候補から出力を選び出す　入れ子の深さが0なら重複回避が働く
	if (inpool&&!ispoolbegin) {
		std::vector<CValue>& thepool = UpperLvCandidatePool->rbegin()->array;
		std::vector<CValue>& thispool = output.values[0].array;

		if(notpoolblock || inmutiarea)
			thepool.insert(thepool.end(), output.Output());
		else
			thepool.insert(thepool.end(), thispool.begin(), thispool.end());
		output.clear();
	}

	if(meltblock){
		std::vector<CValue>& pool_of_uplv = UpperLvCandidatePool->rbegin()->array;
		CValue result = output.Output();

		if (result.GetType() == F_TAG_ARRAY) {
			for(size_t j = 0; j < result.array_size(); ++j) {
				pool_of_uplv.emplace_back(result.array()[j]);
			}
		}
		else
			pool_of_uplv.emplace_back(result);
		output.clear();
	}
	// 終了時の処理
	lvar.DelDepth();

	return CFunction::ExecutionInBraceResult( output,i );
}
/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::Foreach
 *  機能概要：  foreach処理を行います
 *
 *  実際に送るのは"}"の1つ手前の行の位置です
 * -----------------------------------------------------------------------
 */
void	CFunction::Foreach(CLocalVariable &lvar, CSelecter &output,size_t line,int &exitcode, std::vector<CVecValue>* UpperLvCandidatePool, bool inpool)
{
	CStatement &st0 = statement[line];
	CStatement &st1 = statement[line + 1];

	// 代入値を求める
	// 注意：foreach中の副作用を回避するため必ず参照にはしないこと
	const CValue value = GetFormulaAnswer(lvar, st0);

	// 代入値の要素数を求める
	// 簡易配列かつ変数からの取得の場合、その変数に設定されているデリミタを取得する
	bool isPseudoarray = false;

	ptrdiff_t sz;
	std::vector<aya::string_t>	s_array;
	if (value.IsString()) {
		isPseudoarray = true;

		aya::string_t delimiter = VAR_DELIMITER;
		if (st0.cell_size() == 1) {
			if (st0.cell()[0].value_GetType() == F_TAG_VARIABLE) {
				delimiter = pvm->variable().GetDelimiter(st0.cell()[0].index);
			}
			else if (st0.cell()[0].value_GetType() == F_TAG_LOCALVARIABLE)
				delimiter = lvar.GetDelimiter(st0.cell()[0].name);
		}
		else {
			CCell *l_cell = &(st0.cell()[st0.serial()[st0.serial_size() - 1].tindex]);
			if (l_cell->value_GetType() == F_TAG_VARIABLE)
				delimiter = pvm->variable().GetDelimiter(l_cell->index);
			else if (l_cell->value_GetType() == F_TAG_LOCALVARIABLE)
				delimiter = lvar.GetDelimiter(l_cell->name);
			else if (l_cell->value_GetType() == F_TAG_ARRAYORDER) {
				l_cell = &(st0.cell()[st0.serial()[st0.serial_size() - 1].tindex - 1]);
				if (l_cell->value_GetType() == F_TAG_VARIABLE)
					delimiter = pvm->variable().GetDelimiter(l_cell->index);
				else if (l_cell->value_GetType() == F_TAG_LOCALVARIABLE)
					delimiter = lvar.GetDelimiter(l_cell->name);
			}
		}
		sz = SplitToMultiString(value.GetValueString(), &s_array, delimiter);
	}
	else if (value.IsArray()) {
		sz = value.array_size();
	}
    else if (value.IsHash()) {
        sz = value.hash_size();
	}
	else {
		sz = -1;
	}

	CValue	t_value;
	int type;
	int fromtype = value.GetType();

	CValueHash::const_iterator hash_iterator;
	if ( fromtype == F_TAG_HASH ) {
		hash_iterator = value.hash().begin();
	}
	
	for(int foreachcount = 0; foreachcount < sz; ++foreachcount ) {
		// 代入する要素値を取得
		if (isPseudoarray) {
			t_value = s_array[foreachcount];
		}
		else if ( fromtype == F_TAG_ARRAY ) {
			t_value = value.array()[foreachcount];
		}
		else if ( fromtype == F_TAG_HASH ) {
			t_value.SetType(F_TAG_ARRAY);
			//t_value.array().push_back(hash_iterator->first); //second(value)だけで良い
			t_value.array().push_back(hash_iterator->second);
			hash_iterator++;
		}

		// 代入
		type = st1.cell()[0].value_GetType();
		if ( type == F_TAG_VARIABLE ) {
			pvm->variable().SetValue(st1.cell()[0].index, t_value);
		}
		else if ( type == F_TAG_LOCALVARIABLE ) {
			lvar.SetValue(st1.cell()[0].name, t_value);
		}
		else {
			pvm->logger().Error(E_E, 28, dicfilename, st1.linecount);
			break;
		}

		t_value = ExecuteInBrace(line + 3, lvar, BRACE_LOOP, exitcode, UpperLvCandidatePool, inpool);
		if(exitcode == ST_RETURN_WITH_VALUE)
			output.clear();
		output.Append(t_value);

		if (exitcode == ST_BREAK) {
			exitcode = ST_NOP;
			break;
		}
		else if (exitcode == ST_RETURN)
			break;
		else if(exitcode == ST_RETURN_WITH_VALUE)
			break;
		else if (exitcode == ST_CONTINUE)
			exitcode = ST_NOP;
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::GetFormulaAnswer
 *  機能概要：  数式を演算して結果を返します
 * -----------------------------------------------------------------------
 */
const CValue& CFunction::GetFormulaAnswer(CLocalVariable &lvar, CStatement &st)
{
	size_t	o_index = 0;

	if ( st.serial_size() ) { //高速化用
		for (std::vector<CSerial>::iterator it = st.serial().begin(); it != st.serial().end(); it++) {
			o_index = it->tindex;
			CCell	&o_cell = st.cell()[o_index];
			if (o_cell.value_GetType() >= F_TAG_ORIGIN_VALUE) {
				o_cell.ansv() = GetValueRefForCalc(o_cell, st, lvar);
				break;
			}

			CCell	*s_cell = &(st.cell()[it->index[0]]);
			CCell	*d_cell = NULL;
			if ( it->index.size() >= 2 ) {
				d_cell = &(st.cell()[it->index[1]]);
			}

			switch(o_cell.value_GetType()) {
			case F_TAG_COMMA:
				{
					std_shared_ptr<CValue> tmp_ansv = o_cell.ansv_shared_create();
					if (Comma(tmp_ansv, it->index, st, lvar)) {
						pvm->logger().Error(E_E, 33, L",", dicfilename, st.linecount);
					}
					o_cell.ansv_shared() = tmp_ansv;
				}
				break;
			case F_TAG_EQUAL:
			case F_TAG_PLUSEQUAL:
			case F_TAG_MINUSEQUAL:
			case F_TAG_MULEQUAL:
			case F_TAG_DIVEQUAL:
			case F_TAG_SURPEQUAL:
			case F_TAG_COMMAEQUAL:
				{
					std_shared_ptr<CValue> tmp_ansv = o_cell.ansv_shared_create();
					if(Subst(o_cell.value_GetType(), tmp_ansv, it->index, st, lvar)) {
						pvm->logger().Error(E_E, 33, L"=", dicfilename, st.linecount);
					}
					o_cell.ansv_shared() = tmp_ansv;
				}
				break;
			case F_TAG_PLUS:
				{
					// 演算順序を必ず守るため、一旦左辺の結果を取ってから右辺と演算する
					// これを怠ると右辺から先に計算する
					// 以降同じ

					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = lv + GetValueRefForCalc(*d_cell, st, lvar);
					break;
				}
			case F_TAG_MINUS:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = lv - GetValueRefForCalc(*d_cell, st, lvar);
					break;
				}
			case F_TAG_MUL:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = lv * GetValueRefForCalc(*d_cell, st, lvar);
					break;
				}
			case F_TAG_DIV:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = lv / GetValueRefForCalc(*d_cell, st, lvar);
					break;
				}
			case F_TAG_SURP:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = lv % GetValueRefForCalc(*d_cell, st, lvar);
					break;
				}
			case F_TAG_IFEQUAL:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = lv == GetValueRefForCalc(*d_cell, st, lvar);
					break;
				}
			case F_TAG_IFDIFFER:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = lv != GetValueRefForCalc(*d_cell, st, lvar);
					break;
				}
			case F_TAG_IFGTEQUAL:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = lv >= GetValueRefForCalc(*d_cell, st, lvar);
					break;
				}
			case F_TAG_IFLTEQUAL:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = lv <= GetValueRefForCalc(*d_cell, st, lvar);
					break;
				}
			case F_TAG_IFGT:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = lv > GetValueRefForCalc(*d_cell, st, lvar);
					break;
				}
			case F_TAG_IFLT:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = lv < GetValueRefForCalc(*d_cell, st, lvar);
					break;
				}
			case F_TAG_IFIN:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv().SetType(F_TAG_INT);
					o_cell.ansv().i_value = _in_(lv,GetValueRefForCalc(*d_cell, st, lvar));
					break;
				}
			case F_TAG_IFNOTIN:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv().SetType(F_TAG_INT);
					o_cell.ansv().i_value = not_in_(lv,GetValueRefForCalc(*d_cell, st, lvar));
					break;
				}
			case F_TAG_OR:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = CValue(lv.GetTruth() || GetValueRefForCalc(*d_cell, st, lvar).GetTruth());
					break;
				}
			case F_TAG_AND:
				{
					const CValue& lv = GetValueRefForCalc(*s_cell, st, lvar);
					o_cell.ansv() = CValue(lv.GetTruth() && GetValueRefForCalc(*d_cell, st, lvar).GetTruth());
					break;
				}
			case F_TAG_FUNCPARAM:
				{
					std_shared_ptr<CValue> tmp_ansv = o_cell.ansv_shared_create();
					if (ExecFunctionWithArgs(tmp_ansv, it->index, st, lvar)) {
						pvm->logger().Error(E_E, 33, pvm->function_exec().func[st.cell()[it->index[0]].index].name, dicfilename, st.linecount);
					}
					o_cell.ansv_shared() = tmp_ansv;
				}
				break;
			case F_TAG_SYSFUNCPARAM:
				if (ExecSystemFunctionWithArgs(o_cell, it->index, st, lvar))
					pvm->logger().Error(E_E, 33, CSystemFunction::GetNameFromIndex(st.cell()[it->index[0]].index), dicfilename, st.linecount);
				break;
			case F_TAG_ARRAYORDER:
				if (Array(o_cell, it->index, st, lvar))
					pvm->logger().Error(E_E, 33, L",", dicfilename, st.linecount);
				break;
			case F_TAG_FEEDBACK:
				if (Feedback(o_cell, it->index, st, lvar))
					pvm->logger().Error(E_E, 33, L"&", dicfilename, st.linecount);
				break;
			case F_TAG_EXC:
				o_cell.ansv().SetType(F_TAG_INT);
				o_cell.ansv().i_value = !GetValueRefForCalc(*d_cell, st, lvar).GetTruth();
				break;
			default:
				pvm->logger().Error(E_E, 34, dicfilename, st.linecount);
				return emptyvalue;
			};
		}
	}

	return st.cell()[o_index].ansv();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::GetValueRefForCalc
 *  機能概要：  与えられた項に対応する値へのポインタを取得します
 * -----------------------------------------------------------------------
 */
const CValue& CFunction::GetValueRefForCalc(CCell &cell, CStatement &st, CLocalVariable &lvar)
{
	// 即値はv、関数/変数/演算子項ならansvから取得　関数/変数の場合その値や実行結果が取得される

	// %[n]処理
	if (cell.value_GetType() == F_TAG_SYSFUNCPARAM) {
		if ( cell.index == CSystemFunction::HistoryIndex() ) {
			ExecHistoryP2(cell, st);
		}
	}

	// 演算が完了している（はずの）項ならそれを返す
	if (cell.value_GetType() < F_TAG_ORIGIN_VALUE)
		return cell.ansv();

	// 即値ならそれをそのまま返す
	if (cell.value_GetType() <= F_TAG_STRING)
		return cell.value();

	// 関数なら実行して結果を、変数ならその内容を返す
	switch(cell.value_GetType()) {
	case F_TAG_STRING_EMBED:
		SolveEmbedCell(cell, st, lvar);
		return cell.ansv();
	case F_TAG_SYSFUNC: {
			CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
			std::vector<CCell *> pcellarg; //dummy
			std::vector<CValue> pvaluearg; //dummy
			cell.ansv() =  pvm->sysfunction().Execute(cell.index, arg, pcellarg, pvaluearg, lvar, st.linecount, this);
			return cell.ansv();
		}
	case F_TAG_USERFUNC: {
		CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
		CLocalVariable	t_lvar(*pvm);
		cell.ansv() = pvm->function_exec().func[cell.index].Execute(arg, t_lvar);
		return cell.ansv();
	}
	case F_TAG_VARIABLE:
		return pvm->variable().GetPtr(cell.index)->call_watcher(*pvm,cell.ansv());
	case F_TAG_LOCALVARIABLE:
		return lvar.GetPtr(cell.name)->call_watcher(*pvm,cell.ansv());
	default:
		pvm->logger().Error(E_E, 16, dicfilename, st.linecount);
		return emptyvalue;
	};
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::ReindexUserFunctions
 *  機能概要：  F_TAG_USERFUNCのindexを再設定します
 * -----------------------------------------------------------------------
 */
int CFunction::ReindexUserFunctions(void)
{
	int error = 0;

	for ( size_t i = 0 ; i < statement.size() ; ++i ) {
		CStatement &st = statement[i];

		for ( size_t j = 0 ; j < st.cell_size() ; ++j ) {
			CCell &cl = st.cell()[j];

			if ( cl.value_GetType() == F_TAG_USERFUNC ) {
				ptrdiff_t index = pvm->function_parse().GetFunctionIndexFromName(cl.name);
				if ( index < 0 ) {
					pvm->logger().Error(E_E, 71, dicfilename, st.linecount);
					error += 1;
				}
				else {
					cl.index = index;
				}
			}
		}
	}

	return error;
}


/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::SolveEmbedCell
 *  機能概要：  %埋め込み付文字列項の値を求めます
 * -----------------------------------------------------------------------
 */
void	CFunction::SolveEmbedCell(CCell &cell, CStatement &st, CLocalVariable &lvar)
{
	// 分割する位置を求める
	int	solve_src;	// 種別 0/1/2/3=ローカル変数/変数/関数/システム関数
	size_t	max_len = 0;	// 最長一致検索用

	if (cell.value_const().s_value[0] == L'_') {
		// ローカル変数
		solve_src = 0;
		max_len   = lvar.GetMacthedLongestNameLength(cell.value_const().s_value);
	}
	else {
		// 変数
		solve_src = 1;
		max_len   = pvm->variable().GetMacthedLongestNameLength(cell.value_const().s_value);
		// 関数
		size_t	t_len = 0;
		for(std::vector<CFunction>::iterator it = pvm->function_exec().func.begin(); it != pvm->function_exec().func.end(); it++)
			if (!it->name.compare(0,it->namelen,cell.value_const().s_value,0,it->namelen))
				if (t_len < it->namelen)
					t_len = it->namelen;
		if (t_len > max_len) {
			solve_src = 2;
			max_len   = t_len;
		}
		// システム関数
		if ( max_len < (size_t)CSystemFunction::GetMaxNameLength() ) {
			t_len = CSystemFunction::FindIndexLongestMatch(cell.value_const().s_value,max_len);
			if (t_len > max_len) {
				solve_src = 3;
				max_len   = t_len;
			}
		}
	}
	// 存在しなければ全体が文字列ということになる
	if (!max_len) {
		cell.ansv()     = L"%" + cell.value_const().s_value;
		cell.emb_ansv() = L"%" + cell.value_const().s_value;
		return;
	}

	// 関数/システム関数の場合は引数部分を探す
	size_t	len = cell.value_const().s_value.size();
	if (solve_src >= 2) {
		size_t	depth = 1;
		size_t i = 0;
		for(i = max_len + 1; i < len; i++) {
			depth += ((cell.value_const().s_value[i] == L'(') - (cell.value_const().s_value[i] == L')'));
			if (!depth)
				break;
		}
		if (!depth)
			max_len = i + 1;
	}

	// 配列部分を探す
	size_t	depth = 1;
	size_t i = 0;
	for(i = max_len + 1; i < len; i++) {
		if (!depth && cell.value_const().s_value[i] != L'[')
			break;
		depth += ((cell.value_const().s_value[i] == L'[') - (cell.value_const().s_value[i] == L']'));
	}
	if (!depth)
		max_len = i;

	// 埋め込まれた要素とそれ以降の文字列に分割する
	//aya::string_t	s_value(cell.value_const().s_value.substr(0, max_len));
	//aya::string_t	d_value(cell.value_const().s_value.substr(max_len, len - max_len));
	aya::string_t::const_iterator it_split = cell.value_const().s_value.begin() + max_len;
	aya::string_t s_value(cell.value_const().s_value.begin(),it_split);
	aya::string_t d_value(it_split,cell.value_const().s_value.end());

	// 埋め込まれた要素を数式に変換する　失敗なら全体が文字列
	CStatement	t_state(ST_FORMULA, st.linecount);
	if (pvm->parser0().ParseEmbedString(s_value, t_state, dicfilename, st.linecount)) {
		cell.ansv()       = L"%" + cell.value_const().s_value;
		cell.emb_ansv() = L"%" + cell.value_const().s_value;
		return;
	}

	// 埋め込み要素の値を取得して応答文字列を作成
	aya::string_t	result = GetFormulaAnswer(lvar, t_state).GetValueString();
	cell.emb_ansv()  = result;
	cell.ansv()      = result + d_value;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::Comma
 *  機能概要：  ,演算子を処理します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
bool CFunction::Comma(CValueRef answer, std::vector<size_t> &sid, CStatement &st, CLocalVariable &lvar) {
	// 結合して配列値を作成
	CValueArray	t_array;

	for(std::vector<size_t>::iterator it = sid.begin(); it != sid.end(); it++) {
		const CValue &addv = GetValueRefForCalc(st.cell()[*it], st, lvar);
		
		if (addv.GetType() == F_TAG_ARRAY) {
			t_array.insert(t_array.end(), addv.array().begin(), addv.array().end());
		}
		else {
			t_array.emplace_back(addv);
		}
	}

	answer.SubstToArray(t_array);
	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::CommaAdd
 *  機能概要：  ,=演算子を処理します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
bool CFunction::CommaAdd(CValueRef answer, std::vector<size_t> &sid, CStatement &st, CLocalVariable &lvar) {
	if ( answer.GetType() != F_TAG_ARRAY ) {
		CValue st(answer);
		answer.SetType(F_TAG_ARRAY);
		answer.array().emplace_back(st);
	}
	CValueArray &t_array = answer.array();

	std::vector<size_t>::iterator it = sid.begin();
	it++; //最初＝左辺は代入先なので飛ばす

	for( ; it != sid.end(); it++) {
		const CValue &addv = GetValueRefForCalc(st.cell()[*it], st, lvar);
		
		if (addv.GetType() == F_TAG_ARRAY) {
			t_array.insert(t_array.end(), addv.array().begin(), addv.array().end());
		}
		else {
			t_array.emplace_back(addv);
		}
	}

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::Subst
 *  機能概要：  代入演算子を処理します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
bool CFunction::Subst(int type, CValueRef answer, std::vector<size_t> &sid, CStatement &st, CLocalVariable &lvar) {
	CCell	*sid_0_cell = &(st.cell()[sid[0]]);
	CCell	*sid_1_cell = &(st.cell()[sid[1]]);

	int sid_0_cell_type = sid_0_cell->value_GetType();

	//既存変数への代入の場合だけは特殊扱いする
	if ( sid_0_cell_type == F_TAG_VARIABLE || sid_0_cell_type == F_TAG_LOCALVARIABLE ) {
		CVariable* pSubstTo;

		if ( sid_0_cell_type == F_TAG_VARIABLE ) {
			pSubstTo = pvm->variable().GetPtr(sid_0_cell->index);
		}
		else {
			pSubstTo = lvar.GetPtr(sid_0_cell->name);
		}

		if ( pSubstTo ) {
			CValue varback = pSubstTo->value();
			CValue &substTo = pSubstTo->value();

			answer.array_clear();

			switch(type) {
			case F_TAG_EQUAL:
				substTo = GetValueRefForCalc(*sid_1_cell, st, lvar);
				break;
			case F_TAG_PLUSEQUAL:
				substTo += GetValueRefForCalc(*sid_1_cell, st, lvar);
				break;
			case F_TAG_MINUSEQUAL:
				substTo -= GetValueRefForCalc(*sid_1_cell, st, lvar);
				break;
			case F_TAG_MULEQUAL:
				substTo *= GetValueRefForCalc(*sid_1_cell, st, lvar);
				break;
			case F_TAG_DIVEQUAL:
				substTo /= GetValueRefForCalc(*sid_1_cell, st, lvar);
				break;
			case F_TAG_SURPEQUAL:
				substTo %= GetValueRefForCalc(*sid_1_cell, st, lvar);
				break;

				//カンマ特殊処理
			case F_TAG_COMMAEQUAL:
				if(CommaAdd(pSubstTo->value_shared(), sid, st, lvar)) {
					return 1;
				}
				break;
			default:
				return 1;
			};

			// **HACK** constにしてarrayの場合answerと強制共有
			// 後で代入演算がもしあった時に配列の時の代入コストを省略できる
			answer = const_cast<const CValue&>(substTo);

			// グローバル変数の場合、削除済みの場合があるのでここで再Enable
			if ( sid_0_cell_type == F_TAG_VARIABLE ) {
				pvm->variable().EnableValue(sid_0_cell->index);
			}

			pSubstTo->call_setter(*pvm,varback);

			return 0;
		}
	}

	// 代入元の値を取得　演算子つきなら演算も行う
	switch(type) {
	case F_TAG_EQUAL:
		answer = GetValueRefForCalc(*sid_1_cell, st, lvar);
		break;
	case F_TAG_PLUSEQUAL:
		{
			// 演算順序を必ず守るため、一旦左辺の結果を取ってから右辺と演算する
			// これを怠ると右辺から先に計算する
			// 以降同じ
			const CValue &lv = GetValueRefForCalc(*sid_0_cell, st, lvar);
			answer = lv + GetValueRefForCalc(*sid_1_cell, st, lvar);
			break;
		}
	case F_TAG_MINUSEQUAL:
		{
			const CValue &lv = GetValueRefForCalc(*sid_0_cell, st, lvar);
			answer = lv - GetValueRefForCalc(*sid_1_cell, st, lvar);
			break;
		}
	case F_TAG_MULEQUAL:
		{
			const CValue &lv = GetValueRefForCalc(*sid_0_cell, st, lvar);
			answer = lv * GetValueRefForCalc(*sid_1_cell, st, lvar);
			break;
		}
	case F_TAG_DIVEQUAL:
		{
			const CValue &lv = GetValueRefForCalc(*sid_0_cell, st, lvar);
			answer = lv / GetValueRefForCalc(*sid_1_cell, st, lvar);
			break;
		}
	case F_TAG_SURPEQUAL:
		{
			const CValue &lv = GetValueRefForCalc(*sid_0_cell, st, lvar);
			answer = lv % GetValueRefForCalc(*sid_1_cell, st, lvar);
			break;
		}
	case F_TAG_COMMAEQUAL:
		if (Comma(answer, sid, st, lvar))
			return 1;
		break;
	default:
		return 1;
	};

	// 代入を実行
 	// 配列要素への代入は操作が複雑なので、さらに他の関数へ処理を渡す
	switch(sid_0_cell->value_GetType()) {
	case F_TAG_VARIABLE:
		pvm->variable().SetValue(sid_0_cell->index, answer);
		return 0;
	case F_TAG_LOCALVARIABLE:
		lvar.SetValue(sid_0_cell->name, answer);
		return 0;
	case F_TAG_ARRAYORDER: {
		size_t sid_0 = sid[0];
		if(sid_0 > 0)
			return SubstToArray(st.cell()[sid_0 - 1], *sid_0_cell, answer, st, lvar, sid_0);
		else
			return 1;
	}
	default:
 		return 1;
	};
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::SubstToArray
 *  機能概要：  配列要素への代入を処理します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
bool CFunction::SubstToArray(CCell &vcell, CCell &ocell, CValueRef answer, CStatement &st, CLocalVariable &lvar, size_t sid_begin) {
	// 序数を取得
	CValue	t_order;
	EncodeArrayOrder(vcell, ocell.order(), lvar, t_order);

	if (t_order.GetType() == F_TAG_UNKNOWN)
		return 1;

	// 値を取得
	CValueRef value = CValueRef(GetValueRefForCalc(vcell, st, lvar));

	// 更新
    value.SetArrayValue(t_order, answer);

	// 代入
	switch(vcell.value_GetType()) {
	case F_TAG_VARIABLE:
		pvm->variable().SetValue(vcell.index, value);
		return 0;
	case F_TAG_LOCALVARIABLE:
		lvar.SetValue(vcell.name, value);
		return 0;
	case F_TAG_HOOKBRACKETOUT: {
		auto	 &cells	 = st.cell();
		size_t	sid_index	= sid_begin;
		size_t bracket_num = 0;
		while(sid_index > 0) {
			sid_index--;
			auto type=cells[sid_index].value_GetType();
			if(F_TAG_ISOUT(type)) {
				bracket_num++;
			}
			else if(F_TAG_ISIN(type)) {
				bracket_num--;
			}
			if(bracket_num == 0)
				break;
		}
		if(bracket_num != 0)
			return 1;
		if(sid_index < 2)
			return 1;
		sid_index--;
		return SubstToArray(cells[sid_index - 1], cells[sid_index], value, st, lvar, sid_index);
	}
	default:
		return 1;
	};
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::Array
 *  機能概要：  配列[]演算子を処理します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
bool CFunction::Array(CCell &anscell, std::vector<size_t> &sid, CStatement &st, CLocalVariable &lvar) {
	CCell	*v_cell = &(st.cell()[sid[0]]);
	CCell	*n_cell = &(st.cell()[sid[1]]);

	// 序数を取得
	anscell.order() = GetValueRefForCalc(*n_cell, st, lvar);

	CValue	t_order;
	EncodeArrayOrder(*v_cell, anscell.order(), lvar, t_order);

	if (t_order.GetType() == F_TAG_UNKNOWN) {
		anscell.ansv().SetType(F_TAG_VOID);
		return 1;
	}

	// 値を取得
	anscell.ansv_shared() = GetValueRefForCalc(*v_cell, st, lvar)[t_order].self_shared();

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::_in_
 *  機能概要：  _in_演算子を処理します
 * -----------------------------------------------------------------------
 */
bool CFunction::_in_(const CValue &src, const CValue &dst)
{
	if (src.IsString() && dst.IsString())
		return (dst.s_value.find(src.s_value) != aya::string_t::npos) ? 1 : 0;

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::not_in_
 *  機能概要：  !_in_演算子を処理します
 * -----------------------------------------------------------------------
 */
bool CFunction::not_in_(const CValue &src, const CValue &dst)
{
	return ! _in_(src,dst);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::ExecFunctionWithArgs
 *  機能概要：  引数付きの関数を実行します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
bool CFunction::ExecFunctionWithArgs(CValueRef answer, std::vector<size_t> &sid, CStatement &st, CLocalVariable &lvar) {
	// 関数の格納位置を取得
	std::vector<size_t>::iterator it = sid.begin();
	size_t index = st.cell()[*it].index;
	it++;

	// 引数作成
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);	
	std::vector<size_t>::size_type sidsize = sid.size();

	for( ; it != sid.end(); it++) {
		const CValue &addv = GetValueRefForCalc(st.cell()[*it], st, lvar);
		
		if (addv.GetType() == F_TAG_ARRAY) {
			if ( sidsize <= 2 ) { //配列1つのみが与えられている->最適化のためスマートポインタ代入のみで済ませる
				arg.array_shared() = addv.array_shared();
			}
			else {
				arg.array().insert(arg.array().end(), addv.array().begin(), addv.array().end());
			}
		}
		else {
			arg.array().emplace_back(addv);
		}
	}

	// 実行
	CLocalVariable	t_lvar(*pvm);
	answer = pvm->function_exec().func[index].Execute(arg, t_lvar);

	// フィードバック
	const CValue *v_argv = &(t_lvar.GetArgvPtr()->value_const());
	int	i = 0;
	int	errcount = 0;

	for(it = sid.begin() + 1; it != sid.end(); it++, i++) {
		if (st.cell()[*it].value_GetType() == F_TAG_FEEDBACK) {
			CValueRef v_value = v_argv->array()[i];

			if (st.cell()[*it].order_const().GetType() != F_TAG_NOP)
				errcount += SubstToArray(st.cell()[(*it) + 1], st.cell()[*it], v_value, st, lvar);
			else {
				switch(st.cell()[(*it) + 1].value_GetType()) {
				case F_TAG_VARIABLE:
					pvm->variable().SetValue(st.cell()[(*it) + 1].index, v_value);
					break;
				case F_TAG_LOCALVARIABLE:
					lvar.SetValue(st.cell()[(*it) + 1].name, v_value);
					break;
				default:
					break;
				};
			}
		}
	}

	assert((errcount == 0)||(errcount == 1));
	return errcount ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::ExecSystemFunctionWithArgs
 *  機能概要：  引数付きのシステム関数を実行します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
bool CFunction::ExecSystemFunctionWithArgs(CCell &cell, std::vector<size_t> &sid, CStatement &st, CLocalVariable &lvar) {
	// 関数の格納位置を取得
	std::vector<size_t>::iterator it = sid.begin();
	size_t	func_index = *it;
	size_t	index = st.cell()[func_index].index;
	it++;

	// 引数作成
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
	std::vector<CCell *> pcellarg;
	std::vector<CValue> valuearg;
	std::vector<size_t>::size_type sidsize = sid.size();

	for( ; it != sid.end(); it++) {
		const CValue &addv = GetValueRefForCalc(st.cell()[*it], st, lvar);
		
		if (addv.GetType() == F_TAG_ARRAY) {
			if ( sidsize <= 2 ) { //配列1つのみが与えられている->最適化のためスマートポインタ代入のみで済ませる
				arg.array_shared() = addv.array_shared();
			}
			else {
				arg.array().insert(arg.array().end(), addv.array().begin(), addv.array().end());
			}
		}
		else {
			arg.array().emplace_back(addv);
		}

		valuearg.emplace_back(addv);
		pcellarg.emplace_back(&(st.cell()[*it]));
	}

	// 実行　%[n]処理関数のみ特例扱い
	if (index == CSystemFunction::HistoryIndex())
		ExecHistoryP1(func_index - 2, cell, arg, st);
	else
		cell.ansv() = pvm->sysfunction().Execute(index, arg, pcellarg, valuearg, lvar, st.linecount, this);

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::ExecHistoryP1
 *  機能概要：  %[n]（置換済の値の再利用）を処理します（前処理）
 *
 *  処理は二段階で行われます。前処理では本処理のための値を演算子の項へセットします。
 * -----------------------------------------------------------------------
 */
void	CFunction::ExecHistoryP1(size_t start_index, CCell& cell, const CValue &arg, CStatement &st)
{
	if (arg.array_size()) {
		cell.ansv()    = CValue((aya::int_t)start_index);
		cell.order()   = arg.array()[0];
	}
	else {
		pvm->logger().Error(E_E, 90, dicfilename, st.linecount);
		cell.ansv().SetType(F_TAG_VOID);
		cell.order().SetType(F_TAG_VOID);
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::ExecHistoryP2
 *  機能概要：  %[n]（置換済の値の再利用）を処理します（本処理）
 *
 *  処理は二段階で行われます。本処理では前処理で埋め込んだ値を参照して値を取得します。
 * -----------------------------------------------------------------------
 */
void	CFunction::ExecHistoryP2(CCell& cell, CStatement &st)
{
	if (!cell.order_const().IsNum())
		return;

	aya::int_t	index = cell.order_const().GetValueInt();
	if (index < 0)
		return;

	ptrdiff_t start = (ptrdiff_t)std::min(aya::int_t(st.cell().size())-1,cell.ansv_const().GetValueInt());

	for(ptrdiff_t i = start ; i >= 0; i--) {
		if (st.cell()[i].value_GetType() == F_TAG_STRING_EMBED) {
			if (!index) {
				cell.ansv_shared() = st.cell()[i].emb_shared_get();
				return;
			}
			index--;
		}
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::Feedback
 *  機能概要：  &演算子を処理します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
bool CFunction::Feedback(CCell &anscell, std::vector<size_t> &sid, CStatement &st, CLocalVariable &lvar) {
	CCell	*v_cell = &(st.cell()[sid[1]]);

	// 値は右辺をそのままコピー
	anscell.ansv() = GetValueRefForCalc(*v_cell, st, lvar);

	// 右辺が配列序数を指定する演算子だった場合はそこから序数をコピー
	// 配列でなかった場合は序数を格納する変数の型をNOPにしてフラグとする
	if (v_cell->value_GetType() == F_TAG_ARRAYORDER)
		anscell.order_shared() = v_cell->order_shared();
	else
		anscell.order().SetType(F_TAG_NOP);

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::EncodeArrayOrder
 *  機能概要：  配列の序数を作成して返します
 *
 *  CValue operator [] は引数として要素数2以上の配列型のCValueを要求します。
 *  （第一引数が序数、第二引数がデリミタ）
 *  この関数はそれを作成します。
 *
 *  エラーが発生した場合は型のない（F_TAG_UNKNOWN）値を返します。（呼び出し側はこれを見てエラー処理します）
 * -----------------------------------------------------------------------
 */
void CFunction::EncodeArrayOrder(CCell &vcell, const CValue &order, CLocalVariable &lvar, CValue &result)
{
	result.SetType(F_TAG_ARRAY);

	// 序数
	switch(order.GetType()) {
	case F_TAG_ARRAY:
		result = order;
		break;
	default:
		result.array().emplace_back(order);
		break;
	};

	// デリミタ
	if (result.array_size() < 2) {
		CValue	adddlm(VAR_DELIMITER);
		if (vcell.value_GetType() == F_TAG_VARIABLE)
			adddlm = pvm->variable().GetDelimiter(vcell.index);
		else if (vcell.value_GetType() == F_TAG_LOCALVARIABLE)
			adddlm = lvar.GetDelimiter(vcell.name);
		result.array().emplace_back(adddlm);
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::FeedLineToTail
 *  機能概要：  現在の{}の終端"}"まで実行行カウンタを送ります
 *
 *  実際に送るのは"}"の1つ手前の行の位置です
 * -----------------------------------------------------------------------
 */
void	CFunction::FeedLineToTail(size_t&line)
{
	size_t	t_statelenm1 = statelenm1;

	size_t	depth = 1;
	line++;
	for( ; line < t_statelenm1; line++) {
		depth += ((statement[line].type == ST_OPEN) - (statement[line].type == ST_CLOSE));
		if (!depth)
			break;
	}

	line--;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunctionDef::GetFunctionIndexFromName
 *  機能概要：  関数名に対応する配列の序数を取得します
 * -----------------------------------------------------------------------
 */
ptrdiff_t CFunctionDef::GetFunctionIndexFromName(const aya::string_t& name)
{
	if ( map.empty() ) {
		RebuildFunctionMap();
	}

	aya::indexmap::const_iterator it = map.find(name);
	if ( it != map.end() ) {
		return it->second;
	}
	return -1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunctionDef::AddFunctionIndex
 *  機能概要：  関数名に対応する配列の序数を追加します
 * -----------------------------------------------------------------------
 */
void CFunctionDef::AddFunctionIndex(const aya::string_t& name,size_t index)
{
	map.insert(aya::indexmap::value_type(name, index));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunctionDef::ClearFunctionIndex
 *  機能概要：  配列の序数キャッシュ用ハッシュを削除します
 * -----------------------------------------------------------------------
 */
void CFunctionDef::ClearFunctionIndex(void)
{
	map.clear();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunctionDef::RebuildFunctionMap
 *  機能概要：  配列の序数キャッシュ用ハッシュを構築します
 * -----------------------------------------------------------------------
 */
void CFunctionDef::RebuildFunctionMap(void)
{
	ClearFunctionIndex();
	for (size_t fcnt = 0; fcnt < func.size(); ++fcnt) {
		map.insert(aya::indexmap::value_type(func[fcnt].name,fcnt));
	}
}

void CFunctionDef::deep_copy_func(CFunctionDef &from)
{
	for (size_t fcnt = 0; fcnt < from.func.size(); ++fcnt) {
		func[fcnt].deep_copy_statement(from.func[fcnt]);
	}
}
