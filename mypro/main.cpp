#include <stdlib.h>
#include <stdio.h>
#include "define.h"
#include<iostream>

int main(int argc, const char * argv[]) {
    sum_op=1;
    while (sum_op) {
        system("cls");
        printf("\n---Menu For Puzzle Or SAT-----\n");
		printf("1.SAT        2.Sudoku\n");
		printf("0.Exit\n");
		printf("------------------------------\n");
		printf("请输入0~2：\n");
		printf("----------\n");
        scanf("%d",&sum_op);
        switch (sum_op) {
            case 1:
                SAT();
                break;
            case 2:
                Sudoku();
                break;
            case 0:
                break;
            default:
                printf("输入错误！");
                getchar();getchar();
                break;
        }
    }
    return 0;
}
