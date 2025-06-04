# Mini 语言编译器前端

本项目实现了一个 Mini 语言的编译器前端，包括词法分析器和语法分析器，能够分析源程序并输出分析结果。

## 目录结构

```
ex-2/
├── lexer.h         // 词法分析器头文件
├── lexer.cpp       // 词法分析器实现
├── parser.h        // 语法分析器头文件
├── parser.cpp      // 语法分析器实现
├── main.cpp        // 主程序
├── main.md         // 项目文档
├── README.md       // 本文档
├── examples/       // 示例代码
│   └── e1.cpp      // 示例源代码
└── tests/          // 测试用例
    ├── test1.txt   // 基本测试用例
    ├── test2.txt   // 基本测试用例
    ├── test3.txt   // 复杂测试用例
    └── mini-code/  // Mini语言代码示例
```

## 功能特性

### 词法分析器

1. 支持 Mini 语言的所有词法元素，包括：
   - 关键字（int、double、float、if、then、else、return、while）
   - 运算符（+、-、*、/、=、==、<、<=、>、>=）
   - 分隔符（(、)、[、]、{、}、,、;）
   - 常量（整型、浮点型）
   - 标识符

2. 提供灵活的接口，便于语法分析器调用：
   - `void initLexer(FILE* fp)`：初始化词法分析器
   - `TokenAttr getNextToken()`：获取下一个 Token
   - `void ungetToken()`：回退一个 Token（预读功能）

3. 错误处理：
   - 检测并报告非法标识符、非法数字格式等词法错误
   - 提供详细的错误信息，包括错误类型和行号

### 语法分析器

1. 支持 Mini 语言的语法结构，包括：
   - 程序结构和函数定义
   - 变量声明和赋值语句
   - 条件语句（if-else）
   - 循环语句（while）
   - 表达式计算

2. 提供语法分析接口：
   - `void initParser(FILE* fp)`：初始化语法分析器
   - `ParserResult parse()`：执行语法分析
   - `const std::vector<ParserError>& getParserErrors()`：获取语法错误信息

3. 错误处理：
   - 检测并报告语法错误
   - 实现简单的错误恢复机制，能够在发现错误后继续分析

## 使用方法

### 编译

```bash
g++ -std=c++11 main.cpp lexer.cpp parser.cpp -o compiler
```

### 运行

```bash
./compiler
```

或指定输入文件：

```bash
./compiler input_file.txt
```

### 输出说明

程序会在输入文件的同级目录下创建一个以文件名加"-output"为名的目录，其中包含：
- `tokens.txt`：包含所有识别出的 Token 信息
- `errors.txt`：包含所有词法和语法错误信息（如果有的话）
- `ast.txt`：语法分析生成的抽象语法树（如果语法分析成功）

## Mini 语言简介

Mini 语言是一个简化的类 C 语言，支持基本的语法结构，如变量声明、赋值、条件语句、循环语句等。它的设计目的是作为编译原理课程的教学示例语言。

### 示例程序

```c
int main() {
    int a = 10;
    int b = 20;
    
    if (a < b) {
        a = a + 1;
    } else {
        b = b - 1;
    }
    
    return 0;
}
```

## 未来扩展

本项目设计为模块化结构，便于后续扩展为完整的编译器。计划中的扩展包括：

1. 语义分析：类型检查、作用域分析等
2. 中间代码生成：生成三地址码或抽象语法树
3. 优化：常量折叠、死代码消除等
4. 目标代码生成：生成汇编代码或机器码 