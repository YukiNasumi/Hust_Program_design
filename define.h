#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TRUE 1
#define FALSE 0
#define OK 1
#define ERROR 0
#define INFEASTABLE -1
#define OVERFLOW -2
#define INCREASEMENT 100

typedef int status;


/*定义子句链表结点结构类型*/
typedef struct Clause{
    int literal;//记录子句中的文字
    int flag;
	//标记该文字是否已被删除，未删除时值为0，否则值为使之删除的变元序号便于回复 
    struct Clause *nextl;//指向该子句中下一文字的指针
    struct Clause *litline;
	//指向整个CNF公式邻接链表中下一个文字相同的子句结点，用于回复与方便查找 
}Clause;

/*定义CNF范式链表结点（即子句链表头结点）结构类型*/
typedef struct Paradigm{
    int number;//子句中显示的文字数，便于找单子句 
    int flag;
	//标记该子句是否已被删除，未删除时值为0，否则值为使之删除的变元序号便于回复 
    struct Paradigm *nextc;//指向下一子句的头结点
    struct Clause *sentence;//子句头指针
}Paradigm;

/*定义CNF范式链表头结点类型，存储CNF范式信息*/
typedef struct Root{
    int litsize;//存储文字数量
    int parasize;//存储子句数量
    Paradigm *first;//指向第一个子句
}Root;
 
//Dpll会用到的两个结构，用于更快寻找正文字和负文字 
/*定义指向子句链表头结点的指针链表结点结构类型*/
typedef struct Paraline{
    Paradigm *claline;//指向子句链表头结点Paradigm
    struct Paraline *next;//指向下一链表结点
} Paraline;

/*定义文字相关信息链表结构类型*/
typedef struct LitTrabverse{
    Paraline *Tra_cla;//指向含有该正文字或负文字的子句头结点链表的头结点
    Clause *Tra_lit;//指向该正文字或负文字的文字结点
}LitTrabverse;

/*定义求解数独文件时所需的存储变元信息结构类型*/
typedef struct sudokusolver{
    int x;//存储行信息
    int y;//存储列信息
    int z;//存储变元对应1～9数值信息
} sudokusolver;

/*定义存储变元信息的变元线性表结点结构类型*/
typedef struct ArgueValue{
    int Value;//变元的真值
    int IsInit;//变元是否已赋值
    int Occur_times;//变元在所有子句中出现的总次数，用于分裂政策的选择 
    LitTrabverse Pos;//变元所有正文字的相关信息结构
    LitTrabverse Neg;//变元所有负文字的相关信息结构
    sudokusolver xyz;//求解数独文件时所需的变元信息
} ArgueValue;


//全局变量
char filename[1000]; //文件名 
long instacksize;//记录栈stack的大小
ArgueValue *ValueList;//变元真值表 
Root *r;//链表 
int *Frequency;//变元出现次数辅助数组 
int *stack;//DPLL辅助栈 
int sudoku_table[9][9];//定义全局变量int类型二维数组存储数独盘
int users_sudoku[9][9];//存储输出的含空格数独问题格局
int sum_op;//总选择 
/*------------------------CNF文件处理------------------------*/

status CreateParadigm(FILE **fp);//创建CNF范式邻接链表及变元表
int CreateClause(FILE **fp,Clause **sentence,Paradigm *ClausHead,int first);//创建子句链表及文字链表
status DestroyParadigm(Root *r);//销毁所有链表及线性表结构
int HasUnitClause(Root *r);//判断CNF范式中是否还含有单子句
Clause * HasUnitClause_Before(Root *r);//判断CNF范式中是否还含有单子句（优化前版本）
status isUnitClause(Paradigm *c);//判断指针c指向的子句链表是否为单子句链表

/*在整个CNF公式中取一个文字*/
int FindLiteral1(Root *r);//取每次DPLL处理后公式中Occur_Times最大的文字
int FindLiteral2(Root *r);//取原公式中Occur_Times最大的文字
int FindLiteral3(Root *r);//取子句中第一个未被删除（flag为0）的文字
int FindLiteral4(Root *r);//变元表正序正序第一个未赋真值（IsInit=0）的变元的正文字
Clause * FindLiteral_Before(Root *r);//在整个CNF公式中取一个文字（优化前版本）

status DeleteClause(Root *r,int l);//删除出现了文字l的所有单子句
status AddClause(Root *r,int l);//在CNF范式邻接链表表头添加只含有文字l的单子句链表
status RemoveHeadClaus(Root *r,int l);//删除CNF范式邻接链表中从表头开始第一个只含有文字l的单子句链表
status DeleteLiteral(Root *r,int l);//删除所有文字为-l的子句链表结点
status RecoverCNF(Root *r,int l);//恢复认为文字l为真时对CNF范式邻接链表所作的操作
void ParadigmTrabverse(Root *r);//遍历CNF范式邻接链表
status SaveValue(ArgueValue *ValueList,int solut,int time);//保存CNF范式的解及求解时间信息

/*------------------------DPLL算法------------------------*/

status SAT(void);
status DPLL1(int num,int op,int times);//采用第一种变元选取策略的递归算法DPLL函数
status DPLL2(int num,int op,int times);//采用第二种变元选取策略的递归算法DPLL函数
status DPLL3(int num,int op,int times);//采用第三种变元选取策略的递归算法DPLL函数
status DPLL4(int num,int op,int times);//采用第四种变元选取策略的递归算法DPLL函数
status DPLL_Before(Root *r,int op);//优化前版本DPLL
void AnswerComplete(void);//完善SAT求解结果
status AnswerCheck(int solut);//检查SAT求解结果正确性

/*------------------------SuDoKu------------------------*/
status Sudoku(void);
FILE * CreateSudokuFile(void);//创建数独问题转化为SAT问题后的cnf文件
status CreateSudoku(void);//创建新的随机9*9数独终盘
status SolveSudoku(void);//求解数独cnf文件并转化为数独终盘形式输出
status DigHole(int num);//对已知数独终盘挖num个洞生成有唯一解的数独问题
status SudokuComplete(void);//完善数独终盘
FILE * SetCNFfile(void);//将数独txt文件转化为cnf 
status CNFSudokuTableTransform(void);//数独对应SAT变元表转化为二维数组
status SudokuTablePrint(void);//输出数独盘
