#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <vector>
#include <string>

// 语法分析结果状态
enum ParserResult {
    RESULT_SUCCESS,      // 语法分析成功
    RESULT_ERROR         // 语法分析失败
};

// 语法错误信息结构体
struct ParserError {
    int line;               // 错误所在行
    std::string message;    // 错误信息
};

/* INFO 语法分析器接口 */

// 初始化语法分析器
void initParser(FILE* fp);

// 执行语法分析
ParserResult parse();

// 获取所有语法错误信息
const std::vector<ParserError>& getParserErrors();

// 重置语法分析器
void resetParser();

// 关闭语法分析器
void closeParser();

#endif /* PARSER_H */ 