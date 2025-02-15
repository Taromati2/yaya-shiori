﻿// 
// AYA version 5
//
// 数式の項を扱うクラス　CCell/CSerial
// written by umeici. 2004
// 
// CCellはステートメント内の数式の項を、CSerialはその演算順序を保持するだけのクラスです。
// 操作はこれらのインスタンスを持つクラスが行ないます。
//

#ifndef	CELLH
#define	CELLH

//----

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
#endif

#include <vector>
#include <memory>

#include "value.h"
#include "globaldef.h"

class	CSerial
{
public:
	size_t	tindex;						// 演算子のCCell位置番号
	std::vector<size_t> index;				// 演算対象のCCell位置番号のリスト
									// 演算子がF_TAG_FUNCPARAM/F_TAG_SYSFUNCPARAMの場合は、index[0]が関数を示します
public:
	CSerial(int t) { tindex = t; }

	CSerial(void) { tindex = 0;  }
	~CSerial(void) {}
};

//----

class	CCell
{
public:
	aya::string_t	name;			// この項の"名前"（thisがローカル変数/関数の時に使用します）
	ptrdiff_t		index;			// 位置番号（thisが変数/ローカル変数/関数の際に使用します）
	ptrdiff_t		depth;			// {}入れ子の深さ（thisがローカル変数の時に使用します）

private:
	int		m_type;									// m_valueにタイプしか格納しない場合に使用

	mutable std_shared_ptr<CValue> m_value;		// 値（thisがリテラル値の際に使用します）
	mutable std_shared_ptr<CValue> m_ansv;		// 値（thisが変数/ローカル変数/関数の際に、その内容/返値を格納します）
	mutable std_shared_ptr<CValue> m_order;		// 演算時に使用（配列の序数を一時的に保持します）
	mutable std_shared_ptr<CValue> m_emb_ansv;	// 値（%[n]で参照される値を保持します）

public:
	CCell(int t)
	{
		index = -1;
		depth = -1;
		m_type  = t;
	}

	CCell(void)
	{
		index = -1;
		depth = -1;
		m_type  = F_TAG_STRING; //標準はSTRING
	}

	~CCell(void) {
	}

	//////////////////////////////////////
	void value_SetType(int type) {
		if( ! m_value.get() ) {
			m_type = type;
		}
		else {
			m_value->SetType(type);
		}
	}
	int value_GetType(void) const {
		if( ! m_value.get() ) {
			return m_type;
		}
		else {
			return m_value->GetType();
		}
	}
	//-------------------//
	std_shared_ptr<CValue> &value_shared(void) const {
		return m_value;
	}
	const CValue &value_const(void) const {
		if( ! m_value.get() ) {
			m_value=std::make_shared<CValue>(m_type,0);
		}
		return *m_value;
	}
	inline const CValue &value(void) const {
		return value_const();
	}
	CValue &value(void) {
		if( ! m_value.get() ) {
			m_value=std::make_shared<CValue>(m_type,0);
		}
		return *m_value;
	}
	void value_Delete(void) {
		if( m_value.get() ) {
			m_type = m_value->GetType();
			m_value.reset();
		}
	}
	//////////////////////////////////////
	std_shared_ptr<CValue> &ansv_shared(void) const {
		return m_ansv;
	}
	std_shared_ptr<CValue> &ansv_shared_create(void) const {
		if( ! m_ansv.get() ) {
			m_ansv=std::make_shared<CValue>();
		}
		else if( m_ansv.use_count() >= 2 ) {
			CValue *pV = m_ansv.get();
			m_ansv=std::make_shared<CValue>(*pV);
		}
		return m_ansv;
	}
	std_shared_ptr<CValue> &ansv_shared_get(void) const {
		if( ! m_ansv.get() ) {
			m_ansv=std::make_shared<CValue>();
		}
		return m_ansv;
	}
	const CValue &ansv_const(void) const {
		if( ! m_ansv.get() ) {
			return emptyvalue;
		}
		return *m_ansv;
	}
	inline const CValue &ansv(void) const {
		return ansv_const();
	}
	CValue &ansv(void) {
		if( ! m_ansv.get() ) {
			m_ansv=std::make_shared<CValue>();
		}
		return *m_ansv;
	}
	//////////////////////////////////////
	std_shared_ptr<CValue> &order_shared(void) const {
		return m_order;
	}
	const CValue &order_const(void) const {
		if( ! m_order.get() ) {
			return emptyvalue;
		}
		return *m_order;
	}
	inline const CValue &order(void) const {
		return order_const();
	}
	CValue &order(void) {
		if( ! m_order.get() ) {
			m_order=std::make_shared<CValue>();
		}
		return *m_order;
	}
	//////////////////////////////////////
	std_shared_ptr<CValue> &emb_ansv_shared(void) const {
		return m_emb_ansv;
	}
	std_shared_ptr<CValue> &emb_shared_create(void) const {
		if( ! m_emb_ansv.get() ) {
			m_emb_ansv=std::make_shared<CValue>();
		}
		else if( m_emb_ansv.use_count() >= 2 ) {
			CValue *pV = m_emb_ansv.get();
			m_emb_ansv=std::make_shared<CValue>(*pV);
		}
		return m_emb_ansv;
	}
	std_shared_ptr<CValue> &emb_shared_get(void) const {
		if( ! m_emb_ansv.get() ) {
			m_emb_ansv=std::make_shared<CValue>();
		}
		return m_emb_ansv;
	}
	const CValue &emb_ansv_const(void) const {
		if( ! m_emb_ansv.get() ) {
			return emptyvalue;
		}
		return *m_emb_ansv;
	}
	inline const CValue &emb_ansv(void) const {
		return emb_ansv_const();
	}
	CValue &emb_ansv(void) {
		if( ! m_emb_ansv.get() ) {
			m_emb_ansv=std::make_shared<CValue>();
		}
		return *m_emb_ansv;
	}
	//////////////////////////////////////
	void tmpdata_cleanup(void) const {
		{
			const CValue &c = ansv_const();
			if( (c.s_value.size() > 10000) || c.array_size() ) {
				m_ansv.reset();
			}
		}
		{
			const CValue &c = emb_ansv_const();
			if( (c.s_value.size() > 10000) || c.array_size() ) {
				m_emb_ansv.reset();
			}
		}
	}
};

//----

#endif
