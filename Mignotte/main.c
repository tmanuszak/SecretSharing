#include <gmp.h>
#include <ctype.h>

// not yet implemented
mpz_t generate_secret() {
	mpz_t s;
	mpz_init(s);
	return s
}

// not yet implemented
*mpz_t generate_shares() {
	mpz_t *shares;
	return shares
}

// not yet implemented
mpz_t recover_secret() {
	mpz_t s;
	mpz_init(s);
	return s
}

int main(int argc, int *argv[]) {
	int t, n;
	
	// Make sure we have parameters t and n such that t <= n
	if (argc > 2) {
		t = atoi(argv[1]);
		n = atoi(argv[2]);
		if (t > n) {
			printf("Mignotte (%d,%d) scheme is not valid.\n", t, d);
		}
	else {
		printf("Must input a threshold and number of parties.\n");
	}

	generate_secret();
	generate_shares();
	recover_secret();

	return 0;
}
