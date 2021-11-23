// 
// AYA version 5
//
// 雑用関数
// written by umeici. 2004
// 

#ifndef	MISCH
#define	MISCH

//----

#if defined(WIN32) || defined(_WIN32_WCE)
# include "stdafx.h"
#endif

#include <vector>

#include "globaldef.h"

aya::string_t::size_type Find_IgnoreDQ(const aya::string_t &str, const aya::char_t *findstr);
aya::string_t::size_type Find_IgnoreDQ(const aya::string_t &str, const aya::string_t &findstr);

aya::string_t::size_type find_last_str(const aya::string_t &str, const aya::char_t *findstr);
aya::string_t::size_type find_last_str(const aya::string_t &str, const aya::string_t &findstr);

char	Split(const aya::string_t &str, aya::string_t &dstr0, aya::string_t &dstr1, const aya::char_t *sepstr);
char	Split(const aya::string_t &str, aya::string_t &dstr0, aya::string_t &dstr1, const aya::string_t &sepstr);
char	SplitOnly(const aya::string_t &str, aya::string_t &dstr0, aya::string_t &dstr1, aya::char_t *sepstr);
char	Split_IgnoreDQ(const aya::string_t &str, aya::string_t &dstr0, aya::string_t &dstr1, const aya::char_t *sepstr);
char	Split_IgnoreDQ(const aya::string_t &str, aya::string_t &dstr0, aya::string_t &dstr1, const aya::string_t &sepstr);
size_t	SplitToMultiString(const aya::string_t &str, std::vector<aya::string_t> *array, const aya::string_t &delimiter);

void	CutSpace(aya::string_t &str);
void	CutStartSpace(aya::string_t &str);
void	CutEndSpace(aya::string_t &str);

void	CutDoubleQuote(aya::string_t &str);
void	CutSingleQuote(aya::string_t &str);
void	UnescapeSpecialString(aya::string_t &str);
void	AddDoubleQuote(aya::string_t &str);
void	CutCrLf(aya::string_t &str);

aya::string_t	GetDateString(void);

extern const aya::string_t::size_type IsInDQ_notindq;
extern const aya::string_t::size_type IsInDQ_runaway;
extern const aya::string_t::size_type IsInDQ_npos;
aya::string_t::size_type IsInDQ(const aya::string_t &str, aya::string_t::size_type startpoint, aya::string_t::size_type checkpoint);

char	IsDoubleButNotIntString(const aya::string_t &str);
char	IsIntString(const aya::string_t &str);
char	IsIntBinString(const aya::string_t &str, char header);
char	IsIntHexString(const aya::string_t &str, char header);

char	IsLegalFunctionName(const aya::string_t &str);
char	IsLegalVariableName(const aya::string_t &str);
char	IsLegalStrLiteral(const aya::string_t &str);
char	IsLegalPlainStrLiteral(const aya::string_t &str);

bool	IsUnicodeAware(void);

void	EscapeString(aya::string_t &wstr);
void	UnescapeString(aya::string_t &wstr);

void	EncodeBase64(aya::string_t &out,const char *in,size_t in_len);
void	DecodeBase64(std::string &out,const aya::char_t *in,size_t in_len);
void	EncodeURL(aya::string_t &out,const char *in,size_t in_len,bool isPlusPercent);
void	DecodeURL(std::string &out,const aya::char_t *in,size_t in_len,bool isPlusPercent);

inline bool IsSpace(const aya::char_t &c) {
#if !defined(POSIX) && !defined(__MINGW32__)
	return c == L' ' || c == L'\t' || c == L'　';
#else
	return c == L' ' || c == L'\t' || c == L'\u3000';
#endif
}

//----

// 関数呼び出しの限界を検査するためのクラス

#define	CCALLLIMIT_CALLDEPTH_MAX	32		//呼び出し限界デフォルト
#define	CCALLLIMIT_LOOP_MAX			10000	//ループ処理限界デフォルト
class	CCallLimit
{
protected:
	int	depth;
	int	maxdepth;
	int maxloop;
	std::vector<aya::string_t> stack;

public:
	CCallLimit(void) { depth = 0; maxdepth = CCALLLIMIT_CALLDEPTH_MAX; maxloop = CCALLLIMIT_LOOP_MAX; }

	void	SetMaxDepth(int value) { maxdepth = value; }
	int 	GetMaxDepth(void) { return maxdepth; }

	void	SetMaxLoop(int value) { maxloop = value; }
	int		GetMaxLoop(void) { return maxloop; }

	void	InitCall(void) { depth = 0; stack.clear(); }

	char	AddCall(const aya::string_t &str) {
		depth++;
		stack.emplace_back(str);
		if (maxdepth && depth > maxdepth)
			return 0;
		else
			return 1;
	}

	void	DeleteCall(void) {
		if ( depth ) {
			depth--;
			stack.erase(stack.end()-1);
		}
	}

	std::vector<aya::string_t> &StackCall(void) {
		return stack;
	}

	int temp_unlock() {
		int aret=0;
		using std::swap;
		swap(aret, maxdepth);
		return aret;
	}
	void reset_lock(int lock) {
		using std::swap;
		swap(lock, maxdepth);
	}
};

//----

#endif
