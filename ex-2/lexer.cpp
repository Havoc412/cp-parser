#include "lexer.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <cstdlib>

/* 全局变量 */
static FILE* g_fp = nullptr;              // 文件指针
static int g_row = 1;                     // 当前行号
static TokenAttr g_lastToken;             // 上一个Token（用于回退）
static bool g_hasUnget = false;           // 是否有回退的Token
static std::vector<ErrorInfo> g_errors;   // 错误信息列表

/* 关键字表 */
static const int keyWordTokenNum = 8;
static const char keyWords[][10] = {
    "int", "double", "float", "if", "then", "else", "return", "while"
};

/* TokenCode到关键字的映射 */
static const TokenCode keyWordCodes[] = {
    KW_INT, KW_DOUBLE, KW_FLOAT, KW_IF, KW_THEN, KW_ELSE, KW_RETURN, KW_WHILE
};

/**
 * 数字后可跟随的字符
 * 用于"吞噬"错误的数字词法，确保正确处理数字后接的合法字符。
 * 例如，在"10+"中，数字10后跟的+是合法的。
*/
static const int numberNextPassNum = 16;
static const char numberNextPass[100] = {
    '+', '-', '*', '/', '=', '>', '<', '&', '|',
    '^', '%', '?', ')', ',', ';', ']'
};

/* INFO 符号表 */
static std::map<TokenCode, int> tokenCodeMap;
static std::map<std::string, int> constantsMap;

/* 辅助函数 */
// 判断是否为字母
static bool isLetter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

// 判断是否为非ASCII字符（多字节UTF-8字符）
// 支持处理中文等非ASCII字符，增强词法分析器的健壮性
static bool isNonAscii(unsigned char ch) {
    return (ch >= 0x80);  // UTF-8编码中，第一个字节>=0x80表示多字节字符
}

// 检查数字后是否为合法字符
// 如果返回true，说明ch是数字后面可以直接跟的合法字符
// 如果返回false，说明ch是非法字符，表明这是一个数字词法错误
static bool checkNumberNext(char ch) {
    for (int i = 0; i < numberNextPassNum; i++) {
        if (ch == numberNextPass[i])
            return true;
    }
    return false;
}

// 尝试获取关键字ID
// 如果token是关键字，返回其在keyWords数组中的索引
// 否则返回-1，表示这是一个普通标识符
static int tryGetKeyWordID(const std::string& token) {
    for (int i = 0; i < keyWordTokenNum; i++) {
        if (token == keyWords[i]) 
            return i;
    }
    return -1;
}

// 添加错误信息
// 将错误添加到错误列表中，并输出到标准错误流
static void addError(const std::string& message) {
    ErrorInfo error = { g_row, message };
    g_errors.push_back(error);
    std::cerr << "Error at line " << g_row << ": " << message << std::endl;
}

