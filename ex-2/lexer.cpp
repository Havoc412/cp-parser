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

/* 数字后可跟随的字符 */
static const int numberNextPassNum = 16;
static const char numberNextPass[100] = {
    '+', '-', '*', '/', '=', '>', '<', '&', '|',
    '^', '%', '?', ')', ',', ';', ']'
};

/* 符号表 */
static std::map<TokenCode, int> tokenCodeMap;
static std::map<std::string, int> constantsMap;

/* 辅助函数 */
// 判断是否为字母
static bool isLetter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

// 判断是否为非ASCII字符（多字节UTF-8字符）
static bool isNonAscii(unsigned char ch) {
    return (ch >= 0x80);  // UTF-8编码中，第一个字节>=0x80表示多字节字符
}

// 检查数字后是否为合法字符
static bool checkNumberNext(char ch) {
    for (int i = 0; i < numberNextPassNum; i++) {
        if (ch == numberNextPass[i])
            return true;
    }
    return false;
}

// 尝试获取关键字ID
static int tryGetKeyWordID(const std::string& token) {
    for (int i = 0; i < keyWordTokenNum; i++) {
        if (token == keyWords[i]) 
            return i;
    }
    return -1;
}

// 添加错误信息
static void addError(const std::string& message) {
    ErrorInfo error = { g_row, message };
    g_errors.push_back(error);
    std::cerr << "Error at line " << g_row << ": " << message << std::endl;
}

// 处理一个Token
static TokenAttr processToken() {
    TokenAttr result;
    result.line = g_row;
    result.type = Table_TAG;
    result.table_row = 0;
    
    char ch;
    std::string token = "";
    TokenCode code = TK_UNDEF;
    
    // 跳过空白字符
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
            g_row++;
            continue;
        }
        
        if (ch == ' ' || ch == '\t' || ch == '\r')
            continue;
            
        break;  // 找到非空白字符，退出循环
    }
    
    // 处理各种Token类型
    if (isLetter(ch)) {  // 标识符或关键字
        token = ch;
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
            code = TK_IDENT;
        }
    }
    else if (isdigit(ch)) {  // 数字常量
        token = ch;
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
            code = (dotCount == 1) ? TK_DOUBLE : TK_INT;
        }
    }
    else {  // 运算符和分隔符
        token = ch;
        
        switch (ch) {
            case '+': code = TK_PLUS; break;
            case '-': code = TK_MINUS; break;
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
            case '(': code = TK_OPENPA; break;
            case ')': code = TK_CLOSEPA; break;
            case '[': code = TK_OPENBR; break;
            case ']': code = TK_CLOSEBR; break;
            case '{': code = TK_BEGIN; break;
            case '}': code = TK_END; break;
            case ',': code = TK_COMMA; break;
            case ';': code = TK_SEMOCOLOM; break;
            
            case '=': {  // = 或 ==
                ch = fgetc(g_fp);
                if (ch == '=') {
                    token += ch;
                    code = TK_EQ;
                } else {
                    ungetc(ch, g_fp);
                    code = TK_ASSIGN;
                }
                break;
            }
            
            case '<': {  // < 或 <=
                ch = fgetc(g_fp);
                if (ch == '=') {
                    token += ch;
                    code = TK_LEQ;
                } else {
                    ungetc(ch, g_fp);
                    code = TK_LT;
                }
                break;
            }
            
            case '>': {  // > 或 >=
                ch = fgetc(g_fp);
                if (ch == '=') {
                    token += ch;
                    code = TK_GEQ;
                } else {
                    ungetc(ch, g_fp);
                    code = TK_GT;
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
void initLexer(FILE* fp) {
    g_fp = fp;
    g_row = 1;
    g_hasUnget = false;
    g_errors.clear();
    tokenCodeMap.clear();
    constantsMap.clear();
}

TokenAttr getNextToken() {
    if (g_hasUnget) {
        g_hasUnget = false;
        return g_lastToken;
    }
    
    g_lastToken = processToken();
    return g_lastToken;
}

void ungetToken() {
    g_hasUnget = true;
}

int getCurrentLine() {
    return g_row;
}

const std::vector<ErrorInfo>& getErrors() {
    return g_errors;
}

void resetLexer() {
    if (g_fp) {
        rewind(g_fp);
        g_row = 1;
        g_hasUnget = false;
    }
}

void closeLexer() {
    g_fp = nullptr;
    g_hasUnget = false;
} 