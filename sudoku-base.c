#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>

#define SIZE 9 /* grid size = 9x9 */

int puzzle[SIZE][SIZE];

/* struct containing information about the work to be done */
typedef struct {
	pthread_t tid; /* worker thread id */
	int id; /* worker thread internal id */
	size_t start, end; /* start and end job numbers */
	int nerrors; /* number of error in the batch */
} batch;

/* reads puzzle input file */
int load_puzzle(char *filename)
{
	FILE *input_file = fopen(filename, "r");

	if (input_file == NULL)
		return 1;

	for(int i = 0; i < SIZE; i++)
		for(int j = 0; j < SIZE; j++)
			fscanf(input_file, "%d", &puzzle[i][j]);
	fclose(input_file);

	return 0;
}

/* checks row correcteness */
int check_row(int id, int row)
{
	int flag = 0x0;
	for (int i = 0; i < SIZE; i++)
		flag |= 1 << puzzle[row][i];

	if (flag == 0x03FE) /* 0x03FE means all numbers from 1 to 9 are present */
		return 0;

	printf("Thread %d: Erro na linha %d\n", id, row + 1); 
	return 1;
}

/* checks column correctness */
int check_column(int id, int column)
{
	int flag = 0x0;
	for (int i = 0; i < SIZE; i++)
		flag |= 1 << puzzle[i][column];

	if (flag == 0x03FE) /* 0x03FE means all numbers from 1 to 9 are present */
		return 0;

	printf("Thread %d: Erro na coluna %d\n", id, column + 1); 
	return 1;
}

/* checks region correctness */
int check_region(int id, int region)
{
	int flag = 0x0;

	/* calculates region's first element row and column */
	int region_sz = (int) sqrt(SIZE);
	int si = (region / region_sz) * region_sz;
	int sj = (region % region_sz) * region_sz;

	for (int i = 0; i < region_sz; i++) {
		for (int j = 0; j < region_sz; j++) 
			flag |= 1 << puzzle[si + i][sj + j];
	}

	if (flag == 0x03FE) /* 0x03FE means all numbers from 1 to 9 are present */
		return 0;

	printf("Thread %d: Erro na regiao %d\n", id, region + 1); 
	return 1;
}

/* initial function for threads */
void *verify_puzzle(void *arg)
{
	batch *current_batch = (batch*) arg;

	for (int i = current_batch->start; i < current_batch->end; i++) {
		int job_type = i / 9;
		int where = i % 9; /* job row, column or region*/

		switch (job_type) {
			case 0:
				current_batch->nerrors += check_row(current_batch->id, where);
				break;
			case 1:
				current_batch->nerrors += check_column(current_batch->id, where);
				break;
			default:
				current_batch->nerrors += check_region(current_batch->id, where);
				break;
		}
	}

	pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("Erro: numero errado de parametros.\n"
			   "Uso: %s <arquivo de entrada> <numero de threads>\n\n", argv[0]);
		return 1;
	}
	if (!atoi(argv[2])) {
		printf("Erro: numero invalido de threads.\n"
			   "Uso: %s <arquivo de entrada> <numero de threads>\n\n", argv[0]);
		return 2;
	}
	if (load_puzzle(argv[1]) == 1) {
		printf("Erro: nao foi possivel abrir o arquivo \"%s\".\n", argv[1]);
		return 3;
	}

	// Imprime o puzzle na tela
	printf("Quebra-cabecas fornecido:\n");
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++)
			printf("%d ", puzzle[i][j]);
		printf("\n");
	}
	
	/* batch creation */
	int nthreads = atoi(argv[2]);
	batch batches[nthreads];
	float load = (SIZE * 3) / (float) nthreads; /* average number of jobs per thread */

	/* divide jobs between threads and start verification */
	for (size_t i = 0; i < nthreads; i++) {
		batches[i].id = i + 1;
		batches[i].start = (int) (i * load);
		batches[i].end =  (int) ((i + 1) * load);
		batches[i].nerrors = 0;
		pthread_create(&batches[i].tid, NULL, verify_puzzle, (void*) &batches[i]);
	}

	// wait for threads to finish its jobs and compute the number of errors
	int nerrors = 0;
	for (size_t i = 0; i < nthreads; i++) {
		pthread_join(batches[i].tid, NULL);
		nerrors += batches[i].nerrors;
	}

	printf("Erros encontrados: %d\n", nerrors);

	return 0;
}
