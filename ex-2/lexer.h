#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <cstdio>

/* 单词编码 */
enum TokenCode
{
    /* 未定义 */
    TK_UNDEF = 0,
    /* 关键字 */
    KW_INT,      // int
    KW_DOUBLE,   // double
    KW_FLOAT,    // float
    KW_IF,       // if
    KW_THEN,     // then
    KW_ELSE,     // else
    KW_RETURN,   // return
    KW_WHILE,    // while
    /* 运算符 */
    TK_PLUS,     // +
    TK_MINUS,    // -
    TK_STAR,     // *
    TK_DIVIDE,   // /
    TK_ASSIGN,   // =
    TK_EQ,       // ==
    TK_LT,       // <
    TK_LEQ,      // <=
    TK_GT,       // >
    TK_GEQ,      // >=
    /* 分隔符 */
    TK_OPENPA,   // (
    TK_CLOSEPA,  // )
    TK_OPENBR,   // [
    TK_CLOSEBR,  // ]
    TK_BEGIN,    // {
    TK_END,      // }
    TK_COMMA,    // ,
    TK_SEMOCOLOM, // ;
    /* 常量 */
    TK_INT,      // 整型常量
    TK_DOUBLE,   // 浮点型常量
    /* 标识符 */
    TK_IDENT,
    /* 文件结束 */
    TK_EOF       // 文件结束标记
};

/* 符号表类型 */
enum TableTypeId {
    Table_TAG = 1,
    Table_CONSTANT,
};

/* Token属性结构体 */
struct TokenAttr {
    TokenCode code;      // Token类型
    int line;            // 行号
    TableTypeId type;    // 符号表类型
    int table_row;       // 符号表行号
    std::string value;   // Token的值
};

/* 错误信息结构体 */
struct ErrorInfo {
    int line;               // 错误所在行
    std::string message;    // 错误信息
};

/* 词法分析器接口 */
// 初始化词法分析器
void initLexer(FILE* fp);

// 获取下一个Token
TokenAttr getNextToken();

// 回退一个Token（用于预读）
void ungetToken();

// 获取当前行号
int getCurrentLine();

// 获取所有错误信息
const std::vector<ErrorInfo>& getErrors();

// 重置词法分析器
void resetLexer();

// 关闭词法分析器
void closeLexer();

#endif /* LEXER_H */ 