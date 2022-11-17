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

  free(instance);
  return;
}

struct shamir *init_instance(int t, int n, int lambda)
{
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

  instance->passedInit = 1;

  return instance;
}

void generate_secret(struct shamir *instance)
{
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
	
  
  instance->hasSecret = 1; // so free_instance knows to free s
  return;
}

void generate_shares(struct shamir *instance)
{
  if (instance->hasShares != 0)
  {
    printf("Cannot generate shares on an Shamir instance that has already got shares, or has not been initialized.\n");
    return;
  }

  instance->hasShares = 1;
  
  return;
}

int recover_secret(struct shamir *instance)
{
	if (instance->hasShares != 1) {
		printf("Cannot recover secret if no shares exist.\n");
		free(instance);
		exit(EXIT_FAILURE);
	}

	int found_secret = 0;

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
  
	}

  // print shares
  if (instance->hasShares == 1)
  {
  
	}

  return;
}
