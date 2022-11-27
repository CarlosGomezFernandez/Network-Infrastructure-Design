limipiarDirectorios:
	-rm -f pract2

compilarSinFiltro:
	@read -p "Introduce la cantidad de procesos trabajadores a levantar: " trabajadores; \
	mpicc pract2.c -o pract2 -lX11 -DN_TRABAJADORES=$$trabajadores -DFILTRO=0

compilarFiltroBlancoYNegro:
	@read -p "Introduce la cantidad de procesos trabajadores a levantar: " trabajadores; \
	mpicc pract2.c -o pract2 -lX11 -DN_TRABAJADORES=$$trabajadores -DFILTRO=1

compilarFiltroSepia:
	@read -p "Introduce la cantidad de procesos trabajadores a levantar: " trabajadores; \
	mpicc pract2.c -o pract2 -lX11 -DN_TRABAJADORES=$$trabajadores -DFILTRO=2

compilarFiltroNegativo:
	@read -p "Introduce la cantidad de procesos trabajadores a levantar: " trabajadores; \
	mpicc pract2.c -o pract2 -lX11 -DN_TRABAJADORES=$$trabajadores -DFILTRO=3

ejecutarRenderizado:
	mpirun --oversubscribe -np 1 ./pract2