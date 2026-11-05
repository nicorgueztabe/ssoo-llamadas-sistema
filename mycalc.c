#include <fcntl.h> /* Para usar open, read, write, close */
#include <unistd.h>
#include <string.h> //Para usar strcmp y strlen
#include <stdlib.h> //Para usar strtol
#include <errno.h>  //Para usar errno
#include <limits.h> //Para LONG_MAX y LONG_MIN
const char *log_file = "mycalc.log";
int modo_calculadora(char **argv);
int modo_historial(char **argv);
int imprimir_operacion(int fd, char **argv, long resultado);
int write_num(int fd,long n);
int comprobar_overflow(char *s,long *num);

int main(int argc, char **argv) {
    //Mensaje que usaremos siempre en caso de error
        const char *msg =
                        "Recordamos:\n"
                        "MODO CALCULADORA: ./mycalc <num1> <operacion (+|-|x|/)> <num2>\n"
                        "MODO HISTÓRICO: ./mycalc -b <num operacion>\n";

    //Si argc tiene 4 elementos, puede ser modo calculadora, vamos a comprobarlo dentro de la funcion, para lograr mayor cohesion
    //Asi ademas no tendre que 'recalcular' las variables que necesito
    if(argc == 4) return modo_calculadora(argv);
    //Si argc tiene 3 elementos y el segundo elemento de argv es -b, entonces puede ser modo historial, lo comprobaremos
    //tambien dentro de la propia funcion
    else if(argc == 3){
        // strcmp compara dos cadenas de caracteres y devuelve 0 si son iguales. Debemos usarlo porque argv[1] es un puntero
        // que apunta a una cadena de caracteres.
        if(strcmp(argv[1], "-b")!= 0){
            const char *msg1 = "ERROR: Según tu número de elementos introducidos, parece que quieres entrar en modo histórico, pero el flag es incorrecto\n";
            if (write(2,msg1,strlen(msg1))<0) return -1;
            if (write(2,msg,strlen(msg))<0) return -1;
                    return -1;
        }
        else{
        return modo_historial(argv);
        }
    }
    else{
        const char *msg1 = "ERROR: argc NO cuenta con los elementos necesarios para elegir un modo\n";
                if (write(2,msg1,strlen(msg1))<0) return -1;
        if (write(2,msg,strlen(msg))<0) return -1;
        return -1;
    }
  return 0;
}

