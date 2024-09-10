#include <stdio.h>

// 生成每个单元格的格约束
void generate_cell_constraints(FILE *fp) {
    for (int i = 1; i <= 9; i++) {
        for (int j = 1; j <= 9; j++) {
            // 生成每个单元格必须填一个数字的子句
            for (int k = 1; k <= 9; k++) {
                fprintf(fp, "%d ", (i - 1) * 81 + (j - 1) * 9 + k);
            }
            fprintf(fp, "0\n");

            // 生成每个单元格不能填两个数字的子句
            for (int k = 1; k <= 8; k++) {
                for (int l = k + 1; l <= 9; l++) {
                    fprintf(fp, "-%d -%d 0\n", 
                        (i - 1) * 81 + (j - 1) * 9 + k, 
                        (i - 1) * 81 + (j - 1) * 9 + l);
                }
            }
        }
    }
}

// 生成每行的行约束
void generate_row_constraints(FILE *fp) {
    for (int k = 1; k <= 9; k++) {
        for (int i = 1; i <= 9; i++) {
            // 每个数字在每一行中只能出现一次
            for (int j = 1; j <= 9; j++) {
                fprintf(fp, "%d ", (i - 1) * 81 + (j - 1) * 9 + k);
            }
            fprintf(fp, "0\n");

            // 生成每个数字在每行不能重复的子句
            for (int j1 = 1; j1 <= 8; j1++) {
                for (int j2 = j1 + 1; j2 <= 9; j2++) {
                    fprintf(fp, "-%d -%d 0\n", 
                        (i - 1) * 81 + (j1 - 1) * 9 + k, 
                        (i - 1) * 81 + (j2 - 1) * 9 + k);
                }
            }
        }
    }
}

// 生成每列的列约束
void generate_column_constraints(FILE *fp) {
    for (int k = 1; k <= 9; k++) {
        for (int j = 1; j <= 9; j++) {
            // 每个数字在每一列中只能出现一次
            for (int i = 1; i <= 9; i++) {
                fprintf(fp, "%d ", (i - 1) * 81 + (j - 1) * 9 + k);
            }
            fprintf(fp, "0\n");

            // 生成每个数字在每列不能重复的子句
            for (int i1 = 1; i1 <= 8; i1++) {
                for (int i2 = i1 + 1; i2 <= 9; i2++) {
                    fprintf(fp, "-%d -%d 0\n", 
                        (i1 - 1) * 81 + (j - 1) * 9 + k, 
                        (i2 - 1) * 81 + (j - 1) * 9 + k);
                }
            }
        }
    }
}

// 生成3x3盒子的盒约束
void generate_box_constraints(FILE *fp) {
    for (int k = 1; k <= 9; k++) {
        for (int boxRow = 0; boxRow < 3; boxRow++) {
            for (int boxCol = 0; boxCol < 3; boxCol++) {
                // 每个数字在每个3x3盒子中只能出现一次
                for (int i = 1; i <= 3; i++) {
                    for (int j = 1; j <= 3; j++) {
                        fprintf(fp, "%d ", 
                            (boxRow * 3 + i - 1) * 81 + 
                            (boxCol * 3 + j - 1) * 9 + k);
                    }
                }
                fprintf(fp, "0\n");

                // 生成每个数字在每个3x3盒子不能重复的子句
                for (int i1 = 1; i1 <= 9; i1++) {
                    for (int i2 = i1 + 1; i2 <= 9; i2++) {
                        fprintf(fp, "-%d -%d 0\n", 
                            (boxRow * 3 + (i1 - 1) / 3) * 81 + 
                            (boxCol * 3 + (i1 - 1) % 3) * 9 + k, 
                            (boxRow * 3 + (i2 - 1) / 3) * 81 + 
                            (boxCol * 3 + (i2 - 1) % 3) * 9 + k);
                    }
                }
            }
        }
    }
}

int main() {
    FILE *fp = fopen("sudoku.cnf", "w");
    if (fp == NULL) {
        printf("无法创建文件\n");
        return 1;
    }

    // 生成格约束、行约束、列约束和盒约束
    generate_cell_constraints(fp);
    generate_row_constraints(fp);
    generate_column_constraints(fp);
    generate_box_constraints(fp);

    fclose(fp);
    printf("CNF文件生成完毕\n");
    return 0;
}
