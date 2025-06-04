#include "parser.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

/* 全局变量 */
static TokenAttr g_token;               // 当前分析的Token
static std::vector<ParserError> g_errors; // 语法错误列表
static bool g_hasError = false;         // 是否有语法错误

/*
 * Mini语言BNF文法定义 - 分层结构
 * ===========================
 *
 * 第1层：程序结构层
 * <program> ::= <function-definition>+  // 程序由一个或多个函数定义组成
 *
 * 第2层：函数定义层
 * <function-definition> ::= <type-specifier> <identifier> '(' <parameter-list>? ')' <compound-statement>
 * <type-specifier> ::= 'int' | 'double' | 'float'  // 函数返回类型
 * <parameter-list> ::= <parameter-declaration> | <parameter-list> ',' <parameter-declaration>
 * <parameter-declaration> ::= <type-specifier> <identifier>
 *
 * 第3层：语句层
 * <compound-statement> ::= '{' <statement-list>? '}'  // 复合语句（代码块）
 * <statement-list> ::= <statement> | <statement-list> <statement>
 * <statement> ::= <expression-statement>  // 表达式语句
 *               | <compound-statement>    // 复合语句
 *               | <selection-statement>   // 选择语句（if-else）
 *               | <iteration-statement>   // 循环语句（while）
 *               | <return-statement>      // 返回语句
 *
 * 第4层：具体语句定义
 * <expression-statement> ::= <expression>? ';'
 * <selection-statement> ::= 'if' '(' <expression> ')' <statement> ('else' <statement>)?
 *                         | 'if' '(' <expression> ')' 'then' <statement> ('else' <statement>)?
 * <iteration-statement> ::= 'while' '(' <expression> ')' <statement>
 * <return-statement> ::= 'return' <expression>? ';'
 *
 * 第5层：表达式层
 * <expression> ::= <assignment-expression>
 * <assignment-expression> ::= <identifier> '=' <logical-or-expression> | <logical-or-expression>
 *
 * 第6层：逻辑表达式层
 * <logical-or-expression> ::= <logical-and-expression> | <logical-or-expression> '||' <logical-and-expression>
 * <logical-and-expression> ::= <equality-expression> | <logical-and-expression> '&&' <equality-expression>
 * <equality-expression> ::= <relational-expression> | <equality-expression> '==' <relational-expression>
 *
 * 第7层：关系表达式层
 * <relational-expression> ::= <additive-expression>
 *                           | <relational-expression> '<' <additive-expression>
 *                           | <relational-expression> '>' <additive-expression>
 *                           | <relational-expression> '<=' <additive-expression>
 *                           | <relational-expression> '>=' <additive-expression>
 *
 * 第8层：算术表达式层
 * <additive-expression> ::= <multiplicative-expression>
 *                         | <additive-expression> '+' <multiplicative-expression>
 *                         | <additive-expression> '-' <multiplicative-expression>
 * <multiplicative-expression> ::= <primary-expression>
 *                               | <multiplicative-expression> '*' <primary-expression>
 *                               | <multiplicative-expression> '/' <primary-expression>
 *
 * 第9层：基本表达式层
 * <primary-expression> ::= <identifier>  // 标识符
 *                        | <constant>    // 常量
 *                        | '(' <expression> ')'  // 括号表达式
 */

/* 前向声明所有解析函数 */
static bool program();
static bool functionDefinition();
static bool typeSpecifier();
static bool parameterList();
static bool parameterDeclaration();
static bool compoundStatement();
static bool statementList();
static bool statement();
static bool expressionStatement();
static bool selectionStatement();
static bool iterationStatement();
static bool returnStatement();
static bool variableDeclaration();
static bool expression();
static bool assignmentExpression();
static bool logicalOrExpression();
static bool logicalAndExpression();
static bool equalityExpression();
static bool relationalExpression();
static bool additiveExpression();
static bool multiplicativeExpression();
static bool primaryExpression();

/* INFO 辅助函数 */

