// 
// AYA version 5
//
// stl::wstringをchar*風に使うための関数など
// written by umeici. 2004
// 

#ifndef	WSEXH
#define	WSEXH

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
#endif

#include <cstdio>

#include "globaldef.h"

//----

namespace aya {
	static const int WS_EOF = -1;

	int		ws_atoi(const aya::string_t &str, int base = 10);
	aya::int_t ws_atoll(const aya::string_t& str, int base = 10);
	double	ws_atof(const aya::string_t &str);

	aya::string_t	ws_lltoa(aya::int_t num, int base = 10);
	aya::string_t	ws_itoa(int num, int base = 10);
	aya::string_t	ws_ftoa(double num);

	void	ws_eraseend(aya::string_t &str, wchar_t c);
	void	ws_replace(aya::string_t& str, const wchar_t* before, const wchar_t* after, aya::int_t count = 0);

	FILE	*w_fopen(const aya::char_t *fname, const aya::char_t *mode); 
	//void	write_utf8bom(FILE *fp);

	int		ws_fgets(aya::string_t &str, FILE *stream, int charset, int ayc, int lc, int cutspace = true);
	
	int		ws_fputs(const aya::char_t *str, FILE *stream, int charset, int ayc);
	inline int ws_fputs(const aya::string_t &str, FILE *stream, int charset, int ayc) {
		return ws_fputs(str.c_str(),stream,charset,ayc);
	}

	#if defined(__GNUC__)
		int snprintf(aya::char_t* buf, size_t count, const aya::char_t* format, ...)__attribute__((format(printf, 3, 4)));
	#elif defined(_MSC_VER)
		int snprintf(_Pre_notnull_ aya::char_t* buf, size_t count, _Printf_format_string_ const aya::char_t* format, ...);
	#else
		int snprintf(aya::char_t* buf, size_t count, const aya::char_t* format, ...);
	#endif

	template<class T>
	const T* get_safe_str(const T*a) {
		static constexpr T Zero_Termination_String[1] = { 0 };
		return a ? a : Zero_Termination_String;
	}
};

//----

#endif
