#include <Windows.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define D 3		// dimension of sudoku
#define N (D*D)		// number of row, column, box

/***
Struct: node

represents node in dancing link data structure.
has 5 links: left, right(for row); up, down(for column), column(for constraints)
has union value for soulution
***/
struct node {
	union {
		char size;      	// headers exclusively use size (to find most interesting column)
		uint32_t name;  	// cells exclusively use name (to print solution)
	};
	struct node *left, *up, *right, *down;
	struct node *column;
};
/***
enumerator for determining position of node
; cell, row, column, box
***/
enum { CST_CELL, CST_ROW, CST_COL, CST_BOX, CST_N };

#define NODES_HEIGHT (N+1) /* +1 for header */
#define NODES_WIDTH  (CST_N * N*N)
/***
Struct: sudansu

struct for sudoku solution.
has head node and node list for recording solution
has row list for search(cf. Search function)
***/
struct sudansu {
	uint32_t solution[N*N];
	struct node h;
	struct node nodes[NODES_HEIGHT * NODES_WIDTH];
	struct node *rows[N*N*N];
};
/***
Function: get_node_position

determine position of node and return specific unsigned number
***/
static unsigned get_node_position(unsigned type, unsigned r, unsigned c, unsigned z)
{
	switch (type) {
	case CST_CELL:  return c + r*N;
	case CST_ROW:   return z + r*N;
	case CST_COL:   return z + c*N;
	case CST_BOX:   return z + c / D*N + r / D*D*N;
	default: abort();
	}
}
#define CELL_NAME(row, col, numchr) ((row)<<16 | (col)<<8 | (numchr))
/***
Function: init

init dancing link board for sudoku problem.

Param: ctx	pointer of sudansu structure. for being initialized
***/
static void init(struct sudansu *ctx)
{
	unsigned i, j, r, c, z, cst_type;
	struct node *x, *row_nodes[CST_N]; 	// one node per line per constraint type

	memset(ctx, 0, sizeof(*ctx));

	/* vertical links */
	x = ctx->nodes;
	for (j = 0; j < NODES_HEIGHT; j++) {
		for (i = 0; i < NODES_WIDTH; i++) {
			x->up = j == 0 ? &ctx->nodes[(NODES_HEIGHT - 1)*NODES_WIDTH + i] : x - NODES_WIDTH;
			x->down = j == NODES_HEIGHT - 1 ? &ctx->nodes[0 * NODES_WIDTH + i] : x + NODES_WIDTH;
			x->column = &ctx->nodes[i];
			x++;
		}
	}
	/* header horizontal links */
	ctx->h.right = ctx->nodes;
	ctx->h.left = ctx->nodes + NODES_WIDTH - 1;
	for (i = 0; i < NODES_WIDTH; i++) {
		ctx->nodes[i].left = i == 0 ? &ctx->h : &ctx->nodes[i - 1];
		ctx->nodes[i].right = i == NODES_WIDTH - 1 ? &ctx->h : &ctx->nodes[i + 1];
	}

	/* nodes horizontal links */
	i = 0;
	for (r = 0; r < N; r++) {
		for (c = 0; c < N; c++) {
			for (z = 0; z < N; z++) {
				/* locate the CST_N nodes per row */
				for (cst_type = 0; cst_type < CST_N; cst_type++) {
					unsigned col = get_node_position(cst_type, r, c, z);
					struct node *colhead = &ctx->nodes[cst_type*N*N + col];

					// locate first unused node of the colum
					x = colhead->down + NODES_WIDTH * colhead->size++;
					x->name = CELL_NAME(r, c, z + '1');
					row_nodes[cst_type] = x;
				}

				/* link them together */
				for (cst_type = 0; cst_type < CST_N; cst_type++) {
					row_nodes[cst_type]->left = row_nodes[cst_type == 0 ? CST_N - 1 : cst_type - 1];
					row_nodes[cst_type]->right = row_nodes[cst_type == CST_N - 1 ? 0 : cst_type + 1];
				}

				/* index the row since the row positions in ctx->nodes are not linear */
				ctx->rows[i++] = row_nodes[0];
			}
		}
	}
}
/***
Function: cover

cover the column. 

Param: c	pointer of node structure for being covered
***/
static void cover(struct node *c)
{
	struct node *i, *j;

	c->right->left = c->left;
	c->left->right = c->right;
	for (i = c->down; i != c; i = i->down) {
		for (j = i->right; j != i; j = j->right) {
			j->down->up = j->up;
			j->up->down = j->down;
			j->column->size--;
		}
	}
}
/***
Function: uncover

uncover the column. 

Param: c	pointer of node structure for being uncovered
***/
static void uncover(struct node *c)
{
	struct node *i, *j;
	for (i = c->up; i != c; i = i->up) {
		for (j = i->left; j != i; j = j->left) {
			j->column->size++;
			j->down->up = j->up->down = j;
		}
	}
	c->right->left = c->left->right = c;
}
/***
Function: print_solution

print solution and return 1 for informing end of search
***/
static int print_solution(const uint32_t *solution)
{
	unsigned i, j, k;
	char grid[N*N] = { 0 };
	const char *p = grid;

	for (i = 0; i < N*N; i++) 
	{
		const char row = solution[i] >> 16 & 0xff;
		const char col = solution[i] >> 8 & 0xff;
		const char num = solution[i] & 0xff;
		grid[row*N + col] = num;
	}

	for (j = 0; j < N; j++) { /* print the solution board(apply the given form) */
		for (i = 0; i < N; i++) {
			printf(" %c", *p ? *p : '?');
			p++;
		}
		if (j + 1 != N && (j + 1) % N == 0) {
			printf("\n ");
		}
		printf("\n");
	}
	printf("\n");
	return 1;
}
/***
Function: choose_a_column

choose a column for search. 
if c equals h, the sudoku problem is solved because all constraints are satisfied

Param: c	pointer of node structure of head which links constraints node
***/
static struct node *choose_a_column(struct node *h)
{
	int size = INT_MAX;//INT_MAX : 2147483647
	struct node *j, *c = h->right;