// 添加语法错误
static void addError(const std::string& message) {
    ParserError error = { g_token.line, message };
    g_errors.push_back(error);
    g_hasError = true;  // 设置错误标志
    std::cerr << "Syntax Error at line " << g_token.line << ": " << message << std::endl;
}

// 增强的错误报告函数，包含当前Token的详细信息
static void addDetailedError(const std::string& message) {
    std::string detailedMessage = message;
    if (g_token.code != TK_EOF) {
        detailedMessage += " (当前Token: '" + g_token.value + "')";
    }
    ParserError error = { g_token.line, detailedMessage };
    g_errors.push_back(error);  // 确保错误被添加到g_errors向量中
    g_hasError = true;  // 设置错误标志
    std::cerr << "Syntax Error at line " << g_token.line << ": " << detailedMessage << std::endl;
}

// 匹配特定类型的Token
static bool match(TokenCode code) {
    if (g_token.code == code) {
        g_token = getNextToken();
        return true;
    }
    return false;
}

// 判断当前Token是否为目标类型
static bool isToken(TokenCode code) {
    return g_token.code == code;
}

// 记录并跳过语法错误
static void skipUntil(std::vector<TokenCode> syncSet) {
    // 记录跳过的token，用于错误报告
    std::string skippedTokens = "";
    int skipCount = 0;
    const int maxDisplayTokens = 3;  // 最多显示几个跳过的token
    
    while (g_token.code != TK_EOF) {
        for (TokenCode code : syncSet) {
            if (g_token.code == code) {
                if (skipCount > 0) {
                    std::string message = "已跳过 " + std::to_string(skipCount) + " 个token";
                    if (!skippedTokens.empty()) {
                        message += " (包括: " + skippedTokens + ")";
                    }
                    std::cerr << "Info: " << message << std::endl;
                }
                return;
            }
        }
        
        // 记录跳过的token
        if (skipCount < maxDisplayTokens) {
            if (!skippedTokens.empty()) {
                skippedTokens += ", ";
            }
            skippedTokens += "'" + g_token.value + "'";
        } else if (skipCount == maxDisplayTokens) {
            skippedTokens += "...";
        }
        skipCount++;
        
        g_token = getNextToken();
    }
}

/* INFO 递归下降分析函数实现 */

// <program> ::= <function-definition>+
static bool program() {
    bool success = true;
    
    while (g_token.code != TK_EOF) {
        // 检查是否为函数定义的开始（类型说明符）
        if (g_token.code == KW_INT || g_token.code == KW_DOUBLE || g_token.code == KW_FLOAT) {
            if (!functionDefinition()) {
                success = false;
                // 提供更详细的错误信息
                addDetailedError("函数定义语法错误");
                // 尝试同步到下一个函数定义
                std::vector<TokenCode> syncSet = {KW_INT, KW_DOUBLE, KW_FLOAT, TK_EOF};
                skipUntil(syncSet);
            }
        } else {
            // 遇到了非函数定义的「开始」，报错并跳过
            addDetailedError("程序中只能包含函数定义，遇到意外的标记");
            std::vector<TokenCode> syncSet = {KW_INT, KW_DOUBLE, KW_FLOAT, TK_EOF};
            skipUntil(syncSet);
            success = false;
        }
    }
    
    return success;
}

// 第2层：函数定义层
// <function-definition> ::= <type-specifier> <identifier> '(' <parameter-list>? ')' <compound-statement>
static bool functionDefinition() {
    if (!typeSpecifier()) {
        addDetailedError("函数定义缺少类型说明符");
        return false;
    }
    
    if (!isToken(TK_IDENT)) {
        addDetailedError("函数定义缺少函数名");
        return false;
    }
    match(TK_IDENT);
    
    if (!match(TK_OPENPA)) {
        addDetailedError("函数名后缺少左括号 '('");
        return false;
    }
    
    // 可选的参数列表
    if (!isToken(TK_CLOSEPA)) {
        parameterList();
    }
    
    if (!match(TK_CLOSEPA)) {
        addDetailedError("参数列表后缺少右括号 ')'");
        return false;
    }
    
    if (!compoundStatement()) {
        addDetailedError("函数定义缺少函数体");
        return false;
    }
    
    return true;
}

