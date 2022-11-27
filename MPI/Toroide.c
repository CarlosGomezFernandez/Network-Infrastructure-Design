/* OBTENCION DEL VALOR MINIMO DE LOS DATOS DISTRIBUIDOS ENTRE LOS NODOS DE UNA RED TOROIDE */
/* Carlos Gomez Fernandez */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>

/* Archivo donde se almacenan los numeros a procesar */
#define FICHERO "datos.dat"

/* Funciones empleadas para la resolucion del problema */
int obtenerNumeros(double *numeros);
void obtenerVecinos(int rank, int *vecinoNorte, int *vecinoSur, int *vecinoEste, int *vecinoOeste);
double obtenerMinimo(int rank, int vecinoNorte, int vecinoSur, int vecinoEste, int vecinoOeste, double bufNumeros);

int main(int argc, char *argv[]){
    int rank, size, numerosProcesar, vecinoNorte, vecinoSur, vecinoEste, vecinoOeste, control = 1;
    double bufNumeros, minimo;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Control para que el rank 0 sea el que lleve a cabo el procesamiento y distribucion de los datos */
    if (rank == 0){
        /* Control del lanzamiento de procesos suficientes, acorde con el tamano definido de la red */
        if (L*L != size){
            printf("[ERROR] Para un toroide de lado %d deben ser lanzados un total %d procesos\n", L, L*L);
            /* Finalizacion de la ejecucion de los procesos */
            control = 0;
            MPI_Bcast(&control, 1, MPI_INT, 0, MPI_COMM_WORLD);
        } else {
            /* Vector de numeros a repartir entre los nodos de la red */
            double *numeros = malloc(1024 * sizeof(double));
            /* Obtencion de los numeros almacenados en el archivo datos.dat */
            numerosProcesar = obtenerNumeros(numeros);
            /* Control de la cantidad de numeros almacenados en el archivo datos.dat, acorde con el tamano definido de la red */
            if(L*L != numerosProcesar){
                printf("[ERROR] Este toroide necesita únicamente %d numeros y dispone de %d numeros en el fichero\n", L*L, numerosProcesar);
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
        obtenerVecinos(rank, &vecinoNorte, &vecinoSur, &vecinoEste, &vecinoOeste);
        /* Obtencion del valor minimo almacenado en la red */
        minimo = obtenerMinimo(rank, vecinoNorte, vecinoSur, vecinoEste, vecinoOeste, bufNumeros);
        /* Control para que el rank 0 sea el que muestre el valor minimo de la red */
        if (rank == 0){
            printf("El menor numero que almacena este toroide es %.1f\n", minimo);
        }
    }
    /* Finalizacion de la ejecucion */
    MPI_Finalize();
    return 0;
}

/* Funcion para obtener los numeros almacenados en el archivo datos.dat */
int obtenerNumeros (double *numeros){
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
void obtenerVecinos(int rank, int *vecinoNorte, int *vecinoSur, int *vecinoEste, int *vecinoOeste){
    int fila, columna;
    /* Las filas se enumeraran de abajo hacia arriba y las columnas de izquierda a derecha */
    fila = rank/L;
    columna = rank%L;
    /* Control para la busqueda del vecino Sur por parte de la fila inferior */
    if (fila == 0){
        /* Parte superior de la red */
        *vecinoSur = ((L-1)*L)+columna;
    } else {
        /* Control para la busqueda del vecino Sur por parte de cualquier fila */
        *vecinoSur = ((fila-1)*L)+columna;
    }
    /* Control para la busqueda del vecino Norte por parte de la fila superior */
    if (fila == L-1){
        /* Parte inferior de la red */
        *vecinoNorte = columna;
    } else {
        /* Control para la busqueda del vecino Norte por parte de cualquier fila */
        *vecinoNorte = ((fila+1)*L)+columna;
    }
    /* Control para la busqueda del vecino Oeste por parte de la columna izquierda */
    if (columna == 0){
        *vecinoOeste = (fila*L)+(L-1);
    } else {
        /* Control para la busqueda del vecino Oeste por parte de cualquier columna */
        *vecinoOeste = (fila*L)+(columna-1);
    }
    /* Control para la busqueda del vecino Este por parte de la columna derecha */
    if (columna == L-1){
        *vecinoEste = (fila*L);
    } else {
        /* Control para la busqueda del vecino Este por parte de cualquier columna */
        *vecinoEste = (fila*L)+(columna+1);
    }
}

/* Funcion para obtener el valor minimo de toda la red */
double obtenerMinimo(int rank, int vecinoNorte, int vecinoSur, int vecinoEste, int vecinoOeste, double bufNumeros){
    /* Menor numero de toda la red iniciado al valor maximo que puede almacenar un double */
    double minimo = DBL_MAX;

    MPI_Status status;
    /* Calculo del valor minimo por filas, enviando al vecino Este el valor almacenado en cada iteracion */
    for(int i = 0; i<L; i++){
        /* Control para actualizar el valor que almacena cada nodo, quedandose siempre con el de menor valor */
        if (bufNumeros<minimo){
            minimo = bufNumeros;
        }
        /* Envio del menor valor al vecino Este */
        MPI_Send(&minimo, 1, MPI_DOUBLE, vecinoEste, i, MPI_COMM_WORLD);
        /* Recepcion del menor valor por parte del vecino Oeste */
        MPI_Recv(&bufNumeros, 1, MPI_DOUBLE, vecinoOeste, i, MPI_COMM_WORLD, &status);
        /* Control para actualizar el valor que almacena cada nodo, quedandose siempre con el de menor valor */
        if(bufNumeros<minimo){
            minimo = bufNumeros;
        }
    }
    /* Calculo del valor minimo por columnas, enviando al vecino Norte el valor almacenado en cada iteracion */
    for(int i = 0; i<L; i++){
        /* Envio del menor valor al vecino Norte */
        MPI_Send(&minimo, 1, MPI_DOUBLE, vecinoNorte, i, MPI_COMM_WORLD);
        /* Recepcion del menor valor por parte del vecino Sur */
        MPI_Recv(&bufNumeros, 1, MPI_DOUBLE, vecinoSur, i, MPI_COMM_WORLD, &status);
        /* Control para actualizar el valor que almacena cada nodo, quedandose siempre con el de menor valor */
        if (bufNumeros<minimo){
            minimo = bufNumeros;
        }
    }

    return minimo;
}