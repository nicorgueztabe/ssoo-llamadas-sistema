# Operating Systems: system calls practice

Two command-line tools written in C directly on top of the POSIX system-call interface (`open`, `read`, `write`, `lseek`, `close`, `stat`, `opendir`, `readdir`). They use no buffered file I/O (`fopen`, `fread`, `fwrite`), and `mycalc` does not even include `stdio.h`.

## Build

Linux only.

```
gcc -Wall -Wextra -o mycalc mycalc.c
gcc -Wall -Wextra -o mydu mydu.c
```

## mycalc

A calculator with a history.

```
./mycalc <num1> <+|-|x|/> <num2>     calculate and log the operation
./mycalc -b <line>                   print one line of the log
```

- The operands are parsed with `strtol` and checked against the range of a `long`. The result is turned into text by hand and printed with `write`.
- Every successful operation is appended to `mycalc.log`. Errors go to stderr, are not logged, and make the program exit with -1 (255 in the shell).

```
$ ./mycalc 5242 x 3442
Operación: 5242 x 3442 = 18042964
$ ./mycalc 8374 + 1121
Operación: 8374 + 1121 = 9495
$ ./mycalc 3 / 21
Operación: 3 / 21 = 0
$ ./mycalc 8374 / 0
ERROR: NO se puede dividir entre 0
$ ./mycalc 8 ^ 2
ERROR: Operación no definida
Recordamos:
MODO CALCULADORA: ./mycalc <num1> <operacion (+|-|x|/)> <num2>
MODO HISTÓRICO: ./mycalc -b <num operacion>
$ cat mycalc.log
Operación: 5242 x 3442 = 18042964
Operación: 8374 + 1121 = 9495
Operación: 3 / 21 = 0
$ ./mycalc -b 1
Linea 1: Operación: 5242 x 3442 = 18042964
$ ./mycalc -b 3
Linea 3: Operación: 3 / 21 = 0
$ ./mycalc -b -1
ERROR: El número de línea NO es válido.
$ ./mycalc -b 10
ERROR: El número de línea NO es válido
```

## mydu

Computes the size of a directory and everything below it, and keeps a binary history of what it has analysed.

```
./mydu [<directory>]     size of a directory (the current one by default)
./mydu -b                print the history stored in mydu.bin
```

- The tree is walked recursively with `opendir` and `readdir`. The size of each regular file is measured with `lseek(fd, 0, SEEK_END)`, added to the size of the directory itself, and printed in KB, rounded up. Subdirectories are printed before their parents.
- Each directory printed is also appended to `mydu.bin` as a fixed-size record (a `long` and a `char path[PATH_MAX]`) with `write`. `./mydu -b` reads the records back with `read`.
- Regular files and paths that do not exist are rejected with an error on stderr and exit code -1.
- It adds up exact file sizes, not disk blocks, so the numbers can differ from `du -k`.

In the example the size of `.` grows between the first two runs because `mydu.bin` is created inside it.

```
$ ./mydu
21 	 ./folder1/folder2
5 	 ./folder1/folder3
59 	 ./folder1
80 	 .
$ ./mydu .
21 	 ./folder1/folder2
5 	 ./folder1/folder3
59 	 ./folder1
108 	 .
$ ./mydu folder1/folder2
21 	 folder1/folder2
$ ./mydu mydu.bin
ERROR: mydu.bin es un fichero, no un directorio
$ ./mydu noexiste
ERROR: el directorio no se encuentra: No such file or directory
$ ./mydu -b
--- Contenido del archivo binario ---
21 	 ./folder1/folder2
5 	 ./folder1/folder3
59 	 ./folder1
80 	 .
21 	 ./folder1/folder2
5 	 ./folder1/folder3
59 	 ./folder1
108 	 .
21 	 folder1/folder2
```