int modo_calculadora(char **argv){
    //Mensaje que usaremos siempre en caso de error
        const char *msg =
                        "Recordamos:\n"
                        "MODO CALCULADORA: ./mycalc <num1> <operacion (+|-|x|/)> <num2>\n"
                        "MODO HISTÓRICO: ./mycalc -b <num operacion>\n";
    //Primer caracter de argv[2]
        char op = argv[2][0];
        //Segundo caracter de argv[2]
        char op2 = argv[2][1];
        //Comprobamos que no haya mas de un caracter (para evitar "++")
        if(op2 != '\0'){
        const char *msg1 = "ERROR: Operación no definida\n";
        if (write(2, msg1, strlen(msg1)) < 0) return -1;
            if (write(2, msg, strlen(msg)) < 0) return -1;
                return -1;
                }
        //Comprobamos que el primer caracter sea una operacion valida
        if(op != '+' && op !='-' && op != 'x' && op != '/'){
        const char *msg1 = "ERROR: Operación no definida\n";
                if (write(2, msg1, strlen(msg1)) < 0) return -1;
            if (write(2,msg, strlen(msg))<0) return -1;
                return -1;
                }
    //Comprobamos que los dos parametros introducidos (ademas del simbolo de operacion) sean numeros y enteros
        //Para ello usaremos la funcion comprobar_overflow, en la que se hace uso de la funcion strtol (explicada en esta)
        long num1, num2;
    int r1 = comprobar_overflow(argv[1],&num1);
    int r2 = comprobar_overflow(argv[3],&num2);
        //IMPORTANTE, observamos que pasamos la direccion de num1 y num2, esto lo hacemos para que las modificaciones realizadas dentro de la funcion
    //comprobar_overflow les afecten a las variables fuera de esta (quedaran asi asignadas con el string convertido en numero)

    //Para valorar si los operandos son validos o NO, tendremos que atender a lo que devuelve la funcion comprobar_overflow. SI r1 = r2 = 0, todo estara bien.
    if(r1 != 0 || r2 != 0){
        if (r1 == -2 || r2 == -2){
            const char *msg1 = "ERROR OVERFLOW: Operando fuera de rango para long\n";
                    if (write(2, msg1, strlen(msg1)) < 0) return -1;
                        if (write(2,msg, strlen(msg))<0) return -1;
        }
        else{
            const char *msg1 = "ERROR: Sólo puede operarse con dos números enteros\n";
                        if (write(2, msg1, strlen(msg1)) < 0) return -1;
                        if (write(2,msg, strlen(msg))<0) return -1;
        }
        return -1;
               }


    //Una vez finalizadas las comprobaciones, empezamos a programar el funcionamiento, lo mas complicado va a ser el imprimir
    // el resultado por pantalla sin la libreria <stdio>
    // OJO, es posible que, aunque los operandos esten dentro del rango de un long, al operar con ellos se produzca overflow, por lo que tenemos que controlar
    //el overflow en cada operacion.
    // LONG_MAX = 9223372036854775807 y LONG_MIN = -9223372036854775808, es el rango maximo de un tipo long. En caso de overflow, si no lo controlamos, se imprimira
    // ese valor por pantalla
    long resultado;
    if(op == '+'){
        //Overflow si: num1 > LONG_MAX - num2;  num1 < LONG_MIN - num2
        if ((num2 > 0 && num1 > LONG_MAX - num2) ||
                (num2 < 0 && num1 < LONG_MIN - num2)) {
                const char *msg1 = "ERROR: Overflow en suma\n";
                if (write(2, msg1, strlen(msg1)) < 0) return -1;
                return -1;
            }
        resultado = num1 + num2;
    }
    else if(op == '-'){
        //Overflow si: num1 > LONG_MAX + num2;  num1 < LONG_MIN + num2
        if ((num2 < 0 && num1 > LONG_MAX + num2) ||
                (num2 > 0 && num1 < LONG_MIN + num2)) {
                const char *msg1 = "ERROR: Overflow en resta\n";
                if (write(2, msg1, strlen(msg1)) < 0) return -1;
                return -1;
            }
        resultado = num1 - num2;
    }
    else if(op == 'x'){
        //Si multiplico x 0 ya no hay overflow
        if (num1 != 0 && num2 != 0) {
            //Overflow si: num1 > LONG_MAX / num2;  num1 < LONG_MIN / num2
            if (num1 > LONG_MAX / num2 || num1 < LONG_MIN / num2) {
                    const char *msg1 = "ERROR: Overflow en multiplicación\n";
                    if (write(2, msg1, strlen(msg1)) < 0) return -1;
                    if (write(2, msg, strlen(msg)) < 0) return -1;
                    return -1;
                }
        }
        resultado = num1 * num2;
    }
    else if(op=='/'){
        if(num2 == 0){
            const char *msg1 = "ERROR: NO se puede dividir entre 0\n";
                    if (write(2, msg1, strlen(msg1)) < 0) return -1;
            return -1;
        }
        //Caso muy extremo, dividir el LONG_MIN / -1, ya que daria un numero mayor que LONG_MAX
        if (num1 == LONG_MIN && num2 == -1) {
                 const char *msg1 = "ERROR: Overflow en división\n";
                if (write(2, msg1, strlen(msg1)) < 0) return -1;
                return -1;
            }
        else{
            resultado = num1 / num2;
        }
    }
    //Hemos de añadir esta linea para que el programa compile correctamente, ya que si no, nos salta un warning porque puede darse el caso en el que
    // la variable 'resultado' no tenga ningun valor. Podriamos tambien inicializar resultado a 0.
    else {
         return -1;
    }
    //Hacemos una funcion a parte para la impresión de la Operacion para simplificar así el problema. Ademas, dicha funcion la usaremos tambien para escribir en el fichero.
    if (imprimir_operacion(1,argv, resultado)<0) return -1;
    //Si hemos llegado hasta aqui es porque el resultado es un numero valido y correcto, por lo que lo anotamos en el fichero.
        //Al utilizar el flag O_CREATE, si el fichero NO existe, se crea, y si existe, se ignora. Como estamos incluyendo este flag, debemos añadir los permisos.
    //Los permisos seran un tercer parametro de la forma 0644. 0 representa que el numero esta en Octal. Y 644 (rw-r-r) yo puedo leer y escribir y el resto solo leer.
        //El flag O_WRONLY indica solo modo escritura
    //El flag O_APPEND sirva para añadir cada nueva operacion al final del fichero
    //La llamada Open devolvera -1 (fallo) o descriptor de fichero (fd) si acierto.
    int fd = open(log_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
            const char *msg1 = "ERROR: No es posible abrir el fichero";
                if (write(2, msg1, strlen(msg1)) < 0) return -1;
                return -1;
        }
    //Llamamos a la funcion para escribir en el fichero
    if (imprimir_operacion(fd, argv, resultado) < 0) {
        const char *msg1 = "ERROR: No es posible escribir en  el fichero";
                if (write(2, msg1, strlen(msg1)) < 0) return -1;
        //Si hay un fallo al escribir en el fichero, cerramos el fichero y salimos
        close(fd);
        return -1;
    }
    //Finalmente, cerramos el fichero asegurando que todo quede guardado.
    close(fd);
return 0;
}

