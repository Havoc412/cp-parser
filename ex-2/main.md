# 第一部分 语言语法规则（自然语言描述）

我简单构造的Mini语言是一种类C语言的简化版本，基于上一次词法分析的任务，支持基本的语法结构。其主要语法规则如下：

1. **词法元素**：
   - 关键字：int、double、float、if、then、else、return、while
   - 运算符：+、-、*、/、=、==、<、<=、>、>=
   - 分隔符：(、)、[、]、{、}、,、;
   - 标识符：由字母开头，后跟字母、数字的序列
   - 常量：整型常量（如10）和浮点型常量（如3.14）

2. **注释**：支持单行注释，以'//'开头，到行尾结束

3. **表达式**：支持算术表达式、比较表达式和赋值表达式

4. **语句**：支持赋值语句、条件语句、循环语句和返回语句

5. **程序结构**：由函数定义组成，每个函数包含一系列语句

# 第二部分 文法定义

其文法可以用 BNF 表示如下：

## 1. 程序结构层
定义整个程序的基本结构
1. 程序由一个或多个函数定义组成
```
<program> ::= <function-definition>+  // 程序由一个或多个函数定义组成
```

## 2. 函数定义层
定义函数的结构
```
<function-definition> ::= <type-specifier> <identifier> '(' <parameter-list>? ')' <compound-statement> // 函数定义，eg. int func(int a, int b) { ... }
<type-specifier> ::= 'int' | 'double' | 'float'  // 函数返回类型
<parameter-list> ::= <parameter-declaration> | <parameter-list> ',' <parameter-declaration> // 参数列表，eg. int a, int b
<parameter-declaration> ::= <type-specifier> <identifier> // 参数声明，eg. int a, int b
```

## 3. 语句层
定义各种语句结构

```
<compound-statement> ::= '{' <statement-list>? '}'  // 复合语句（代码块）
<statement-list> ::= <statement> | <statement-list> <statement>
<statement> ::= <expression-statement>  // 表达式语句
              | <compound-statement>    // 复合语句
              | <selection-statement>   // 选择语句（if-else）
              | <iteration-statement>   // 循环语句（while）
              | <return-statement>      // 返回语句
```

## 4. 具体语句定义
定义各种具体语句的语法；
1. 句末必须以分号结束；
2. 循环/分支语句的条件放置于括号内；
3. 返回语句必须以return结束；

```
<expression-statement> ::= <expression>? ';'
<selection-statement> ::= 'if' '(' <expression> ')' <statement> ('else' <statement>)?
<iteration-statement> ::= 'while' '(' <expression> ')' <statement>
<return-statement> ::= 'return' <expression>? ';'
```

## 5. 表达式层
定义表达式的层次结构
```
<expression> ::= <assignment-expression>
<assignment-expression> ::= <identifier> '=' <logical-or-expression> | <logical-or-expression> // 赋值表达式，eg. a = b + c
```

## 6. 逻辑表达式层
定义逻辑运算的优先级
```
<logical-or-expression> ::= <logical-and-expression> | <logical-or-expression> '||' <logical-and-expression> // 逻辑或表达式，eg. a || b
<logical-and-expression> ::= <equality-expression> | <logical-and-expression> '&&' <equality-expression> // 逻辑与表达式，eg. a && b
<equality-expression> ::= <relational-expression> | <equality-expression> '==' <relational-expression> // 相等表达式，eg. a == b
```

## 7. 关系表达式层
定义关系运算
```
<relational-expression> ::= <additive-expression> // 关系表达式，eg. a < b + c
                          | <relational-expression> '<' <additive-expression> // 小于表达式，eg. a < b
                          | <relational-expression> '>' <additive-expression> // 大于表达式，eg. a > b
                          | <relational-expression> '<=' <additive-expression> // 小于等于表达式，eg. a <= b
                          | <relational-expression> '>=' <additive-expression> // 大于等于表达式，eg. a >= b
```

## 8. 算术表达式层
定义算术运算的优先级
```
<additive-expression> ::= <multiplicative-expression> // 加减表达式，eg. a + b - c
                        | <additive-expression> '+' <multiplicative-expression> // 加法表达式，eg. a + b
                        | <additive-expression> '-' <multiplicative-expression> // 减法表达式，eg. a - b
<multiplicative-expression> ::= <primary-expression> // 乘除表达式，eg. a * b / c
                              | <multiplicative-expression> '*' <primary-expression> // 乘法表达式，eg. a * b
                              | <multiplicative-expression> '/' <primary-expression> // 除法表达式，eg. a / b
```

