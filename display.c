#include"define.h"

/*-------------------主函数-------------------*/

int main() {
	char filename[100],copy_filename[100],sudo_filename[100];//filename,sudu_filename用于储存文件名，copy_filename用于保存
	char filename_setcnf[100];
	int flag_cnf = 0,flag_sudo=0;//用于判断cnf文件是否读取成功
	int op=1,SAT=1,Sudoku=1;//op用于选择是读取cnf文件，还是Sudoku
	while(op) {
		system("cls");
		printf("\n---Menu For Puzzle Or SAT-----\n");
		printf("1.SAT        2.BinaryPuzzle\n");
		printf("0.Exit\n");
		printf("------------------------------\n");
		printf("请输入0~2：\n");
		printf("----------\n");
		scanf("%d",&op);
		switch(op) {
			case 1:
				while(SAT) {
					system("cls");
					printf("\n--------Menu For SAT--------------\n");
					printf("1.Readfile		    2.Savefile\n");
					printf("3.Traversefile		4.Dpll\n");
					printf("0.Exit\n");
					printf("----------------------------------\n");
					printf("请输入0~4：\n");
					printf("----------\n");
					scanf("%d",&SAT);
					switch(SAT) {
						case 1:
							printf("请输入读取的cnf文件名：\n");
							scanf("%s", filename);
							strcpy(copy_filename, filename);
							struct Clause * root = Readfile(filename);
							if (root != NULL) {
								printf("文件读取成功！\n");
								flag_cnf = 1;
							} else printf("文件读取失败，请检查文件名！\n");
							getchar();
							getchar();
							break;
						case 2:
							if(!flag_cnf) printf("文件尚未读取，请读取后操作！\n");
							else  {
								if (Savefile(root)) {
									printf("文件保存成功！\n");
								} else printf("文件保存失败！\n");
							}
							getchar();
							getchar();
							break;
						case 3:
							if (flag_cnf) Traversefile(root);
							else printf("文件尚未读取，请读取后操作！\n");
							getchar();
							getchar();
							break;
						case 4:
							if(flag_cnf) {
								printf("请选择优化方式：\n");
								printf("1.优化前	2.优化后\n");
								scanf("%d",&WAY);
								clock_t start,end;//用于记录运行时间time.h
								start=clock();
								if(dpll(root)==OK) {
									end=clock();
									printf("该文件有解！\n");
									printf("求解时间：%d ms！\n",end-start);
									for(i=1; i<=wordsum; i++) {
										temp=(ValueList[i].value!=0?1:-1)*i;
										printf("%d ",temp);
										if(i%10==0) printf("\n");
									}
									printf("\n");
									writeResult(copy_filename, 1, end-start);
								} else {
									end=clock();
									printf("该文件无解！\n");
									printf("求解时间：%d ms！\n",end-start);
									writeResult(copy_filename, 0, end - start);
									printf("\n");
								}
								flag_cnf = 0;
							} else printf("文件未读取！\n");
							getchar();
							getchar();
							break;
						case 0:
							break;
					}
				}
				getchar();
				getchar();
				break;
			case 2:
				srand((unsigned)time(NULL));
				while(Sudoku) {
					system("cls");
					printf("\n--------Menu For Sudoku--------------\n");
					printf("1.ReadSudoku 	    2.Play\n");
					printf("3.SetCNFfile		4.Dpll For Answer Check\n");
					printf("0.Exit\n");
					printf("----------------------------------\n");
					printf("请输入0~4：\n");
					printf("----------\n");
					scanf("%d",&Sudoku);
					switch(Sudoku) {
						case 1:
							printf("请输入要读取的数独棋盘文件名：\n");
							scanf("%s", sudo_filename);
							Readqipan(sudo_filename);
							SudokuTablePrint();
							flag_sudo = 1;
							getchar();
							getchar();
							break;
						case 2:
							break;
						case 3:
							if (flag_sudo) {
								printf("请选择：\n1.直接求解！	2.检验Play后的结果！\n");
								scanf("%d", &way_sudo);
								if (way_sudo == 1) {
									SetCNFfile(qipan);
								} else {
									if (Checkqipan(qipan)) SetCNFfile(qipan);
									else printf("棋盘还未填充完整，请填充完毕！\n");
								}
							} else printf("数独文件还未读取！\n");
							getchar();
							getchar();
							break;

					}
				}
				getchar();
				getchar();
				break;
			case 0:
				break;
		}
	}

}

/*---------------------------定义函数-------------------------*/

/*------------------------CNF文件处理------------------------*/

