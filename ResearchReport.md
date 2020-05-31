# 基于llvm的跳转基本块的划分
###### 2018302180148袁寰宇，	2018 302180149马陈军

  


## 1.概述

&emsp;&emsp;本项目是我们与李子昂同学(2018302180162) 等人在信安大赛参加的作品《神经网络分析二进制控制流图的间接跳转》的一个子项目。本项目的主要内容是利用LLVM这一工具，去寻找C语言源代码中的跳转基本块，并找到跳转基本块之间的对应关系。


## 2.LLVM相关技术的背景及现状

&emsp;&emsp;LLVM是伊利诺伊州大学的ChrisLattner在博士期间开发的一个编译器框架。其旨在优化以多种源程序语言编写的程序的编译时间、链接时间、运行时间以及空闲时间。其特点是支持多种源程序语言编译成为统一的LLVM IR从而进行统一的优化，在编译速度，中间代码占用空间和错误提示等方面比传统的GCC编译器做的更好。目前LLVM编译器已经成为美国苹果公司主流编译器，作为一个新生的编译器正在向GCC发起挑战，目前无论从科研角度还是实际用户，LLVM越来越被更多的开发者和用户所接受。

&emsp;&emsp;Clang编译器可以将多种编程语言的源代码进行编译，并生成LLVM特有的中间表示。这是LLVM开发的一款语言，应用于LLVM的中端处理，类似于汇编的虚拟指令集，旨在于帮助开发者分析程序和优化代码等。虽然LLVM中间表示属于低级表示的语言，但是通过指令组合可以支持高级语言的表示，再通过LLVM优化器Opt提供的Pass或者自行实现对应的程序分析的方法实现对程序的分析和转换。  

&emsp;&emsp;LLVM Pass是编译器中的一部分，能够对代码进行转化和优化。Pass就是“遍历一遍IR，可以同时对它做一些操作”的意思。翻译成中文应该叫“趟”。在实现上，LLVM的核心库中会给你一些Pass类去继承。你需要实现它的一些方法。最后使用LLVM的编译器会把它翻译得到的IR传入Pass里，给你遍历和修改。

&emsp;&emsp;LLVM Pass的用处：
> 1. 插桩，在Pass遍历LLVM IR的同时，往里面插入新的代码。
>
> 2. 机器无关的代码优化：IR在被翻译成机器码前会做一些机器无关的优化。但是不同的优化方法之间需要解耦，所以自然要各自遍历一遍IR，实现成了一个个LLVM Pass。最终，基于LLVM的编译器会在前端生成LLVMIR后调用一些LLVM Pass做机器无关优化，然后再调用LLVM后端生成目标平台代码。
>
> 3. 静态分析：像VSCode的C/C++插件就会用LLVM Pass来分析代码，提示可能的错误(无用的变量、无法到达的代码等等)。




## 3.跳转基本块相关应用分析

&emsp;&emsp;程序分析是对软件缺陷采用自动化工具进行分析，相对于程序测试的手动测试，采用这种方式可以提高查找软件缺陷的效率。根据分析方法不同可以分为程序静态分析和程序动态分析，它们各有优缺点。

&emsp;&emsp;程序动态分析目前使用较广，被广泛地用于程序漏洞挖掘方面。程序动态分析使用可执行文件作为输入，具体执行程序，通过有效的策略遍历执行程序的执行路径，发掘程序的缺陷。目前采用的方式有指令插桩、虚拟机模拟运行等方式。因为知道程序的变量值所以具有较低的误报率，但是不清楚程序内部具体结构，有一定的漏报。程序动态分析更加偏向于黑盒fuzz测试。

&emsp;&emsp;程序静态分析近些年在学术界和工业界研究较多，也有部分成熟的产品，包括Coverity Prevent， Klockwork k7等。程序静态分析不执行程序，而是通过对程序源代码或者经过源代码编译产生的中间代码进行分析，常用的分析方法包括：数据流分析、污点分析、符号执行和模型检测等方式。程序静态分析能够得到程序内部的数据流信息和控制流信息，分析结果具有较高的程序路径覆盖率和较低的漏报，但是某些变景的值只有在运行时冰确定，所以误报率比较高。程序静态分析更加偏向于白盒测试。