## 9. 基本表达式层
定义最基本的表达式元素
```
<primary-expression> ::= <identifier>  // 标识符，eg. a
                       | <constant>    // 常量，eg. 10
                       | '(' <expression> ')'  // 括号表达式，eg. (a + b)
```

## 10. 词法元素层
定义基本的词法单元
```
<identifier> ::= <letter> (<letter> | <digit>)*  // 标识符：字母开头，后跟字母或数字，eg. a, b, c
<constant> ::= <integer-constant> | <floating-constant>  // 常量：整数或浮点数，eg. 10, 3.14
<integer-constant> ::= <digit>+  // 整数常量：一个或多个数字，eg. 10
<floating-constant> ::= <digit>+ '.' <digit>*  // 浮点常量：数字.数字，eg. 3.14
<letter> ::= 'a' | 'b' | ... | 'z' | 'A' | 'B' | ... | 'Z'  // 字母，eg. a, b, c
<digit> ::= '0' | '1' | ... | '9'  // 数字，eg. 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
```

# 第三部分 语法分析算法

本项目采用递归下降分析方法进行语法分析。该方法是一种自顶向下的分析方法，通过一组互相递归的函数来实现对输入的分析。

递归下降分析的基本思想是：为文法中的每个非终结符设计一个函数，该函数负责分析该非终结符所代表的语法结构。当需要分析某个非终结符时，就调用相应的函数。

算法的主要步骤如下：

1. **初始化**：设置词法分析器，准备读取第一个Token
2. **开始分析**：从文法的开始符号（<program>）对应的函数开始
3. **递归处理**：
   - 对于每个非终结符，调用相应的函数进行处理
   - 对于选择结构（|），通过前瞻一个或多个Token来决定选择哪个分支
   - 对于重复结构（+、*），使用循环处理
4. **匹配终结符**：当遇到终结符时，检查当前Token是否匹配，匹配则读取下一个Token，否则报错
5. **构建语法树**：在分析过程中，逐步构建抽象语法树
6. **错误处理**：当发生语法错误时，尝试恢复并继续分析

```
/* 表达式语句解析示例 */
bool parseExpressionStatement(void) {
    if (g_token.code != TK_SEMOCOLOM) {
        if (!expression()) {
            return false;
        }
    }
    
    if (!match(TK_SEMOCOLOM)) {
        addError("Expected ';' after expression");
        return false;
    }
    
    return true;
}

/* 匹配特定类型的Token */
bool match(TokenCode code) {
    if (g_token.code == code) {
        g_token = getNextToken();
        return true;
    }
    addError("Expected token: " + getTokenName(code));
    return false;
}
```


# 第四部分 出错处理出口

为了提高编译器的健壮性和用户体验，本项目实现了以下错误处理机制：

1. **词法错误处理**：
   - 非法标识符：出现非法字符时，捕获并报告错误
   - 非法数字格式：如多个小数点，报告错误
   - 未闭合的注释：在文件结束前注释未闭合，报告错误

2. **语法错误处理**：
   - 同步集合：为每个非终结符定义一个同步集合，当发生错误时，跳过输入直到遇到同步集合中的Token
   - 紧急恢复：在某些关键位置（如语句边界）进行特殊处理，尽快恢复分析
   - 错误产生式：对于一些常见的错误模式，提供特殊的处理

3. **错误信息输出**：
   - 详细的错误描述：包括错误类型、位置（行号、列号）和可能的修复建议
   - 错误上下文：显示错误发生的代码上下文，帮助用户定位问题
   - 错误分类：将错误分为致命错误、普通错误和警告三类

错误处理的伪代码示例：

```
void recoverFromError(syncSet) {
    reportError("Syntax error at line " + currentToken.line);
    while (currentToken not in syncSet and currentToken != EOF) {
        currentToken = getNextToken();
    }
}
```

# 第五部分 测试计划（报告）

为了验证Mini语言编译器的正确性和健壮性，我们设计了以下测试计划：