// 第2层：函数定义层
// <type-specifier> ::= 'int' | 'double' | 'float'
static bool typeSpecifier() {
    if (match(KW_INT) || match(KW_DOUBLE) || match(KW_FLOAT)) {
        return true;
    }
    return false;
}

// 第2层：函数定义层
// <parameter-list> ::= <parameter-declaration> | <parameter-list> ',' <parameter-declaration>
static bool parameterList() {
    if (!parameterDeclaration()) {
        return false;
    }
    
    while (match(TK_COMMA)) {
        if (!parameterDeclaration()) {
            addDetailedError("逗号后缺少有效的参数声明");
            return false;
        }
    }
    
    return true;
}

// 第2层：函数定义层
// <parameter-declaration> ::= <type-specifier> <identifier>
static bool parameterDeclaration() {
    if (!typeSpecifier()) {
        addDetailedError("参数声明缺少类型说明符");
        return false;
    }
    
    if (!isToken(TK_IDENT)) {
        addDetailedError("参数声明缺少参数名");
        return false;
    }
    match(TK_IDENT);
    
    return true;
}

// 第3层：语句层
// <compound-statement> ::= '{' <statement-list>? '}'
static bool compoundStatement() {
    if (!match(TK_BEGIN)) {
        addDetailedError("复合语句缺少左大括号 '{'");
        return false;
    }
    
    // 可选的语句列表
    if (!isToken(TK_END)) {
        statementList();
    }
    
    if (!match(TK_END)) {
        addDetailedError("复合语句缺少右大括号 '}'");
        return false;
    }
    
    return true;
}

// 第3层：语句层
// <statement-list> ::= <statement> | <statement-list> <statement>
static bool statementList() {
    bool success = true;
    
    while (g_token.code != TK_END && g_token.code != TK_EOF) {
        if (!statement()) {
            success = false;
            
            // 提供更详细的错误信息
            std::string tokenName = "'" + g_token.value + "'";
            addDetailedError("无法解析语句，遇到意外的标记: " + tokenName);
            
            // 尝试同步到下一个语句
            std::vector<TokenCode> syncSet = {TK_SEMOCOLOM, TK_BEGIN, TK_END, KW_IF, KW_WHILE, KW_RETURN, TK_EOF};
            skipUntil(syncSet);
            
            // 跳过分号以尝试继续解析
            if (g_token.code == TK_SEMOCOLOM) {
                match(TK_SEMOCOLOM); // 跳过分号
            }
        }
    }
    return success;
}

// 第3层：语句层
// <statement> ::= <expression-statement> | <compound-statement> | <selection-statement> | <iteration-statement> | <return-statement> | <variable-declaration>
static bool statement() {
    switch (g_token.code) {
        case TK_BEGIN:
            return compoundStatement();
        case KW_IF:
            return selectionStatement();
        case KW_WHILE:
            return iterationStatement();
        case KW_RETURN:
            return returnStatement();
        case KW_INT:
        case KW_DOUBLE:
        case KW_FLOAT:
            return variableDeclaration();
        default:
            return expressionStatement();
    }
}

// 第4层：具体语句定义
// <expression-statement> ::= <expression>? ';'
static bool expressionStatement() {
    if (g_token.code != TK_SEMOCOLOM) {
        if (!expression()) {
            return false;
        }
    }
    
    if (!match(TK_SEMOCOLOM)) {
        addDetailedError("表达式语句缺少分号 ';'");
        return false;
    }
    
    return true;
}

