/* Pract2  RAP 09/10    Javier Ayllon */
/* SISTEMA DISTRIBUIDO DE RENDERIZADO DE GRAFICOS */
/* Carlos Gomez Fernandez */

#include <openmpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h> 
#include <assert.h>   
#include <unistd.h>  

#define NIL (0)       
//#define N_TRABAJADORES 5
//#define FILTRO 0

/* Archivo grafico a renderizar */
#define IMAGEN "foto.dat"
/* Cantidad total de filas que componen el archivo grafico a procesar */
#define N_FILAS 400
/* Cantidad total de columnas que componen el archivo grafico a procesar */
#define N_COLUMNAS 400
/* Dimensiones del archivo grafico a procesar */
#define TAMANIO_FOTO N_FILAS*N_COLUMNAS
/* Desplazamiento en el eje X, a traves de las columnas */
#define X 0
/* Desplazamiento en el eje Y, a traves de las filas */
#define Y 1
/* Color primario rojo */
#define R 2
/* Color primario verde */
#define G 3
/* Color primario azul */
#define B 4
/* Informacion asociada a cada pixel */
#define PIXEL 5

/* Variables Globales */
XColor colorX;
Colormap mapacolor;
char cadenaColor[]="#000000";
Display *dpy;
Window w;
GC gc;

/* Funciones principales empleadas para la resolucion del problema */
void initX();
void dibujaPunto(int x, int y, int r, int g, int b);
void recibirPunto(MPI_Comm commPadre);
void distribuirFilas(int *filasTrabajador, int *filasNoAsignadas, int *areaTrabajador);
void asignarFilas(int rank, int *filaInicial, int *filaFinal, int filasTrabajador, int filasNoAsignadas);
MPI_File aperturaFoto(int rank, int areaTrabajador);
void obtenerPunto(int rank, int filaInicial, int filaFinal, MPI_File foto, MPI_Comm commPadre);

/* Funciones auxiliares empleadas para la resolucion del problema */
void aplicarFiltro(int fila, int columna, unsigned char *colorPunto, MPI_Comm commPadre);
void comprobarPunto(int *bufPunto);

/* Programa principal */
int main(int argc, char *argv[]){
      int rank, size, codigosError[N_TRABAJADORES];
      MPI_Comm commPadre;
      MPI_Status status;

      MPI_Init(&argc, &argv);
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      MPI_Comm_size(MPI_COMM_WORLD, &size);
      MPI_Comm_get_parent(&commPadre);

      if((commPadre==MPI_COMM_NULL) && (rank==0)){
            /* Codigo del maestro */
            /* Funcion para inicializar la ventana en la que se mostrara el archivo grafico */
            initX();
            /* Creacion de los trabajadores y asociacion del programa a ejecutar a cada uno de ellos */
            MPI_Comm_spawn("pract2", MPI_ARGV_NULL, N_TRABAJADORES, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &commPadre, codigosError);
            /* Funcion para recibir la informacion asociada a cada pixel y mostrar el archivo grafico por pantalla */
            recibirPunto(commPadre);
            /* Espera para mostrar el archivo grafico renderizado */
            sleep(4);
      }else{
            /* Codigo de todos los trabajadores */
            /* El archivo sobre el que debemos trabajar es foto.dat */

            int filasTrabajador, filasNoAsignadas, areaTrabajador, filaInicial, filaFinal;
            /* Manejador del archivo grafico */
            MPI_File foto;
            /* Funcion para calcular la carga de trabajo de los trabajadores generados, para realizar el renderizado del archivo grafico */
            distribuirFilas(&filasTrabajador, &filasNoAsignadas, &areaTrabajador);
            /* Funcion para distribuir la carga de trabajo entre los trabajadores generados, para realizar el renderizado del archivo grafico */
            asignarFilas(rank, &filaInicial, &filaFinal, filasTrabajador, filasNoAsignadas);
            /* Funcion para abrir el archivo grafico */
            foto = aperturaFoto(rank, areaTrabajador);
            /* Funcion para obtener la informacion, respecto del color, asociada a cada pixel */
            obtenerPunto(rank, filaInicial, filaFinal, foto, commPadre);
            /* Cierre del archivo grafico */
            MPI_File_close(&foto);
            /* Finalizacion de la ejecucion */
            MPI_Finalize();
            return 0;
      }
}

