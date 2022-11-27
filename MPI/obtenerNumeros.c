/* GENERACION DE UN ARCHIVO DE DATOS CON NUMEROS SEPARADOS POR COMAS */
/* Carlos Gomez Fernandez */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Funcion para generar numeros aleatorios en un rango de valores */
double generarNumerosAleatorios(int minimo, int maximo);

int main(int argc, int *argv[]){
    /* Lectura de la entrada estandar en la que se introduce la cantidad de numeros a generar */
    int numerosTotales = atoi(argv[1]);
    /* Apertura del archivo en modo escritura */
    FILE *archivo = fopen("datos.dat", "w");
    /* Creacion de la semilla */
    srand(time(NULL));
    /* Bucle para almacenar en el archivo datos.dat tantos numeros como se hayan introducido por la entrada estandar */
    for (int i=0; i<numerosTotales; i++){
        fprintf(archivo, "%1.2f,", generarNumerosAleatorios(-1000,1000));
    }
}

/* Funcion para generar numeros aleatorios en un rango de valores */
double generarNumerosAleatorios(int minimo, int maximo){
    return (rand() % (maximo - minimo + 1) + minimo)/10.0f;
}