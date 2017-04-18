#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>

/* grid size = 9x9 */
#define SIZE 9

int puzzle[SIZE][SIZE]; // O quebra cabecas

// Struct contendo a thread, um identificador para ela, e a parcela do trabalho que irá realizar
typedef struct {
    size_t start, end;
    int id;
    int where;
    pthread_t tid;
} work;



/* Funcao que le um grid do arquivo "filename" e o armazena em uma matriz */
int load_grid(int grid[][SIZE], char *filename)
{
    FILE *input_file = fopen(filename, "r");

    if (input_file != NULL) {
        for(int i = 0; i < SIZE; i++)
            for(int j = 0; j < SIZE; j++)
                fscanf(input_file, "%d", &grid[i][j]);
        fclose(input_file);
        return 1;
    }

    return 0;
}


// Funcao que checa a corretude das linhas do quebra-cabecas
void check_rows(void * arg)
{
    work * current_thread = (work *) arg;
    int flag = 0x0000; // Máscara binária para controle dos digitos da linha
    //printf("Thread %lu: Verificando linhas %u à %u\n",pthread_self(), current_thread->start, current_thread->end);
    int i = current_thread->where;
        // Se todos os digitos estiverem presentes, flag = 0x01FF
    for (int j = 0; j < SIZE; j++) {
        flag |= 1 << (puzzle[i][j] - 1);
    }
    if (flag != 0x01FF) {
        printf("Thread %d (TID %lu): Erro na linha %d\n", current_thread->id, pthread_self(), i + 1);
    }
}

// Funcao que checa a corretude das colunas do quebra-cabecas
void check_collumns(void * arg)
{

    work * current_thread = (work *) arg;
    //printf("Thread %lu: Verificando colunas %u à %u\n",pthread_self(), current_thread->start, current_thread->end);
    int flag = 0x0000; // Máscara binária para controle dos digitos da linha
        // Se todos os digitos estiverem presentes, flag = 0x01FF
    int j = current_thread->where;
    for (int i = 0; i < SIZE; i++) {
        flag |= 1 << (puzzle[i][j] - 1);
    }
    if (flag != 0x01FF) {
        printf("Thread %d (TID %lu): Erro na coluna %d\n", current_thread->id, pthread_self(), j + 1);
    }
}


// Funcao que checa a corretude dos quadrados internos 3x3 do quebra cabeças
void * check_quadrants(void * arg, int row_collumn[])
{
    work * current_thread = (work *) arg;
    int start = current_thread -> start;
    int end = current_thread -> end;
    //printf ("Thread %lu: Start: %u, end: %u\n",pthread_self(), start, end);
    int flag, si, sj, i, j;
    flag = 0x0000;// Máscara binária para controle dos digitos do quadrado
        // Se todos os digitos estiverem presentes, flag = 0x01FF

    si = row_collumn[0];
    sj = row_collumn[1];

    int quadrant_size = (int) sqrt(SIZE);

    for (i = 0; i < quadrant_size; i++) {
        for (j = 0; j < quadrant_size; j++) {
            flag |= 1 << (puzzle[si + i][sj + j] - 1);
        }
    }
    if (flag != 0x01FF) {
        printf("Thread %d (TID %lu): Erro no quadrante %u\n", current_thread->id, pthread_self(), current_thread->where+1);
    }
}


// Funcao que sera chamada pelas threads, para checar a corretude das linhas e colunas
void * verify_puzzle(void * arg) {

    work * current_thread = (work *) arg;

    int job, where;
    for(int i = current_thread->start; i < current_thread->end; i++) {
        //job 0-8   -> linha
        //job 9-17  -> coluna
        //job 18-26 -> região
        job = i/9;
        //where 0-8 -> linha/coluna/regiao especifica
        where = i%9;


        current_thread->where = where;
        if (job < 1) {
            //printf("Row : Thread %u job %u where %u\n",current_thread->id,job , current_thread->where);
            check_rows(arg);
        } else if (job < 2) {
            //printf("Collumns: Thread %u job %u where %u\n",current_thread->id,job , current_thread->where);
            check_collumns(arg);
        } else {
            //printf("Fild: Thread %u job %u where %u\n",current_thread->id,job , current_thread->where);
            int row_collumn[] = {(where/3)*3,(where%3)*3};
            check_quadrants(arg, row_collumn);
        }
    }
    //check_rows(arg);
    //check_collumns(arg);
    //check_quadrants(arg);
    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{

    if(argc != 3) {
        printf("Erro: informe o arquivo de entrada, e o numero de threads!\nUso: %s <arquivo de entrada> <numero de threads>\n\n", argv[0]);
        return 1;
    }
    if(!atoi(argv[2])) {
        printf("Erro: informe o arquivo de entrada, e o numero de threads!\nUso: %s <arquivo de entrada> <numero de threads>\n\n", argv[0]);
        return 1;
    }

    int nthreads = atoi(argv[2]);
    work threads[nthreads];
    // Divisao inicial do trabalho entre as threads
    size_t index_start, index_end;
    float batch = (float) (SIZE*3) / (float) nthreads;

    // Imprime o puzzle na tela
    if(load_grid(puzzle, argv[1])) {
        printf("Quebra-cabecas fornecido:\n");
        for(int i = 0; i < SIZE; i++) {
            for(int j = 0; j < SIZE; j++)
                printf("%d ", puzzle[i][j]);
            printf("\n");
        }

        // Divide o trabalho e instancia as threads para checar as linhas e colunas do puzzle
        for (size_t i = 0; i < nthreads; i++) {
            // printf("Index start: %d, index_end: %d, index_n: %d\n", index_start, index_end, index_n);

            threads[i].start = (int) (i*batch);
            threads[i].end =  (int) ((i+1)*batch);
            threads[i].id = i;

            //printf("Main: Thread %u Start %u End %u\n",i,threads[i].start,threads[i].end);

            pthread_create(&threads[i].tid, NULL, verify_puzzle, (void*)&threads[i]);
        }

        // Espera as threads terminarem de checar as linhas e colunas
        for (size_t i = 0; i < nthreads; i++) {
            pthread_join(threads[i].tid, NULL);
        }

        printf("\n");
    }

    return 0;
}
