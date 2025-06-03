#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <map>
#include <algorithm>

// 符号表和常量表
std::map<TokenCode, int> tokenCodeMap;
std::map<std::string, int> constantsMap;
std::vector<TokenAttr> tokenList;

// 函数声明
void showUsage(const char* programName);

// 输出TokenCode对应的字符串描述
std::string getTokenName(TokenCode code) {
    switch (code) {
        case TK_UNDEF: return "UNDEFINED";
        case KW_INT: return "KEYWORD_INT";
        case KW_DOUBLE: return "KEYWORD_DOUBLE";
        case KW_FLOAT: return "KEYWORD_FLOAT";
        case KW_IF: return "KEYWORD_IF";
        case KW_THEN: return "KEYWORD_THEN";
        case KW_ELSE: return "KEYWORD_ELSE";
        case KW_RETURN: return "KEYWORD_RETURN";
        case KW_WHILE: return "KEYWORD_WHILE";
        case TK_PLUS: return "OPERATOR_PLUS";
        case TK_MINUS: return "OPERATOR_MINUS";
        case TK_STAR: return "OPERATOR_MULTIPLY";
        case TK_DIVIDE: return "OPERATOR_DIVIDE";
        case TK_ASSIGN: return "OPERATOR_ASSIGN";
        case TK_EQ: return "OPERATOR_EQUAL";
        case TK_LT: return "OPERATOR_LESS_THAN";
        case TK_LEQ: return "OPERATOR_LESS_EQUAL";
        case TK_GT: return "OPERATOR_GREATER_THAN";
        case TK_GEQ: return "OPERATOR_GREATER_EQUAL";
        case TK_OPENPA: return "DELIMITER_OPEN_PARENTHESIS";
        case TK_CLOSEPA: return "DELIMITER_CLOSE_PARENTHESIS";
        case TK_OPENBR: return "DELIMITER_OPEN_BRACKET";
        case TK_CLOSEBR: return "DELIMITER_CLOSE_BRACKET";
        case TK_BEGIN: return "DELIMITER_BEGIN_BRACE";
        case TK_END: return "DELIMITER_END_BRACE";
        case TK_COMMA: return "DELIMITER_COMMA";
        case TK_SEMOCOLOM: return "DELIMITER_SEMICOLON";
        case TK_INT: return "CONSTANT_INTEGER";
        case TK_DOUBLE: return "CONSTANT_DOUBLE";
        case TK_IDENT: return "IDENTIFIER";
        case TK_EOF: return "END_OF_FILE";
        default: return "UNKNOWN";
    }
}

// 输出结果到文件
void outputResults(const std::string& filename, bool lexOnly) {
    // 创建输出目录
    std::string dirName = filename + "-output";
    struct stat info;
    
    if (stat(dirName.c_str(), &info) != 0) { // 检查目录是否存在
        if (mkdir(dirName.c_str(), 0777) == -1) {
            std::cerr << "错误: 无法创建目录 " << dirName << std::endl;
            return;
        }
    } else if (!(info.st_mode & S_IFDIR)) { // 如果存在但不是目录
        std::cerr << "错误: " << dirName << " 已存在但不是目录" << std::endl;
        return;
    }
    
    // 输出Token列表
    std::ofstream tokenFile(dirName + "/tokens.txt");
    if (tokenFile.is_open()) {
        tokenFile << "行号\t类型\t\t值\n";
        tokenFile << "-------------------------------------\n";
        
        for (const auto& token : tokenList) {
            tokenFile << token.line << "\t" 
                     << getTokenName(token.code) << "\t" 
                     << token.value << "\n";
        }
        tokenFile.close();
    }
    
    // 输出词法错误信息
    const std::vector<ErrorInfo>& lexErrors = getErrors();
    if (!lexErrors.empty()) {
        std::ofstream errorFile(dirName + "/lex_errors.txt");
        if (errorFile.is_open()) {
            errorFile << "行号\t错误信息\n";
            errorFile << "-------------------------------------\n";
            
            for (const auto& error : lexErrors) {
                errorFile << error.line << "\t" << error.message << "\n";
            }
            errorFile.close();
        }
    }
    
    // 输出语法错误信息
    if (!lexOnly) {
        const std::vector<ParserError>& parseErrors = getParserErrors();
        if (!parseErrors.empty()) {
            std::ofstream parseErrorFile(dirName + "/parse_errors.txt");
            if (parseErrorFile.is_open()) {
                parseErrorFile << "行号\t错误信息\n";
                parseErrorFile << "-------------------------------------\n";
                
                for (const auto& error : parseErrors) {
                    parseErrorFile << error.line << "\t" << error.message << "\n";
                }
                parseErrorFile.close();
            }
        }
    }
    
    // 简洁的摘要输出
    std::cout << "\n=== 分析完成 ===\n";
    std::cout << "文件: " << filename << "\n";
    
    if (lexOnly) {
        std::cout << "词法分析结果: " << (lexErrors.empty() ? "成功" : "有错误") << "\n";
        std::cout << "Token总数: " << tokenList.size() << "\n";
        std::cout << "词法错误总数: " << lexErrors.size() << "\n";
    } else {
        const std::vector<ParserError>& parseErrors = getParserErrors();
        std::cout << "词法分析结果: " << (lexErrors.empty() ? "成功" : "有错误") << "\n";
        std::cout << "语法分析结果: " << (parseErrors.empty() ? "成功" : "有错误") << "\n";
        std::cout << "Token总数: " << tokenList.size() << "\n";
        std::cout << "词法错误总数: " << lexErrors.size() << "\n";
        std::cout << "语法错误总数: " << parseErrors.size() << "\n";
    }
    
    std::cout << "结果已输出到: " << dirName << "\n";
}

