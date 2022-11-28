#include "shamir.h"

void free_instance(struct shamir *instance)
{
  if (instance->passedInit != 1)
  { // nothing aside from the struct was allocated
    free(instance);
    return;
  } 
 
  // free RNG
  gmp_randclear(instance->state);

	// clear s and shares array
	for (int i = 0; i < instance->t; i++) {
		mpz_clear((instance->s)[i]);
		mpz_clear((instance->shares)[i]);
	}
	free(instance->s);
	free(instance->shares);

	// clear p
	mpz_clear(instance->p);

  free(instance);
  return;
}

struct shamir *init_instance(int t, int n, int lambda)
{

  int64_t tStart,tEnd,tDiff;
  tStart = read_time();

  struct shamir *instance;
  instance = (struct shamir *) malloc(1 * sizeof(struct shamir));

  // set instance state flags
  instance->passedInit = 0;
  instance->hasSecret = 0;
  instance->hasShares = 0;

  if (t > n || t < 2 || lambda < 64 || lambda > 512 || n > 1000)
  {
    printf("Shamir (%d,%d) scheme with security %d is not valid.\n", t, n, lambda);
    free_instance(instance);
    exit(EXIT_FAILURE);
  }

  instance->t = t;
  instance->n = n;
  instance->lambda = lambda;

  // RNG vars
  unsigned long int seed;

  // RNG init
  // get secure random long seed
  getrandom(&seed, sizeof(unsigned long int), GRND_RANDOM);
  gmp_randinit_default(instance->state);
  gmp_randseed_ui(instance->state, seed);

	// secret array allocation
	instance->s = (mpz_t *) malloc(t * sizeof(mpz_t));

	// shares allocation
	instance->shares = (mpz_t *) malloc(n * sizeof(mpz_t));

  // init s and shares values to 0
	for (int i = 0; i < instance->t; i++) {
		mpz_init((instance->s)[i]);
	}
	for (int i = 0; i < instance->n; i++) {
		mpz_init((instance->shares)[i]);
	}

	// init p to 0
	mpz_init(instance->p);
	
	instance->passedInit = 1;

  // BENCHMARKS  
  tEnd = read_time();
  tDiff = tEnd - tStart;
  printf("Shamir,%d,%d,%d,%s,%ld\n",instance->t,instance->n,instance->lambda,__func__,tDiff);

  return instance;
}

void generate_secret(struct shamir *instance)
{
  //BENCHMARKS
  int64_t tStart,tEnd,tDiff;
  tStart = read_time();


  if (instance->passedInit != 1)
  {
    printf("Failed trying to generate secret before instance init.\n");
    free_instance(instance);
    exit(EXIT_FAILURE);
  }
  else if (instance->hasSecret == 1)
  {
    printf("Instance already has a secret.\n");
    return;
  }

  // generate secret
	mpz_urandomb((instance->s)[0], instance->state, instance->lambda);
 
  instance->hasSecret = 1; // so free_instance knows to free s

  // BENCHMARKS  
  tEnd = read_time();
  tDiff = tEnd - tStart;
  printf("Shamir,%d,%d,%d,%s,%ld\n",instance->t,instance->n,instance->lambda,__func__,tDiff);

  return;
}

void generate_shares(struct shamir *instance)
{
  if (instance->hasShares != 0)
  {
    printf("Cannot generate shares on an Shamir instance that has already got shares, or has not been initialized.\n");
    return;
  }

   // BENCHMARKS
   int64_t tStart,tEnd,tDiff;
   tStart = read_time();

	// CHOOSING GALOIS FIELD GF(p)
	// make random p such that s < p < 2^lambda
	mpz_urandomb(instance->p, instance->state, instance->lambda);
	while (mpz_cmp(instance->p, (instance->s)[0]) <= 0) {
		mpz_urandomb(instance->p, instance->state, instance->lambda);
	}
	mpz_nextprime(instance->p, instance->p); // make next prime
	int prime_found = 0;
	while (prime_found == 0) { // make sure p is prime with err prob 1/2^lambda
		int test;
		test = mpz_probab_prime_p(instance->p, (instance->lambda) / 2);
		if (test > 0) { // it was prime
			prime_found = 1;
		} else if (test == 0) {  // it was not prime
			mpz_nextprime(instance->p, instance->p);
		}
	}

	// CHOOSING POLYNOMIAL IN [s[0], ..., s[t-1]] in GF(p)[x^0, ..., x^t-1]
	for (int i = 1; i < instance->t; i++) {
		// s[i] is random number < p
		mpz_urandomm((instance->s)[i], instance->state, instance->p); 
	}

	// Computing shares[i] = poly(i)
	mpz_t term;
	for (int i = 0; i < instance->n; i++) {
		// shares[i] = s[0]
		mpz_set((instance->shares)[i], (instance->s)[0]);
		// term = 1
		mpz_init_set_ui(term, (unsigned long) 1); 
		for (int j = 1; j < instance->t; j++) {
			// term = term * (i+1)
			mpz_mul_ui(term, term, (unsigned long) (i + 1));
			// term = term mod p
			mpz_mod(term, term, instance->p);
			// shares[i] += s[j] * term
			mpz_addmul((instance->shares)[i], (instance->s)[j], term);
			// shares[i] = shares[i] mod p
			mpz_mod((instance->shares)[i], (instance->shares)[i], instance->p); 
		}
	}
	mpz_clear(term);
	
	instance->hasShares = 1;


  // BENCHMARKS  
  tEnd = read_time();
  tDiff = tEnd - tStart;
  printf("Shamir,%d,%d,%d,%s,%ld\n",instance->t,instance->n,instance->lambda,__func__,tDiff);
  
  return;
}

