#include <stdio.h>
#include <stdlib.h>

#define ROW_SIZE 9
#define COL_SIZE 9

//sudoku 판에 숫자를 채워간다 
int fillSudoku(int sudoku[][9], int row, int col);

//promising 인지 확인한다
int isAvailable(int sudoku[][9], int row, int col, int num);

int main(){
	//sudoku 문제 갯수
	int numTestCases ;
	//행, 열 변수
	int row, col ;
	//sudoku 판 변수
	int sudoku[ROW_SIZE][COL_SIZE] ;

	//input.txt에서 문제를 입력받는다.
	FILE *inFile = fopen("input.txt","r");
	if(inFile ==NULL)
		exit(1);

	//sudoku 테스트 케이스의 갯수 입력받는다.
	fscanf(inFile, "%d",&numTestCases);

	//테스트 케이스만큼 반복
	while(numTestCases--){

		//스도쿠판 초기화
		for( row =0 ; row< ROW_SIZE ; row++)
			for(col =0 ; col <COL_SIZE ; col++)
				fscanf(inFile,"%d",&sudoku[row][col]);
		
		//스도쿠 문제를 푼다.
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

//스도쿠판을 채워간다.
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

//promising인지 확인한다.
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