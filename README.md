Install GMP with ```sudo apt-get install libgmp-dev```.

For GMP, add ```<gmp.h>``` to the top of the c file and compile with the following command:

```gcc myfile.c -o myfile -lgmp```.
