#include <gmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/random.h>

// not yet implemented
mpz_t* generate_secret(int lambda) {
	mpz_t *s; // secret
	gmp_randstate_t state; // random number generator state
	unsigned long int seed;

	getrandom(&seed, sizeof(unsigned long int), GRND_RANDOM); // get secure random long seed
	gmp_randinit_default(state); // init RNG
	gmp_randseed_ui(state, seed); // init RNG seed

	s = (mpz_t *) malloc(1 * sizeof(mpz_t));
	mpz_init(*s);
	mpz_urandomb(*s, state, lambda);
	return s;
}

// not yet implemented
mpz_t** generate_shares() {
	mpz_t **shares;
	return shares;
}

// not yet implemented
mpz_t* recover_secret() {
	mpz_t *s = (mpz_t *) malloc(1 * sizeof(mpz_t));
	mpz_init(*s);
	return s;
}

int main(int argc, char *argv[]) {
	int t; // threshold
	int n; // number of parties
	int lambda; // security parameter
	
	// Make sure we have parameters t and n such that t <= n
	if (argc > 3) {
		t = (int) strtol(argv[1], NULL, 10);
		n = (int) strtol(argv[2], NULL, 10);
		lambda = (int) strtol(argv[3], NULL, 10);
		if (t > n) {
			printf("Mignotte (%d,%d) scheme is not valid.\n", t, n);
			return -1;
		}
	} else {
		printf("Must input a threshold, number of parties, and security parameter..\n");
		return -1;
	}

	mpz_t *s = generate_secret(lambda);
	char *s_str = mpz_get_str(NULL, 10, *s);
	printf("s: %s\n", s_str);

	mpz_t **shares = generate_shares();
	mpz_t *recovered_s = recover_secret();

	// frees
	free(s_str);
	mpz_clear(*s);
	free(s);
	mpz_clear(*recovered_s);
	free(recovered_s);
	
	return 0;
}
