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
#define EXIST 0
#define NO_RESULT 5

typedef int status;


/*定义子句链表结点结构类型*/
typedef struct Clause{
    int literal;//记录子句中的文字
    int flag;
	//标记该文字是否已被删除，未删除时值为0(EXIST)，否则值为使之删除的变元序号便于回复 
    struct Clause *nextl;//指向该子句中下一文字的指针
    struct Clause *litline;
	//指向整个CNF公式邻接链表中下一个文字相同的子句结点，用于回复与方便查找 
}Clause;

/*定义CNF范式链表结点（即子句链表头结点）结构类型*/
typedef struct Paradigm{
    int number;//子句中显示的文字数，便于找单子句 
    int flag;
	//标记该子句是否已被删除，未删除时值为0(EXIST)，否则值为使之删除的变元序号便于回复 
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



/*------------------------CNF文件处理------------------------*/

/*创建CNF范式邻接链表及变元表
 参数：(FILE**)open：文件的指针地址*/
status CreateParadigm(FILE **fp) {
	char readfile[20];//定义字符类型数组记录在文件中读到的内容
	int l,j;
	Paradigm *p,*q;
	r=(Root*)malloc(sizeof(Root));
	r->first=NULL;
	while(fscanf(*fp,"%s",readfile)!=EOF) { //循环读文件
		if (strcmp(readfile,"p")==0) //读到‘p’说明开始读文件
			break;
	}
	while (fscanf(*fp, "%s",readfile)!=EOF) {
		if (strcmp(readfile,"cnf")==0) {//从文件中读到字符串“cnf”
			fscanf(*fp, "%d",&l);
			r->litsize=l;//读取CNF文件 变元数 存入r->litsize
			fscanf(*fp, "%d",&l);
			r->parasize=l;//读取CNF文件 子句总数 存入r->parasize
			break;
		}
	}
	int i;
	if(sum_op==1) printf("文字数%d ，子句数%d！\n",r->litsize,r->parasize);
	//创建变元表（初始化）
	ValueList=(ArgueValue*)malloc((r->litsize+1)*sizeof(ArgueValue));
	if (ValueList==NULL) return OVERFLOW;//没分配成功
	for (i=0; i<=r->litsize; i++) {
		ValueList[i].IsInit=0;
		ValueList[i].Occur_times=0;
		ValueList[i].Pos.Tra_cla=NULL;
		ValueList[i].Pos.Tra_lit=NULL;
		ValueList[i].Neg.Tra_cla=NULL;
		ValueList[i].Neg.Tra_lit=NULL;
	}

	Frequency=(int *)malloc((r->litsize)*sizeof(int));

	i=2;
	if (r->parasize!=0) {
		//创建第一个子句头结点链表结点
		if (fscanf(*fp, "%d",&l)!=EOF&&l!=0) {
			p=(Paradigm*)malloc(sizeof(Paradigm));
			if (p==NULL) return OVERFLOW;
			r->first=p;//头节点
			q=p;
			p->number=CreateClause(fp, &p->sentence, p, l);//创建其对应子句链表
			p->flag=EXIST;
			p->nextc=NULL;

			//创建CNF范式链表
			while (i<=r->parasize) {
				i++;
				fscanf(*fp, "%d",&l);
				p=(Paradigm*)malloc(sizeof(Paradigm));
				if (p==NULL) return OVERFLOW;
				p->number=CreateClause(fp, &p->sentence, p, l);
				p->flag=0;
				p->nextc=NULL;
				q->nextc=p;
				q=q->nextc;
			}
		}
	}

	//创建栈stack记录DPLL过程中设为1的文字及随之产生的单子句文字(后续用DPLL)
	stack=(int*)malloc(20000*sizeof(int));
	stack[0]=0;
	instacksize=0;

	/*将变元按出现次数由多至少在Frequency数组中顺序排列*/
	for (i=0; i<r->litsize; i++)
		Frequency[i]=i+1;
	for (i=0; i<r->litsize; i++) {
		for (j=i+1; j<=r->litsize; j++) {
			if (ValueList[Frequency[i]].Occur_times<ValueList[Frequency[j-1]].Occur_times) {
				l=Frequency[i];
				Frequency[i]=Frequency[j-1];
				Frequency[j-1]=l;
			}
		}
	}
	fclose(*fp);
	return OK;
}

/*创建子句链表及文字链表
 参数：(FILE**)open：文件的指针地址
       (Clause **)sentence:子句的头结点 Clause类型指针地址
       (Paradigm *)ClausHead：子句链表头节点指针值
	   (int )first：创建的子句的第一个文字的值
 返回值：i：子句内文字的数量
        ERROR：该子句为空子句
        OVERFLOW：空间不足溢出*/
int CreateClause(FILE **fp,Clause **sentence,Paradigm *ClausHead,int first) {
	Clause *p,*q;
	Paraline *pline;
	int l,l1,i=0;//i记录单个子句中文字数量
	if (first==0) return ERROR;//不存在没有文字的子句

	p=(Clause*)malloc(sizeof(Clause));
	if (p==NULL) return OVERFLOW;
	p->literal=first;
	p->flag=0;
	p->nextl=NULL;
	p->litline=NULL;
	q=p;
	*sentence=p;
	i++;

	l1=abs(first);
	ValueList[l1].Occur_times++;
	if (first>0) { //用来DPLL时寻找相同文字的不同子句以及删除后回复
		p->litline=ValueList[l1].Pos.Tra_lit;
		ValueList[l1].Pos.Tra_lit=p;
		pline=(Paraline*)malloc(sizeof(Paraline));
		pline->claline=ClausHead;
		pline->next=ValueList[l1].Pos.Tra_cla;
		ValueList[l1].Pos.Tra_cla=pline;
	} else { //同上
		p->litline=ValueList[l1].Neg.Tra_lit;
		ValueList[l1].Neg.Tra_lit=p;
		pline=(Paraline*)malloc(sizeof(Paraline));
		pline->claline=ClausHead;
		pline->next=ValueList[l1].Neg.Tra_cla;
		ValueList[l1].Neg.Tra_cla=pline;
	}

	fscanf(*fp, "%d",&l);
	while (l!=0) {
		// 创建文字链表结点
		p=(Clause*)malloc(sizeof(Clause));
		if (p==NULL) return OVERFLOW;
		p->literal=l;
		p->flag=0;
		p->nextl=NULL;
		q->nextl=p;
		q=q->nextl;
		i++;

		// 创建变元链表
		l1=abs(l);
		ValueList[l1].Occur_times++;
		if (l>0) {
			p->litline=ValueList[l1].Pos.Tra_lit;
			ValueList[l1].Pos.Tra_lit=p;
			pline=(Paraline*)malloc(sizeof(Paraline));
			pline->claline=ClausHead;
			pline->next=ValueList[l1].Pos.Tra_cla;
			ValueList[l1].Pos.Tra_cla=pline;
		} else {
			p->litline=ValueList[l1].Neg.Tra_lit;
			ValueList[l1].Neg.Tra_lit=p;
			pline=(Paraline*)malloc(sizeof(Paraline));
			pline->claline=ClausHead;
			pline->next=ValueList[l1].Neg.Tra_cla;
			ValueList[l1].Neg.Tra_cla=pline;
		}
		fscanf(*fp, "%d",&l);//读取下一文字
	}

	return i;
}

/*销毁CNF范式邻接链表*/
status DestroyParadigm(Root *r) {
	Paradigm *p;
	Clause *t;
	Paraline *pline;
	int i;
	if (r->first!=NULL) {
		p=r->first;
		//销毁CNF范式链表
		while (p!=NULL) {
			if (p->sentence!=NULL) {
				t=p->sentence;
				//销毁单个子句链表
				while (t!=NULL) {
					p->sentence=t->nextl;
					free(t);//释放存储文字的单个子句链表结点空间
					t=p->sentence;
				}
			}
			r->first=p->nextc;
			free(p);//释放子句链表头结点存储空间
			p=r->first;
		}
	}
	//释放变元正负文字信息链表存储空间
	for (i=0; i<=r->litsize; i++) {
		for (pline=ValueList[i].Pos.Tra_cla; pline!=NULL; pline=ValueList[i].Pos.Tra_cla) {
			ValueList[i].Pos.Tra_cla=pline->next;
			free(pline);
		}
		ValueList[i].Pos.Tra_lit=NULL;
		for (pline=ValueList[i].Neg.Tra_cla; pline!=NULL; pline=ValueList[i].Neg.Tra_cla) {
			ValueList[i].Neg.Tra_cla=pline->next;
			free(pline);
		}
		ValueList[i].Neg.Tra_lit=NULL;
	}

	free(Frequency);
	free(r);
	return OK;
}

/*判断CNF范式中是否存在单子句
 返回类型：int
 返回值：存在单子句：该单子句所含的唯一文字
       不存在单子句：0*/
int HasUnitClause(Root *r) {
	Paradigm *p;
	Clause *t;
	for(p=r->first; p!=NULL; p=p->nextc) {
		if(p->flag==0) { //子句没有被删除
			if(p->number==1)//子句文字数为1;
				for (t=p->sentence; t!=NULL; t=t->nextl) {
					if (t->flag==0)//文字没有被删除
						return t->literal;
				}

		}
	}
	return 0;
}

/*判断CNF范式中是否还含有单子句（优化前版本）
 返回类型：Clause *
 返回值：存在单子句：该单子句所含的唯一Clause结点的指针
       不存在单子句：NULL*/
Clause * HasUnitClause_Before(Root *r) {
	Paradigm *p;
	Clause *t;
	for(p=r->first; p!=NULL; p=p->nextc) {
		if(p->flag==0) { //子句没有被删除
			if(p->number==1)//子句文字数为1;
				for (t=p->sentence; t!=NULL; t=t->nextl) {
					if (t->flag==EXIST)//文字没有被删除
						return t;
				}

		}
	}
return NULL;
}

/*判断参数指针c指向的子句头结点对应子句是否为单子句；
 返回值：TRUE：是单子句；
       ERROR：不是单子句*/
status isUnitClause(Paradigm *c) {
	if (c->number==1)
		return TRUE;
	else return FALSE;
}

/*在整个CNF公式中取一个文字*/

/*way1: 取每次DPLL处理后公式中出现次数最多（Occur_Times值最大）的文字
 返回类型：int
 返回值：公式非空：选中文字的值
       公式中没有剩余文字：0*/
int FindLiteral1(Root *r) {
	int i,num=0,flag=0;
	for (i=1; i<=r->litsize; i++) {
		if (ValueList[i].IsInit==0) {
			num=i;//找到变元表正序第一个未被赋真值的变元，num记录其序号
			flag=1;//表示变元表内存在未被赋真值的变元
			break;
		}
	}
	if (flag==1) {
		for (i++; i<=r->litsize; i++) {
			if (ValueList[i].IsInit==0) {
				if (ValueList[i].Occur_times>ValueList[num].Occur_times)
					num=i;
			}
		}
	}
	return num;//返回变元表里出现最多的未赋值变元序号
}

/*way2:取原公式中未赋真值的变元中出现次数最多（Occur_Times最大）的变元正文字
 返回类型：int
 返回值：公式非空：选中文字的值
       公式中没有剩余文字：0*/
int FindLiteral2(Root *r) {//frequency  降序记录出现次数的多的序号
	int i=0;
	for (i=0; i<r->litsize; i++) {
		if (ValueList[Frequency[i]].IsInit==0) {
			return Frequency[i];
		}
	}
	return 0;
}

/*way3:取子句中第一个未被删除（flag为0）的文字
 返回类型：int
 返回值：公式非空：选中文字的值
 公式中没有剩余文字：0*/
int FindLiteral3(Root *r){
    Paradigm *p;
    Clause *c;
    for (p=r->first; p!=NULL; p=p->nextc)
        if (p->flag==EXIST) {//子句未被删除
            for (c=p->sentence; c!=NULL; c=c->nextl) {
                if (c->flag==EXIST) {//文字未被删除
                    return c->literal;
                }
            }
        }
    return 0;
}

/*way4: 取变元表正序正序第一个未赋真值（IsInit=0）的变元的正文字
 返回类型：int
 返回值：公式非空：选中文字的值
       公式中没有剩余文字：0*/
int FindLiteral4(Root *r){
    int i=0;
    for (i=1; i<=r->litsize; i++) {
        if (ValueList[i].IsInit==0) {
            return i;
        }
    }
    return 0;
}

/*在整个CNF公式中取一个文字（优化前版本）：取子句中第一个未被删除（flag为0）的文字
 返回类型：Clause *
 返回值：公式非空：选中文字的结点指针
       公式中没有剩余文字：NULL*/
Clause * FindLiteral_Before(Root *r) {
	Paradigm *p;
	Clause *c;
	for (p=r->first; p!=NULL; p=p->nextc) {
		if (p->flag==EXIST) {
			for (c=p->sentence; c!=NULL; c=c->nextl) {
				if (c->flag==EXIST) {
					return c;
				}
			}
		}
	}
	return NULL;
}

/*删除含参数l的子句
 参数l：真值为1的文字（int）
  (此函数不应使公式中的子句数量减少）*///paradigm中num不会变
status DeleteClause(Root *r,int l) {//改变文字中的flag，子句头节点的num，Valuelist里的出现数目
	int l1;
	Paradigm *p;
	Clause *c;
	Paraline *pline;
	l1=abs(l);
	if(l>0) {
		//l为正文字情况，搜索l对应变元的正文字信息链表
		for(pline=ValueList[l1].Pos.Tra_cla; pline; pline=pline->next) {
			p=pline->claline;
			if(p->flag==EXIST) { //p指向的子句头结点对应子句未被删除
				p->flag=l1;//将p指向结点的flag值标记为l，表示该子句因文字l真值为1而被删除
				r->parasize--;
				for (c=p->sentence; c!=NULL; c=c->nextl)//依次修改p指向子句的flag标记值
					if (c->flag==EXIST) {
						c->flag=l1;
						ValueList[abs(c->literal)].Occur_times--;
						p->number--;
					}
			}
		}
	} else {
		//l为负文字情况，搜索l对应变元的负文字信息链表
		for (pline=ValueList[l1].Neg.Tra_cla; pline!=NULL; pline=pline->next) {
			p=pline->claline;
			if (p->flag==0) {
				p->flag=l1;
				r->parasize--;
				for (c=p->sentence; c!=NULL; c=c->nextl)
					if (c->flag==0) {
						c->flag=l1;
						ValueList[abs(c->literal)].Occur_times--;
						p->number--;
					}
			}
		}
	}
	return OK;
}

/*删除所有文字为-l的子句链表结点
 参数l：(int)真值为1的文字；
 返回值：OK：成功删除文字；
       FALSE：公式无解
 (此函数不应使公式中的子句数量减少）*/
status DeleteLiteral(Root *r,int l) {
	Paradigm *p;
	Clause *c;
	Paraline *pline;
	int l1,l_op;
	l1=abs(l);
	l_op=0-l;//要删除的文字
	if (l>0) {
		for (pline=ValueList[l1].Neg.Tra_cla; pline!=NULL; pline=pline->next) {
			p=pline->claline;
			if (p->flag==EXIST) {//p指向的子句链表头结点对应的子句未被删除
				for (c=p->sentence; c!=NULL; c=c->nextl) {
					if (c->literal==l_op) {
						if (c->flag==EXIST) {//c指向子句链表结点内文字未被删除
							if (p->number==1) return FALSE;//要删除的文字为当前子句内的唯一文字，该子句真值为0，公式无解
							c->flag=l1;//标记该结点内flag值为l1，表示因l真值为1而被删除
							ValueList[abs(c->literal)].Occur_times--;//变元表内该变元出现的次数减一
							p->number--;
						}
					}
				}
			}
		}
	} else {
		for (pline=ValueList[l1].Pos.Tra_cla; pline!=NULL; pline=pline->next) {
			p=pline->claline;
			if (p->flag==EXIST) {
				for (c=p->sentence; c!=NULL; c=c->nextl) {
					if (c->literal==l_op) {
						if (c->flag==EXIST) {
							if (p->number==1) return FALSE;
							c->flag=l1;
							ValueList[abs(c->literal)].Occur_times--;
							p->number--;
						}
					}
				}
			}
		}
	}
	return OK;
}

/*在CNF范式邻接链表表头添加只含有文字l的单子句链表
 参数l：(int)增加的单子句内的唯一文字
 返回值：OVERFLOW：空间不足溢出；
        OK：子句增加成功*/
status AddClause(Root *r,int l) {
	Paradigm *p;
	Clause *c;
	Paraline *pline;
	int l1;
	p=(Paradigm*)malloc(sizeof(Paradigm));
	if (p==NULL) return OVERFLOW;
	p->nextc=r->first;
	c=(Clause*)malloc(sizeof(Clause));
	if (c==NULL) return OVERFLOW;
	c->literal=l;
	c->flag=0;
	c->nextl=NULL;
	l1=abs(l);
	if (l>0) {
		//l为正文字，在变元表内l对应变元的正文字信息链表表头增加新增结点信息
		c->litline=ValueList[l1].Pos.Tra_lit;
		ValueList[l1].Pos.Tra_lit=c;
		pline=(Paraline*)malloc(sizeof(Paraline));
		pline->claline=p;
		pline->next=ValueList[l1].Pos.Tra_cla;
		ValueList[l1].Pos.Tra_cla=pline;
	} else {
		//l为负文字，在变元表内l对应变元的负文字信息链表表头增加新增结点信息
		c->litline=ValueList[l1].Neg.Tra_lit;
		ValueList[l1].Neg.Tra_lit=c;
		pline=(Paraline*)malloc(sizeof(Paraline));
		pline->claline=p;
		pline->next=ValueList[l1].Neg.Tra_cla;
		ValueList[l1].Neg.Tra_cla=pline;
	}
	p->sentence=c;
	p->number=1;
	p->flag=0;
	r->first=p;
	r->parasize++;
	return OK;
}

/*删除CNF范式邻接链表中从表头开始第一个只含有文字l的单子句链表
 参数l：(int)要删除的单子句的唯一文字值；
 返回值：OK：删除成功*/
status RemoveHeadClaus(Root *r,int l) {
	Paradigm *p,*q;
	Clause *c;
	Paraline *pline;
	int l1;
	l1=abs(l);
	if (l>0) {
		//l为正文字，要删除的结点信息必在变元表内l对应变元的正文字信息链表表头
		pline=ValueList[l1].Pos.Tra_cla;
		p=pline->claline;
		ValueList[l1].Pos.Tra_cla=pline->next;
		ValueList[l1].Pos.Tra_lit=ValueList[l1].Pos.Tra_lit->litline;
	} else {
		//l为负文字，要删除的结点信息必在变元表内l对应变元的负文字信息链表表头
		pline=ValueList[l1].Neg.Tra_cla;
		p=pline->claline;
		ValueList[l1].Neg.Tra_cla=pline->next;
		ValueList[l1].Neg.Tra_lit=ValueList[l1].Neg.Tra_lit->litline;
	}
	if (r->first==p) {
		r->first=p->nextc;
	} else {
		for (q=r->first; q!=NULL; q=q->nextc) {//寻找从表头开始第一个只含文字l的单子句
			if (q->nextc==p)
				break;
		}
		q->nextc=p->nextc;
	}
	c=p->sentence;
	free(c);
	free(p);
	free(pline);
	r->parasize--;
	return  OK;
}

/*恢复认为文字l为真前的CNF邻接链表
 参数：(int)文字l:真值为1时求解出错
 返回值：OK：邻接链表恢复成功*/
status RecoverCNF(Root *r,int l) {
	Paradigm *p;
	Clause *c;
	Paraline *pline;
	int l1;
	l1=abs(l);
	if (l>0) {
		//l为正文字，对变元l1的正文字信息链表进行搜索，寻找被删除的子句
		for(pline=ValueList[l1].Pos.Tra_cla; pline; pline=pline->next) {
			p=pline->claline;
			if(p->flag==l1) {
				p->flag=0;
				r->parasize++;
				for(c=p->sentence; c; c=c->nextl) {
					if(c->flag==l1) {
						c->flag=0;
						p->number++;
					}
				}
			}
		}
		//l为正文字，对变元l1的负文字信息链表进行搜索，寻找被删除的文字
		for (pline=ValueList[l1].Neg.Tra_cla; pline!=NULL; pline=pline->next) {
			p=pline->claline;
			for (c=p->sentence; c!=NULL; c=c->nextl) {
				if (c->flag==l1) {
					c->flag=0;
					p->number++;
				}
			}
		}
	} else {
		//l为负文字，对变元l1的负文字信息链表进行搜索，寻找被删除的子句
		for (pline=ValueList[l1].Neg.Tra_cla; pline!=NULL; pline=pline->next) {
			p=pline->claline;
			if (p->flag==l1) {
				p->flag=0;
				r->parasize++;
				for (c=p->sentence; c!=NULL; c=c->nextl) {
					if (c->flag==l1) {
						c->flag=0;
						p->number++;
					}
				}
			}
		}
		//l为负文字，对变元l1的正文字信息链表进行搜索，寻找被删除的文字
		for (pline=ValueList[l1].Pos.Tra_cla; pline!=NULL; pline=pline->next) {
			p=pline->claline;
			for (c=p->sentence; c!=NULL; c=c->nextl) {
				if (c->flag==l1) {
					c->flag=0;
					p->number++;
				}
			}
		}
	}
	return OK;
}

/*遍历CNF范式邻接链表*/
void ParadigmTrabverse(Root *r) {
	Paradigm *p;
	Clause *c;
	int l,value,flag,i=1;
	for (p=r->first; p!=NULL; p=p->nextc) {
		flag=0;
		printf("第%4d行  ",i++);
		for (c=p->sentence; c!=NULL; c=c->nextl) {
			printf("%4d  ",c->literal);
		}
		printf("\n");
	}
	printf("遍历完毕！\n");
}

/*保存CNF范式的解及求解时间信息
 参数：(int)solut：CNF公式的解，有解为1，无解为0；
      (int)time:DPLL求解时间；
 返回值：ERROR：文件打开失败；
        OK：解和时间存储成功*/
status SaveValue(ArgueValue *ValueList,int solut,int time) {
	int i=0;
	FILE *save;
	char filenamesave[1000];
	for (i=0; filename[i]!='.'; i++)
		filenamesave[i]=filename[i];
	filenamesave[i++]='.';
	filenamesave[i++]='r';
	filenamesave[i++]='e';
	filenamesave[i++]='s';
	filenamesave[i++]='\0';
	save=fopen(filenamesave, "wb");
	if (save==NULL) {
		printf("文件打开失败！\n");
		return ERROR;
	}
	if(solut==1) {
		fprintf(save,"s %d\n", solut);
		fprintf(save,"v ");
		for (i=1; i<=r->litsize; i++) {
			if (ValueList[i].IsInit==1) {
				if (ValueList[i].Value==1) {
					fprintf(save, "%-7d",i);
				} else fprintf(save, "%-7d",0-i);
				fprintf(save, " ");
			}
			if(i%10==0) fprintf(save,"\n");
		}
		fprintf(save, "\nt %d ms\n", time);
	}
	else if(solut ==0){
		fprintf(save,"s %d\n", solut);
		fprintf(save,"v ");
		fprintf(save, "\nt %d ms\n", time);
	}
	fclose(save);
	return OK;
}

/*------------------------DPLL算法------------------------*/




status SAT(void) {
	int op=1;//记录选择的操作序号
	int choose;//记录选择的算法序号
	int i=0,solut=0;//solut记录CNF公式是否有解，有解为1，无解为0
	clock_t start = 0,finish = 0;//记录DPLL函数调用的起始和终止时间
	int duration=0;//记录SAT求解时间
	FILE *open;
	printf("请输入需求解的cnf文件名：");
	scanf("%s",filename);
	open=fopen(filename, "r");
	if (open==NULL) {
		printf("文件打开错误！\n");
		getchar();
		getchar();
		return ERROR;
	}
	if (CreateParadigm(&open)==ERROR) {//生成CNF公式数据存储结构
		getchar();
		getchar();
		return ERROR;
	}
    getchar(); getchar();
	op=1;
	solut=NO_RESULT;
	while (op) {
		system("cls");
		printf("\t\tChoose an option please.\n");
		printf("---------------------------------------------------------\n");
		printf("  1.求解该算例   2.遍历CNF文件   3.保存求解结果\n");
		printf("  0.退出\n");
		printf("---------------------------------------------------------\n");
		scanf("%d",&op);
		switch (op) {
			case 1:
				if (ValueList[1].IsInit==1) {
					printf("已对公式求解！");
					getchar();
					getchar();
					break;
				}
				printf(" 使用：1.优化1算法       2.优化2算法        3.优化前算法\n");
				scanf("%d",&choose);
				switch (choose) {
					case 1:
						start=clock();
						solut=DPLL3(FindLiteral1(r),1,1);
						finish=clock();
						duration=(finish-start);
						break;
					case 2:
						start=clock();
						solut=DPLL4(FindLiteral2(r),1,1);
						finish=clock();
						duration=(finish-start);
						break;
					case 3:
						start=clock();
						solut=DPLL_Before(r, 1);
						finish=clock();
						duration=(finish-start);
						break;
					default:
						printf("输入错误！\n");
						break;
				}
				if (solut) {
					AnswerComplete();
					for (i=1; i<=r->litsize; i++) {
						if (ValueList[i].IsInit==1) {
							if (ValueList[i].Value==1) {
								printf("%-7d",i);
							} else printf("%-7d",0-i);
						}
						if(i%10==0) printf("\n");
					}
				} else  printf("该公式无解！\n");
				printf("\n计算时间为：%ld ms\n",(finish-start));
				getchar();
				getchar();
				break;
			case 2:
				if (solut==NO_RESULT) {
					printf("请先求解CNF公式！\n");
					getchar();
					getchar();
					break;
				}
				ParadigmTrabverse(r);
				if (AnswerCheck(solut)==OK) {
					printf("公式遍历完成！\n");
				} else {
					printf("ERROR！\n");
				}
				getchar();
				getchar();
				break;
			case 3:
				if (solut==NO_RESULT) {
					printf("请先求解CNF公式！\n");
					getchar();
					getchar();
					break;
				}
				if (SaveValue(ValueList,solut,duration))
					printf("成功存入文件！");
				getchar();
				getchar();
				break;
			case 0:
				break;
			default:
				printf("输入错误！");
				getchar();
				getchar();
				break;
		}
	}
	DestroyParadigm(r);//销毁CNF公式数据存储结构
	return OK;
}


/*采用第一种变元选取策略的递归算法DPLL函数
 参数：(int)num：该次进入函数真值设为1的变元序号；
      (int)op：普通求解cnf文件为1，生成数独终盘为2；
      (int)timesofDPLL：第一次进入DPLL函数为1，其后为2；
 返回值：FALSE：公式无解；
       TRUE：公式有解*/
status DPLL1(int num,int op,int timesofDPLL) {
	int l,i;
	Clause *c;
	Paradigm *p=r->first;
	Paraline *pline;
	i=0;
	/*单子句策略*/
	if (timesofDPLL==1) {
		l=0;//第一次进入DPLL函数，不进入单子句循环
	} else {
		if (op==1)
			l=num;
		else l=0;
	}

	while (l!=0) {
		stack[++instacksize]=l;
		DeleteClause(r, l);
		if (DeleteLiteral(r, l)==FALSE) { //删除子句中文字l的反文字
			//如探测后公式无解，恢复递归进入本次函数前的邻接链表状态
			for (; stack[instacksize]!=num; instacksize--) {
				RecoverCNF(r, stack[instacksize]);
				ValueList[abs(stack[instacksize])].IsInit=0;
			}
			RecoverCNF(r, stack[instacksize]);
			ValueList[abs(stack[instacksize])].IsInit=0;
			instacksize--;
			return FALSE;
		}
		if (l>0) {
			ValueList[l].Value=1;//真值设置为1
			ValueList[l].IsInit=1;//标记已经赋值
		} else {
			ValueList[0-l].Value=0;//真值设置为0
			ValueList[0-l].IsInit=1;//标记已经赋值
		}
		if (FindLiteral1(r)==0)
			return TRUE;//公式中没有剩余的为赋真值的变元，求解成功
		l=HasUnitClause(r);//寻找公式中的单子句并将其文字值赋给l
	}
	if (op==1) {
		l=FindLiteral1(r);
	} else {
		do {
			l=rand()%729+1;
			for (pline=ValueList[l].Neg.Tra_cla; pline!=NULL; pline=pline->next) {
				p=pline->claline;
				if (p->number!=0)
					break;
			}
		} while (ValueList[l].IsInit==1);
		for (c=p->sentence; c!=NULL; c=c->nextl) {
			if (c->flag==0) {
				l=c->literal;
				break;
			}
		}
	}
	if (l==0)
		return TRUE;//公式中没有剩余的为赋真值的变元，求解成功
	if (DPLL1(l, 1,2)) {
		return TRUE;
	}

	l=0-l;//设l为真值为1时探测求解失败，故l真值只能为0，即其反文字真值为1
	if (DPLL1(l,1,2)==FALSE) {
		if (instacksize==0)
			return FALSE; //l已是本次DPLL1函数第一个进行探测的变元，探测失败
		/*恢复递归进入本次函数前的邻接链表状态*/
		for (; stack[instacksize]!=num; instacksize--) {
			RecoverCNF(r, stack[instacksize]);
			ValueList[abs(stack[instacksize])].IsInit=0;
		}
		RecoverCNF(r, stack[instacksize]);
		ValueList[abs(stack[instacksize])].IsInit=0;
		instacksize--;
		return FALSE;
	} else
		return TRUE;
}

/*采用第二种变元选取策略的递归算法DPLL函数
 参数：(int)num：该次进入函数真值设为1的变元序号；
      (int)op：普通求解cnf文件为1，生成数独终盘为2；
      (int)timesofDPLL：第一次进入DPLL函数为1，其后为2；
 返回值：FALSE：公式无解；
       TRUE：公式有解*/
status DPLL2(int num,int op,int timesofDPLL) {
	int l,i;
	Clause *c;
	Paradigm *p=r->first;
	Paraline *pline;
	i=0;
	/*单子句策略*/
	if (timesofDPLL==1) {
		l=0;
	} else {
		if (op==1)
			l=num;
		else l=0;
	}
	while (l!=0) {
		stack[++instacksize]=l;//记录循环过程中真值设为1的文字
		DeleteClause(r, l);//删除含文字l的子句
		if (DeleteLiteral(r, l)==FALSE) { //删除子句中文字l的反文字
			//如探测后公式无解，恢复递归进入本次函数前的邻接链表状态
			for (; stack[instacksize]!=num; instacksize--) {
				RecoverCNF(r, stack[instacksize]);
				ValueList[abs(stack[instacksize])].IsInit=0;
			}
			RecoverCNF(r, stack[instacksize]);
			ValueList[abs(stack[instacksize])].IsInit=0;
			instacksize--;
			return FALSE;
		}
		if (l>0) {
			ValueList[l].Value=1;
			ValueList[l].IsInit=1;
		} else {
			ValueList[0-l].Value=0;
			ValueList[0-l].IsInit=1;
		}
		if (FindLiteral2(r)==0)
			return TRUE;
		l=HasUnitClause(r);
	}
	if (op==1) {
		l=FindLiteral2(r);
	} else {
		do {
			l=rand()%729+1;
			for (pline=ValueList[l].Neg.Tra_cla; pline!=NULL; pline=pline->next) {
				p=pline->claline;
				if (p->number!=0)
					break;
			}
		} while (ValueList[l].IsInit==1);
		for (c=p->sentence; c!=NULL; c=c->nextl) {
			if (c->flag==0) {
				l=c->literal;
				break;
			}
		}
	}
	if (l==0)
		return TRUE;
	if (DPLL2(l, 1,2)) {
		return TRUE;
	}
	l=0-l;//设l为真值为1时探测求解失败，故l真值只能为0，即其反文字真值为1
	if (DPLL2(l,1,2)==FALSE) {
		if (instacksize==0)
			return FALSE;
		for (; stack[instacksize]!=num; instacksize--) {
			RecoverCNF(r, stack[instacksize]);
			ValueList[abs(stack[instacksize])].IsInit=0;
		}
		RecoverCNF(r, stack[instacksize]);
		ValueList[abs(stack[instacksize])].IsInit=0;
		instacksize--;
		return FALSE;
	} else
		return TRUE;
}

/*采用第三种变元选取策略的递归算法DPLL函数
 参数：(int)num：该次进入函数真值设为1的变元序号；
      (int)op：普通求解cnf文件为1，生成数独终盘为2；
      (int)timesofDPLL：第一次进入DPLL函数为1，其后为2；
 返回值：FALSE：公式无解；
       TRUE：公式有解*/
status DPLL3(int num,int op,int timesofDPLL) {
	int l,i;
	Clause *c;
	Paradigm *p=r->first;
	Paraline *pline;
	i=0;
	/*单子句策略*/
	if (timesofDPLL==1) {
		l=0;
	} else {
		if (op==1)
			l=num;
		else l=0;
	}
	while (l!=0) {
		stack[++instacksize]=l;//记录循环过程中真值设为1的文字
		DeleteClause(r, l);//删除含文字l的子句
		if (DeleteLiteral(r, l)==FALSE) { //删除子句中文字l的反文字
			//如探测后公式无解，恢复递归进入本次函数前的邻接链表状态
			for (; stack[instacksize]!=num; instacksize--) {
				RecoverCNF(r, stack[instacksize]);
				ValueList[abs(stack[instacksize])].IsInit=0;
			}
			RecoverCNF(r, stack[instacksize]);
			ValueList[abs(stack[instacksize])].IsInit=0;
			instacksize--;
			return FALSE;
		}
		if (l>0) {
			ValueList[l].Value=1;
			ValueList[l].IsInit=1;
		} else {
			ValueList[0-l].Value=0;
			ValueList[0-l].IsInit=1;
		}
		if (FindLiteral3(r)==0)
			return TRUE;
		l=HasUnitClause(r);
	}
	if (op==1) {
		l=FindLiteral3(r);//顺序选取没有技巧
	} 
	else {
		do {
			l=rand()%729+1;
			for (pline=ValueList[l].Neg.Tra_cla; pline!=NULL; pline=pline->next) {
				p=pline->claline;
				if (p->number!=0)
					break;
			}
		} while (ValueList[l].IsInit==1);
		for (c=p->sentence; c!=NULL; c=c->nextl) {
			if (c->flag==0) {
				l=c->literal;
				break;
			}
		}
	}
	if (l==0)
		return TRUE;
	if (DPLL3(l, 1,2)) {
		return TRUE;
	}
	l=0-l;//设l为真值为1时探测求解失败，故l真值只能为0，即其反文字真值为1
	if (DPLL3(l,1,2)==FALSE) {
		if (instacksize==0)
			return FALSE;
		for (; stack[instacksize]!=num; instacksize--) {
			RecoverCNF(r, stack[instacksize]);
			ValueList[abs(stack[instacksize])].IsInit=0;
		}
		RecoverCNF(r, stack[instacksize]);
		ValueList[abs(stack[instacksize])].IsInit=0;
		instacksize--;
		return FALSE;
	} else
		return TRUE;
}

/*采用第四种变元选取策略的递归算法DPLL函数
 参数：(int)num：该次进入函数真值设为1的变元序号；
      (int)op：普通求解cnf文件为1，生成数独终盘为2；
      (int)timesofDPLL：第一次进入DPLL函数为1，其后为2；
 返回值：FALSE：公式无解；
       TRUE：公式有解*/
status DPLL4(int num,int op,int timesofDPLL) {
	int l,i;
	Clause *c;
	Paradigm *p=r->first;
	Paraline *pline;
	i=0;
	/*单子句策略*/
	if (timesofDPLL==1) {
		l=0;
	} else {
		if (op==1)
			l=num;
		else l=0;
	}
	while (l!=0) {
		stack[++instacksize]=l;//记录循环过程中真值设为1的文字
		DeleteClause(r, l);//删除含文字l的子句
		if (DeleteLiteral(r, l)==FALSE) { //删除子句中文字l的反文字
			//如探测后公式无解，恢复递归进入本次函数前的邻接链表状态
			for (; stack[instacksize]!=num; instacksize--) {
				RecoverCNF(r, stack[instacksize]);
				ValueList[abs(stack[instacksize])].IsInit=0;
			}
			RecoverCNF(r, stack[instacksize]);
			ValueList[abs(stack[instacksize])].IsInit=0;
			instacksize--;
			return FALSE;
		}
		if (l>0) {
			ValueList[l].Value=1;
			ValueList[l].IsInit=1;
		} else {
			ValueList[0-l].Value=0;
			ValueList[0-l].IsInit=1;
		}
		if (FindLiteral4(r)==0)
			return TRUE;
		l=HasUnitClause(r);
	}
	if (op==1) {
		l=FindLiteral4(r);
	} else {
		do {
			l=rand()%729+1;
			for (pline=ValueList[l].Neg.Tra_cla; pline!=NULL; pline=pline->next) {
				p=pline->claline;
				if (p->number!=0)
					break;
			}
		} while (ValueList[l].IsInit==1);
		for (c=p->sentence; c!=NULL; c=c->nextl) {
			if (c->flag==0) {
				l=c->literal;
				break;
			}
		}
	}
	if (l==0)
		return TRUE;
	if (DPLL4(l, 1,2)) {
		return TRUE;
	}
	l=0-l;//设l为真值为1时探测求解失败，故l真值只能为0，即其反文字真值为1
	if (DPLL4(l,1,2)==FALSE) {
		if (instacksize==0)
			return FALSE;
		for (; stack[instacksize]!=num; instacksize--) {
			RecoverCNF(r, stack[instacksize]);
			ValueList[abs(stack[instacksize])].IsInit=0;
		}
		RecoverCNF(r, stack[instacksize]);
		ValueList[abs(stack[instacksize])].IsInit=0;
		instacksize--;
		return FALSE;
	} else
		return TRUE;
}


/*优化前版本DPLL函数
 参数：(int)op：普通求解cnf文件为1，生成数独终盘为2；
 返回值：FALSE：公式无解；
       TRUE：公式有解*/
status DPLL_Before(Root *r,int op) {
	int l,i;
	Clause *c;
	Paradigm *p=r->first;
	Paraline *pline;
	int *Memory;


	Memory=(int*)malloc((r->litsize)*sizeof(int));
	for (i=0; i<r->litsize; i++)
		Memory[i]=0;
	i=0;
	/*单子句策略*/
	if (op==1) {
		c=HasUnitClause_Before(r);//顺序查找单子句
	} else {
		c=NULL;
	}
	//单子句传播
	while (c!=NULL) {
		l=c->literal;
		Memory[i++]=l;//添加单子句的文字
		DeleteClause(r, l);//删除含文字l的子句
		if (DeleteLiteral(r, l)==FALSE) { //删除字句中文字l的负文字
			//如探测后公式无解，恢复递归进入本次函数前的邻接链表状态
			if (Memory[0]!=0)//栈不为空
				RemoveHeadClaus(r, Memory[0]);//删除增加的单子句
			for (i=0; Memory[i]!=0; i++) {
				RecoverCNF(r, Memory[i]);
				ValueList[abs(Memory[i])].IsInit=0;
			}
			return FALSE;
		}

		/*在变元表中记录变元l的真值*/
		if (l>0) {
			ValueList[l].Value=1;
			ValueList[l].IsInit=1;
		} else {
			ValueList[0-l].Value=0;
			ValueList[0-l].IsInit=1;
		}
		if (FindLiteral_Before(r)==NULL)
			return TRUE;//公式中没有剩余的未赋真值的变元，求解成功
		c=HasUnitClause_Before(r);//寻找公式中的单子句并将其文字值赋给l
	}

	Memory[i]=0;//相当于设置line1327的条件
	if (op==1) {
		c=FindLiteral_Before(r);//顺序找到还存在的文字指针
	} else {//?
		do {
			l=rand()%729+1;
			for (pline=ValueList[l].Neg.Tra_cla; pline!=NULL; pline=pline->next) {
				p=pline->claline;
				if (p->number!=0)
					break;
			}
		} while (ValueList[l].IsInit==1);
		for (c=p->sentence; c!=NULL; c=c->nextl) {
			if (c->flag==0)
				break;
		}
	}
	if (c==NULL)//全被赋值了，一个子句也没有
		return TRUE;
	l=c->literal;
	AddClause(r, l);//设文字l真值为1，在CNF范式邻接链表表头增加含文字l的单子句链表
	if (DPLL_Before(r, 1)) {
		return TRUE;
	}

	l=0-l;//设l为真值为1时探测求解失败，故l真值只能为0，即其反文字真值为1
	AddClause(r, l);
	if (DPLL_Before(r,1)==FALSE) {

		if (Memory[0]!=0)
			RemoveHeadClaus(r, Memory[0]);
		for (i=0; Memory[i]!=0; i++) {
			RecoverCNF(r, Memory[i]);
			ValueList[abs(Memory[i])].IsInit=0;
		}
		return FALSE;
	} else
		return TRUE;
}

/*将未赋真值的变元（求解过程中化简舍去）赋真值1*/
void AnswerComplete(void) {
	int i;
	for (i=1; i<=r->litsize; i++) {
		if (ValueList[i].IsInit==0) {
			ValueList[i].IsInit=1;
			ValueList[i].Value=1;
		}
	}
}

/*检查SAT求解结果正确性
 参数：(int)solut：公式求解结果，有解为1，无解为0；
 返回值：TRUE：求解正确；
        FALSE：求解错误*/
status AnswerCheck(int solut) {
	Paradigm *p;
	Clause *c;
	int flag,l,value,i=0;
	if (solut==1) {//公式有解
		for (p=r->first; p!=NULL; p=p->nextc) {
			i++;
			flag=0;
			for (c=p->sentence; c!=NULL; c=c->nextl) {
				l=abs(c->literal);
				if (c->literal>0) {
					value=ValueList[l].Value;
				} else value=1-ValueList[l].Value;
				if (value==1) {
					flag=1;//子句中有文字真值为1，子句真值为1
					break;
				}
			}
			if (flag==0) {

				return FALSE;//子句中无真值为1的文字，子句真值为0，求解错误
			}
		}
		return TRUE;
	} else {//公式无解
		for (p=r->first; p!=NULL; p=p->nextc) {
			flag=0;
			for (c=p->sentence; c!=NULL; c=c->nextl) {
				l=abs(c->literal);
				if (c->literal>0) {
					value=ValueList[l].Value;
				} else value=1-ValueList[l].Value;
				if (value==1) flag=1;//子句中有文字真值为1，子句真值为1
			}
			if (flag==0) return TRUE;//子句真值为0，求解正确
		}
		return FALSE;
	}
}


/*------------------------SuDoKu------------------------*/

status Sudoku(void) {
	int op=1,difficulty=0,i,j,flag=1,choose=1;//op和choose记录用户选择的操作，difficulty记录用户选择的数独问题难度
	int solut=0;
	char c;
	int x[3];
	srand((unsigned)time(NULL));
	while (op) {
		system("cls");
		printf("\t\tChoose an option please.\n");
		printf("---------------------------------------------------------\n");
		printf("  1.数独游戏         2.求解一个数独文件         0.退出\n");
		printf("---------------------------------------------------------\n");
		scanf("%d",&op);
		switch (op) {
			case 1:
				solut=CreateSudoku();//生成基础数独终盘对应变元真值表
				if (solut) {
					CNFSudokuTableTransform();//将求解的变元真值表转换为二维数组数独终盘
					printf("请选择数独难度：\n1.easy\t\t2.medium\t\t3.Hard\n");
					scanf("%d",&difficulty);
					//SudokuTablePrint();//测试用句，先输出终盘答案
					printf("Loading...");
					/*根据用户选择数独问题难度对数独终盘随机挖相应数量的洞生成问题*/
					switch (difficulty) {
						case 1:
							DigHole(30);
							break;
						case 2:
							DigHole(50);
							break;
						case 3:
							DigHole(64);
							break;
						default:
							printf("输入出错！\n");
							break;
					}
					choose=1;
					while (choose) {
						flag=1;//flag标记用户求解答案正确与否，正确为1，错误为0
						printf("按“行列值”的顺序，依次输入你的答案（例：“436”代表第四行第三列的空格内填入6），每输入一个答案用回车键断开，输入完成后请输入空格并按回车继续：\n");
						getchar();
						c=getchar();
						while (c!='\n') {
							i=0;
							while (c!=' ') {
								x[i++]=c-'0';
								c=getchar();
							}
							users_sudoku[x[0]][x[1]]=x[2];//记录用户输入答案
							c=getchar();
						}
						for (i=0; i<9; i++) {
							for (j=0; j<9; j++)
								//对比用户解答和数独答案，判断其求解正确性
								if (sudoku_table[i][j]!=users_sudoku[i][j]) {
									printf("答案错误！\n");
									flag=0;
									break;
								}
							if (flag==0) {
								break;
							}
						}
						if (flag) {
							printf("恭喜！答案正确！\n");
							SudokuTablePrint();//输出数独终盘
							choose=0;
						} else {
							printf("是否查看答案？0.是  1.否\n");
							scanf("%d",&choose);
							switch (choose) {
								case 1:
									break;
								case 0:
									SudokuTablePrint();
									break;
								default:
									printf("输入错误！\n");
									break;
							}
						}
					}
				} else  printf("生成失败！\n");
				getchar();
				getchar();
				break;
			case 2:
				SolveSudoku();
				getchar();
				getchar();
				break;
			case 0:
				break;
			default:
				printf("输入错误！");
				getchar();
				getchar();
				break;
		}
	}

	return OK;
}

/*创建数独问题转化为SAT问题后的cnf文件*/
FILE * CreateSudokuFile(void) {
	int x,y,z,i,j,k,l;//x代表数独的行，y代表数独的列，z取1～9分别代表该格中数独填入值为1～9中任一值
	FILE *fp;
	fp=fopen("SudokuTableBase.cnf", "wb");
	if (fp==NULL) {
		printf("文件打开失败！\n");
		return NULL;
	}
	fprintf(fp, "p cnf 729 10287\n");//共有729个变元，9*9个数独空格每个格对应9个变元，填入1～9中某一值则对应变元为真，其他为假
	for (x=0; x<9; x++) {
		for (y=0; y<9; y++)
			for (z=1; z<9; z++)
				for (i=z+1; i<=9; i++)
					fprintf(fp, "%d %d 0\n",-(81*x+9*y+z),-(81*x+9*y+i));
	}                        //每个cell只能取1～9的一个值
	for (x=0; x<9; x++) {
		for (z=1; z<=9; z++)
			for (y=0; y<8; y++)
				for (i=y+1; i<=8; i++)
					fprintf(fp, "%d %d 0\n",-(81*x+9*y+z),-(81*x+9*i+z));
	}                        //每行1～9只能各出现一次
	for (y=0; y<9; y++) {
		for (z=1; z<=9; z++)
			for (x=0; x<8; x++)
				for (i=x+1; i<=8; i++)
					fprintf(fp, "%d %d 0\n",-(81*x+9*y+z),-(81*i+9*y+z));
	}                        //每列1～9只能各出现一次
	for (i=0; i<3; i++) {
		for (j=0; j<3; j++)
			for (z=1; z<=9; z++)
				for (x=0; x<3; x++)
					for (y=0; y<3; y++)
						for (k=x+1; k<3; k++)
							for (l=0; l<3; l++)
								if (y!=l)
									fprintf(fp, "%d %d 0\n",-(81*(3*i+x)+9*(3*j+y)+z),-(81*(3*i+k)+9*(3*j+l)+z));
	}                        //每个region中1～9只能各出现一次
	for (x=0; x<9; x++) {
		for (y=0; y<9; y++) {
			for (z=1; z<=9; z++)
				fprintf(fp, "%d ",81*x+9*y+z);
			fprintf(fp, "0\n");
		}
	}                        //每个cell必须取1～9中的一个值
	// 添加对主对角线的约束
for (z = 1; z <= 9; z++) {
    for (x = 0; x < 8; x++) {
        for (i = x + 1; i < 9; i++) {
            fprintf(fp, "%d %d 0\n", -(81 * x + 9 * x + z), -(81 * i + 9 * i + z));
        }
    }
}

// 添加对副对角线的约束
for (z = 1; z <= 9; z++) {
    for (x = 0; x < 8; x++) {
        for (i = x + 1; i < 9; i++) {
            fprintf(fp, "%d %d 0\n", -(81 * x + 9 * (8 - x) + z), -(81 * i + 9 * (8 - i) + z));
        }
    }
}

	fclose(fp);
	fp=fopen("SudokuTableBase.cnf", "r");
	return fp;
}

/*创建新的随机9*9数独终盘*/
status CreateSudoku(void) {
	int x,y,z,i,d;
	int order[82],randnum;
	FILE *fp;
	for (x=0; x<9; x++)
		for (y=0; y<9; y++)
			sudoku_table[x][y]=0;
	do {
		fp=CreateSudokuFile();
		if (fp==NULL) {
			printf("文件打开失败！\n");
			return ERROR;
		}
		CreateParadigm(&fp);//创建数独文件对应CNF范式邻接链表

		/*对1～81的数字进行随机排序*/
		for (i=0; i<=81; i++)
			order[i]=i;
		for (i=81; i>=1; i--) {
			randnum=rand()%81+1;//生成1到81的随机数
			if (i!=randnum) {//随机与一个数交换
				d=order[i];
				order[i]=order[randnum];
				order[randnum]=d;
			}
		}

		for (i=1; i<=10; i++) {                 //在棋盘中随机选10个格子随机填入1~9
			x=(order[i]-1)/9;//顺序为i的cell在棋盘的行数
			y=(order[i]-1)%9;//该cell在棋盘的列数
			z=rand()%9+1;//1～9的随机数

			/*将已经确定的cell的值记入变元真值表中*/
			for (d=1; d<=9; d++) {
				ValueList[81*x+9*y+d].IsInit=1;
				if (d==z) {
					ValueList[81*x+9*y+d].Value=1;
					DeleteClause(r, (81*x+9*y+d));
					DeleteLiteral(r, (81*x+9*y+d));
				} else {
					ValueList[81*x+9*y+d].Value=0;
					DeleteClause(r, -(81*x+9*y+d));
					DeleteLiteral(r, -(81*x+9*y+d));
				}
			}

		}
	} while (DPLL2(FindLiteral2(r),2,1)==FALSE);//进入SAT求解器求解，直到得到数独终盘
	return OK;
}

/*求解数独cnf文件并转化为数独终盘形式输出*/
status SolveSudoku(void) {
	FILE *fp,*open;
	char filename[100];
	char read;
	int i,j,k,x,y,z,d,cham;
	int exitnum[10];
	int table[82];
	printf("请输入存储数独的文件名：");
	scanf("%s",filename);
	fp=fopen(filename, "r");
	if (fp==NULL) {
		printf("文件打开错误！\n");
		return ERROR;
	}

	/*读取数独文件中的数独问题盘*/
	for (i=0; i<9; i++) {
		j=0;
		fscanf(fp, "%c",&read);
		while (read!='\n') {
			if (read=='.') {
				sudoku_table[i][j]=0;
			} else if (read<='9'&&read>='1') {
				sudoku_table[i][j]=(int)(read-'0');
			} else j=-1;
			j++;
			fscanf(fp, "%c",&read);
		}
	}
	fclose(fp);
	open=SetCNFfile();
	if (open==NULL) {
		printf("文件打开失败！\n");
		return ERROR;
	}
	CreateParadigm(&open);
	i=0;
	DPLL_Before(r,1);//求解数独文件
	for (k=1; k<=r->litsize; k++) {
		if (ValueList[k].Value==1)
			table[i++]=(k%9==0)?9:k%9;
	}//将变元表真值信息转换为二维独信息*/
	k=0;
	for(i=0; i<9; i++)
		for(j=0; j<9; j++)
			sudoku_table[i][j]=table[k++];
	printf("数独的解为：\n");
	SudokuTablePrint();//输出数独终盘
	return OK;
}
//将数独txt文件转化为cnf
FILE * SetCNFfile(void) {
	int i,j,x,y,z,k,l;
	printf("输入要转化为CNF文件的文件名：\n");
	scanf("%s", filename);
	FILE *fp;
	fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("文件打开失败！\n");
		return NULL;
	}
	int f_sum = 0;//公式数计数器
	for (i = 0; i < 9; i++) {
		for(j=0; j<9; j++)
			if (sudoku_table[i][j] != 0) f_sum++;
	}
	fprintf(fp,"p cnf 729 %d\n", f_sum + 10287);

	for (i = 0; i < 9; i++) {
		for(j=0; j<9; j++)
			if (sudoku_table[i][j] != 0)
				fprintf(fp, "%d 0\n", 81*i+9*j+sudoku_table[i][j]);
	}

	for (x=0; x<9; x++) {
		for (y=0; y<9; y++)
			for (z=1; z<9; z++)
				for (i=z+1; i<=9; i++)
					fprintf(fp, "%d %d 0\n",-(81*x+9*y+z),-(81*x+9*y+i));
	}                        //每个cell只能取1～9的一个值
	for (x=0; x<9; x++) {
		for (z=1; z<=9; z++)
			for (y=0; y<8; y++)
				for (i=y+1; i<=8; i++)
					fprintf(fp, "%d %d 0\n",-(81*x+9*y+z),-(81*x+9*i+z));
	}                        //每行1～9只能各出现一次
	for (y=0; y<9; y++) {
		for (z=1; z<=9; z++)
			for (x=0; x<8; x++)
				for (i=x+1; i<=8; i++)
					fprintf(fp, "%d %d 0\n",-(81*x+9*y+z),-(81*i+9*y+z));
	}                        //每列1～9只能各出现一次
	for (i=0; i<3; i++) {
		for (j=0; j<3; j++)
			for (z=1; z<=9; z++)
				for (x=0; x<3; x++)
					for (y=0; y<3; y++)
						for (k=x+1; k<3; k++)
							for (l=0; l<3; l++)
								if (y!=l)
									fprintf(fp, "%d %d 0\n",-(81*(3*i+x)+9*(3*j+y)+z),-(81*(3*i+k)+9*(3*j+l)+z));
	}                        //每个region中1～9只能各出现一次
	for (x=0; x<9; x++) {
		for (y=0; y<9; y++) {
			for (z=1; z<=9; z++)
				fprintf(fp, "%d ",81*x+9*y+z);
			fprintf(fp, "0\n");
		}
	}                        //每个cell必须取1～9中的一个值
	fclose(fp);
	fp=fopen(filename, "r");
	return fp;
}

