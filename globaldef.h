﻿#ifndef AYA_STRTYPE_H__
#define AYA_STRTYPE_H__

#include <cwchar>
#include <string>
#include <map>

#include <memory>

#include "fix_old_compiler.h"

//32/64bit
#define JGX_WIN64_AWARE 

#ifdef _MSC_VER
#pragma warning (disable: 4786)
#pragma warning (disable: 4819)

#pragma warning (3: 4032 4057 4092)
#pragma warning (3: 4115 4121 4125 4130 4132 4134 4152 4189)
#pragma warning (3: 4207 4208 4209 4211 4212 4220 4223 4234 4238)
#pragma warning (3: 4355 4504 4505 4515)
//#pragma warning (3: 4663 4665 4670 4671 4672 4673 4674)
#pragma warning (3: 4665 4670 4671 4672 4673 4674)
//#pragma warning (3: 4701 4702 4705 4706 4709 4727)
#pragma warning (3: 4701 4705 4706 4709 4727)

#if _MSC_VER >= 1300
#undef JGX_WIN64_AWARE
#define JGX_WIN64_AWARE __w64
#endif //1300

#if _MSC_VER <= 1200
#define for if(0);else for
#endif //1200

#if _MSC_VER < 1900
#define emplace_back(p) push_back(p)
#endif //1900

#endif //MSC_VER

namespace aya {

#if _MSC_VER <= 1200
	typedef __int64 int64;
	typedef unsigned __int64 uint64;
#define	INT64_IS_NOT_STD
#else
	typedef long long int int64;
	typedef unsigned long long int uint64;
#ifdef _MSVC_LANG
#if _MSVC_LANG >= 201703
	static_assert(sizeof(int64)==8);
	static_assert(sizeof(uint64)==8);
#endif
#endif //_MSVC_LANG

#endif //1200

	typedef wchar_t char_t;
	typedef std::basic_string<char_t> string_t;
	typedef int64 int_t;

	typedef std_shared_ptr<string_t> share_string_t;
	typedef std_shared_ptr<const string_t> const_share_string_t;

	typedef std::map<aya::string_t,int> indexmap;

	template<class T> void shared_ptr_deep_copy(const std_shared_ptr<T> &from,std_shared_ptr<T> &to) {
		if( from.get() ) {
			to.reset(new T(*from));
		}
		else {
			to.reset();
		}
	}

#ifdef _WIN64
	typedef yaya_int64 native_signed;
#else
	typedef int JGX_WIN64_AWARE native_signed;
#endif

}; // namespace aya {

#endif // #ifndef AYA_STRTYPE_H__