// 第4层：具体语句定义
// <selection-statement> ::= 'if' '(' <expression> ')' <statement> ('else' <statement>)?
//                        | 'if' '(' <expression> ')' 'then' <statement> ('else' <statement>)?
static bool selectionStatement() {
    if (!match(KW_IF)) {
        addDetailedError("预期关键字 'if'");
        return false;
    }
    
    if (!match(TK_OPENPA)) {
        addDetailedError("if语句缺少左括号 '('");
        return false;
    }
    
    if (!expression()) {
        addDetailedError("if条件表达式无效或缺失");
        return false;
    }
    
    if (!match(TK_CLOSEPA)) {
        addDetailedError("if条件缺少右括号 ')'");
        return false;
    }
    
    // 检查是否有 then 关键字
    if (match(KW_THEN)) {
        if (!statement()) {
            addDetailedError("'then'后应有语句块");
            return false;
        }
    } else {
        if (!statement()) {
            addDetailedError("if语句体缺失或无效");
            return false;
        }
    }
    
    // 可选的else部分
    if (match(KW_ELSE)) {
        if (!statement()) {
            addDetailedError("else语句体缺失或无效");
            return false;
        }
    }
    
    return true;
}

// 第4层：具体语句定义
// <iteration-statement> ::= 'while' '(' <expression> ')' <statement>
static bool iterationStatement() {
    if (!match(KW_WHILE)) {
        addDetailedError("预期关键字 'while'");
        return false;
    }
    
    if (!match(TK_OPENPA)) {
        addDetailedError("while语句缺少左括号 '('");
        return false;
    }
    
    if (!expression()) {
        addDetailedError("while条件表达式无效或缺失");
        return false;
    }
    
    if (!match(TK_CLOSEPA)) {
        addDetailedError("while条件缺少右括号 ')'");
        return false;
    }
    
    if (!statement()) {
        addDetailedError("while循环体缺失或无效");
        return false;
    }
    
    return true;
}

// 第4层：具体语句定义
// <return-statement> ::= 'return' <expression>? ';'
static bool returnStatement() {
    if (!match(KW_RETURN)) {
        addDetailedError("预期关键字 'return'");
        return false;
    }
    
    // 可选的表达式
    if (g_token.code != TK_SEMOCOLOM) {
        if (!expression()) {
            return false;
        }
    }
    
    if (!match(TK_SEMOCOLOM)) {
        addDetailedError("return语句缺少分号 ';'");
        return false;
    }
    
    return true;
}

// 第5层：表达式层
// <expression> ::= <assignment-expression>
static bool expression() {
    return assignmentExpression();
}

// 第5层：表达式层
// <assignment-expression> ::= <identifier> '=' <logical-or-expression> | <logical-or-expression>
static bool assignmentExpression() {
    if (g_token.code == TK_IDENT) {
        TokenAttr savedToken = g_token;
        match(TK_IDENT);
        
        if (match(TK_ASSIGN)) {
            if (!logicalOrExpression()) {
                addDetailedError("赋值运算符 '=' 后缺少有效的表达式");
                return false;
            }
            return true;
        } else {
            // 不是赋值表达式，回退Token
            ungetToken();
            g_token = savedToken;
        }
    }
    
    return logicalOrExpression();
}

// 第6层：逻辑表达式层
// <logical-or-expression> ::= <logical-and-expression> | <logical-or-expression> '||' <logical-and-expression>
// 注意：由于Mini语言语法中没有定义||运算符，这里简化处理
static bool logicalOrExpression() {
    return logicalAndExpression();
}

// 第6层：逻辑表达式层
// <logical-and-expression> ::= <equality-expression> | <logical-and-expression> '&&' <equality-expression>
// 注意：由于Mini语言语法中没有定义&&运算符，这里简化处理
static bool logicalAndExpression() {
    return equalityExpression();
}

// 第6层：逻辑表达式层
// <equality-expression> ::= <relational-expression> | <equality-expression> '==' <relational-expression>
static bool equalityExpression() {
    if (!relationalExpression()) {
        return false;
    }
    
    while (g_token.code == TK_EQ) {
        match(TK_EQ);
        if (!relationalExpression()) {
            addDetailedError("'=='运算符后缺少有效的表达式");
            return false;
        }
    }
    
    return true;
}

