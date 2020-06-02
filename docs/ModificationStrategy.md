# 修改策略

## 原来的文法与目标文法的对比

### 原来的文法
```
program		->  stmt-sequence
stmt-sequence	->  stmt-sequence ; statement | statement
statement	->  if-stmt | repeat-stmt | assign-stmt | read-stmt | write-stmt
if-stmt	->  if exp then stmt-sequence end | if exp then stmt-sequence else stmt-sequence end
repeat-stmt	->  repeat stmt-sequence until exp
assign-stmt	->  identifier:=exp
read-stmt	->  read identifier
write-stmt	->  write exp
exp	->  simple-exp comparison-op simple-exp | simple-exp
comparison-op ->  < | =
simple-exp  ->  simple-exp addop term | term
addop	->  + | -
term	->  term mulop factor | factor
mulop	->  * | /
factor	->  (exp) | number | identifier
```
 ### 目标文法
```
program -> declaration-list; stmt-sequence
declaration-list → declaration-list declaration | declaration
declaration → type-specifier identifier;
type-specifier → int | char
stmt-sequence -> stmt-sequence; statement | statement
statement -> if-stmt | repeat-stmt | assign-stmt | read-stmt | write-stmt
if-stmt -> if (exp) then stmt-sequence end
| if (exp) then stmt-sequence else stmt-sequence end
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

## 修改过程

### 添加对变量名中带数字的支持
原来的TINY中，变量名必须得全是字母，但是TINY+中变量名可以含有数字。  
如果把SAMPLE.TNY中的x换成x1y，就会报错，原因在于词法分析时将1看作数字。  
在SCAN.C的getToken函数中，case INID里面  
```
if (!isalpha(c))
```
改成
```
if (!isalpha(c) && !isdigit(c))
```
即可。

### 常数不能以0开头
原来的TINY中，检测到数字就认为进入了一个NUM的token。但是TINY+中的NUM不能以0开头。比如在SAMPLE.TNY中输入x := 010;是不会报错的。  
为了让这种情况报错，在scan.c的getToken中将case START中的 if (isdigit(c)) 改成 if (isdigit(c) && c != '0')。  
这样就能让这种情况报错了，然而这样就不能表示0了。  
经过讨论后，认为还是允许NUM以0开头算了，于是又改回来了。

### 添加NUM中对正负号的支持
原来的TINY中，不支持正负号。但是支持正负号不是一件容易的事，因为原来的文法是一个LL(1)文法，只用看一个符号就能决定下一步的动作。但是加入正负号后，遇到一个'+'或者'-'无法立即反应出这是正负号还是加减号，这给程序的编写带来麻烦。  
想到的解决方法是，在遇到+或者-时，查看前一个token，如果前一个token是NUM或者ID，说明这是个加减号，否则说明这是个正负号。  
给scan.c中的getToken加一个形参TokenType *prev_token，这个指针指向的token值为前一个token，这样就可以根据prev_token来判断这是正负号还是加减号了。相应的，PARSE.C里面要加一个静态全局变量prev_token。  
不过如果这样改，这个正负号不能用于变量。但课程考核要求里面也没说要实现正负号用于变量，问题不大。