int main(int argc, char* argv[]) {
    // 输入文件路径
    std::string filename;
    FILE* fp;
    bool showProcess = true;   // 是否显示分析过程
    bool lexOnly = false;      // 是否仅进行词法分析
    
    // 检查命令行参数
    if (argc < 2) {
        showUsage(argv[0]);
        return 1;
    }
    
    // 解析命令行选项
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            showUsage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "Mini语言编译器 v1.0\n";
            return 0;
        } else if (arg == "-q" || arg == "--quiet") {
            showProcess = false;
        } else if (arg == "-l" || arg == "--lex-only") {
            lexOnly = true;
        } else if (arg[0] == '-') {
            std::cerr << "错误: 未知选项 " << arg << "\n";
            showUsage(argv[0]);
            return 1;
        } else {
            filename = arg;
        }
    }
    
    if (filename.empty()) {
        std::cerr << "错误: 未指定输入文件\n";
        showUsage(argv[0]);
        return 1;
    }
    
    // 尝试打开文件
    fp = fopen(filename.c_str(), "r");
    if (fp == nullptr) {
        std::cerr << "错误: 无法打开文件 " << filename << std::endl;
        return 1;
    }
    
    if (lexOnly) {
        // 仅进行词法分析
        // 初始化词法分析器
        initLexer(fp);
        
        // 进行词法分析
        TokenAttr token;
        if (showProcess) {
            std::cout << "开始词法分析...\n";
        }
        
        do {
            token = getNextToken();
            tokenList.push_back(token);
            
            // 显示分析过程（可选）
            if (showProcess && token.code != TK_EOF) {
                std::cout << "行 " << token.line << ": [" 
                         << getTokenName(token.code) << "] " 
                         << token.value << std::endl;
            }
        } while (token.code != TK_EOF);
        
        // 关闭文件
        fclose(fp);
        
        // 输出分析结果
        outputResults(filename, true);
    } else {
        // 进行词法和语法分析
        // 初始化解析器
        initParser(fp);
        
        if (showProcess) {
            std::cout << "开始分析...\n";
        }
        
        // 执行语法分析
        ParserResult result = parse();
        
        // 收集词法分析产生的token（为了输出token列表）
        resetParser();
        TokenAttr token;
        do {
            token = getNextToken();
            tokenList.push_back(token);
        } while (token.code != TK_EOF);
        
        // 关闭文件
        fclose(fp);
        
        // 输出分析结果
        if (showProcess) {
            if (result == RESULT_SUCCESS) {
                std::cout << "语法分析成功！\n";
            } else {
                std::cout << "语法分析失败。\n";
            }
        }
        
        outputResults(filename, false);
    }
    
    return 0;
}

// 显示使用说明
void showUsage(const char* programName) {
    std::cout << "用法: " << programName << " [选项] <文件路径>\n\n";
    std::cout << "选项:\n";
    std::cout << "  -h, --help      显示此帮助信息\n";
    std::cout << "  -v, --version   显示版本信息\n";
    std::cout << "  -q, --quiet     安静模式，不显示分析过程\n";
    std::cout << "  -l, --lex-only  仅进行词法分析，不进行语法分析\n\n";
    std::cout << "示例: " << programName << " ./example.txt\n";
    std::cout << "      " << programName << " -q ./example.txt\n";
    std::cout << "      " << programName << " -l ./example.txt\n";
}