int recover_secret(struct shamir *instance)
{
	if (instance->hasShares != 1) {
		printf("Cannot recover secret if no shares exist.\n");
		free(instance);
		exit(EXIT_FAILURE);
	}

   // BENCHMARKS
   int64_t tStart,tEnd,tDiff;
   tStart = read_time();

	// LAGRANGE INTERPOLATION
	mpz_t result, product, denom;
	mpz_init(result); // result = 0
	mpz_init(product); // product = 0
	mpz_init(denom);  // den = 0
	for (int i = 0; i < instance->t; i++) {
		mpz_set_ui(product, (unsigned long int) 1); // product = 1
		for (int j = 0; j < instance->t; j++) {
			if (j != i) {
				// multiplying the numerator
				// product = product * (j + 1) mod p
				mpz_mul_ui(product, product, (unsigned long int) (j + 1));
				mpz_mod(product, product, instance->p);

				// multiplying the denominator
				// denom = (j+1) - (i+1)
				mpz_set_si(denom, (signed long int) ((j + 1) - (i  +1)));
				// denom = denom mod p
				mpz_fdiv_r(denom, denom, instance->p);
				// denom = denom^-1
				mpz_invert(denom, denom, instance->p);
				// product = product * denom
				mpz_mul(product, product, denom);
			}
		}

		// result = result + shares[i] * product mod p
		mpz_addmul(result, (instance->shares)[i], product);
		mpz_mod(result, result, instance->p);
	}

	int found_secret = 0;
	if (mpz_cmp(result, (instance->s)[0]) == 0) { // SUCCESS!
		found_secret = 1;
	}

	mpz_clear(result);
	mpz_clear(product);
	mpz_clear(denom);


   // BENCHMARKS  
   tEnd = read_time();
   tDiff = tEnd - tStart;
   printf("Shamir,%d,%d,%d,%s,%ld\n",instance->t,instance->n,instance->lambda,__func__,tDiff);

	return found_secret;
}

void print_instance(struct shamir *instance)
{
  if (instance->passedInit != 1)
  {
    printf("Instance has not been initialized.\n");
  }

  // print secret array
  if (instance->hasSecret == 1)
  {
		char *str;
		str = mpz_get_str(NULL, 10, (instance->s)[0]);
		printf("Secret: %s\n", str);
		free(str);
	}

  // print shares
  if (instance->hasShares == 1)
  {
		char *str;

		// print p
		str = mpz_get_str(NULL, 10, instance->p);
		printf("p: %s\n", str);
		free(str);

		// print poly
		printf("Poly: ");
		for (int i = 0; i < instance->t; i++) {
			str = mpz_get_str(NULL, 10, (instance->s)[i]);
			if (i != (instance->t) - 1) {
				printf("%sx^%d + ", str, i);
			} else {
				printf("%sx^%d\n", str, i);
			}
			free(str);
		}

		// print shares
		for (int i = 0; i < instance->n; i++) {
			str = mpz_get_str(NULL, 10, (instance->shares)[i]);
			printf("Share %d: (%d,%s)\n", i + 1, i + 1, str);
			free(str);
		}
	}

  return;
}

int64_t read_time()
{
	  unsigned long lo, hi;
	  __asm__ volatile ( "rdtsc" : "=a" (lo), "=d" (hi) ); 
	  return( lo | (hi << 32) );
}
