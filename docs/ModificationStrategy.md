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