// 处理一个Token
// 这是词法分析器的核心函数，负责从输入流中读取字符并识别Token
static TokenAttr processToken() {
    TokenAttr result;
    result.line = g_row;
    result.type = Table_TAG;
    result.table_row = 0;
    
    char ch;
    std::string token = "";
    TokenCode code = TK_UNDEF;
    
    // 跳过空白字符（空格、制表符、换行符等）
    while (true) {
        ch = fgetc(g_fp);
        if (ch == EOF) {
            result.code = TK_EOF;
            result.value = "EOF";
            return result;
        }
        
        if (ch == '#')  { // 特殊终止符
            result.code = TK_EOF;
            result.value = "#";
            return result;
        }
        
        if (ch == '\n') {
            g_row++;  // 遇到换行符，行号加1
            continue;
        }
        
        if (ch == ' ' || ch == '\t' || ch == '\r')
            continue;
            
        break;  // 找到非空白字符，退出循环
    }
    
    // 处理各种Token类型
    if (isLetter(ch)) {  // 标识符或关键字
        token = ch;
        // 读取标识符或关键字的剩余部分
        while (true) {
            ch = fgetc(g_fp);
            if (!(isLetter(ch) || isdigit(ch))) {
                ungetc(ch, g_fp);  // 回退一个字符
                break;
            }
            token += ch;
        }
        
        // 检查是否为关键字
        int keywordIndex = tryGetKeyWordID(token);
        if (keywordIndex != -1) {
            code = keyWordCodes[keywordIndex];
        } else {
            code = TK_IDENT;  // 不是关键字，是标识符
        }
    }
    else if (isdigit(ch)) {  // 处理数字（整数或浮点数）
        token = ch;
        int dotCount = 0;
        
        while (true) {
            ch = fgetc(g_fp);
            if (!(isdigit(ch) || ch == '.')) {
                break;
            }
            
            if (ch == '.') {
                dotCount++;  // 记录小数点的数量
            }
            
            token += ch;
        }
        
        // 检查数字格式是否正确
        if (!checkNumberNext(ch)) {
            // 非法后缀，继续读取错误标记
            token += ch;
            while (true) {
                ch = fgetc(g_fp);
                if (!(isLetter(ch) || isdigit(ch))) {
                    ungetc(ch, g_fp);
                    break;
                }
                token += ch;
            }
            code = TK_UNDEF;
            addError("Invalid number format: " + token);
        }
        else if (dotCount > 1) {
            code = TK_UNDEF;
            addError("Invalid number format (multiple decimal points): " + token);
        }
        else {
            ungetc(ch, g_fp);  // 回退用于检查的字符
            code = (dotCount == 1) ? TK_DOUBLE : TK_INT;  // 有小数点是浮点数，否则是整数
        }
    }
    else {  // 处理运算符和分隔符
        token = ch;
        
        switch (ch) {
            case '+': code = TK_PLUS; break;
            case '-': 
                // 检查是否为负数（如-10）或减号运算符
                ch = fgetc(g_fp);
                if (isdigit(ch)) {
                    // 处理负数
                    token += ch;  // 添加数字到token
                    int dotCount = 0;
                    
                    while (true) {
                        ch = fgetc(g_fp);
                        if (!(isdigit(ch) || ch == '.')) {
                            break;
                        }
                        
                        if (ch == '.') {
                            dotCount++;
                        }
                        
                        token += ch;
                    }
                    
                    // 检查数字格式是否正确
                    if (!checkNumberNext(ch)) {
                        // 非法后缀，继续读取错误标记
                        token += ch;
                        while (true) {
                            ch = fgetc(g_fp);
                            if (!(isLetter(ch) || isdigit(ch))) {
                                ungetc(ch, g_fp);
                                break;
                            }
                            token += ch;
                        }
                        code = TK_UNDEF;
                        addError("Invalid number format: " + token);
                    }
                    else if (dotCount > 1) {
                        code = TK_UNDEF;
                        addError("Invalid number format (multiple decimal points): " + token);
                    }
                    else {
                        ungetc(ch, g_fp);  // 回退用于检查的字符
                        code = (dotCount == 1) ? TK_DOUBLE : TK_INT;  // 有小数点是浮点数，否则是整数
                    }
                } else {
                    // 不是负数，是减号运算符
                    ungetc(ch, g_fp);
                    code = TK_MINUS;
                }
                break;
            case '*': code = TK_STAR; break;
            case '/': {
                // 检查是否为注释
                ch = fgetc(g_fp);
                if (ch == '/') {
                    // 处理单行注释，读取直到行尾
                    while ((ch = fgetc(g_fp)) != EOF && ch != '\n');
                    if (ch == '\n') {
                        g_row++; // 增加行号
                    }
                    // 递归调用以获取下一个真正的token
                    return processToken();
                } else {
                    // 不是注释，回退字符
                    ungetc(ch, g_fp);
                    code = TK_DIVIDE;
                }
                break;
            }
            // 处理各种分隔符
            case '(': code = TK_OPENPA; break;
            case ')': code = TK_CLOSEPA; break;
            case '[': code = TK_OPENBR; break;
            case ']': code = TK_CLOSEBR; break;
            case '{': code = TK_BEGIN; break;
            case '}': code = TK_END; break;
            case ',': code = TK_COMMA; break;
            case ';': code = TK_SEMOCOLOM; break;
            
            case '=': {  // 处理 = 或 ==
                ch = fgetc(g_fp);
                if (ch == '=') {
                    token += ch;
                    code = TK_EQ;  // 等于运算符 ==
                } else {
                    ungetc(ch, g_fp);
                    code = TK_ASSIGN;  // 赋值运算符 =
                }
                break;
            }
            
            case '<': {  // 处理 < 或 <=
                ch = fgetc(g_fp);
                if (ch == '=') {
                    token += ch;
                    code = TK_LEQ;  // 小于等于运算符 <=
                } else {
                    ungetc(ch, g_fp);
                    code = TK_LT;  // 小于运算符 <
                }
                break;
            }
            
            case '>': {  // 处理 > 或 >=
                ch = fgetc(g_fp);
                if (ch == '=') {
                    token += ch;
                    code = TK_GEQ;  // 大于等于运算符 >=
                } else {
                    ungetc(ch, g_fp);
                    code = TK_GT;  // 大于运算符 >
                }
                break;
            }
            
            default: 
                if (isNonAscii((unsigned char)ch)) {
                    // 跳过非ASCII字符（如中文），不报错
                    // 读取此UTF-8字符的剩余字节
                    int byteCount = 0;
                    if ((ch & 0xE0) == 0xC0) byteCount = 1;      // 2字节字符
                    else if ((ch & 0xF0) == 0xE0) byteCount = 2; // 3字节字符
                    else if ((ch & 0xF8) == 0xF0) byteCount = 3; // 4字节字符
                    
                    // 跳过剩余字节
                    for (int i = 0; i < byteCount; i++) {
                        fgetc(g_fp);
                    }
                    
                    // 递归调用以获取下一个真正的token
                    return processToken();
                } else {
                    code = TK_UNDEF;
                    addError("Unknown symbol: " + token);
                }
                break;
        }
    }
    
    // 填充结果
    result.code = code;
    result.value = token;
    
    // 处理符号表
    if (code == TK_IDENT) {
        if (tokenCodeMap.find(code) == tokenCodeMap.end()) {
            tokenCodeMap[code] = tokenCodeMap.size() + 1;
        }
        result.table_row = tokenCodeMap[code];
    }
    else if (code == TK_INT || code == TK_DOUBLE) {
        result.type = Table_CONSTANT;
        if (constantsMap.find(token) == constantsMap.end()) {
            constantsMap[token] = constantsMap.size() + 1;
        }
        result.table_row = constantsMap[token];
    }
    
    return result;
}

/* 接口实现 */
// 初始化词法分析器
void initLexer(FILE* fp) {
    g_fp = fp;
    g_row = 1;
    g_hasUnget = false;
    g_errors.clear();
    tokenCodeMap.clear();
    constantsMap.clear();
}

// 获取下一个Token
// 如果有回退的Token，则直接返回；否则处理并返回新的Token
TokenAttr getNextToken() {
    if (g_hasUnget) {  // 回退 token
        g_hasUnget = false;
        return g_lastToken;
    }
    
    g_lastToken = processToken();
    return g_lastToken;
}

// 回退一个Token
// 标记有回退的Token，下次调用getNextToken时将返回此Token
void ungetToken() {
    g_hasUnget = true;
}

// 获取当前行号
int getCurrentLine() {
    return g_row;
}

// 获取所有词法错误信息
const std::vector<ErrorInfo>& getErrors() {
    return g_errors;
}

// 重置词法分析器
// 将文件指针重置到文件开头，重新开始词法分析
void resetLexer() {
    if (g_fp) {
        rewind(g_fp);
        g_row = 1;
        g_hasUnget = false;
    }
}

// 关闭词法分析器
void closeLexer() {
    g_fp = nullptr;
    g_hasUnget = false;
}