/*对已知数独终盘挖num个洞生成有唯一解的数独问题
参数：(int)num：挖洞的数目*/
status DigHole(int num) {
	int x,y,z,i,j,d;
	int rank,origin,result=0,circle;
	int table_diged[9][9];
	int diged_cells[82];
	FILE *fp;

	for (i=0; i<=81; i++)
		diged_cells[i]=0;
	fp=CreateSudokuFile();
	if (fp==NULL) {
		printf("文件打开失败！\n");
		return ERROR;
	}
	CreateParadigm(&fp);

	/*将数独信息填入对应变元真值表中，并对CNF范式邻接链表作相应处理*/
	for (i=0; i<9; i++)
		for (j=0; j<9; j++) {
			d=sudoku_table[i][j];
			if (d!=0)
				for (z=1; z<=9; z++) {
					ValueList[81*i+9*j+z].IsInit=1;
					if (d==z)
						ValueList[81*i+9*j+z].Value=1;
					else
						ValueList[81*i+9*j+z].Value=0;
				}
		}
	for (i=1; i<=r->litsize; i++) {
		if (ValueList[i].Value==1) {
			DeleteClause(r, i);
			DeleteLiteral(r, i);
		} else {
			DeleteClause(r, 0-i);
			DeleteLiteral(r, 0-i);
		}
	}

	/*复制sudoku_table的值至数组table_diged*/
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			table_diged[i][j]=sudoku_table[i][j];

	/*挖第一个洞*/
	rank=rand()%81+1;
	diged_cells[rank]=1;
	x=(rank-1)/9;//顺序为rank的cell在棋盘的行数
	y=(rank-1)%9;//该cell在棋盘的列数
	origin=sudoku_table[x][y];
	table_diged[x][y]=0;//记录挖洞的位置

	/*将CNF公式邻接链表恢复至未知第一个洞位置的填入值，即其对应变元真值时的状态*/
	for (z=1; z<=9; z++) {
		if (z==origin)
			RecoverCNF(r, 81*x+9*y+z);
		else RecoverCNF(r, -(81*x+9*y+z));
		ValueList[81*x+9*y+z].IsInit=0;
		ValueList[81*x+9*y+z].Value=0;
	}
	DestroyParadigm(r);

	for (circle=1; circle<num; circle++) {
		/*每次循环重新建立当前数独信息对应CNF公式邻接链表及变元表并对变元表填入相应信息*/
		fp=CreateSudokuFile();
		if (fp==NULL) {
			printf("文件打开失败！\n");
			return ERROR;
		}
		CreateParadigm(&fp);
		for (i=0; i<9; i++)
			for (j=0; j<9; j++) {
				d=table_diged[i][j];
				if (d!=0)
					for (z=1; z<=9; z++) {
						ValueList[81*i+9*j+z].IsInit=1;
						if (d==z)
							ValueList[81*i+9*j+z].Value=1;
						else
							ValueList[81*i+9*j+z].Value=0;
					}
			}
		for (i=1; i<=r->litsize; i++) {
			if (ValueList[i].Value==1) {
				DeleteClause(r, i);
				DeleteLiteral(r, i);
			} else {
				DeleteClause(r, 0-i);
				DeleteLiteral(r, 0-i);
			}
		}

		/*挖下一个洞*/
		rank=rand()%81+1;
		x=(rank-1)/9;//顺序为rank的cell在棋盘的行数
		y=(rank-1)%9;//该cell在棋盘的列数
		if (diged_cells[rank]==1) {
			circle--;//该洞已经被挖过
			continue;
		}
		diged_cells[rank]=1;//记录挖洞的位置
		origin=sudoku_table[x][y];
		table_diged[x][y]=0;

		/*对挖洞位置填入除原终盘数值外的8个数值，依次用SAT求解器进行求解，如有解则说明挖洞后解不唯一，该洞不可挖*/
		for (z=1; z<=9; z++) {
			result=0;
			if (z!=origin) {
				/*处理CNF范式链表*/
				for (d=1; d<=9; d++) {
					ValueList[81*x+9*y+d].IsInit=1;
					if (d==z) {
						ValueList[81*x+9*y+d].Value=1;
						DeleteClause(r, 81*x+9*y+d);
						DeleteLiteral(r, 81*x+9*y+d);
					} else {
						ValueList[81*x+9*y+d].Value=0;
						DeleteClause(r, -(81*x+9*y+d));
						DeleteLiteral(r, -(81*x+9*y+d));
					}
				}
				DPLL1(FindLiteral1(r), 1,1);
				SudokuComplete();
				result=AnswerCheck(1);
				if (result==1) {
					table_diged[x][y]=origin;
					break;
				} else {
					for (d=1; d<=9; d++) {
						if (d==z)
							RecoverCNF(r, 81*x+9*y+d);
						else RecoverCNF(r, -(81*x+9*y+d));
						ValueList[81*x+9*y+d].IsInit=0;
						ValueList[81*x+9*y+d].Value=0;
					}
				}
			}
		}
		if (result==1) {
			circle--;//本次循环挖洞不成功
			continue;
		}
		DestroyParadigm(r);
	}

	printf("Complete!\n");
	/*输出生成的数独题*/
	printf("   0   1   2   3   4   5   6   7   8  \n");
	printf(" +---+---+---+---+---+---+---+---+---+\n");
	for (i=0; i<9; i++) {
		printf("%d",i);
		for (j=0; j<9; j++) {
			if (j==0||j==3||j==6) {
				printf("|");
			} else printf(" ");
			if (table_diged[i][j]!=0)
				printf(" %d ",table_diged[i][j]);
			else
				printf("   ");
		}
		printf("|\n");
		if (i==2||i==5||i==8) {
			printf(" +---+---+---+---+---+---+---+---+---+\n");
		} else printf(" |           |           |           |\n");
	}
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			users_sudoku[i][j]=table_diged[i][j];/*复制table_diged的值至数组users_sudoku*/
	return OK;
}

