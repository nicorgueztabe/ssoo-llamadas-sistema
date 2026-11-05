#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h> // Para usar open
#include <unistd.h> // Para usar lseek, , read, write, close
#include <dirent.h>  // Para usar opendir, readdir, closedir
#include <linux/limits.h>  // Para usar PATH_MAX (el tamaño máximo de una ruta)

/* nombre del fichero binario a escribir/leeer */
const char *binary_file = "mydu.bin";
// Creamos nuestro propio tipo de variable mediante la función struct
struct info_directorio {
    long tamano;               // Para guardar el tamaño
    char path[PATH_MAX];       // Para guardar la ruta completa
};
//La funcion tamano_directorio recibira una direccion de memoria de un string, un puntero. Añadimos ademas 'const' porque NO se debe modificar los caracteres de la ruta en la funcion.
long tamano_directorio(const char*path);
//En la funcion imprimir binario NO vamos a pasar nignun parametro
int imprimir_binario(void);


int main(int argc, char **argv) {
    //En funcion de lo que introduzca el usuario, implementaremos un modo u otro. Controlaremos la eleccion de los modos en el main.
    //1º.1 MODO. Si no se pasa ningun argumento significa que queremos calcular el tamaño del directorio actual.
    if(argc == 1){
        //1º.1 MODO
        if (tamano_directorio(".") < 0) return -1;
    return 0;
    }
    //1º.2 MODO. Si paso un diretorio como argumento (argc ==2), significa que queremos calcular el tamaño de dicho directorio.Dentro de la propia funcion ya comprobaremos
    //si el argumento pasado es el adecuado.
    //2º MODO. Si paso como argumento el flag -b (argc == 2), significa que se quiere mostrar por pantalla el contenido del fichero binario  mydu.bin, donde se han ido
    //almacenando los resultados anteriores.
    if(argc == 2){
        // strcmp compara dos cadenas de caracteres y devuelve 0 si son iguales. Debemos usarlo porque argv[1] es un puntero que apunta a una cadena de caracteres.
    if(strcmp(argv[1], "-b") == 0){
            if (imprimir_binario() < 0) return -1;
      return 0;
        }
        else{
            //1º.2 MODO. Comprobaremos dentro de la funcion si el argumento pasado es correcto
            if (tamano_directorio(argv[1]) < 0) return -1;
      return 0;
        }
    }
    fprintf(stderr, "ERROR: Has introducido un número de argumentos incorrecto\n");
  fprintf(stderr, "Recordamos:\nMODO 1: ./mydu [<directorio>]\nMODO 2: ./mydu [-b]\n");
  return -1;
}