&emsp;&emsp;程序静态分析经过多年的发展，逐渐形成一个体系，根据采用的技术不同分为不同的发展方向，其中包括，控制流分析、数据流分析、符号执行、污点分析、模型检测、定理证明等方向。这里主要介绍控制流图分析：

&emsp;&emsp;计算机程序的本质是指令对数据进行操作，控制流分析是根据程序的指令对程序的执行路径进行分析。一般情况下，程序都不是按照指令顺序从开始执行到结束，程序之中而是存在一些控制结构包括：顺序结构、分支结构和循环结构。顺序结构中除了最后一条指令，不存在跳转和返回的指令，几条顺序结构的指令构成一个基本块；分支结构根据顺序结构最后一条指令将程序流转向另外一个基本块，分支结构连接一个前趋基本块和后继基本块，并有前趋指向后继；循环结构则是分支结构的一种特殊情况，在某些情况之下分支结构的会构成环结构，这样就产生循环结构。

&emsp;&emsp;由上述三种结构构成程序流图，顺序结构就是程序流图中的基本块，分支结构则构成程序流图中的有向边，循环结构则是程序流图中的环。控制流分支则是按照程序流图的结构对程序中的数据进行分析。

&emsp;&emsp;控制流分析一般采用深度优先遍历算法和广度优先遍历算法。因为程序分析是在前趋基本块分析完成之后，再对后继基本块进行分析，所以一般采用先序遍历的思想。在程序静态分析中，先序深度优先遍历算法采用栈的思想，在该基本块分析完成之后，对第一个后继基本块及其所有的后继基本块进行分析，在分析第二个以此类推。先序广度优先遍历算法采用队列的思想，首先分析第一个基本块，然后分析其所有后继基本块，然后分析这些基本块的后继基本块。无论是采用深度优先遍历算法，还是采用广度优先的遍历算法，都需要对控制流图中的循环有一定的处理方式，否则容易产生死循环，耗尽CPU资源和内存资源。先序深度优先遍历相对广度优先遍历耗内存较少，而广度优先遍历相对于深度优先遍历更加适合并行化处理。

&emsp;&emsp;目前控制流分析使用非常普遍，在很多别的分析过程中也采用它作为基本的技术，程序控制流图的引入提高了程序分析的精确性。

&emsp;&emsp;为了实现更好的实现控制流分析，这里我们实现了一个基于LLVM Pass的跳转基本块寻找程序。