//新建文字
struct Literal *CreateLiteral() {
	struct Literal * new_literal = malloc(sizeof(struct Literal));
	new_literal->next=NULL;
	new_literal->value=0;
	return new_literal;
}

//新建子句
struct Clause * CreateClause() {
	struct Clause * new_clause = malloc(sizeof(struct Clause));
	new_clause->next=NULL;
	new_clause->head=NULL;
	return new_clause;
}

//读入cnt文件
struct Clause * Readfile(char * filename) {
	FILE * fp;
	fp = fopen(filename,"r");
	if(fp == NULL) {
		printf("文件打开失败!\n");
		return NULL;
	}
	char s[100];
	char *c;
	struct Clause * root = NULL, * current_Clause = NULL, * pre_Clause = NULL;
	struct Literal * current_Literal = NULL, * pre_Literal = NULL;
	while(fgets(s, 100, fp)) { //按行读取
		if(s[0] == 'c') continue;
		else if(s[0] == 'p') {
			sscanf(s, "p cnf %d %d", &wordsum, &forusum);//按格式读入文字数与公式数
			printf("文字数: %d	公式数: %d\n", wordsum, forusum);
			ValueList=(struct ValueList *)calloc(wordsum + 1, sizeof(struct ValueList));
			for(i = 1; i <= wordsum; i++) ValueList[i].value = -1;//初始化数组，-1表示未确定真值
		} else {
			//构建存储结构
			current_Clause = CreateClause();
			if(root == NULL) root = current_Clause;
			if(pre_Clause != NULL)  pre_Clause->next = current_Clause;
			c = strtok(s, (const char *) " ");//分割字符串
			while(c != NULL) {
				current_Literal = CreateLiteral();
				temp = atoi(c);
				current_Literal->value = temp;
				if(temp != 0) {
					if(current_Clause->head == NULL) current_Clause->head = current_Literal;
					if(pre_Literal != NULL) pre_Literal->next = current_Literal;
				}
				pre_Literal = current_Literal;
				c = strtok(NULL, (const char *) " ");
			}
			pre_Clause = current_Clause;
		}
	}
	fclose(fp);
	return root;
}

//遍历输出cnf文件
void Traversefile(struct Clause * root) {
	struct Clause* Temp = root;
	i=0;
	while(Temp) {
		struct Literal *temp=Temp->head;
		i++;
		printf("第%d个子句为： ",i);
		while(temp) {
			printf("%d ",temp->value);
			temp=temp->next;
		}
		if(Temp->next!=NULL) printf("\n");
		Temp=Temp->next;
	}
	printf("\n");
	printf("文件遍历完成！\n");
}

//保存cnt文件
status Savefile(struct Clause * root) {
	FILE *fp;
	char filename[100];
	printf("请输入想要保存的文件名：");
	scanf("%s",filename);
	if((fp=fopen(filename,"wb"))==NULL) {
		printf("文件打开失败！\n");
		return ERROR;
	} else {
		fprintf(fp, "%s ", "p cnf");
		fprintf(fp, "%d ", wordsum);
		fprintf(fp, "%d\n", forusum);//打印在文件里
		struct Clause *T=root;
		while(T) {
			struct Literal *temp=T->head;
			while(temp) {
				fprintf(fp, "%d ", temp->value);
				temp=temp->next;
			}
			if(T->next!=NULL) fprintf(fp, "0\n");
			T=T->next;
		}
	}
	fclose(fp);
	return OK;
}

/*------------------------DPLL算法--------------------------*/

//判断正负
status judgesign(int a) {
	return (a>0)?1:-1;
}

//判断并寻找单子句
status judgesimple(struct Clause *root) {
	struct Clause *T=root;
	while(T) {
		if(T->head==NULL) {
			T=T->next;
			continue;
		}
		if(T->head->next==NULL) return T->head->value;
		T=T->next;
	}
	return ERROR;//没有单子句
}

//处理单子句（单子句规则）
status solvesimple(struct Clause *root) {
	temp=judgesimple(root);
	if(temp == 0) return ERROR; //表示没有单子句
	ValueList[abs(temp)].value = temp > 0 ? 1 : 0;
	struct Clause * T = root, *pre_T = CreateClause();
	while(T != NULL) {
		struct Literal * current_Literal = T->head, * pre_Literal = CreateLiteral();
		while(current_Literal != NULL) {
			if(current_Literal->value == temp) { //删除包含该文字的子句
				if(T == root) {
					*(root) = *(root->next);
					T = NULL;
				} else {
					pre_T->next = T->next;
					T = pre_T;
				}
				break;
			} else if (current_Literal->value == -temp) { //删除互为相反数的文字
				if(current_Literal == T->head) T->head = current_Literal->next;
				else pre_Literal->next = current_Literal->next;
				current_Literal = pre_Literal;
				continue;
			}
			pre_Literal = current_Literal;
			current_Literal = current_Literal->next;
		}
		pre_T = T;
		T = (T == NULL) ? root : T->next;
	}
	return OK;
};

