limpiarDirectoriosToroide:
	-rm -f Toroide
	-rm -f datos.dat
	-rm -f obtenerNumerosToroide
	
obtenerNumerosToroide:
	gcc obtenerNumeros.c -o obtenerNumerosToroide
	@read -p "Introduce la cantidad de números necesarios para el toroide: " numeros; \
	./obtenerNumeros $$numeros

compilarToroide: 
	@read -p "Introduce la cantidad de elementos del lado del toroide: " elementos; \
	mpicc -g Toroide.c -o Toroide -DL=$$elementos

ejecutarToroide: 
	@read -p "Introduce la cantidad de procesos a lanzar con el toroide: " procesos; \
	mpirun --oversubscribe -n $$procesos Toroide

ToroideCompleto: limpiarDirectoriosToroide obtenerNumerosToroide compilarToroide ejecutarToroide

limpiarDirectoriosHipercubo:
	-rm -f Hipercubo
	-rm -f datos.dat
	-rm -f obtenerNumerosHipercubo
	
obtenerNumerosHipercubo:
	gcc obtenerNumeros.c -o obtenerNumerosHipercubo
	@read -p "Introduce la cantidad de números necesarios para el hipercubo: " numeros; \
	./obtenerNumeros $$numeros

compilarHipercubo: 
	@read -p "Introduce las dimensiones del hipercubo: " dimensiones; \
	mpicc -g Hipercubo.c -lm -o Hipercubo -DL=$$dimensiones

ejecutarHipercubo: 
	@read -p "Introduce la cantidad de procesos a lanzar con el hipercubo: " procesos; \
	mpirun --oversubscribe -n $$procesos Hipercubo

HipercuboCompleto: limpiarDirectoriosHipercubo obtenerNumerosHipercubo compilarHipercubo ejecutarHipercubo