#include <gmp.h>
#include <stdio.h>

int main(int argc, int *argv[]) {
	mpz_t x, y, z;
	char *buffer;

	mpz_init(x);
	mpz_init(y);
	mpz_init(z);

	mpz_set_str(x, "11111111111111111111", 10);
	mpz_set_str(y, "22222222222222222222", 10);

	mpz_add(z,x,y);
	buffer = mpz_get_str(NULL, 10, z);

	printf("%s\n", buffer);
	
	return 0;
}
