﻿// 
// AYA version 5
//
// 値を扱うクラス　CValue/CValue
// written by umeici. 2004
// 
// CValueは型フラグと型別の情報を持っています。その中にvector<CValue>という可変長配列があり、
// これが汎用配列となります。CValueクラスは配列を持たないほかはCValueと類似しています（汎用
// 配列を多次元化出来ない、というAYA5の制限はこの構造に由来しています）。
// 

#ifndef	VALUEH
#define	VALUEH

//----

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
#endif

#include <vector>
#include <memory>
//modified by Neo 2015.oct
#include <climits>
//modified by Neo 2015.oct

#include "manifest.h"
#include "globaldef.h"

class CValue;

typedef std::vector<CValue> CValueArray;
typedef std::map<CValue, CValue> CValueHash;

class	CValue
{
	friend class CValue;

protected:
	int	type;						// 型
public:
	aya::string_t	s_value;				// 文字列値
	double			d_value;				// 実数値
	aya::int_t		i_value;				// 整数値

	int Compare(const CValue &value) const;
	int Great(const CValue &value) const;
	int Less(const CValue &value) const;
private:
	mutable std_shared_ptr<CValueArray> m_array;		// 汎用配列
    mutable std_shared_ptr<CValueHash> m_hash;  // �n�b�V��

private:
	int CalcEscalationTypeNum(const int rhs) const;
	int CalcEscalationTypeStr(const int rhs) const;

public:
	CValue(const CValue &rhs)
	{
		type = rhs.type;
		//i,dはサイズが小さいのでコピーしたほうが手っ取り早い
		i_value = rhs.i_value;
		d_value = rhs.d_value;

		if( type == F_TAG_ARRAY ) {
			m_array = rhs.m_array;
		}
        else if (type == F_TAG_HASH) {
            m_hash = rhs.m_hash;
        }
		else if ( type == F_TAG_STRING ) {
			s_value = rhs.s_value;
		}
	}
	CValue& operator =(const CValue &rhs)
	{
		type = rhs.type;
		//i,dはサイズが小さいのでコピーしたほうが手っ取り早い
		i_value = rhs.i_value;
		d_value = rhs.d_value;

		if( type == F_TAG_ARRAY ) {
			m_array = rhs.m_array;
		}
        else if (type == F_TAG_HASH) {
            m_hash = rhs.m_hash;
        }
		else {
			m_array.reset();
            m_hash.reset((CValueHash*)NULL);
			if ( type == F_TAG_STRING ) {
				s_value = rhs.s_value;
			}
		}
		return *this;
	}

	CValue(int value) :
		type(F_TAG_INT) , d_value(0.0) , i_value(value) { }

	CValue(aya::int_t value) :
		type(F_TAG_INT) , d_value(0.0) , i_value(value) { }

	CValue(double value) :
		type(F_TAG_DOUBLE) , d_value(value), i_value(0) { }

	CValue(const aya::string_t &value) :
		type(F_TAG_STRING) , s_value(value), d_value(0.0), i_value(0) { }

	CValue(const aya::char_t *value) :
		type(F_TAG_STRING) , s_value(value), d_value(0.0), i_value(0) { }

	CValue(int tp, int) : type(tp), d_value(0.0), i_value(0) { }	// 型指定して初期化
	CValue(void) : type(F_TAG_VOID), d_value(0.0), i_value(0) { }
	~CValue(void) {}

	inline void		SetType(int tp) { type = tp; }
	inline int		GetType(void) const { return type; }

	inline bool		IsVoid(void) const { return type == F_TAG_VOID; }
	inline bool		IsString(void) const { return type == F_TAG_STRING || type == F_TAG_VOID; }
	inline bool		IsStringReal(void) const { return type == F_TAG_STRING; }
	inline bool		IsInt(void) const { return type == F_TAG_INT || type == F_TAG_VOID; }
	inline bool		IsIntReal(void) const { return type == F_TAG_INT; }
	inline bool		IsDouble(void) const { return type == F_TAG_DOUBLE || type == F_TAG_VOID; }
	inline bool		IsDoubleReal(void) const { return type == F_TAG_DOUBLE; }
	inline bool		IsArray(void) const { return type == F_TAG_ARRAY; }
    inline bool     IsHash(void) const { return type == F_TAG_HASH; }

	inline bool		IsNum(void) const { return type == F_TAG_INT || type == F_TAG_DOUBLE || type == F_TAG_VOID; }

	bool	GetTruth(void) const
	{
		switch(type) {
		case F_TAG_VOID:   return 0;
		case F_TAG_INT:	   return i_value;
		case F_TAG_DOUBLE: return d_value;
		case F_TAG_STRING: return s_value.size();
		case F_TAG_ARRAY:
			if( m_array.get() ) {
				return m_array->size();
			}
			else {
				return 0;
			}
        case F_TAG_HASH:
            if ( m_hash.get() ) {
                return m_hash->size() != 0;
            }
            else {
                return 0;
            }
		default:
			break;
		};
		return 0;
	}