	if (c == h)
		return NULL;
	for (j = h->right; j != h; j = j->right)	/* choose the most interesting column */
		if (j->size < size)		/* traveling to find the node which has smallest node->size*/ 
			c = j, size = j->size;
	return c;
}
/***
Function: search

choose a column in head node for satisfying constraint.
if chosen column is NULL, all constraints are satisfied: print solution

if chosen column is not NULL, cover the column
and choose a row expected to be included in solution,
cover about this row.
if this row is determined to be not solution,
uncover about row and choose another row(backtracking)

Param: ctx		pointer of sudansu structure that has information of sudoku board
k		number of solutions in sudoku board that is given by input file

return: if search is successful, return 1
if not successful, return 0
***/
static int search(struct sudansu *ctx, unsigned k)
{
	struct node *c, *r, *j;

	c = choose_a_column(&ctx->h);	 /* step 1: chose a column */	

	if (!c)	return print_solution(ctx->solution);

	cover(c);		/* step 2: cover by the column chosen at step1 */

	for (r = c->down; r != c; r = r->down)	/* step 3: travel the sudoku board via Dancing Link */
	{				
		ctx->solution[k] = r->name;	/* 3-1: mark the node name to solution board */
		for (j = r->right; j != r; j = j->right) /*    3-2: covering */
			cover(j->column);
		if (search(ctx, k + 1))	/* 3-3: recursive call(then find next value which holds sudoku rule */
			return 1;	/* if find perfect solution, then finish whole progress */

		ctx->solution[k] = 0;	/* step 4: if this step is excuting, It is necessary to backtracking*/  
		for (j = r->left; j != r; j = j->left)	 /* so doing uncover	 */
			uncover(j->column);
	}
	uncover(c);
	return 0;
}
/***
Function: parse_grid

input from file, and cover about given solution

Param: ctx		pointer of sudansu structure that has information of sudoku board
stream	file stream for input

return: if input error, return -1
if parse is successful, return number of given solution
***/
static int parse_grid(struct sudansu *ctx, FILE *stream)
{
	char buf[16];
	unsigned i, j, l,k = 0;


	for (j = 0; j < N; j++) 
	{
		for(l = 0; l<N; l++)	/* read initial conditions for each row via the stream */ 
		{
			fscanf(stream,"%s",&buf[l]);
			if(buf[l] == -1)
				return -1;
		}

		for (i = 0; i < N; i++)	 /* setting the solution board which is covered by all initial conditions */
		{
			if (buf[i] >= '1' && buf[i] <= '1' + N) 
			{
				struct node *x, *r = ctx->rows[j*N*N + i*N + buf[i] - '1'];

				ctx->solution[k++] = CELL_NAME(j, i, buf[i]);
				cover(r->column);
				for (x = r->right; x != r; x = x->right)
					cover(x->column);
			}
		}
	}
	return k;
}

int main(int ac, char **av)
{
	int k = 0, ncase = 0, loop = 0;
	struct sudansu ctx;
	FILE *in_file = NULL;

	DWORD Time1, Time2;
	Time1 = GetTickCount();

	in_file = fopen("input.txt","r");

	fscanf(in_file,"%d",&ncase);		/*read the number of cases in "input.txt"*/

	for(loop = 0; loop<ncase; loop++)
	{
		init(&ctx);			/* init sudoku board */
		k = parse_grid(&ctx, in_file);	/* enter the initial condition of sudoku quiz*/
		search(&ctx, k);		/* serch for solution(recursive)*/
	}

	Time2 = GetTickCount();
	printf("%.3fÃÊ", (Time2-Time1)/1000.0);
	return 0;
}