1. **单元测试**：
   - 词法分析器测试：测试各类Token的识别正确性
     * 关键字识别（int, if, else, while, return等）
     * 运算符识别（+, -, *, /, =, ==, <, <=, >, >=）
     * 分隔符识别（(), [], {}, ,, ;）
     * 标识符识别（字母开头的字母数字序列）
     * 常量识别（整数和浮点数）
     * 注释识别（//开头的单行注释）
   - 语法分析器测试：测试各种语法结构的解析
     * 函数定义和参数列表
     * 复合语句和语句列表
     * 条件语句（if-else）
     * 循环语句（while）
     * 返回语句
     * 表达式语句
   - 错误处理测试：测试在不同错误情况下的恢复能力
     * 缺少分号的语句
     * 缺少括号的条件/循环语句
     * 缺少大括号的代码块
     * 未定义的运算符使用

2. **集成测试**：
   - 词法分析器与语法分析器的集成测试
     * 基本程序结构测试（test1.txt）
     * 错误程序恢复测试（test2.txt）
     * 复杂程序功能测试（test3.txt）
   - 整个编译前端的集成测试
     * 递归函数调用
     * 嵌套条件语句
     * 复杂表达式计算

3. **测试用例设计**：
   - 合法程序测试：
     * 基本语法结构（变量声明、赋值、条件、循环）
     * 函数定义和调用
     * 复杂表达式计算
   - 边界情况测试：
     * 空函数体
     * 单语句代码块
     * 最大嵌套深度
   - 错误程序测试：
     * 语法错误（缺少分号、括号等）
     * 语义错误（未定义运算符使用）
     * 结构错误（不匹配的括号等）

4. **测试环境**：
   - 操作系统：macOS
   - 编译器：Clang 13.0.0
   - 测试工具：自定义测试框架
   - 测试数据：包含test1.txt、test2.txt、test3.txt等测试用例

5. **测试结果分析**：
   - 功能完整性：
     * 所有语法特性都能正确解析
     * 错误检测和恢复机制有效
     * 支持复杂的程序结构
   - 错误处理能力：
     * 能够准确报告错误位置和类型
     * 能够从错误中恢复并继续分析
     * 提供有意义的错误信息
   - 性能评估：
     * 词法分析速度
     * 语法分析效率
     * 内存使用情况
   - 测试覆盖率：
     * 语句覆盖率
     * 分支覆盖率
     * 错误处理路径覆盖率

6. **测试案例解析**：

   a. **合法程序示例（test1.txt）**：
      ```
      int main() {
          int a = 10;
          int b = 20;
          
          if (a < b) {
              a = a + 1;
          } else {
              b = b - 1;
          }
          
          while (a < b) {
              a = a + 1;
          }
          
          return 0;
      }
      ```
      该程序展示了以下合法语法特性：
      - 函数定义和参数列表
      - 变量声明和初始化
      - 条件语句（if-else）
      - 循环语句（while）
      - 算术表达式
      - 比较表达式
      - 返回语句

   b. **语法错误示例（test2.txt）**：
      ```
      int main() {
          int a = 10    // 缺少分号
          int b = 20;   
          
          if a < b {    // 缺少括号
              a = a + 1;
          } else {
              b = b - 1  // 缺少分号
          }
          
          while (a < b) 
              a = a + 1;  // 缺少大括号
          
          return 0;
      }
      ```
      该程序包含以下语法错误：
      - 语句缺少分号
      - 条件语句缺少括号
      - 循环语句缺少大括号
      错误处理机制能够：
      - 检测并报告错误位置
      - 继续分析后续代码
      - 提供清晰的错误信息

   c. **语义错误示例（test3.txt）**：
      ```
      int gcd(int a, int b) {
          while (b != 0) {
              int temp = b;
              b = a % b;  // 使用未定义的%运算符
              a = temp;
          }
          return a;
      }
      ```
      该程序包含以下语义错误：
      - 使用未定义的运算符（%）
      - 使用未定义的运算符（!=）
      错误处理机制能够：
      - 识别未定义的运算符
      - 报告具体的错误位置
      - 提供错误类型信息

7. **错误处理机制验证**：

   a. **词法错误处理**：
      - 能够识别并报告非法字符
      - 能够处理格式错误的数字
      - 能够继续分析后续代码

   b. **语法错误处理**：
      - 能够检测缺少的分号、括号等
      - 能够从错误中恢复并继续分析
      - 能够提供准确的错误位置信息

   c. **语义错误处理**：
      - 能够识别未定义的运算符
      - 能够检测类型不匹配
      - 能够提供有意义的错误信息

通过以上测试案例的详细解析，我们可以验证编译器的错误处理能力和恢复机制的有效性。这些测试案例涵盖了常见的编程错误，有助于确保编译器在实际使用中的可靠性。