int imprimir_operacion(int fd, char**argv, long resultado){

    //Al no contar con la libreria <stdio>, tendremos que hacer tambien una funcion que convierta un numero (long) a
    // caracteres y sacarlo con write, para poder imprimir asi el resultado, ya que este NO esta en argv
    //Observamos que NO ponemos \n para que se imprima todo de forma consecutiva
    //OJO, la variable fd indicara si escrbimos por pantalla (1) o en el archivo (fd de open)
    if (write(fd, "Operación:  ", 11) < 0) return -1;
    if (write(fd, " ", 1) < 0) return -1;
    //Sabemos que en argv se encuentran los numeros en modo texto, y podemos imprimirlos directamente.
    //Es cierto que al imprimir literalmente argv[1], se podran imprimir numeros como '008', pero se comento en clase que no se consideraba
    // como un problema. Para evitar esto, podriamos usar la funcion (write_num), pero habria que pasar mas parametros a la funcion
    if (write(fd, argv[1], strlen(argv[1])) < 0) return -1;
    //Tengo que tener en cuenta tambien los espacios de la frase
        if (write(fd, " ", 1) < 0) return -1;
    //En la tercera posicion de argv nos encontramos el signo de la operacion.
        if (write(fd, argv[2], strlen(argv[2])) < 0) return -1;
        if (write(fd, " ", 1) < 0) return -1;
    //En la cuarta posicion de argv nos encontramos el segundo numero de la operacion.
        if (write(fd, argv[3], strlen(argv[3])) < 0) return -1;
        if (write(fd, " = ", 3) < 0) return -1;
    //Ahora viene el problema, y es que el resultado NO esta en argv, asi que utilizamos write_num para pasarlo a modo texto.
        if (write_num(fd, resultado) < 0) return -1;
    //Añadimos al final un \n para que las operaciones en el archivo se guarden en lineas distintas
        if (write(fd, "\n", 1) < 0) return -1;
    return 0;
}

