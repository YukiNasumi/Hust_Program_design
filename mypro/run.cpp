#include <stdlib.h>
#include <stdio.h>
#include "define.h"
#include<chrono>
/*输入文件名和优化方案*/
int main(int args,char *argv[]){
    int op=1;//记录选择的操作序号
	int choose;//记录选择的算法序号
	int i=0,solut=0;//solut记录CNF公式是否有解，有解为1，无解为0
	//clock_t start = 0,finish = 0;//记录DPLL函数调用的起始和终止时间
	int duration=0;//记录SAT求解时间
	FILE *open;
	printf("请输入需求解的cnf文件名：");
	strcpy(filename,argv[1]);
	open=fopen(filename, "r");
	if (open==NULL) {
		printf("文件打开错误！\n");
		//getchar();
		//getchar();
		return ERROR;
	}
	if (CreateParadigm(&open)==ERROR) {//生成CNF公式数据存储结构
		//getchar();
		//getchar();
		return ERROR;
	}
    //getchar(); //getchar();
	op=1;
	solut=NO_RESULT;

	while (op) {
		system("cls");
		printf("\t\tChoose an option please.\n");
		printf("---------------------------------------------------------\n");
		printf("  1.求解该算例   2.遍历CNF文件   3.保存求解结果\n");
		printf("  0.退出\n");
		printf("---------------------------------------------------------\n");
		//scanf("%d",&op);
		switch (op) {
			case 1:
				if (ValueList[1].IsInit==1) {
					printf("已对公式求解！");
					//getchar();
					//getchar();
					break;
				}
				printf(" 使用：1.优化1算法       2.优化2算法        3.优化前算法\n");
				//scanf("%d",&choose);
                choose = atoi(argv[2]);
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
				printf("\n计算时间为：%ld cpu clock\n",(finish-start));
				//getchar();
				//getchar();
				break;
			case 2:
				if (solut==NO_RESULT) {
					printf("请先求解CNF公式！\n");
					//getchar();
					//getchar();
					break;
				}
				ParadigmTrabverse(r);
				if (AnswerCheck(solut)==OK) {
					printf("公式遍历完成！\n");
				} else {
					printf("ERROR！\n");
				}
				//getchar();
				//getchar();
				break;
			case 3:
				if (solut==NO_RESULT) {
					printf("请先求解CNF公式！\n");
					//getchar();
					//getchar();
					break;
				}
				if (SaveValue(ValueList,solut,duration))
					printf("成功存入文件！");
				//getchar();
				//getchar();
				break;
			case 0:
				break;
			default:
				printf("输入错误！");
                op=-1;
				//getchar();
				//getchar();
				break;
		}
        op++;
	}
	DestroyParadigm(r);//销毁CNF公式数据存储结构
	return OK;

}