具体的代码实现如下：
```c
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace llvm;

namespace {
  
  struct SwitchBB : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    SwitchBB() : ModulePass(ID) {}
    std::string get_block_reference(BasicBlock *BB){ //基本块信息获取
        std::string block_address;
        raw_string_ostream string_stream(block_address);
        BB->printAsOperand(string_stream, false);
        return string_stream.str();
    }
    virtual bool runOnModule(Module &M) {
        std::string filename = M.getSourceFileName();
        std::string dir_info = "";
        std::string outfile_1 = ""; //输出内容1
        errs()<<filename<<"\n";
        std::string name = "output1.txt";
        std::ofstream outfile1; //输出流1
        errs()<<dir_info + name<<"\n"; //写入的txt文件
        for(Module::iterator Mit=M.begin(),Mite=M.end();Mit!=Mite;Mit++){
            StringRef function_name = Mit->getName();
            std::ofstream outfile2; //输出流2
            std::string dir_edge = "";
            std::string name_1 = "output2.txt";
            std::string outfile_2 = ""; //输出内容2
            errs()<<dir_edge + name_1<<"\n";
            unsigned this_function_has_switch = 0;
            StringRef cur_switch_block;
            std::vector<int> cur_switch_line;
            for(llvm::Function::iterator Fit=Mit->begin(),Fite=Mit->end();Fit!=Fite;Fit++){
                BasicBlock* bb_0 = dyn_cast<BasicBlock>(&*Fit);
                StringRef bb_name = get_block_reference(bb_0);
                std::vector<int> bb_line;
                for(BasicBlock::iterator Bit=Fit->begin(),Bite=Fit->end();Bit!=Bite;Bit++){
                    const DebugLoc &location = Bit->getDebugLoc();//??
                    Instruction* curr_inst = dyn_cast<Instruction>(&*Bit);
                    if(isa<Instruction>(&*Bit)){//??
                        if(bool(location)){
                            bb_line.push_back(location.getLine());//??
                        }
                    }
                    SwitchInst* switchInst = dyn_cast<SwitchInst>(&*Bit);//??
                    if(isa<SwitchInst>(&*Bit)){
                        this_function_has_switch = 1;
                        unsigned line_of_switch = location.getLine();
                        errs()<<"line of switch:"<<line_of_switch<<"\n";
                        cur_switch_block = bb_name;
                        //修改输出内容1
                        outfile_1 = outfile_1+"function"+function_name.str()+"\nbasic_block:"+bb_name.str()+"\nswitch_line:"+std::to_string(line_of_switch)+"\nswitch_block";
                        sort(bb_line.begin(),bb_line.end());
                        bb_line.erase(unique(bb_line.begin(), bb_line.end()), bb_line.end());
                        for(int i=0;i<bb_line.size();++i){
                            outfile_1 = outfile_1+std::to_string(bb_line[i])+" ";
                        }
                        outfile_1 += "\n";
                        cur_switch_line.assign(bb_line.begin(), bb_line.end());
                        for(Function::iterator f = Mit->begin(),fite = Mit->end();f!=fite;f++){//??
                            BasicBlock* bb_1 = dyn_cast<BasicBlock>(&*f);
                            StringRef bb_name = get_block_reference(bb_1);
                            std::vector<int> bb_line_case;
                            for(BasicBlock::iterator bit=f->begin(),bite=f->end();bit!=bite;bit++){
                                const DebugLoc &location = bit->getDebugLoc();
                                Instruction* curr_inst = dyn_cast<Instruction>(&*bit);
                                if(isa<Instruction>(&*bit)){
                                    if(bool(location)){
                                        bb_line_case.push_back(location.getLine());
                                    }
                                }
                            }
                            unsigned flag = 0;
                            sort(bb_line_case.begin(),bb_line_case.end());
                            bb_line_case.erase(unique(bb_line_case.begin(), bb_line_case.end()), bb_line_case.end());
                            BasicBlock* bb = dyn_cast<BasicBlock>(&*f);
                            for(BasicBlock *Pred : predecessors(bb)){
                                std::string pre_name = get_block_reference(Pred);
                                if(pre_name.compare(cur_switch_block) == 0){
                                    flag = 1;
                                    break;
                                }
                            }
                            if(flag == 1){
                            outfile_2+=std::to_string(cur_switch_line[cur_switch_line.size()-1])+" -> "+std::to_string(bb_line_case[0])+"\n";
                            }
                        }//??
                        
                    }
                }
            }
            errs()<<"out2:"<<outfile_2<<"\n";
            if(outfile_2 != ""){
                outfile2.open(dir_edge + name_1);
                outfile2<<outfile_2;
                outfile2.close();
            }
        }
        
        //outfile_1 = "Test.";
        errs()<<"out1:"<<outfile_1<<"\n";
        if(outfile_1 != ""){
            outfile1.open(dir_info + name);
            outfile1<<outfile_1;
            outfile1.close();
        }
        return false;
    }
  };
}

char SwitchBB::ID = 0;
static RegisterPass<SwitchBB> X("SwitchBB", "SWITCH.");
```
以一个简单c语言程序来展示该LLVM Pass的作用

```c
#include<stdio.h>
#include<stdlib.h>
int main(){
    int n = 5;
    switch(n){
        case 1:break;
        case 2:break;
        case 3:break;
        case 5:printf("get it."); break;
        default:break;
    }
    return 0;
}
```

利用Clang把C语言代码生成为LLVM字节码的文本形式 .ll 文件

```bash
clang test.c  -O0 -g -S -emit-llvm -o test.ll 
```
再利用LLVM Pass编译出来的 .so 文件对LLVM字节码进行分析
```bash
opt -load /root/llvm/build/src/libTest.so -SwitchBB test.ll
```
最终生成的结果如下：

output1.txt
```bash
functionmain
basic_block:%0
switch_line:5
switch_block:4 5 
```
output2.txt
```bash
5 -> 6
5 -> 7
5 -> 8
5 -> 9
5 -> 10
```
output1展示了switch基本块的信息，output2展示了switch语句与case语句的对应关系

## 4.参考文献