//选取假设真值文字
status chooseLiteral(struct Clause *root) {
	return root->head->value;
}

//判断是否存在空子句
status judgeEmptyClause(struct Clause * root) {
	struct Clause * T = root;
	while(T != NULL) {
		if(T->head == NULL) return OK;
		T = T->next;
	}
	return ERROR;
}

//判断是否为一种形式出现的文字
status judgePureLiteral(struct Clause *root) {
	int *mark=(int *)calloc(wordsum+1,sizeof(int));//标记数组,并置为0
	struct Clause *T=root;
	while(T) {
		struct Literal *current_literal=T->head;
		while(current_literal) {
			if(mark[abs(current_literal->value)]!=2) {
				temp=mark[abs(current_literal->value)];
				t=judgesign(current_literal->value);
				if(temp==0) mark[abs(current_literal->value)]=t;
				else if(temp==1&&t==-1) mark[abs(current_literal->value)]=2;
				else if(temp==-1&&t==1) mark[abs(current_literal->value)]=2;
				//两种形式，故置为2;
			}
			current_literal=current_literal->next;
		}
		T=T->next;
	}
	for(i=1; i<=wordsum; i++) {
		if(mark[i]==1||mark[i]==-1)
			return i*mark[i]; //返回只有一种形式的文字的值
	}
	return 0;//不存在只有一种形式的文字
}

//处理一种形式的文字
status solvepureliteral(struct Clause *root) {
	temp=judgePureLiteral(root);//找到只有一个形式的文字；
	if(temp==0) return ERROR;//表示没有找到
	ValueList[abs(temp)].value=(temp>0)?1:0;//-1为未确定真值，1为正真值（可填入），0为负真值（不可填入）
	struct Clause *T=root,*pre_T=CreateClause();
	while(T) {
		struct Literal *current_literal=T->head,*pre_literal=CreateLiteral();
		while(current_literal) {
			if(current_literal->value==temp) {//删除包含该文字的子句
				if(T==root) {
					*(root)=*(root->next);
					T=NULL;
				} else {
					pre_T->next=T->next;
					//free(T);
					T=pre_T;
				}
				break;
			} else if(current_literal->value==-temp) { //删除互为相反数的文字
				if(current_literal==T->head) {
					T->head=current_literal->next;
					//free(current_literal);
				} else {
					pre_literal->next=current_literal->next;
					//free(current_literal);
				}
				pre_literal=current_literal;
				current_literal=current_literal->next;
			}
			current_literal=current_literal->next;
		}
		pre_T=T;
		T=(T==NULL)?root:T->next;
	}
	return OK;
}
//判断文字均为一种形式出现
status judgeAllliteral(struct Clause *root) {
	int *mark=(int *)calloc(wordsum+1,sizeof(int));//标记数组,并置为0
	struct Clause *T=root;
	while(T) {
		struct Literal *current_literal=T->head;
		while(current_literal) {
			temp=mark[abs(current_literal->value)];
			t=judgesign(current_literal->value);
			if(temp==0) mark[abs(current_literal->value)]=t;
			else if(temp!=t)  return ERROR;//说明有两种形式
			current_literal=current_literal->next;
		}
		T=T->next;
	}
	T=root;
	while(T) {
		struct Literal * current_literal = T->head;
		while(current_literal) {
			if(ValueList[abs(current_literal->value)].value==-1) {
				ValueList[abs(current_literal->value)].value=(current_literal->value > 0) ? 1 : 0;
			}
			current_literal=current_literal->next;
		}
		T=T->next;
	}//已经确定全是一种形式，故可以更新标记数组得到最终结果
	return OK;
}



//判断是否满足要求
status check(struct Clause *root) {
	if(judgeEmptyClause(root)) return ERROR;//不满足
	if(judgeAllliteral(root)) return OK;//满足要求
	return UNKNOW;//未确认状态
}

//销毁所有子句
void destroyClause(struct Clause *root) {
	while(root != NULL) {
		struct Clause * next_r = root->next;
		if(root->head != NULL) {
			struct Literal * current_Literal = root ->head;
			while(current_Literal != NULL) {
				struct Literal * next_Literal = current_Literal->next;
				free(current_Literal);
				current_Literal = next_Literal;
			}
		}
		free(root);
		root = next_r;
	}
}

