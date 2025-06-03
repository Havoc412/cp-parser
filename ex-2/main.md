# 第一部分 语言语法规则（自然语言描述）

Mini语言是一种类C语言的简化版本，支持基本的语法结构。其主要语法规则如下：

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

Mini语言的文法可以用BNF（巴科斯范式）表示如下：

```
<program> ::= <function-definition>+

<function-definition> ::= <type-specifier> <identifier> '(' <parameter-list>? ')' <compound-statement>

<type-specifier> ::= 'int' | 'double' | 'float'

<parameter-list> ::= <parameter-declaration> | <parameter-list> ',' <parameter-declaration>

<parameter-declaration> ::= <type-specifier> <identifier>

<compound-statement> ::= '{' <statement-list>? '}'

<statement-list> ::= <statement> | <statement-list> <statement>

<statement> ::= <expression-statement>
              | <compound-statement>
              | <selection-statement>
              | <iteration-statement>
              | <return-statement>

<expression-statement> ::= <expression>? ';'

<selection-statement> ::= 'if' '(' <expression> ')' <statement> ('else' <statement>)?

<iteration-statement> ::= 'while' '(' <expression> ')' <statement>

<return-statement> ::= 'return' <expression>? ';'

<expression> ::= <assignment-expression>

<assignment-expression> ::= <identifier> '=' <logical-or-expression> | <logical-or-expression>

<logical-or-expression> ::= <logical-and-expression> | <logical-or-expression> '||' <logical-and-expression>

<logical-and-expression> ::= <equality-expression> | <logical-and-expression> '&&' <equality-expression>

<equality-expression> ::= <relational-expression> | <equality-expression> '==' <relational-expression>

<relational-expression> ::= <additive-expression>
                          | <relational-expression> '<' <additive-expression>
                          | <relational-expression> '>' <additive-expression>
                          | <relational-expression> '<=' <additive-expression>
                          | <relational-expression> '>=' <additive-expression>

<additive-expression> ::= <multiplicative-expression>
                        | <additive-expression> '+' <multiplicative-expression>
                        | <additive-expression> '-' <multiplicative-expression>

<multiplicative-expression> ::= <primary-expression>
                              | <multiplicative-expression> '*' <primary-expression>
                              | <multiplicative-expression> '/' <primary-expression>

<primary-expression> ::= <identifier>
                       | <constant>
                       | '(' <expression> ')'

<identifier> ::= <letter> (<letter> | <digit>)*

<constant> ::= <integer-constant> | <floating-constant>

<integer-constant> ::= <digit>+

<floating-constant> ::= <digit>+ '.' <digit>*

<letter> ::= 'a' | 'b' | ... | 'z' | 'A' | 'B' | ... | 'Z'

<digit> ::= '0' | '1' | ... | '9'
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

伪代码示例（以<expression-statement>为例）：

```
function parseExpressionStatement():
    if currentToken is not ';':
        expr = parseExpression()
    match(';')  // 确保语句以分号结束
    return new ExpressionStatement(expr)

function match(expectedToken):
    if currentToken == expectedToken:
        currentToken = getNextToken()
    else:
        reportError("Expected " + expectedToken)
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
function recoverFromError(syncSet):
    reportError("Syntax error at line " + currentToken.line)
    while currentToken not in syncSet and currentToken != EOF:
        currentToken = getNextToken()
```

# 第五部分 测试计划（报告）

为了验证Mini语言编译器的正确性和健壮性，我们设计了以下测试计划：

1. **单元测试**：
   - 词法分析器测试：测试各类Token的识别正确性
   - 语法分析器测试：测试各种语法结构的解析
   - 错误处理测试：测试在不同错误情况下的恢复能力

2. **集成测试**：
   - 词法分析器与语法分析器的集成测试
   - 整个编译前端的集成测试

3. **测试用例设计**：
   - 合法程序测试：包含Mini语言的各种语法特性
   - 边界情况测试：测试极端情况下的行为
   - 错误程序测试：包含各种常见错误的程序

4. **测试环境**：
   - 操作系统：Windows 10、macOS、Linux
   - 编译器：GCC 11.2.0、Clang 13.0.0
   - 测试工具：GoogleTest、Valgrind

5. **测试结果分析**：
   - 功能完整性：确保所有语法特性都能正确解析
   - 错误处理能力：确保编译器能够处理各种错误情况
   - 性能评估：测试编译器在处理大型程序时的性能

通过上述测试计划，我们将全面验证Mini语言编译器的功能和性能，确保它能够稳定、高效地完成编译任务。

