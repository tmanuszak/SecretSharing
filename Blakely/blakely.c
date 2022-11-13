#include "blakely.h"

void free_instance(struct blakely *instance)
{
  if (instance->passedInit != 1)
  { // nothing aside from the struct was allocated
    free(instance);
    return;
  } 
 
  // free RNG
  gmp_randclear(instance->state);

  // free s array
  for (int i = 0; i < instance->t; i++)
  {
    mpz_clear((instance->s)[i]);
  }
  free(instance->s);

  // free p
  mpz_clear(instance->p);

  // free shares matrix
  for (int i = 0; i < instance->n; i++)
  {
    for (int j = 0; j < instance->t; j++)
    {
      mpz_clear((instance->shares)[i][j]);
    }
    free((instance->shares)[i]);
  }
  free(instance->shares);

  free(instance);
  return;
}

struct blakely *init_instance(int t, int n, int lambda)
{
  struct blakely *instance;
  instance = (struct blakely *)malloc(1 * sizeof(struct blakely));

  // set instance state flags
  instance->passedInit = 0;
  instance->hasSecret = 0;
  instance->hasShares = 0;

  if (t > n || t < 2 || lambda < 64 || lambda > 512 || n > 1000)
  {
    printf("Blakely (%d,%d) scheme with security %d is not valid.\n", t, n, lambda);
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

  // init secret array s
  instance->s = (mpz_t *) malloc(t * sizeof(mpz_t));
  for (int i = 0; i < t; i++) {
    mpz_init((instance->s)[i]);
  }

  // init shares
  instance->shares = (mpz_t **) malloc(n * sizeof(mpz_t *));
  for (int i = 0; i < n; i++) {
    (instance->shares)[i] = (mpz_t *) malloc(t * sizeof(mpz_t));
    for (int j = 0; j < t; j++) {
      mpz_init((instance->shares)[i][j]);
    }
  }

  instance->passedInit = 1;

  return instance;
}

void generate_secret(struct blakely *instance)
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
  mpz_urandomb((instance->s)[0], instance->state, instance->lambda);
  
  // set prime p of length lambda and p > s
  mpz_init(instance->p);
  mpz_urandomb(instance->p, instance->state, instance->lambda);
  while (mpz_cmp(instance->p, (instance->s)[0]) <= 0)
  { // while (p <= s) get new p
    mpz_urandomb(instance->p, instance->state, instance->lambda);
  }
  mpz_nextprime(instance->p, instance->p); 

  // make sure p is prime with err probability 1/2^lambda
  int prime_found = 0;
  while (prime_found == 0)
  {
    int test;
    test = mpz_probab_prime_p(instance->p, instance->lambda / 2);
    if (test > 0)
    { // it was prime
      prime_found = 1;
    }
    else if (test == 0)
    { // it was not prime
      mpz_nextprime(instance->p, instance->p);
    }
  }

  // generate s[i] for 1<=i<t. Remember, s is the intersection point.
  for (int i = 1; i < instance->t; i++)
  {
    mpz_urandomm((instance->s)[i], instance->state, instance->p);
  }
  
  instance->hasSecret = 1; // so free_instance knows to free s
  return;
}

void generate_shares(struct blakely *instance)
{
  if (instance->hasShares != 0)
  {
    printf("Cannot generate shares on an Asmuth-Bloom instance that has already got shares, or has not been initialized.\n");
    return;
  }

  // generating shares[i][j] where 0 <= i < n, 0 <= j < t-1
  for (int i = 0; i < instance->n; i++)
  {
    for (int j = 0; j < (instance->t) - 1; j++)
    {
      mpz_urandomm((instance->shares)[i][j], instance->state, instance->p);
    }
  }

  // Computing shares[i][t-1] = s[t-1] - shares[i][0]*s[0] - shares[i][1]*s[1] - ...
  // - shares[t-2]*s[t-2] mod p
  mpz_t temp;
  mpz_init(temp);
  for (int i = 0; i < instance->n; i++)
  {
    mpz_set(temp, (instance->s)[(instance->t) - 1]); // temp = s[t-1]
    for (int j = 0; j < (instance->t) - 1; j++)
    {
      // temp = temp - shares[i][j] * s[j]
      mpz_submul(temp, (instance->shares)[i][j], (instance->s)[j]);

      // temp = temp mod p
      mpz_fdiv_r(temp, temp, instance->p);
    }
    // shares[i][0] = temp
    mpz_set((instance->shares)[i][(instance->t) - 1], temp);
  }
  mpz_clear(temp);

  instance->hasShares = 1;
  
  return;
}

// Not yet implemented
int recover_secret(struct blakely *instance)
{

  return 0;
}

void print_instance(struct blakely *instance)
{
  if (instance->passedInit != 1)
  {
    printf("Instance has not been initialized.\n");
  }

  // print secret array
  if (instance->hasSecret == 1)
  {
    char *s_str;
    char *p_str;
    s_str = mpz_get_str(NULL, 10, (instance->s)[0]);
    printf("Secret: %s\n", s_str);
    free(s_str);
    p_str = mpz_get_str(NULL, 10, instance->p);
    printf("p: %s\n", p_str);
    free(p_str);
    char *s_point;
    printf("Intersection Point: (");
    for (int i = 0; i < instance->t; i++)
    {
      s_point = mpz_get_str(NULL, 10, (instance->s)[i]);
      if (i != (instance->t) - 1)
      {
        printf("%s, ", s_point);
      }
      else
      {
        printf("%s)\n", s_point);
      }
      free(s_point);
    }
  }

  // print shares
  if (instance->hasShares == 1)
  {
    for (int i = 0; i < instance->n; i++)
    {
      printf("Share %d: (", i + 1);
      for (int j = 0; j < instance->t; j++)
      {
        char *temp;
        temp = mpz_get_str(NULL, 10, (instance->shares)[i][j]);
        if (j == (instance->t) - 1)
        {
          printf("%s)\n", temp);
        }
        else
        {
          printf("%s, ", temp);
        }
        free(temp);
      }
    }
  }

  return;
}