//克隆子句
struct Clause *cloneClause(struct Clause *root) {
	struct Clause *new_Clause=CreateClause();
	struct Literal *current_literal=root->head,*pre_literal=NULL;
	while(current_literal) {
		struct Literal *new_literal=CreateLiteral();
		new_literal->value=current_literal->value;
		if(new_Clause->head==NULL) new_Clause->head=new_literal;
		if(pre_literal!=NULL) pre_literal->next=new_literal;
		pre_literal=new_literal;
		current_literal=current_literal->next;
	}
	return new_Clause;
}

//克隆所有子句并将选取真值加入子句中并作为单子句
struct Clause *cloneAll(int a,struct Clause *root) {
	ValueList[abs(a)].value=(a>0)?1:0;
	struct Clause *T=root,*new_root=NULL,*pre_Clause=NULL;
	while(T) {
		struct Clause *current_Clause=cloneClause(T);
		if(new_root==NULL) new_root=current_Clause;
		if(pre_Clause!=NULL) pre_Clause->next=current_Clause;
		pre_Clause = current_Clause;
		T=T->next;
	}//复制所有子句
	//现在添加新的单子句
	struct Clause *new_Clause=CreateClause();
	struct Literal *new_Literal=CreateLiteral();
	new_Literal->value=a;
	new_Clause->head=new_Literal;
	new_Clause->next=new_root;
	return new_Clause;
}

//DPLL算法（采用销毁回溯法）
status dpll(struct Clause * root) {
	temp = check(root);
	if(temp != UNKNOW) {
		destroyClause(root);
		return temp;
	}//不是未确定的情况，返回结果
	while(1) { //处理单子句
		temp = check(root);
		if(temp != UNKNOW) {
			destroyClause(root);
			return temp;
		}//同上
		if(!solvesimple(root)) break;//处理单子句，没有的话跳出循环
	}
	if (WAY == 2) {
		while(1) { //处理只以一种形式出现的文字
			temp = check(root);
			if(temp != UNKNOW) {
				destroyClause(root);
				return temp;
			}
			if(!solvepureliteral(root)) break;
		}
	}
	int new_count = chooseLiteral(root);//选取假设真值
	if(dpll(cloneAll(new_count, root)) == OK) return OK;
	else {
		temp = dpll(cloneAll(-new_count, root));//如果不成功，选取假设真值的相反数
		destroyClause(root);
		return temp;
	}
}
/*将结果写入同名文件*/
void writeResult(char * filename, int f, int time) {
	int len=strlen(filename)-3;
	filename[len ] = 'r',filename[len + 1] = 'e',filename[len + 2] = 's';
	FILE * fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("求解结果保存失败！\n");
		exit(1);
	}
	if(f == 1) {
		fprintf(fp,"s 1\n");
		fprintf(fp,"v ");
		for (i = 1; i <= wordsum; i++) {
			temp=(ValueList[i].value!=0?1:-1)*i;
			fprintf(fp, "%d ", temp);
			if (i%10 == 0) fprintf(fp, "\n");
		}
		fprintf(fp, "\nt %d ms\n", time);
	} else {
		fprintf(fp,"s 0\n");
		fprintf(fp,"v \n");
		fprintf(fp, "t %d ms\n", time);
	}
	printf("求解结果保存成功！\n");
	fclose(fp);
}

/*------------------------Sudoku--------------------------*/


//读取棋盘文件
void Readqipan(char *filename) {
	FILE *fp;
	fp = fopen(filename,"r");
	ValueList=(struct ValueList *)calloc(730, sizeof(struct ValueList));
	if(fp == NULL) {
		printf("文件打开失败!\n");
		return ;
	} else {
		char c;
		i=0;
		while(fscanf(fp, "%c", &c) != EOF) {
			if(c!='\n')
			{
			if(c!='.') {
				qipan[i].num=c-'0';
				qipan[i].flag=1;
			} else if(c=='.') qipan[i].num=0,qipan[i].flag=0;
			i++;
			}
		}
	}
	for(i=0; i<=81; i++) {
		x=i/9;
		y=i%9;
		sudoku_table[x][y]=qipan[i].num;
	}
	fclose(fp);
	printf("读取成功！\n");
}

//输出数独盘
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
	return OK;
}

//检查棋盘是否填充满
status Checkqipan(struct Qipan qipan[]) {
	for (i = 0; i < 81; i++) {
		if (qipan[i].num == 0) return ERROR;
	}
	return OK;
}