//El objetivo de esta funcion es convertir un long n a texto y escribirlo con write
int write_num(int fd,long n){
    //Nuestra filosofia sera ir hallando el resto de dividir el numero entre 10, e ir almacenando dicho resto en el buf.
    //De tal forma que dentro del buf nos quedara el numero que queremos pasado a texto, pero estara al reves
    char buf[32];
    // i = indice para ir rellenando el array  buf
    int i = 0;
    //Si el numero es 0, el while nunca se ejecuta. Es un caso especial que hay que tratar a parte. Se imprime '0' y termina.
    if (n == 0){
     char c = '0';
     //El segundo parametro del write debe ser un puntero a la memoria que quiero escribir. Como 'c' es un solo caracter, debo pasar
         // la direccion donde se encuentra. Tambien ocurre esto en algunas lineas posteriores.
         if(write(fd, &c, 1) < 0){
         return -1;
     }
    else{
        return 0;
    }
       }
    //Si el numero es negativo, hay que imprimir primero un '-' delante, y el resto del numero se trata igual que un positivo.
    if (n < 0) {
     char menos = '-';
         if (write(fd, &menos, 1) < 0) return -1;
        //Convertimos el numero en positivo
        n = -n;
    }
        // Guardamos los dígitos al revés
    while (n > 0) {
    // n%10 te da el ultimo digito, es el resto de dividir entre 10.
    //Al sumar al caracter '0' cualquier numero del 0-9, obtenemos el codigo ASCII del numero deseado.Se almacena como char en el buf, pues es un array de chars
        buf[i] =(char)('0' + (n % 10));
    //Incrementamos i para guardar el siguiente digito en la siguiente posicion del array
    i++;
    //Tenemos que dividir entre 10 para 'quitar' el digito que ya hemos guardado en buf
        n /= 10;
    }
    // Escribimos al derecho (del final al principio). i contiene el número de elementos del array.
    while (i > 0) {
    i--;
        char c = buf[i];
    //OJO, paso la direccion de c (&c)
        if (write(fd, &c, 1) < 0) return -1;
     }

    return 0;
}

int modo_historial(char **argv){
    //Mensaje que usaremos siempre en caso de error
        const char *msg =
                        "Recordamos:\n"
                        "MODO CALCULADORA: ./mycalc <num1> <operacion (+|-|x|/)> <num2>\n"
                        "MODO HISTÓRICO: ./mycalc -b <num operacion>\n";
    //Ya hemos comprobado anteriormente argc y el flag -b, pero debemos comprobar ahora el numero de linea.
    //Hemos decidido comprobarlo aquí y NO en el main para no tener que volver a hallar alguna variable.
    //Comprobamos que el <num_operacion> sea un numero entero
        //Para ello usamos 'strtol', que va a ir conviertiendo 1 a 1 caracteres en numeros hasta no poder mas (\0 o caracter != de numero)
        //Comprobaremos el valor del puntero end, que apunta automaticamente al final, para determinar el resultado
        char *end;
        //La función strtol devuelve un long, por lo que trabajaremos con long, para evitar asi el overflow (podriamos pasar a int, pero
        //habria que comprobar que no hubiese desbordamiento, y operariamos con numeros mas pequeños (mas limitaciones)

    //No consideramos necesario comprobar si el numero que se introduce para indicar la linea a imprimir tiene overflow, por lo que no llamamos a la
        //funcion comprobar_overflow, y hacemos la comprobacion directamente aqui
    long num1 = strtol(argv[2],&end,10);
        if(*end != '\0'){
                const char *msg1 = "ERROR: La línea a leer debe indicarse con un número entero\n";
                if (write(2, msg1, strlen(msg1)) < 0) return -1;
                if (write(2,msg, strlen(msg))<0) return -1;
                return -1;
               }
    if(num1 <= 0){
        const char *msg1 = "ERROR: El número de línea NO es válido.\n";
                if (write(2, msg1, strlen(msg1)) < 0) return -1;
                return -1;
               }
    //Una vez comprobado que el <num_operacion> es valido, tendremos que comprobar si existe dicha linea e imprimir por pantalla su contenido.
    //Habra que leer el fichero con read hasta llegar a la linea solicitada (si existe).
    long linea_objetivo = num1;
    long linea_actual = 1;
    //Los bytes leidos por el read (lo que devuelve la funcion read). Ponemos de tipo de dato ssize_t porque es lo que devuelve read
    ssize_t  bytes_leidos;
    //Array de 128 (por ejemplo) bytes donde almacenamos los datos leidos. El read copia, desde el fichero al array los datos leidos.
    char buf[128];
    int encontrada = 0;
    //Para saber si llegamos a la linea objetivo
    int encabezado_impreso = 0;
    //Variable para imprimir Linea <x>: para ajustarnos al enunciado

    //La filosofia sera, leer bloques, buscar \n que indican el final de linea y contar lineas para ver si el <num_operacion> es correcto.
    //Abrimos el fichero
    int fd = open(log_file, O_RDONLY);
    if (fd < 0) {
        const char *msg1 = "ERROR: No es posible abrir el fichero\n";
            if (write(2, msg1, strlen(msg1)) < 0) return -1;
            return -1;
        }
    //El read irá leyendo todo el fichero de 128 en 128 bytes, que irá almacenando dentro de buf. Cada vez que se vuelva al while, se sobrescribira lo
    // guardado anteriormente en buf. Asi, da un poco igual el tamaño que pongamos a buf.
    while ((bytes_leidos = read(fd,buf,sizeof(buf))) > 0){
    //Siempre que leamos más de 0 bytes
        for(int i = 0; i < bytes_leidos;i++){
        //Recorremos todo el buf, todo lo que hemos leido en esta pasada del while. Siempre que i sea menor que los bytes leidos.
            if (linea_actual == linea_objetivo) {
                     //Estamos dentro de la línea buscada, imprimimos primero el encabezamiento acorde al enunciado, y una vez escrito, pasamos al caracter actual
                     encontrada = 1;
                if(encabezado_impreso != 1){
                    if (write(1, "Linea ", 6) < 0) {
                         close(fd);
                         return -1;
                    }
                    if (write(1,argv[2],strlen(argv[2])) < 0) {
                    //El <num_operacion> esta en forma de string en argv
                            close(fd);
                            return -1;
                    }
                    if (write(1, ": ", 2) < 0) {
                            close(fd);
                            return -1;
                    }
                encabezado_impreso = 1;
                }
                        if (write(1, &buf[i], 1) < 0){
                    close(fd);
                    return -1;
                    }
            }

                if (buf[i] == '\n') {
                    //Si encontramos fin de linea
                if (linea_actual == linea_objetivo) {
                            // Acabamos de imprimir la línea completa. Esta bien imprimir el \n, asi se hace un salto de linea.
                                close(fd);
                                return 0;
                        }
                //Si encontramos un \n significa que pasamos a la siguiente linea
                         linea_actual++;
                    }
             }
        }

    // Si read devolvio < 0, hubo error de lectura
     if (bytes_leidos < 0) {
            const char *msg1 = "ERROR: Fallo al leer el archivo\n";
            if (write(2, msg1, strlen(msg1)) < 0){
            close(fd);
                    return -1;
        }
            close(fd);
            return -1;
        }

     //Si no se encontró la línea
     if (encontrada == 0) {
        const char *msg1 = "ERROR: El número de línea NO es válido\n";
                if (write(2, msg1, strlen(msg1)) < 0){
                        close(fd);
                        return -1;
                }
                close(fd);
                return -1;
    }

    close(fd);

    return 0;
}

