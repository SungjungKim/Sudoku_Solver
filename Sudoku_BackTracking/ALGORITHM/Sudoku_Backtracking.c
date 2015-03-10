#include <stdio.h>
#include <stdlib.h>

#define ROW_SIZE 9
#define COL_SIZE 9

//sudoku �ǿ� ���ڸ� ä������ 
int fillSudoku(int sudoku[][9], int row, int col);

//promising ���� Ȯ���Ѵ�
int isAvailable(int sudoku[][9], int row, int col, int num);

int main(){
	//sudoku ���� ����
	int numTestCases ;
	//��, �� ����
	int row, col ;
	//sudoku �� ����
	int sudoku[ROW_SIZE][COL_SIZE] ;

	//input.txt���� ������ �Է¹޴´�.
	FILE *inFile = fopen("input.txt","r");
	if(inFile ==NULL)
		exit(1);

	//sudoku �׽�Ʈ ���̽��� ���� �Է¹޴´�.
	fscanf(inFile, "%d",&numTestCases);

	//�׽�Ʈ ���̽���ŭ �ݺ�
	while(numTestCases--){

		//�������� �ʱ�ȭ
		for( row =0 ; row< ROW_SIZE ; row++)
			for(col =0 ; col <COL_SIZE ; col++)
				fscanf(inFile,"%d",&sudoku[row][col]);
		
		//������ ������ Ǭ��.
		if(fillSudoku(sudoku,0,0) ){
			for( row =0 ; row< ROW_SIZE ; ++row){
				for(col =0 ; col<COL_SIZE ; ++col)
					printf("%d ", sudoku[row][col]);
				printf("\n");
			}
			printf("-------------------------------\n");
		}
		else
			printf("\n\nNO SOLUTION\n\n");

	}
}

//���������� ä������.
int fillSudoku(int sudoku[][9], int row, int col){
	int i;
	if( row<9 && col<9 ){
		//pre filled
		if( sudoku[row][col] != 0 )  {
			if( (col+1)<9 )
				return fillSudoku(sudoku, row, col+1);
			else if( (row+1)<9 )
				return fillSudoku(sudoku, row+1, 0);
			else
				return 1;
		}

		else{
			for(i=0; i<9; ++i){
				if( isAvailable(sudoku, row, col, i+1) ){
					sudoku[row][col] = i+1;

					if( (col+1)<9 ){
						if( fillSudoku(sudoku, row, col +1) )
							return 1;
						else
							sudoku[row][col] = 0;
					}
					else if( (row+1)<9 ){
						if( fillSudoku(sudoku, row+1, 0) )    
							return 1;
						else
							sudoku[row][col] = 0;
					}
					else
						return 1;
				}
			}
		}
		return 0;
	}
	else{
		return 1;
	}
} 

//promising���� Ȯ���Ѵ�.
int isAvailable(int sudoku[][9], int row, int col, int num){
	int i, j;
	//checking in the grid
	int rowStart = (row/3) * 3;
	int colStart = (col/3) * 3;
	
	for(i=0; i<9; ++i)
		if( (sudoku[row][i] == num) || ( sudoku[i][col] == num )  )//checking in row and col
			return 0;

	for(i=rowStart; i<(rowStart+3); ++i){
		for(j=colStart; j<(colStart+3); ++j){
			if( sudoku[i][j] == num )
				return 0;
		}
	}    
	return 1;
}    