	aya::int_t		GetValueInt(void) const;
	double			GetValueDouble(void) const;
	aya::string_t	GetValueString(void) const;
	aya::string_t	GetValueStringForLogging(void) const;

	void	SetArrayValue(const CValue &oval, const CValue &value);

	bool	DecodeArrayOrder(size_t& order, size_t& order1, aya::string_t& delimiter) const;

	CValue	&operator =(aya::int_t value)&;
	CValue	&operator =(bool value)&;
	CValue	&operator =(double value)&;
	CValue	&operator =(const aya::string_t &value)&;
	CValue	&operator =(aya::string_t&& value)&;
	CValue	&operator =(const aya::char_t *value)&;
	CValue	&operator =(const CValueArray &value)&;
    CValue  &operator =(const CValueHash &value)&;

	void SubstToArray(CValueArray &value)&;

	CValue	operator +(const CValue &value) const;
	CValue	operator -(const CValue &value) const;
	CValue	operator *(const CValue &value) const;
	CValue	operator /(const CValue &value) const;
	CValue	operator %(const CValue &value) const;

	void	operator +=(const CValue &value)&;
	void	operator -=(const CValue &value)&;
	void	operator *=(const CValue &value)&;
	void	operator /=(const CValue &value)&;
	void	operator %=(const CValue &value)&;

	CValue	operator [](const CValue &value) const;

	inline bool operator ==(const CValue &value) const {
		return Compare(value);
	}
	inline bool operator !=(const CValue &value) const {
		return !Compare(value);
	}
	inline bool operator>(const CValue &value) const {
		return Great(value);
	}
	inline bool operator<=(const CValue &value) const {
		return !Great(value);
	}
	inline bool operator<(const CValue &value) const {
		return Less(value);
	}
	inline bool operator>=(const CValue &value) const {
		return !Less(value);
	}

	inline bool operator||(const CValue &value) const {
		return GetTruth() || value.GetTruth();
	}
	inline bool operator&&(const CValue &value) const {
		return GetTruth() && value.GetTruth();
	}

	//////////////////////////////////////////////
	CValueArray::size_type array_size(void) const {
		if( ! m_array.get() ) {
			return 0;
		}
		else {
			return m_array->size();
		}
	}
	std_shared_ptr<CValueArray> &array_shared(void) const {
		return m_array;
	}
	const CValueArray& array(void) const {
		if( ! m_array.get() ) {
			m_array=std::make_shared<CValueArray>();
		}
		return *m_array;
	}
	CValueArray& array(void) {
		if( ! m_array.get() ) {
			m_array=std::make_shared<CValueArray>();
		}
		else if( m_array.use_count() >= 2 ) {
			CValueArray *pV = m_array.get();
			m_array=std::make_shared<CValueArray>(*pV);
		}
		return *m_array;
	}

	//////////////////////////////////////////////
	CValueHash::size_type hash_size(void) const {
		if ( ! m_hash.get() ) {
			return 0;
		}
		else {
			return m_hash->size();
		}
	}
	std::shared_ptr<CValueHash> &hash_shared(void) const {
		return m_hash;
	}
	const CValueHash &hash(void) const;
	CValueHash	   &hash(void);
	inline void array_clear(void) {
		m_array.reset();
	}
};
//----
// std::hash 的自定义特化能注入 namespace std
namespace std {
	template<>
	struct hash<CValue> {
		typedef CValue		argument_type;
		typedef std::size_t result_type;
		result_type			operator()(argument_type const &s) const {
			using std::hash;
			switch(s.GetType()) {
			default:
				return 0;
			case F_TAG_INT:
				return std::hash<aya::int_t>{}(s.GetValueInt());
			case F_TAG_DOUBLE:
				return std::hash<double>{}(s.GetValueDouble());
			case F_TAG_STRING:
				return std::hash<aya::string_t>{}(s.GetValueString());
			case F_TAG_VOID:
				return 0;
			case F_TAG_ARRAY:
				{
					std::size_t hash_value = 0;
					for(CValueArray::size_type i=0; i<s.array_size(); ++i) {
						hash_value += hash<CValue>{}(s.array()[i]);
						hash_value *= 17;
					}
					return hash_value;
				}
			case F_TAG_HASH:
				{
					std::size_t hash_value = 0;
					for(CValueHash::const_iterator it=s.hash().begin(); it!=s.hash().end(); ++it) {
						hash_value += hash<CValue>{}(it->first);
						hash_value *= 31;
						hash_value += hash<CValue>{}(it->second);
						hash_value *= 17;
					}
					return hash_value;
				}
			}
		}
	};
}		// namespace std


//からっぽ変数（ダミー用）
extern const CValue emptyvalue;

#endif