/*完善数独终盘*/
status SudokuComplete(void) {
	int i,j,z,y,x,d;
	int complete_table[9][9];
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			complete_table[i][j]=0;
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			for (z=1; z<=9; z++)
				if (ValueList[81*i+9*j+z].Value==1)
					complete_table[i][j]=z;

	for (i=0; i<9; i++) {
		for (j=0; j<9; j++) {
			if (complete_table[i][j]==0) {
				complete_table[i][j]=1;
				for (d=0; d<9; d++)
					for (y=0; y<9; y++)//两重循环扫了9遍保证横向一定没有重复
						if (y!=j)
							if (complete_table[i][j]==complete_table[i][y])
								complete_table[i][j]++;
				for (d=0; d<9; d++)
					for (x=0; x<9; x++)//两重循环扫了9遍保证竖向一定没有重复
						if (x!=i)
							if (complete_table[x][j]==complete_table[i][j])
								complete_table[i][j]++;
			}
		}
	}
	for (i=0; i<9; i++)
		for (j=0; j<9; j++) {
			d=complete_table[i][j];
			if (d!=0)
				for (z=1; z<=9; z++) {
					ValueList[81*i+9*j+z].IsInit=1;
					if (d==z)
						ValueList[81*i+9*j+z].Value=1;
					else
						ValueList[81*i+9*j+z].Value=0;
				}
		}
	return OK;
}

/*数独对应SAT变元表转化为二维数组*/
status CNFSudokuTableTransform(void) {
	int i,j,z;
	for (i=0; i<9; i++) {
		for (j=0; j<9; j++) {
			for (z=1; z<=9; z++) {
				if (ValueList[81*i+9*j+z].Value==1) {
					sudoku_table[i][j]=z;
				}
			}
		}
	}
	return OK;
}

/*输出数独盘*/
status SudokuTablePrint(void) {
	int i,j;
	printf("   0   1   2   3   4   5   6   7   8  \n");//列标号
	printf(" +---+---+---+---+---+---+---+---+---+\n");
	for (i=0; i<9; i++) {
		printf("%d",i);//行标号
		for (j=0; j<9; j++) {
			if (j==0||j==3||j==6) {
				printf("|");//region间间隔线
			} else printf(" ");
			printf(" %d ",sudoku_table[i][j]);
		}
		printf("|\n");
		if (i==2||i==5||i==8) {
			printf(" +---+---+---+---+---+---+---+---+---+\n");//region间间隔线
		} else printf(" |           |           |           |\n"); //region间间隔线
	}
	printf("打印完成！\n");
	return OK;
}