// 第7层：关系表达式层
// <relational-expression> ::= <additive-expression>
//                          | <relational-expression> '<' <additive-expression>
//                          | <relational-expression> '>' <additive-expression>
//                          | <relational-expression> '<=' <additive-expression>
//                          | <relational-expression> '>=' <additive-expression>
static bool relationalExpression() {
    if (!additiveExpression()) {
        return false;
    }
    
    while (g_token.code == TK_LT || g_token.code == TK_GT || 
           g_token.code == TK_LEQ || g_token.code == TK_GEQ) {
        match(g_token.code);
        if (!additiveExpression()) {
            addDetailedError("关系运算符后缺少有效的表达式");
            return false;
        }
    }
    
    return true;
}

// 第8层：算术表达式层
// <additive-expression> ::= <multiplicative-expression>
//                        | <additive-expression> '+' <multiplicative-expression>
//                        | <additive-expression> '-' <multiplicative-expression>
static bool additiveExpression() {
    if (!multiplicativeExpression()) {
        return false;
    }
    
    while (g_token.code == TK_PLUS || g_token.code == TK_MINUS) {
        match(g_token.code);
        if (!multiplicativeExpression()) {
            addDetailedError("'+' 或 '-' 运算符后缺少有效的表达式");
            return false;
        }
    }
    
    return true;
}

// 第8层：算术表达式层
// <multiplicative-expression> ::= <primary-expression>
//                              | <multiplicative-expression> '*' <primary-expression>
//                              | <multiplicative-expression> '/' <primary-expression>
static bool multiplicativeExpression() {
    if (!primaryExpression()) {
        return false;
    }
    
    while (g_token.code == TK_STAR || g_token.code == TK_DIVIDE) {
        match(g_token.code);
        if (!primaryExpression()) {
            addDetailedError("'*' 或 '/' 运算符后缺少有效的表达式");
            return false;
        }
    }
    
    return true;
}

// 第9层：基本表达式层
// <primary-expression> ::= <identifier>
//                       | <constant>
//                       | '(' <expression> ')'
static bool primaryExpression() {
    if (g_token.code == TK_IDENT) {
        match(TK_IDENT);
        return true;
    } else if (g_token.code == TK_INT || g_token.code == TK_DOUBLE) {
        match(g_token.code);
        return true;
    } else if (match(TK_OPENPA)) {
        if (!expression()) {
            addDetailedError("括号内缺少有效的表达式");
            return false;
        }
        
        if (!match(TK_CLOSEPA)) {
            addDetailedError("表达式后缺少右括号 ')'");
            return false;
        }
        
        return true;
    } else {
        addDetailedError("预期标识符、常量或左括号 '('");
        return false;
    }
}

// <variable-declaration> ::= <type-specifier> <identifier> ('=' <expression>)? ';'
static bool variableDeclaration() {
    if (!typeSpecifier()) {
        addDetailedError("变量声明缺少类型说明符");
        return false;
    }
    
    if (!isToken(TK_IDENT)) {
        addDetailedError("变量声明缺少变量名");
        return false;
    }
    match(TK_IDENT);
    
    // 可选的赋值表达式
    if (match(TK_ASSIGN)) {
        if (!expression()) {
            addDetailedError("赋值运算符 '=' 后缺少有效的表达式");
            return false;
        }
    }
    
    if (!match(TK_SEMOCOLOM)) {
        addDetailedError("变量声明缺少分号 ';'");
        return false;
    }
    
    return true;
}

/* INFO 接口实现 */

void initParser(FILE* fp) {
    initLexer(fp);
    g_errors.clear();
    g_hasError = false;  // 初始化错误标志
    g_token = getNextToken();
}

ParserResult parse() {
    bool success = program();
    // 如果有语法错误，返回错误结果
    return (success && !g_hasError) ? RESULT_SUCCESS : RESULT_ERROR;
}

const std::vector<ParserError>& getParserErrors() {
    return g_errors;
}

void resetParser() {
    resetLexer();
    g_errors.clear();
    g_hasError = false;  // 重置错误标志
    g_token = getNextToken();
}

void closeParser() {
    closeLexer();
} 