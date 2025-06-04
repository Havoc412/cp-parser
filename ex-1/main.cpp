#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <algorithm>
#include <cstdlib>
using namespace std;

/* 单词编码 */
enum TokenCode
{
    /* 未定义 */
    TK_UNDEF = 0,
    /* 关键字 */
    KW_INT, // int
    KW_DOUBLE, // double
    KW_IF, // if
    KW_ELSE, // else
    KW_RETURN, // return
    /* 运算符 */
    TK_PLUS, // +
    TK_MINUS, // -
    TK_STAR, // *
    TK_DIVIDE, // /
    TK_ASSIGN, // =
    TK_EQ, // ==
    TK_LT, // <
    TK_LEQ, // <=
    TK_GT, // >
    TK_GEQ, // >=
    /* 分隔符 */
    TK_OPENPA, // (
    TK_CLOSEPA, // )
    TK_OPENBR, // [
    TK_CLOSEBR, // ]
    TK_BEGIN, // {
    TK_END, // }
    TK_COMMA, // ,
    TK_SEMOCOLOM, // ;
    /* 常量 */
    TK_INT, // 整型常量
    TK_DOUBLE, // 浮点型常量
    /* 标识符 */
    TK_IDENT
};
enum TableTypeId {
    Table_TAG = 1,
    Table_CONSTANT,
};
// TAG KeyWord Define
const int keyWordTokenNum = 6;
char keyWord[][10] = { "int", "double", "float", "if", "then", "else", "return", "while" };
// Number next
const int numberNextPassNum = 15;
char numberNextPass[100] = { '+', '-', '*', '/', '=', '>', '<', '&', '|',
    '^', '%', '?', ')', ',', ';' };
// State
int row = 1; // 记录字符所在的⾏数
string token = ""; // ⽤于存储单词
TokenCode code = TK_UNDEF; // code
// Load
map<TokenCode, int> tokenCodeMap;
map<string, int> constantsMap;
struct tokenAttr {
    TokenCode code;
    int line;
    TableTypeId type;
    int table_row;
    string value;
};
vector<tokenAttr> tokenList;
// TAG ---------
// 新增: 错误列表
vector<string> errorList;
// 修改: load 函数，将错误信息添加到 errorList
void load(TokenCode code) {
    TableTypeId table_type = Table_TAG;
    int table_row = 0;
    switch (code) {
        /*未识别的符号*/
        case TK_UNDEF: {
            string errorMsg = "Undefined Symbol: " + token + " at line " +
                to_string(row);
            cerr << errorMsg << endl;
            errorList.push_back(errorMsg); // 新增: 将错误信息添加到 errorList
            // 存在 BUG，直接报错停⽌
            // exit(0); 
        }
        break;
        /* Keywords */
        case KW_INT: // int
        case KW_DOUBLE: // double
        case KW_IF: // if
        case KW_ELSE: // else
        case KW_RETURN: // return
        /* Operators */
        case TK_PLUS: // +
        case TK_MINUS: // -
        case TK_STAR: // *
        case TK_DIVIDE: // /
        case TK_ASSIGN: // =
        case TK_EQ: // ==
        case TK_LT: // <
        case TK_LEQ: // <=
        case TK_GT: // >
        case TK_GEQ: // >=
        /* Delimiters */
        case TK_OPENPA: // (
        case TK_CLOSEPA: // )
        case TK_OPENBR: // [
        case TK_CLOSEBR: // ]
        case TK_BEGIN: // {
        case TK_END: // }
        case TK_COMMA: // ,
        case TK_SEMOCOLOM: // ;
        /* ID */
        case TK_IDENT: {
            if (tokenCodeMap.find(code) == tokenCodeMap.end()) {
                tokenCodeMap[code] = tokenCodeMap.size() + 1;
            }
            cout << '(' << code << ',' << token << ")" << endl;
            table_row = tokenCodeMap[code];
        } break;

        /* Constants */
        case TK_INT:
        case TK_DOUBLE: {
            table_type = Table_CONSTANT;
            if (constantsMap.find(token) == constantsMap.end()) {
                constantsMap[token] = constantsMap.size() + 1;
            }
            cout << '(' << code << ',' << (token.find('.')==token.npos ?
                atoi(token.c_str()) : atof(token.c_str())) << ")" << endl;
            table_row = constantsMap[token];
        }
        break;
        
        default:
        break;
    }
    // tokensList
    tokenList.push_back({ code, row, table_type, table_row, token });
}
// TAG ---------
int tryGetKeyWordID(string token) {
    for (int i = 0; i < keyWordTokenNum; i++) { // 关键字的内码值为keyWord数组中对应的下标加1
        if (token.compare(keyWord[i]) == 0) 
            return i+1;
    }
    return -1;
}
bool isLetter(char letter) {
    if ((letter >= 'a' && letter <= 'z') || (letter >= 'A' && letter <= 'Z'))
        return true;
    return false;
}
bool checkNumberNext(char ch) {
    for (int i = 0; i < numberNextPassNum; i++) {
        if (ch == numberNextPass[i])
            return true;
    }
    return false;
}
void lexicalAnalysis(FILE *fp)
{
    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '#')
            break; // INFO 题⽬要求，虽然感觉⽆必要性。
        token = ch;
        if (ch == ' ' || ch == '\t' || ch == '\n') { // 忽略空格、Tab和回⻋；设计是类似 C++ 的 mini语⾔⻛格。 // not like python/go
            if (ch == '\n')
                row++;
            continue;
        }
        else if (isLetter(ch)) { // Let Entrance
            token = "";
            while (isLetter(ch) || isdigit(ch)) {
                token += ch;
                ch = fgetc(fp);
            }
            // INFO ⽂件指针后退⼀个字节；
            fseek(fp, -1L, SEEK_CUR);
            if (int idx = tryGetKeyWordID(token) != -1) // TODO: fix this: `if (int idx = tryGetKeyWordID(token) != -1)` is likely a bug, should be `if ((idx = tryGetKeyWordID(token)) != -1)`
                code = TokenCode(idx);
            else
                code = TK_IDENT; // 为标识符
        }
        else if (isdigit(ch)) { // Dig Entrance
            token = "";
            int dot_num = 0;
            while (isdigit(ch) || ch == '.') {
                token += ch;
                if (ch == '.') { // UPDATE 这样就能吞噬掉关于 . 的异常词法。
                    dot_num++;
                }
                ch = fgetc(fp); //从⽂件中获取下⼀个字符
            }
            // TIP check again
            if (!checkNumberNext(ch)) {
                code = TK_UNDEF;
                // INFO 针对 letter 和 digit "捕获"。
                while(isdigit(ch) || isLetter(ch)) {
                    token += ch;
                    ch = fgetc(fp);
                }
            } else if (dot_num > 1) {
                code = TK_UNDEF;
            } else {
                code = dot_num ? TK_DOUBLE : TK_INT;
            }
            fseek(fp, -1L, SEEK_CUR);
        }
        else switch (ch) { 
            case '+': code = TK_PLUS; break;
            case '-': code = TK_MINUS; break;
            case '*': code = TK_STAR; break;
            case '/': code = TK_DIVIDE; break;
            case '=': {
                ch = fgetc(fp);
                if (ch == '=') {
                    token += ch;
                    code = TK_EQ;
                } else {
                    code = TK_ASSIGN;
                    fseek(fp, -1L, SEEK_CUR);
                }
            } break;
            case '<': {
                ch = fgetc(fp);
                if (ch == '=') {
                    token += ch;
                    code = TK_LEQ;
                } else {
                    code = TK_LT;
                    fseek(fp, -1L, SEEK_CUR);
                }
            } break;
            case '>': {
                ch = fgetc(fp);
                if (ch == '=') {
                    token += ch;
                    code = TK_GEQ;
                } else {
                    code = TK_GT;
                    fseek(fp, -1L, SEEK_CUR);
                }
            } break;
            case '(': code = TK_OPENPA; break;
            case ')': code = TK_CLOSEPA; break;
            case '[': code = TK_OPENBR; break;
            case ']': code = TK_CLOSEBR; break;
            case '{': code = TK_BEGIN; break;
            case '}': code = TK_END; break;
            case ',': code = TK_COMMA; break;
            case ';': code = TK_SEMOCOLOM; break;
            default: code = TK_UNDEF; break;
        }
        load(code);
    }
}

