﻿// 
// AYA version 5
//
// ログメッセージ
// written by umeici. 2004
// 
// 英語のメッセージは基本的に以下のサイト等で自動翻訳したものです。
// excite翻訳
// http://www.excite.co.jp/world/
//

#include "messages.h"
#include "file.h"
void ClearMessageArrays(){
	msgf.clear();
	msge.clear();
	msgw.clear();
	msgn.clear();
	msgj.clear();
}
void LoadMessageFromTxt(const aya::string_t &file,char cset){
	CFile1 txt(file,cset,L"r");
	txt.Open();
	MessageArray*ptr=&msgf;
	aya::string_t line;
	ClearMessageArrays();
	while(txt.Read(line)==1){
		if(line.substr(0,3)==L"!!!"){
			line=line.substr(3);
			#define tmp(name) \
			if(line==L ## #name)\
				ptr=&name
			tmp(msgf);
			else tmp(msge);
			else tmp(msgw);
			else tmp(msgn);
			else tmp(msgj);
			#undef tmp
		}
		else if(line.substr(0,2)==L"//")
			continue;
		else if(line.substr(0,1)==L"*")
			ptr->push_back(line.substr(1));
	}
	txt.Close();
}
// フェータルエラー文字列（日本語）
MessageArray msgf={
	L"",
};

// エラー文字列（日本語）
MessageArray msge= {
	L"error E0000 : 未知のエラーです.",
	L"error E0001 : 対応する関数名が見つかりません.",
	L"error E0002 : '}' 過多です.",
	L"error E0003 : 不正な関数名です.",
	L"error E0004 : 解析不能. '{' が必要と判断されます.",
	L"error E0005 : ファイルを開けません.",
	L"error E0006 : 簡易配列の変数名がありません.",
	L"error E0007 : 不完全な文字列定数です.",
	L"error E0008 : 不正なダブルクォーテーションです.",
	L"error E0009 : 内部エラー1が発生しました.",
	L"error E0010 : 解析不能な文字列.",
	L"error E0011 : 変数名(?)に使用できない文字が含まれています.",
	L"error E0012 : 変数名が予約語と一致しています.",
	L"error E0013 : 同一名の関数が重複して定義されています.",
	L"error E0014 : 簡易配列の序数が空です.",
	L"error E0015 : 内部エラー2が発生しました.",
	L"error E0016 : 内部エラー3が発生しました.",
	L"error E0017 : 文字列中に埋め込まれた簡易配列の変数名がありません.",
	L"error E0018 : 文字列中に埋め込まれた簡易配列の序数が空です.",
	L"error E0019 : 式の()が閉じていません.",
	L"error E0020 : 簡易配列の[]が不正、もしくは閉じていません.",
	L"error E0021 : 内部エラー4が発生しました.",
	L"error E0022 : 数式の記述に誤りがあります.",
	L"error E0023 : ','で区切られた要素を正しく取得できません.",
	L"error E0024 : ','で区切られた要素を正しく取得できません.",
	L"error E0025 : ','で区切られた引数に対応する関数が見つかりません.",
	L"error E0026 : 内部エラー5が発生しました.",
	L"error E0027 : 有効な要素が何も記述されていません.",
	L"error E0028 : 内部エラー23が発生しました.",
	L"error E0029 : この代入は処理できません.",
	L"error E0030 : 不正な重複回避モードが指定されました.",
	L"error E0031 : 内部エラー6が発生しました.",
	L"error E0032 : 必須の関数がありません.",		// no use
	L"error E0033 : 内部エラー7が発生しました.",
	L"error E0034 : 内部エラー8が発生しました.",
	L"error E0035 : 'if'に続く'{'がありません.",
	L"error E0036 : 'elseif'に続く'{'がありません.",
	L"error E0037 : 'else'に続く'{'がありません.",
	L"error E0038 : 'switch'に続く'{'がありません.",
	L"error E0039 : 'while'に続く'{'がありません.",
	L"error E0040 : 'for'の終了条件式が異常です.",
	L"error E0041 : 'for'のループ式が異常です.",
	L"error E0042 : 'for'に続く'{'がありません.",
	L"error E0043 : 'foreach'の値取得変数が不正です.",
	L"error E0044 : 'foreach'に続く'{'がありません.",
	L"error E0045 : 'when'ラベルに定数でない要素が記述されています.",
	L"error E0046 : 'when'ラベルには、変数/関数を埋め込んだ文字列は使用できません.",
	L"error E0047 : 対応する'if'、'elseif'もしくは'case'が見つかりません.",
	L"error E0048 : 式の()が閉じていません.",
	L"error E0049 : 内部エラー9が発生しました.",
	L"error E0050 : 'when'ラベルに定数でない要素が記述されています.",
	L"error E0051 : 'case'に続く'{'がありません.",
	L"error E0052 : 内部エラー10が発生しました.",
	L"error E0053 : 'when'ラベルの書式が不正です.",
	L"error E0054 : 'when'ラベルの書式が不正です.",
	L"error E0055 : 内部エラー11が発生しました.",
	L"error E0056 : 内部エラー12が発生しました.",
	L"error E0057 : 変数値をファイルに保存できません.",
	L"error E0058 : 内部エラー13が発生しました.",
	L"error E0059 : 数式の記述に誤りがあります.",
	L"error E0060 : 文字列中に\"%()\"で埋め込まれた要素の()が閉じていません.",
	L"error E0061 : 文字列中に\"%()\"で埋め込まれた要素が空です.",
	L"error E0062 : 内部エラー14が発生しました.",
	L"error E0063 : 'when'に対応する'case'がありません.",
	L"error E0064 : 'when'に対応する'case'がありません.",
	L"error E0065 : 'when'ラベルの書式が不正です.",
	L"error E0066 : 'when'ラベルの書式が不正です.",
	L"error E0067 : 内部エラー15が発生しました.",
	L"error E0068 : この'elseif'もしくは'when'を処理できませんでした.",
	L"error E0069 : この'else'もしくは'others'を処理できませんでした.",
	L"error E0070 : 内部エラー16が発生しました.",
	L"error E0071 : 存在しない関数を実行しようとしています.",
	L"error E0072 : 簡易配列の変数名がありません.",
	L"error E0073 : 簡易配列の使用法に不正があります. 変数が見つかりません.",
	L"error E0074 : プリプロセッサの書式に誤りがあります.",
	L"error E0075 : プリプロセッサの書式に誤りがあります.",
	L"error E0076 : 不正なプリプロセス名です.",
	L"error E0077 : 内部エラー17が発生しました.",
	L"error E0078 : 文字列中に'%[]'で埋め込まれた要素の[]が閉じていません.",
	L"error E0079 : 文字列中に'%[]'で埋め込まれた要素が空です.",
	L"error E0080 : 内部エラー18が発生しました.",
	L"error E0081 : '%[]'に先行する関数、もしくは変数の呼び出しがありません.",
	L"error E0082 : 内部エラー19が発生しました.",
	L"error E0083 : 内部エラー20が発生しました.",
	L"error E0084 : 内部エラー21が発生しました.",		// not using
	L"error E0085 : 内部エラー22が発生しました.",		// not using
	L"error E0086 : 書式エラー.",
	L"error E0087 : フィードバック演算子の位置が不正です.",
	L"error E0088 : 内部エラー24が発生しました.",
	L"error E0089 : 内部エラー25が発生しました.",
	L"error E0090 : 内部エラー26が発生しました.",
	L"error E0091 : 内部エラー27が発生しました.",
	L"error E0092 : ()、もしくは[]に続く演算子が見つかりません.",
	L"error E0093 : 不正なシングルクォーテーションです.",
	L"error E0094 : 辞書内部で{}が不正、もしくは閉じていません.",
	L"error E0095 : 読み込み済の辞書ファイルを再度読もうとしました.",
};

// ワーニング文字列（日本語）
MessageArray msgw = {
	L"warning W0000 : 文法エラー. この行は無視します.",
	L"warning W0001 : この行の変数の復元に失敗しました. この行を解析できません.",
	L"warning W0002 : この行の変数の復元に失敗しました. 変数名を取得できませんでした.",
	L"warning W0003 : この行の変数の復元に失敗しました. 変数値またはデリミタを取得できませんでした.",
	L"warning W0004 : この行の変数の復元に失敗しました. 変数値が不正です.",
	L"warning W0005 : この行の変数の復元に失敗しました. デリミタを取得できませんでした.",
	L"warning W0006 : この行を処理中に内部エラーが発生しました. この行を解析できません.",
	L"warning W0007 : この変数の値を保存できませんでした.",
	L"warning W0008 : 引数が不足しています.",
	L"warning W0009 : 引数の型が不正です.",
	L"warning W0010 : 空文字は指定できません.",
	L"warning W0011 : 結果を変数に対して設定できません.",
	L"warning W0012 : 指定された値は範囲外、もしくは無効です.",
	L"warning W0013 : 処理に失敗しました.",
	L"warning W0014 : 指定されたライブラリはロードされていません.",
	L"warning W0015 : 指定されたファイルはオープンしていません.",
	L"warning W0016 : 正規表現に誤りがあります. 処理されませんでした.",
	L"warning W0017 : 正規表現処理中に未定義のエラーが発生しました.",
	L"warning W0018 : 変数を指定してください.",
	L"warning W0019 : 割り算にゼロを指定することはできません.",
	L"warning W0020 : 連想配列の生成には偶数個の要素が必要です.",
	L"warning W0021 : 空のヒアドキュメントがあります.",
	L"warning W0022 : ヒアドキュメント開始文字列が(ヒアドキュメント中に)見つかりました.",
};

// 注記文字列（日本語）
MessageArray msgn = {
	L"note N0000 : 前回実行時の変数値の復元は行われませんでした.",
	L"note N0001 : 無効な文字コードセットが指定されました. OSのデフォルト値を使用します.",
};

// その他のログ文字列（日本語）
MessageArray msgj = {
	L"// 文　リクエストログ\n// load 時刻 : ",
	L"// unload 時刻 : ",
	L"",	// 欠番
	L"// 辞書ファイル読み込み\n",
	L"// 構文解析結果のレポート (@:関数、$:システム関数、#:配列)\n\n",
	L"// 変数定義状態レポート\n",
	L"// 変数値の復元 ",
	L"// 変数値の保存 ",
	L"...完了.\n\n",
	L"// 辞書の構文解析と中間コード生成\n",
	L"// 辞書エラーを検出：緊急モードで再読み込み中...\n",
	L"// 辞書エラーを検出：緊急モードで再読み込みしました\n",
};