/* Funcion para inicializar la ventana en la que se mostrara el archivo grafico */
void initX(){

      dpy = XOpenDisplay(NIL);
      assert(dpy);

      int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
      int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

      w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 400, 400, 0, blackColor, blackColor);
      XSelectInput(dpy, w, StructureNotifyMask);
      XMapWindow(dpy, w);
      gc = XCreateGC(dpy, w, 0, NIL);
      XSetForeground(dpy, gc, whiteColor);

      for(;;){
            XEvent e;
            XNextEvent(dpy, &e);
            if(e.type == MapNotify){
                  break;
            }
      }

      mapacolor = DefaultColormap(dpy, 0);
}

/* Funcion para mostrar por pantalla cada uno de los pixeles que componen el archivo grafico */
void dibujaPunto(int x, int y, int r, int g, int b){
      
      sprintf(cadenaColor, "#%.2X%.2X%.2X", r, g, b);
      XParseColor(dpy, mapacolor, cadenaColor, &colorX);
      XAllocColor(dpy, mapacolor, &colorX);
      XSetForeground(dpy, gc, colorX.pixel);
      XDrawPoint(dpy, w, gc, x, y);
      XFlush(dpy);
}

/* Funcion para recibir la informacion asociada a cada pixel y mostrar el archivo grafico por pantalla */
void recibirPunto(MPI_Comm commPadre){
      /* Vector de enteros, con cinco posiciones, que almacena la informacion asociada a cada pixel */
      int bufPunto[PIXEL];
      /* Bucle para la recepcion de la informacion asociada a cada pixel, con tantas iteraciones como pixeles disponga el archivo grafico, para su posterior muestra */
      for(int i = 0; i < TAMANIO_FOTO; i++){
            /* Recepcion de cada uno de los pixeles que conforman el archivo grafico */
            MPI_Recv(&bufPunto, PIXEL, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, commPadre, MPI_STATUSES_IGNORE);
            /* En algun momento dibujamos puntos en la ventana algo como dibujaPunto(x,y,r,g,b); */
            dibujaPunto(bufPunto[X],bufPunto[Y],bufPunto[R],bufPunto[G],bufPunto[B]);
      }
}

/* Funcion para calcular la carga de trabajo de los trabajadores generados, para realizar el renderizado del archivo grafico */
void distribuirFilas(int *filasTrabajador, int *filasNoAsignadas, int *areaTrabajador){
      /* Calcular las filas que procesara cada trabajador */
      *filasTrabajador = N_FILAS/N_TRABAJADORES;
      /* Calcular las filas no asignadas a ningun trabajador, las cuales serán procesadas por el último trabajador */
      *filasNoAsignadas = N_FILAS%N_TRABAJADORES;
      /* Calcular el area que procesara cada trabajador */
      *areaTrabajador = *filasTrabajador*N_COLUMNAS*3*sizeof(unsigned char);
}

/* Funcion para distribuir la carga de trabajo entre los trabajadores generados, para realizar el renderizado del archivo grafico */
void asignarFilas(int rank, int *filaInicial, int *filaFinal, int filasTrabajador, int filasNoAsignadas){
      /* Calcular la fila desde la que comenzaran a trabajar los trabajadores generados, para realizar el renderizado del archivo grafico */
      *filaInicial = rank*filasTrabajador;
      /* Calcular la fila hasta la que trabajaran los trabajadores generados, para realizar el renderizado del archivo grafico */
      if(rank != N_TRABAJADORES-1){
            /* Fila hasta la que trabajaran los trabajadores generados, para realizar el renderizado del archivo grafico */
            *filaFinal = (rank+1)*filasTrabajador;
      }else{
            /* Fila hasta la que trabajara el ultimo de los trabajadores generados, para realizar el renderizado del archivo grafico */
            *filaFinal = (rank+1)*filasTrabajador+filasNoAsignadas;
      }
}

/* Funcion para abrir el archivo grafico */
MPI_File aperturaFoto(int rank, int areaTrabajador){
      /* Manejador del archivo grafico */
      MPI_File foto;
      /* Apertura, por parte de todos los procesos, del archivo grafico "foto.dat" en modo solo lectura */
      MPI_File_open(MPI_COMM_WORLD, IMAGEN, MPI_MODE_RDONLY, MPI_INFO_NULL, &foto);
      /* Asignar el area del archivo grafico visible a cada trabajador generado */
      MPI_File_set_view(foto, rank*areaTrabajador, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, "native", MPI_INFO_NULL);
      
      return foto;
}

