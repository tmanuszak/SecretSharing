#include <gcrypt.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, int* argv[]) {
	char x_string[] = "12345678901234567890";
	char y_string[] = "12345678901234567890";
	char *w_string = (char *) malloc(100 * sizeof(char));
	gcry_mpi_t *x;
	gcry_mpi_t *y;
	gcry_mpi_t *w;
	gcry_error_t error_scan_x = gcry_mpi_scan(x, GCRYMPI_FMT_USG, x_string, 21, NULL);
	gcry_error_t error_scan_y = gcry_mpi_scan(y, GCRYMPI_FMT_USG, y_string, 20, NULL);
	gcry_mpi_add (*w, *x, *y);
	gcry_error_t error_print = gcry_mpi_print(GCRYMPI_FMT_USG, w_string, 100, NULL, w);
	printf("%s\n", w_string);
	free(w);
	gcry_mpi_release(x);
	gcry_mpi_release(y);
	gcry_mpi_release(w);
}