int comprobar_overflow(char *s,long *num){
    //Comprobamos que el operando introducido sea un numero, entero, y dentro del rango
        //Para ello usamos 'strtol', que va a ir conviertiendo 1 a 1 caracteres en numeros hasta no poder mas (\0 o caracter != de numero)
        //Comprobaremos el valor del puntero end, que apunta automaticamente al final, para determinar el resultado
        char *end;

    errno = 0;
        //Limpiamos la variable errno antes de usarla, porque strtol la usa. Nos servira para identificar el overflow

        //La función strtol devuelve un long, por lo que trabajaremos con long, para tener un mayor rango antes del overflow (podriamos pasar a int, pero
        //habria que comprobar que no hubiese desbordamiento, y operariamos con numeros mas pequeños (mas limitaciones)
        //El tercer parametro (10) es por numero decimal.
        *num  = strtol(s,&end,10);
    // IMPORTANTE. Trabajamos con el contenido (*num) de la dirección pasada (&variable long), para que asi todas las modificaciones que sufra la
    // variable dentro de la funcion, se apliquen fuera de esta.

        //Comprobamos que NO haya basura despues del numero. Evitamos casos como 56fg8, 87.6...
        if(*end != '\0'){
                return -1;
               }
    //Comprobamos que el numero NO sea demasiado grande ni demasiado pequeño, en el caso en el que lo sean, devolveremos -2 para poder
    //tratar el caso concreto. ERANGE es un codigo estandar de C que significa Error de Rango
    if(errno == ERANGE){
                return -2;
               }
    return 0;
}
