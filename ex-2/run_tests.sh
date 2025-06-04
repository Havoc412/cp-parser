#!/bin/bash

# 测试脚本 - 用于测试语法分析器的错误报告功能

# 切换到ex-2目录
cd "$(dirname "$0")"

# 清理
if [ "$1" = "clean" ]; then
    echo "清理编译文件..."
    rm -f parser
    exit 0
fi

if [ "$1" = "clean-output" ]; then
    echo "清理输出目录..."
    rm -rf tests/*-output
    exit 0
fi

# 编译
echo "编译程序..."
g++ -o parser lexer.cpp parser.cpp main.cpp

# 确保输出目录存在
mkdir -p tests/test1.txt-output
mkdir -p tests/test2.txt-output
mkdir -p tests/test3.txt-output

# 运行测试
echo "测试文件 1 (正确语法)..."
./parser tests/test1.txt

# echo "测试文件 2 (含语法错误)..."
# ./parser tests/test2.txt

# echo "测试文件 3 (含多种语法错误)..."
# ./parser tests/test3.txt

echo "测试完成，请查看输出目录中的错误报告。" 