//数独对应SAT变元表转化为二维数组
status CNFSudokuTableTransform(void) {
	int i,j,z;
	for (i=0; i<9; i++) {
		for (j=0; j<9; j++) {
			for (z=1; z<=9; z++) {
				if (ValueList[81*i+9*j+z].value==1) {
					sudoku_table[i][j]=z;
				}
			}
		}
	}
	return OK;
}
//转化为CNF文件
void SetCNFfile(struct Qipan qipan[]) {
	int x,y,z,i,d;
	FILE *fp;
	char sudo_filename[100];
	printf("请输入想要转换的文件名：");
	scanf("%s",sudo_filename);
	fp=CreateSudokuFile(sudo_filename);
	if (fp==NULL) {
		printf("文件打开失败！\n");
		return ;
	}
	struct Clause *root=Readfile(sudo_filename);
	
	fclose(fp);
	return;

}

//删除选中的文字值l的子句,并删除选中的文字值-l的节点
status DeleteCandL(struct Clause *root,int l) {
	temp=l;//选中的文字；
	if(temp==0) return ERROR;//表示没有找到
	ValueList[abs(temp)].value=(temp>0)?1:0;//-1为未确定真值，1为正真值（可填入），0为负真值（不可填入）
	struct Clause *T=root,*pre_T=CreateClause();
	while(T) {
		struct Literal *current_literal=T->head,*pre_literal=CreateLiteral();
		while(current_literal) {
			if(current_literal->value==temp) {//删除包含该文字的子句
				if(T==root) {
					*(root)=*(root->next);
					T=NULL;
				} else {
					pre_T->next=T->next;
					//free(T);
					T=pre_T;
				}
				break;
			} else if(current_literal->value==-temp) { //删除互为相反数的文字
				if(current_literal==T->head) {
					T->head=current_literal->next;
					//free(current_literal);
				} else {
					pre_literal->next=current_literal->next;
					//free(current_literal);
				}
				pre_literal=current_literal;
				current_literal=current_literal->next;
			}
			current_literal=current_literal->next;
		}
		pre_T=T;
		T=(T==NULL)?root:T->next;
	}
	return OK;
}


//创建数独问题转化为SAT问题后的cnf文件
FILE * CreateSudokuFile(char *filename) {
	int x,y,z,i,j,k,l;//x代表数独的行，y代表数独的列，z取1～9分别代表该格中数独填入值为1～9中任一值
	FILE *fp;
	fp=fopen(filename, "wb");
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
	fclose(fp);
	fp=fopen(filename, "r");
	return fp;
}

/*求解数独cnf文件并转化为数独终盘形式输出*/
status SolveSudoku(void){
    FILE *fp;
    char filename[100];
    char read;
    int i,j,k,x,y,d,cham;
    int exitnum[10];
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
            }
            else if (read<='9'&&read>='1') {
                sudoku_table[i][j]=(int)(read-'0');
            }
            else j=-1;
            j++;
            fscanf(fp, "%c",&read);
        }
    }
    
    fclose(fp);
    fp=fopen(filename,"r");
    struct Clause *root=Readfile(filename);
    
    /*处理数独文件中得到的已知数独信息，建立变元与cell填入值的对应关系*/
    k=1;
    for (i=0; i<9; i++) {
        for (j=0; j<9; j++)
            if (sudoku_table[i][j]==0) {
                for (d=0; d<=9; d++)
                    exitnum[d]=0;
                for (y=0; y<9; y++) {
                    exitnum[sudoku_table[i][y]]=1;
                }//处理行，记录行中已存在的数
                for (x=0; x<9; x++) {
                    exitnum[sudoku_table[x][j]]=1;
                }//处理列，记录列中已存在的数
                for (x=(i/3)*3; x<(i/3)*3+3; x++) {
                    for (y=(j/3)*3; y<(j/3)*3+3; y++)
                        exitnum[sudoku_table[x][y]]=1;
                }//处理region，记录region中已存在的数
                for (cham=1; cham<=9; cham++)
                    if (exitnum[cham]==0) {
                        ValueList[k].x=i;//记录该变元对应的数独cell的行数
                        ValueList[k].y=j;//记录该变元对应的数独cell的列数
                        ValueList[k].z=cham;//记录该变元对应的数独的填入值
                        k++;//处理下一变元
                    }
            }
    }
    
    dpll(root);//求解数独文件
    for (k=1; k<=wordsum; k++) {
        if (ValueList[k].value==1)
            sudoku_table[ValueList[k].x][ValueList[k].y]=ValueList[k].z;
    }//将变元表真值信息转换为二维数组的数独信息
    printf("数独的解为：\n");
    SudokuTablePrint();//输出数独终盘
    return OK;
    }
