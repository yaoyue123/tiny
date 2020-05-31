# 对代码的理解

## 原来的代码

### GLOBALS.H

#### MAXRESERVED
保留字的个数。保留字包括：if, then, else, end, repeat, until, read, write  
存储在scan.c中的一个名为reservedWords的数组中，数组的长度即为MAXRESERVED

#### TokenType
枚举类型，意思是标记(token)的类型。包括：  
book-keeping(?)：ENDFILE,ERROR  
保留字：IF,THEN,ELSE,END,REPEAT,UNTIL,READ,WRITE  
多字符token：ID(变量名), NUM(数)  
特殊符号：ASSIGN(赋值), EQ(等号), LT(小于), PLUS(加), MINUS(减), TIMES(乘), OVER(除), LPAREN(左括号), RPAREN(右括号), SEMI(分号)

#### TreeNode
语法树结点类型，是一个结构体  
child数组：长度为三，表示一个结点最多有三个子结点  
sibling：指向兄弟结点  
lineno：行号  
nodekind：结点类型，分为StmtK(statement, 语句)和ExpK(expression, 表达式)
其中，StmtK又可以细分为If, Repeat, Assign, Read, Write  
ExpK又可以细分为Op(运算符号), Const(常量), Id(变量标识符)  
attr：猜测是表达式中结点的属性，其中：  
|结点类型|属性类型|
|-|-|
|Op(运算符号)|TokenType op|
|Const(常量)|int val|
|Id(变量名)|char *name|
type：应该是expression的数据类型，包括void, int, bool  

### UTIL.H

#### printToken
根据标记(token)类型打印词素，具体来说：  
保留字，如if, then, else, end, repeat等等，直接打印出if, then, else, end等等  
特殊符号，如:=, =, +, -, *, /等等，打印出对应的符号  
常数与变量，打印token类型以及具体内容  
ERROR，打印ERROR类型以及ERROR的内容  

#### newStmtNode
创建statement类型的结点，传入的参数指明statement的具体类型：If, Repeat, Assign, Read, Write  
返回指向新的树结点的指针

#### newExpNode
创建expression类型的结点，传入的参数指明expression的具体类型：Op(运算符号), Const(常量), Id(变量标识符)  
返回指向新的树结点的指针

#### copyString
新分配一段内存，将已有字符串复制一份放入新分配内存中，返回新分配内存的指针

#### printTree
打印语法树，用缩进来指明子树  
测试这一函数需要在main.c中将TraceParse置为TRUE，以下是对sample.tny打印的语法树  
```
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
大胆猜测：缩进代表的是它处在语法树的哪一层，缩进相同且上下紧挨在一起的是兄弟结点，它们的双亲结点为：往上走遇到的第一个缩进比它们小的结点。

### SCAN.H SCAN.C

#### StateType
表示DFA中状态的一个数据类型，包括：  
|名字|释义|
|-|-|
|START|开始|
|INASSIGN|在赋值语句中|
|INCOMMENT|在注释中|
|INNUM|在数当中|
|INID|在变量标识符当中|
|DONE|结束|

#### tokenString
一个字符串，存储一个token的词素，如变量标识符具体的变量名，保留字具体的那个字符串(如"if", "then"等等)

#### getNextChar
从当前行中取下一个字符，并返回这个字符。  
具体实现原理是先读取一行，将其放进一个数组中，再从这个数组中返回字符  
如果fgets读取失败，就会返回EOF

#### ungetNextChar
回退一个字符，具体操作就是指向字符的指针减一

#### reservedWords
一个储存保留字的数组，表现了字符串和保留字类型的对应关系

#### reservedLookup
判断一个字符串是保留字还是变量标识符。如果是保留字，返回保留字对应的TokenType，如果是变量标识符则返回ID

#### getToken
SCAN.C定义的函数当中，唯一一个可以被别的文件调用的函数  
具体功能：依次返回源代码中token的类型  
具体算法：  
首先将state置为START，然后便会进入switch语句中的case START部分  
在这一部分中，首先便会判断c(首字符)的类型，根据c的类型选择相应的操作  
|c的类型|操作|
|-|-|
|c是数字(isdigit)|state = INNUM;  进入一个数|
|c是字母(isalpha)|state = INID;  进入一个变量标识符|
|c是冒号(:)|state = INASSIGN; 进入一个赋值符号|
|c是分隔符(空格, \t, \n)|save = FALSE;  不要将这个字符存入tokenString|
|c是左大括号({)|state = INCOMMENT;  进入一段注释<br>save = FALSE;  不要将这个字符存入tokenString|
|c是其他字符|state = DONE;  这直接就是最后一次循环了。然后根据c的具体内容给currentToken赋值，最后函数会返回currentToken的值|

在一次循环的switch结束后，还会进行以下两个操作：  
1.根据save的取值决定是否将字符存入tokenString(存储词素的字符串)  
2.如果state == DONE，则在tokenString末尾加上'\0'，并且再判断一下tokenString究竟是变量标识符还是保留字，根据这个修改一下currentToken。这个时候其实已经到了最后一次循环了，可以等着返回了

第一次循环后state已经不再是START，而是根据第一个字符的类型已经有了不同的值，第二次及之后的state的值与对应的操作如下表：
|state的值|操作|
|-|-|
|INCOMMENT|读取到的字符不会被存入tokenString<br>如果读到文件末尾，则currentToken为ENDFILE，循环结束了<br>如果遇到右大括号，说明注释读完了，于是将state置为START，继续下一个循环|
|INASSIGN|说明之前读到的第一个是冒号，所以接下来必须读到等号<br>如果读到的是等号，则循环结束，函数返回ASSIGN，否则返回ERROR|
|INNUM|说明现在正在读一个数，如果读到一个非数字，说明这个数读完了，退出循环，函数返回NUM<br>这个时候数的内容保存在tokenString当中|
|INID|说明正在读一个变量名，如果读到一个非字母，说明这个变量名读完了，退出循环，函数返回ID<br>从这段代码可以看出，原来TINY的变量名只能包含字母，但TINY+变量名是可以包含数字的，因此是一个修改的点|

通过将TraceScan置为TRUE，可以打印token的类型与词素。打印SAMPLE.TNY的scan结果可以得到以下内容：
```
        5: reserved word: read
        5: ID, name= x        
        5: ;
        6: reserved word: if  
        6: NUM, val= 0        
        6: <
        6: ID, name= x        
        6: reserved word: then
        7: ID, name= fact
        7: :=
        7: NUM, val= 1
        7: ;
        8: reserved word: repeat
        9: ID, name= fact
        9: :=
        9: ID, name= fact
        9: *
        9: ID, name= x
        9: ;
        10: ID, name= x
        10: :=
        10: ID, name= x
        10: -
        10: NUM, val= 1
        11: reserved word: until
        11: ID, name= x
        11: =
        11: NUM, val= 0
        11: ;
        12: reserved word: write
        12: ID, name= fact
        13: reserved word: end
        14: EOF
```