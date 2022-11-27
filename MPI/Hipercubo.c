/* OBTENCION DEL VALOR MAXIMO DE LOS DATOS DISTRIBUIDOS ENTRE LOS NODOS DE UNA RED HIPERCUBO */
/* Carlos Gomez Fernandez */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <math.h>

/* Archivo donde se almacenan los numeros a procesar */
#define FICHERO "datos.dat"

/* Funciones empleadas para la resolucion del problema */
int obtenerNumeros(double *numeros);
void obtenerVecinos(int rank, int *vecinos);
double obtenerMaximo(int rank, int *vecinos, double bufNumeros);

int main(int argc, char *argv[]){
    int rank, size, numerosProcesar, control = 1, procesos = pow(2,L), vecinos[L];
    double bufNumeros, maximo;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Control para que el rank 0 sea el que lleve a cabo el procesamiento y distribucion de los datos */
    if (rank == 0){
        /* Control del lanzamiento de procesos suficientes, acorde con el tamano definido de la red */
        if (procesos != size){
            printf("[ERROR] Para un hipercubo de %d dimensiones deben ser lanzados un total %d procesos\n", L, procesos);
            /* Finalizacion de la ejecucion de los procesos */
            control = 0;
            MPI_Bcast(&control, 1, MPI_INT, 0, MPI_COMM_WORLD);
        } else {
            /* Vector de numeros a repartir entre los nodos de la red */
            double *numeros = malloc(1024 * sizeof(double));
            /* Obtencion de los numeros almacenados en el archivo datos.dat */
            numerosProcesar = obtenerNumeros(numeros);
            /* Control de la cantidad de numeros almacenados en el archivo datos.dat, acorde con el tamano definido de la red */
            if(procesos != numerosProcesar){
                printf("[ERROR] Este hipercubo necesita únicamente %d numeros y dispone de %d numeros en el fichero\n", procesos, numerosProcesar);
                /* Finalizacion de la ejecucion de los procesos */
                control = 0;
                MPI_Bcast(&control, 1, MPI_INT, 0, MPI_COMM_WORLD);
            } else {
                /* Continuar con la ejecucion de los procesos */
                MPI_Bcast(&control, 1, MPI_INT, 0, MPI_COMM_WORLD);
                /* Envio de los numeros a cada uno de los nodos de la red */
                for(int i = 0; i<numerosProcesar; i++){
                    bufNumeros = numeros[i];
                    MPI_Send(&bufNumeros, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
                }
            }
        }
    }
    /* El rank 0 envia un mensaje para continuar con la ejecucion de los procesos, tras el envio de los numeros a cada uno de los nodos de la red */
    MPI_Bcast(&control, 1, MPI_INT, 0, MPI_COMM_WORLD);
    /* Control para la continuacion de la ejecucion de los procesos */
    if (control != 0){
        /* Recepcion de todos los nodos de los numeros enviados por el rank 0 */
        MPI_Recv(&bufNumeros, 1, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        /* Cada nodo obtiene sus vecinos */
        obtenerVecinos(rank, vecinos);
        /* Obtencion del valor maximo almacenado en la red */
        maximo = obtenerMaximo(rank, vecinos, bufNumeros);
        /* Control para que el rank 0 sea el que muestre el valor maximo de la red */
        if (rank == 0){
            printf("El mayor numero que almacena este hipercubo es %.1f\n", maximo);
        }
    }
    /* Finalizacion de la ejecucion */
    MPI_Finalize();
    return 0;
}

/* Funcion para obtener los numeros almacenados en el archivo datos.dat */
int obtenerNumeros(double *numeros){
    /* Vector auxiliar de char para almacenar los numeros separados por comas */
    char *numerosFichero = malloc(1024 * sizeof(char));
    int *numeroActual;
    int totalNumeros = 0;

    /* Apertura del archivo en modo lectura */
    FILE *archivo = fopen(FICHERO, "r");
    /* Control de la correcta apertura del archivo */
    if (archivo == NULL){
        fprintf(stderr, "[ERROR] Error en la apertura del archivo\n");
        return 0;
    }
    /* Copia de los numeros en un vector auxiliar */
    fscanf(archivo, "%s", numerosFichero);
    /* Cierre del archivo */
    fclose(archivo);
    /* Lectura del primer numero entre comas mediante strtok y transformacion a double con atof */
    numeros[totalNumeros++] = atof(strtok(numerosFichero,","));
    /* Lectura de los numeros entre comas*/
    while ((numeroActual = strtok(NULL,",")) != NULL){
        numeros[totalNumeros++] = atof(numeroActual);
    }

    return totalNumeros;
}

/* Funcion para obtener los vecinos de cada nodo */
void obtenerVecinos(int rank, int *vecinos){
    /* Todos los nodos tienen tantos vecinos como dimensiones tiene la red */
    for (int i = 0; i<L; i++){
        /* XOR entre el rank y la dimension de la red y almacenamiento de los vecinos en el vector */
        vecinos[i] = (rank^((int)pow(2,i)));
    }
}

/* Funcion para obtener el valor maximo de toda la red */
double obtenerMaximo(int rank, int *vecinos, double bufNumeros){
    /* Mayor numero de toda la red iniciado al valor minimo que puede almacenar un double */
    double maximo = DBL_MIN;

    MPI_Status status;
    /* Calculo del valor maximo entre los vecinos de cada nodo, enviando a todos los vecinos el valor almacenado en cada iteracion */
    for(int i = 0; i<L; i++){
        /* Control para actualizar el valor que almacena cada nodo, quedandose siempre con el de mayor valor */
        if (bufNumeros>maximo){
            maximo = bufNumeros;
        }
        /* Envio del mayor valor a todos los vecinos del nodo */
        MPI_Send(&maximo, 1, MPI_DOUBLE, vecinos[i], i, MPI_COMM_WORLD);
        /* Recepcion del mayor valor por parte de todos los vecinos del nodo */
        MPI_Recv(&bufNumeros, 1, MPI_DOUBLE, vecinos[i], i, MPI_COMM_WORLD, &status);
        /* Control para actualizar el valor que almacena cada nodo, quedandose siempre con el de mayor valor */
        if(bufNumeros>maximo){
            maximo = bufNumeros;
        }
    }
    return maximo;
}