//Desarrollamos la funcion para calcular el tamaño de un directorio
long tamano_directorio(const char *path){
    //Lo primero es validar si el path pasado es correcto o no, es decir, si existe y si es un directorio
    //Utilizaremos la llamada al sistema 'stat', que pregunta por el archivo/directorio de un path dado y guarda la info en una estructura (un tipo que agrupa varias variables)
    struct stat estado;
    // La estructura struct stat es una estructura ya definida por el sistema. Estamos creando una variable (estado) de la estructura del sistema.
    //Primero comprobamos que path exista y tengamos permisos para consultarlo
    if (stat(path, &estado) < 0) {
        perror("ERROR: el directorio no se encuentra");
        return -1;
    }
    // La informacion del fichero se almacenara en la variable estado
    // Segundo comprobamos que path NO sea un fichero
    if (S_ISREG(estado.st_mode)) {
        fprintf(stderr, "ERROR: %s es un fichero, no un directorio\n", path);
            return -1;
    }
    // Finalmente comprobamos que path SI sea un directorio
    if (!S_ISDIR(estado.st_mode)) {
            fprintf(stderr, "ERROR: %s no es un directorio\n", path);
            return -1;
    }
  // Abrimos el directorio
  DIR*dir = opendir (path);
  // Comprobacion de error al abrirlo, si no se encuentra se cierra el programa
  if (dir == NULL) {
    perror ("ERROR: el directorio no se encuentra");
    return -1;
  }
  // Establecemos el molde para guardar la informacion del archivo encontrado
  // El puntero * apunta a la direccion del archivo de entrada
  struct dirent *archivo_encontrado;
  // Variable que acumula el tamano total del directorio
  // La inicializamos en estado.st_size en vez de en 0 para que realice la misma funcion que du
  // Inicializandola en 0 no tendria en cuenta el "peso" del propio directorio en el que se almacenan los archivos
  long tamano_total = estado.st_size;

  // Bucle que hace la cuenta del tamano del directorio mientras este no este vacio
  while ((archivo_encontrado = readdir(dir)) != NULL) {
    // Ignoramos . (la carpeta actual) y .. (la carpeta padre)
    // Asi evitamos que nuestro programa entre en un bucle infinito abriendo repetidamente la misma carpeta
    if (strcmp(archivo_encontrado -> d_name, ".") == 0 || strcmp(archivo_encontrado->d_name, "..") == 0) {
      continue;
      }
    // Creamos la variable vacia para guardar la ruta completa
    char ruta_nueva[PATH_MAX];
    // Utilizamos snprintf para generar e imprimir el texto de la ruta, la barra "/"" y el nombre.
    // Evitamos un posible desbordamiento al pasarle sizeof(ruta_nueva)
    snprintf(ruta_nueva, sizeof(ruta_nueva), "%s/%s", path, archivo_encontrado->d_name);

    // Estructura para almacenar los datos del elemento actual
    struct stat estado_nuevo;
    // Comprobacion del estado de la ruta
    // Si la ruta no encuentra el archivo (o no tiene permisos para abrirlo) da error y sale de la ejecucion
    if (stat(ruta_nueva, &estado_nuevo) < 0) {
      perror("ERROR: Fallo al obtener la informacion del archivo");
      closedir(dir);
      return -1;
      }

    // Toma de decision basada en el tipo de elemento
    // A: SI es un directorio
    if (S_ISDIR(estado_nuevo.st_mode)) {
      // Llamamos de manera recursiva a nuestra propia función pasándole la nueva ruta.
      long sub_tamano = tamano_directorio(ruta_nueva);

      // Control de errores de la llamada recursiva
      // Si el directorio no se puede abrir lanzamos un error y salimos
      if (sub_tamano < 0) {
        closedir(dir);
        return -1;
        }
      // Sumamos lo que pesa el directorio encontrado a nuestro acumulador total
      tamano_total = tamano_total + sub_tamano;
      }

    // B: SI es un fichero
    else if (S_ISREG(estado_nuevo.st_mode)) {
      // Abrimos el fichero en modo solo lectura
      int fd_archivo = open(ruta_nueva, O_RDONLY);
      // Si el archivo no se puede abrir lanzamos un error y salimos
      if (fd_archivo < 0) {
        perror("ERROR: Fallo al obtener la informacion del archivo");
        closedir(dir);
        return -1;
        }
      // Movemos el puntero al final del archivo con lseek, que nos devuelve la cantidad de bytes que ha tenido que saltar
      // El desplazamiento coincide con el tamano del archivo que es lo que buscamos
      long tamano_archivo = lseek(fd_archivo, 0, SEEK_END);
      // Si ocurre un error al llamar a lseek lanzamos el error y retornamos
      // Hay que cerrar el archivo y el directorio para que el sistema operativo no tenga una fuga de memoria
      if (tamano_archivo < 0) {
        perror("ERROR: Fallo al calcular el tamaño con lseek");
        close(fd_archivo);
        closedir(dir);
        return -1;
          }
      // Cerramos el fichero en el que nos encontramos despues de la llamada lseek
      close(fd_archivo);
      // Sumamos los bytes obtenidos a nuestro total
      tamano_total = tamano_total + tamano_archivo;
        }
  }
    // Una vez que el bucle termina cerramos el directorio para liberar la memoria del sistema operativo.
    closedir(dir);
    // Convertimos el tamano total a KB
    // Redondeamos hacia arriba como hace la funcion du
    long tamano_total_kb = (tamano_total + 1023) / 1024;

    // Para completar la funcion imprimimos el tamano total en KB y el camino que ha recorrido
    printf ("%ld \t %s\n", tamano_total_kb, path);
    // Preparamos los datos del archivo que queremos guardar con el struct realizado al inicio del programa
    struct info_directorio registro_directorio;
    // Guardamos la variable ya convertida a KB
    registro_directorio.tamano = tamano_total_kb;
    // Copiamos la ruta
    // Utilizamos strncpy en vez de strcpy para asegurar que la ruta sea menor al tamano maximo posible de PATH_MAX
    strncpy(registro_directorio.path, path, PATH_MAX - 1);
    // Nos aseguramos de que el camino maximo acabe en 0 para evitar desbordamientos
    registro_directorio.path[PATH_MAX - 1] = '\0';

    // Pedimos permiso al disco duro para acceder al archivo mydu.bin (guardado previamente en binary_file)
    // Al utilizar el flag O_CREATE, si el archivo NO existe, se crea, y si existe, se ignora. Como estamos incluyendo este flag, debemos añadir los permisos.
      // Los permisos seran un tercer parametro de la forma 0644. 0 representa que el numero esta en Octal. Y 644 (rw-r-r) yo puedo leer y escribir y el resto solo leer.
    // El flag O_WRONLY indica solo modo escritura
      // El flag O_APPEND sirva para añadir cada nueva operacion al final del fichero
      // La llamada Open devolvera -1 (fallo) o descriptor de fichero (fd) si acierto.
    int fd_bin = open(binary_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
      if (fd_bin < 0) {
        perror("ERROR: No se pudo abrir o cerrar el archivo en binario");
        return -1;
        }

    // Escribimos la estructura en fomato binario en el historial
    // Lo hacemos con write para volcar directamente el contenido de nuestra variable
    if (write(fd_bin, &registro_directorio, sizeof(registro_directorio)) < 0) {
      perror("ERROR al escribir el binario");
      close(fd_bin);
      return -1;
      }

    // Cerramos el descriptor de fichero (fd) para liberar espacio del sistema operativo
    close (fd_bin);
    // Terminamos devolviendo el valor que se nos solicita en bytes
    // Se retorna en bytes para que al hacer la llamada recursiva el programa sume correctamente el tamano_total
    return tamano_total;
}

//Desarrollamos la funcion para imprimir el archivo binario mydu.bin
int imprimir_binario(void){
    //Abrimos el archivo en modo lectura
    int fd = open("mydu.bin", O_RDONLY);

    //comprobamos si hay algun error al abrir el archivo
    if (fd < 0) {
        perror("ERROR: No se pudo abrir el archivo en binario");
        return -1;
        }

    //preparamos la variable para leer los daros
    struct info_directorio elemento_leido;

    // Imprimimos una cabecera opcional para que quede bonito (como en el ejemplo del Word)
    printf("--- Contenido del archivo binario ---\n");

    // Bucle de lectura hasta llegar al final del archivo
    while (read(fd, &elemento_leido, sizeof(elemento_leido)) > 0) {
        //Imprimimos el contenido de la esctructura actaul
        printf("%ld \t %s\n", elemento_leido.tamano, elemento_leido.path);
    }
    //Cerramos el archivo
    close (fd);
    return 0;
}
