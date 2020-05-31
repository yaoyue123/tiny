# 《编译原理》课程实验报告
###### 2018302180148袁寰宇，	2018 302180149马陈军




## 1.文法描述
Tiny文法
```
program -> stmt-sequence
stmt-sequence -> stmt-sequence; statement | statement
statement -> if-stmt | repeat-stmt | assign-stmt | read-stmt | write-stmt
if-stmt -> if (exp) then stmt-sequence end | if (exp) then stmt-sequence else stmt-sequence end
repeat-stmt -> repeat stmt-sequence until exp
assign-stmt -> identifier := exp
read-stmt -> read identifier
write-stmt -> write exp
exp -> simple-exp comparson-op simple-exp | simple-exp
comparison -> < | =
simple-exp -> simple-exp addop term | term
addop -> + | -
term -> term mulop factor | factor
mulop -> * | /
factor -> (exp) | number | identifier
number -> (+|-)?[1-9][0-9]*
identifier -> [a-zA-Z]([0-9]| [a-zA-Z])*
```
Tiny+文法
```
program -> declaration-list; stmt-sequence
declaration-list → declaration-list declaration | declaration
declaration → type-specifier identifier;
type-specifier → int | char
stmt-sequence -> stmt-sequence; statement | statement
statement -> if-stmt | repeat-stmt | assign-stmt | read-stmt | write-stmt
if-stmt -> if (exp) then stmt-sequence end | if (exp) then stmt-sequence else stmt-sequence end
repeat-stmt -> repeat stmt-sequence until exp
assign-stmt -> identifier := exp
read-stmt -> read identifier
write-stmt -> write exp
exp -> simple-exp comparson-op simple-exp | simple-exp
comparison -> < | =
simple-exp -> simple-exp addop term | term
addop -> + | -
term -> term mulop factor | factor
mulop -> * | /
factor -> (exp) | number | identifier
number -> (+|-)?[1-9][0-9]*
identifier -> [a-zA-Z]([0-9]| [a-zA-Z])*
```

## 2.程序框架

### 2.1 整体框架
#### 2.1.1 Tiny程序流程
```flow
A=>operation: Tiny源代码
B=>subroutine: 词法分析
C=>subroutine: 语法分析
D=>subroutine: 语义分析
E=>subroutine: 代码生成
F=>operation: Tiny目标程序
A(right)->B(right)->C(right)->D(right)->E(right)->F
```
#### 2.1.2 Tm程序流程
```flow
A=>operation: Tiny目标代码
B=>subroutine: 解释执行程序
C=>operation: 代码运行结果
A(right)->B(right)->C(right)
```
### 2.2 词法分析
编译器执行的第一步就是词法分析程序，该程序的功能是当语法分析程序发出一个getToken()调用时，该程序返回一个token值，同时如果token为NUM或ID时，在tokenString中装入NUM的值或ID的名字。Token有五个基本类型：保留字、特殊符号、数、标识符和其它符号。具体见下表
保留字 | 特殊符号 | 数 | 标识符 | 标识符
:-: | :-: | :-: | :-: | :-:
if | bbb | ccc | ddd | eee| 
then | ggg| hhh | iii | 000|
else | ggg| hhh | iii | 000|
fff | ggg| hhh | iii | 000|
fff | ggg| hhh | iii | 000|
fff | ggg| hhh | iii | 000|
fff | ggg| hhh | iii | 000|
fff | ggg| hhh | iii | 000|
fff | ggg| hhh | iii | 000|
fff | ggg| hhh | iii | 000|

开始状态:首先要读进一个字符(调用函数getNextChar)。如读入一个空白字符,则跳过它,继续读字符,直到读入一个非空字符为止。接下去根据所读进的非空字符跳转至相应的程序进行处理。

标识符状态:识别并组合成标识符以后,调用保留字查表函数reservedlookup,用以确定是保留字还是用户自定义的标识符。按情况输出相应单词:如是标识符则输出单词ID,并将标识符的名字保存在tokenString中,如是保留字则输出相应的单词标志码。 

整数状态:识别并组合数字，输出单词NUM并将数值结果保存在tokenString中。 

单分界符状态:只需输出其内部的单词标志码。 

双分界符状态:如果读入的下一个字符不是”=”则报错,输出单词ERROR。否则,成功识别,输出单词ASSIGN。 

注释状态:略过注释内容，直到遇到注释结束标志或者是文件结束标志EOF,并不生成单词。 

错误状态:表示词法分析程序从源程序读入了一个不合法的字符,打印错误信息,输出单词ERROR,略过产生错误的字符，转开始状态继续识别和组合下一个单词符号。 
### 2.3 语法分析

### 2.4 语义分析

### 2.5 代码生成


## 3.测试案例与测试结果


## 4.提交内容说明

### 4.1源程序运行环境说明

### 4.2程序运行操作说明

## 5.实验小结