/* Funcion para obtener la informacion, respecto del color, asociada a cada pixel */
void obtenerPunto(int rank, int filaInicial, int filaFinal, MPI_File foto, MPI_Comm commPadre){
      /* Vector de chars, con tres posiciones, que almacena la informacion, respecto del color, asociada a cada pixel */
      char colorPunto[3];
      /* Bucle para la recepcion de la informacion, respecto del color de cada pixel, con tantas iteraciones como pixeles disponga el archivo grafico, para su posterior muestra */
      for(int i = filaInicial; i<filaFinal; i++){
            for(int j = 0; j<N_COLUMNAS; j++){
                  /* Lectura y almacenamiento de los valores, respecto del color, asociado a cada uno de los pixeles */
                  MPI_File_read(foto, colorPunto, 3, MPI_UNSIGNED_CHAR, MPI_STATUS_IGNORE);
                  /* Funcion para aplicar un filtro a cada pixel del archivo grafico */
                  aplicarFiltro(i, j, colorPunto, commPadre);
            }
      }
}

/* Funcion para aplicar un filtro a cada pixel del archivo grafico */
void aplicarFiltro(int fila, int columna,  unsigned char *colorPunto, MPI_Comm commPadre){
      /* Vector de enteros, con cinco posiciones, que almacena la informacion asociada a cada pixel */
      int bufPunto[PIXEL];
      /* Valor asociado al desplazamiento en el eje X, a traves de las columnas */
      bufPunto[X] = columna;
      /* Valor asociado al desplazamiento en el eje Y, a traves de las columnas */
      bufPunto[Y] = fila;
      /* Seleccion del filtro a aplicar en  */
      switch(FILTRO){
            /* Imagen original */
            case 0:
                  bufPunto[R] = ((int)colorPunto[0]);
                  bufPunto[G] = ((int)colorPunto[1]);
                  bufPunto[B] = ((int)colorPunto[2]);
                  break;
            /* Filtro escala de grises a aplicar sobre el archivo grafico */
            case 1:
                  bufPunto[R] = (((int)colorPunto[0])*0.299)+(((int)colorPunto[1])*0.587)+(((int)colorPunto[2])*0.114);
                  bufPunto[G] = (((int)colorPunto[0])*0.299)+(((int)colorPunto[1])*0.587)+(((int)colorPunto[2])*0.114);
                  bufPunto[B] = (((int)colorPunto[0])*0.299)+(((int)colorPunto[1])*0.587)+(((int)colorPunto[2])*0.114);
                  break;
            /* Filtro sepia a aplicar sobre el archivo grafico */
            case 2:
                  bufPunto[R] = (((int)colorPunto[0])*0.393)+(((int)colorPunto[1])*0.769)+(((int)colorPunto[2])*0.189);
                  bufPunto[G] = (((int)colorPunto[0])*0.349)+(((int)colorPunto[1])*0.686)+(((int)colorPunto[2])*0.168);
                  bufPunto[B] = (((int)colorPunto[0])*0.272)+(((int)colorPunto[1])*0.534)+(((int)colorPunto[2])*0.131);
                  break;
            /* Filtro negativo a aplicar sobre el archivo grafico */
            case 3:
                  bufPunto[R] = 255-((int)colorPunto[0]);
                  bufPunto[G] = 255-((int)colorPunto[1]);
                  bufPunto[B] = 255-((int)colorPunto[2]);
                  break;
      }
      /* Funcion para comprobar que el valor tras aplicar un filtro es valido, es decir, que se encuentra en el rango de 0 a 255 */
      comprobarPunto(bufPunto);
      /* Envio del pixel con el filtro aplicado al maestro */
      MPI_Send(&bufPunto, PIXEL, MPI_INT, 0, 1, commPadre);
}

/* Funcion para comprobar que el valor tras aplicar un filtro es valido, es decir, que se encuentra en el rango de 0 a 255 */
void comprobarPunto(int *bufPunto){
      /* Comprobacion del valor rojo del pixel */
      if(bufPunto[R]>255){
            bufPunto[R]=255;
      }
      /* Comprobacion del valor verde del pixel */
      if(bufPunto[G]>255){
            bufPunto[G]=255;
      }
      /* Comprobacion del valor azul del pixel */
      if(bufPunto[B]>255){
            bufPunto[B]=255;
      }
}