// TAG ---------
void outputFile(const string& filename) {
    // 创建输出⽬录
    string dirName = filename + "-output";
    struct stat info;
    if (stat(dirName.c_str(), &info) != 0) { // 检查⽬录是否存在
        if (mkdir(dirName.c_str(), 0777) == -1) {
            cerr << "Error: ⽆法创建⽬录 " << dirName << endl;
            return;
        }
    } else if (!(info.st_mode & S_IFDIR)) { // 如果存在但不是⽬录
        cerr << "Error: " << dirName << " 已存在但不是⽬录" << endl;
        return;
    }
    // Tables;
    ofstream outFile1(dirName + "/table_tag.csv");
    // 将map转换为vector并按值排序
    vector<pair<TokenCode, int>> sortedTokenCode(tokenCodeMap.begin(),
        tokenCodeMap.end());
    sort(sortedTokenCode.begin(), sortedTokenCode.end(), 
        [](const pair<TokenCode, int>& a, const pair<TokenCode, int>& b) {
        return a.second < b.second;
    });
    // 输出排序后的结果
    for (auto& [k, v] : sortedTokenCode) {
        outFile1 << v << ',' << k << "\n";
    }
    outFile1.close();
    ofstream outFile2(dirName + "/table_const.csv");
    // 将map转换为vector并按值排序
    vector<pair<string, int>> sortedConstants(constantsMap.begin(),
        constantsMap.end());
    sort(sortedConstants.begin(), sortedConstants.end(),
        [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second < b.second;
    });
    // 输出排序后的结果
    for (auto& [k, v] : sortedConstants) {
        outFile2 << v << ',' << k << "\n";
    }
    outFile2.close();
    // Token
    ofstream outFile(dirName + "/tokens.csv");
    for (auto& token : tokenList) {
        outFile << token.code << ',' << token.line << ',' << token.type <<
            ',' << token.table_row << ',' << token.value << "\n";
    }
    outFile.close();
}

void outputErrorLog(const string& filename) {
    string logFileName = filename + "-output/error.log";
    ofstream errorLog(logFileName);
    if (errorLog.is_open()) {
        for (const auto& error : errorList) {
            errorLog << error << "\n";
        }
        errorLog.close();
    } else {
        cerr << "Error: Unable to open error log file " << logFileName << endl;
    }
}

int main()
{
    // Input
    string filename; 
    FILE* fp; 
    cout << "Input the file path: " << endl;
    while (true) {
        cin >> filename;
        fp = fopen(filename.c_str(), "r");
        if (fp != nullptr)
            break;
        else
            cout << "ERROR: unavild file path !" << endl;
    }
    // TAG 词法分析
    lexicalAnalysis(fp);
    fclose(fp);
    // Output
    outputFile(filename);
    outputErrorLog(filename);
    return 0;
}