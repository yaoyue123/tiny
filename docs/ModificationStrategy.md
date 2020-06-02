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
declaration-list → declaration-list; declaration | declaration
declaration → type-specifier identifier
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

### 添加对两种数据类型int, char的支持
原来的TINY中，只有一种数据类型int，因此变量也可以不用声明，直接使用。  
TINY+当中添加了一个新的数据类型char，因此必须得声明变量的数据类型才能使用了。  
声明语句是之前TINY没有的全新的语句，里面的词也是新的。因此得从最基本的词法分析开始改，估计得一直改到ANALYZE这一层。  
INT和CHAR可以看作是保留字，因此将其放到GLOBALS.H中的TokenType当中。  
然后在SCAN.C当中，要把INT和CHAR放到保留字的数组里面。同时，UTIL.C中的printToken函数也要修改一下。  

这样就完成了int和char的词法分析。接下来修改构建语法树的代码。我的想法大致是，将declaration通过sibling串起来，然后尾部的一个declaration和statement通过sibling连接，最后，statement再通过sibling串起来。最后，declaration和statement通过sibling构成一个串。  
首先得在GLOBALS.H的StmtKind里面加两个类型：IntK和CharK。  
然后再到PARSE.C当中，添加declaration-list的代码、declaration的代码。  
现在看看declaration的相关文法，发现如果按照要求里面的文法，最后declaration-list末尾会有两个分号，于是便擅自对文法进行了改动。  
declaration-list → declaration-list; declaration | declaration  
declaration → type-specifier identifier  
根据这个文法加函数，然后再改一改parse函数，最后还得去UTIL.C把printTree改一改。  
现在就可以成功生成想要的语法树了。在源代码的开头随便加一点声明，然后parse看看生成的语法树。
```
Syntax tree:
  Int: x
  Char: y
  Char: z
  Int: w
  Int: opqrst
  Read: x
  If
    Op: <
      Const: 0
      Id: x
    Assign to: fact
      Const: 1
    Repeat
      Assign to: fact
        Op: *
          Id: fact
          Id: x
      Assign to: x
        Op: -
          Id: x
          Const: 1
      Op: =
        Id: x
        Const: 0
    Write
      Id: fact
```