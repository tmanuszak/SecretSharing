// Code implementing Asmuth-Bloom secret sharing
// For Arizona State University's CSE539: Applied Cryptography course.

#include "asmuthbloom.h"

// Free an Asmuth-Bloom instance
void free_instance(struct asmuth_bloom *instance)
{
  // instance never passed init, i.e. just free instance memory
  if (instance->passedInit == 0)
  {
    free(instance);
    return;
  }
  else if (instance->passedInit != 1)
  { // likely never allocated instance at all
    return;
  }

  // free m
  if (instance->hasM != 0)
  {
    for (int i = 0; i <= instance->n; i++)
    {
      mpz_clear((instance->m)[i]);
    }
    free(instance->m);
    instance->hasM = 0;
  }

  if (instance->hasShares != 0)
  {
    for (int i = 0; i < instance->n; i++)
    {
      mpz_clear((instance->shares)[i]);
    }
    free(instance->shares);
    instance->hasShares = 0;
  }

  // free s and alpha
  mpz_clear(instance->s);
  mpz_clear(instance->alpha);

  free(instance);
  return;
}

// Allocates, initializes, and returns Asmuth-Bloom secret sharing instance
// with parameters (t,n,lambda). Parameters must be in the following range:
// 2 <= t <= n <= 1000
// 64 <= lambda <= 512
struct asmuth_bloom *init_instance(int t, int n, int lambda)
{
  struct asmuth_bloom *instance;
  instance = (struct asmuth_bloom *)malloc(1 * sizeof(struct asmuth_bloom));

  // set instance state flags
  instance->passedInit = 0;
  instance->hasM = 0;
  instance->hasSecret = 0;
  instance->hasShares = 0;

  // Make sure we have parameters t and n such that t <= n
  if (t > n || t < 2 || lambda < 64 || lambda > 512 || n > 1000)
  {
    printf("Asmuth-Bloom (%d,%d) scheme with security %d is not valid.\n", t, n, lambda);
    free_instance(instance);
    exit(EXIT_FAILURE);
  }

  instance->t = t;
  instance->n = n;
  instance->lambda = lambda;
  instance->passedInit = 1;
  return instance;
}

// generate random secret of length lambda
void generate_secret(struct asmuth_bloom *instance)
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

  // RNG vars
  gmp_randstate_t state;
  unsigned long int seed;

  // RNG init
  // get secure random long seed
  getrandom(&seed, sizeof(unsigned long int), GRND_RANDOM);
  gmp_randinit_default(state);  // init RNG
  gmp_randseed_ui(state, seed); // init RNG seed

  // getting random secret
  mpz_init(instance->s);
  mpz_urandomb(instance->s, state, instance->lambda);
  instance->hasSecret = 1;

  return;
}

// Find the next prime after num with err probability 1/(2^lambda)
// Set it to set
void get_next_prime(mpz_t *set, mpz_t num, int lambda)
{
  mpz_t *prime = (mpz_t *)malloc(1 * sizeof(mpz_t));
  mpz_init(*prime);
  mpz_nextprime(*prime, num);

  int prime_found = 0;
  while (prime_found == 0)
  {
    int test;
    test = mpz_probab_prime_p(*prime, lambda / 2);
    if (test > 0)
    {
      prime_found = 1;
    }
    else if (test == 0)
    {
      mpz_nextprime(*prime, num);
    }
  }

  mpz_set(*set, *prime);
  return;
}

// This has been thoroughly tested to always pass, not necessary to run anymore.
// This will check two necessary properties of m.
// First, m_0 < m_1 < ... < m_n
// Second, m_0 * prod_{i = n-k+2}^n (m_i) < prod_{i = 1}^k (m_i)
// See wikipedia
void check_m(struct asmuth_bloom *instance)
{
  if (instance->hasM != 1)
  {
    printf("Trying to check m on Asmuth-Bloom instance and it has no m array.\n");
    return;
  }

  for (int i = 0; i < instance->n; i++)
  {
    if (mpz_cmp((instance->m)[i], (instance->m)[i + 1]) >= 0)
    {
      printf("Failed less than check on (%d,%d) and lambda=%d\n",
             instance->t,
             instance->n,
             instance->lambda);
      free_instance(instance);
      exit(EXIT_FAILURE);
    }
  }

  mpz_t lhs;
  mpz_t rhs;
  mpz_init(lhs);
  mpz_init(rhs);
  mpz_set_ui(lhs, (unsigned long int)1);
  mpz_set_ui(rhs, (unsigned long int)1);
  for (int i = instance->n - instance->t + 2; i <= instance->n; i++)
  {
    mpz_mul(lhs, lhs, (instance->m)[i]);
  }
  for (int i = 1; i <= instance->t; i++)
  {
    mpz_mul(rhs, rhs, (instance->m)[i]);
  }
  if (mpz_cmp(lhs, rhs) >= 0)
  {
    printf("Failed lhs < rhs chech on (%d,%d) and lambda=%d\n",
           instance->t,
           instance->n,
           instance->lambda);
    free_instance(instance);
    exit(EXIT_FAILURE);
  }

  return;
}

// generates shares for Asmuth-Bloom instance
void generate_shares(struct asmuth_bloom *instance)
{
  if (instance->hasShares != 0 && instance->hasM != 0)
  {
    printf("Cannot generate shares on an Asmuth-Bloom instance that has already got shares, or has not been initialized.\n");
    return;
  }

  // init a temp and ub variable
  mpz_t temp;
  mpz_init(temp);
  // init an upper bound to alpha variable
  mpz_t ub;
  mpz_init(ub);
  // init alpha
  mpz_init(instance->alpha);
  // init and malloc shares
  instance->shares = (mpz_t *)malloc(instance->n * sizeof(mpz_t));
  for (int i = 0; i < instance->n; i++)
  {
    mpz_init((instance->shares)[i]);
  }
  instance->hasShares = 1;
  // init and malloc m
  instance->m = (mpz_t *)malloc((instance->n + 1) * sizeof(mpz_t));
  for (int i = 0; i <= instance->n; i++)
  {
    mpz_init((instance->m)[i]);
  }
  instance->hasM = 1;

  // gnereating m_0 and m_1
  get_next_prime(&((instance->m)[0]), instance->s, instance->lambda); // m_0 = next prime after s
  mpz_mul_si(temp, (instance->m)[0], (long int)2);                    // temp = s * 2
  get_next_prime(&((instance->m)[1]), temp, instance->lambda);        // m_1 = next prime after 2*m_0

  // iteratively generating m_2...m_n from m_(i-1)
  for (int i = 2; i <= instance->n; i++)
  {
    get_next_prime(&((instance->m)[i]), (instance->m)[i - 1], instance->lambda);
  }

  check_m(instance); // Not necessary. Trey has shown always mathematically passes.

  // get an upper bound for a
  mpz_set_ui(ub, (unsigned long int)1);
  for (int i = 1; i <= instance->t; i++)
  {
    mpz_mul(ub, ub, (instance->m)[i]);
  }
  mpz_sub(ub, ub, instance->s);
  mpz_cdiv_q(ub, ub, (instance->m)[0]);

  // generate a random alpha
  gmp_randstate_t state; // random number generator state
  unsigned long int seed;
  getrandom(&seed, sizeof(unsigned long int), GRND_RANDOM); // get secure random long seed
  gmp_randinit_default(state);                              // init RNG
  gmp_randseed_ui(state, seed);                             // init RNG seed
  mpz_urandomm(instance->alpha, state, ub);

  // generate shares
  for (int i = 0; i < instance->n; i++)
  {
    mpz_set((instance->shares)[i], instance->s); // shares[i] = s
    // shares[i] = shares[i] + alpha * m[0]
    mpz_addmul((instance->shares)[i], instance->alpha, (instance->m)[0]);
    // shares[i] = shares[i] mod m[i+1]
    mpz_mod((instance->shares)[i], (instance->shares)[i], (instance->m)[i + 1]);
  }

  mpz_clear(temp);
  mpz_clear(ub);
  return;
}

// not yet implemented
// output 1 if secret successfully recovered. 0 else.
int recover_secret(struct asmuth_bloom *instance)
{
  return 0;
}

void print_instance(struct asmuth_bloom *instance)
{
  if (instance->passedInit == 1)
  {
    printf("t = %d, n = %d, lambda = %d\n", instance->t, instance->n, instance->lambda);
  }
  else
  {
    printf("Not initialized.");
    return;
  }

  if (instance->hasSecret == 1)
  {
    char *s_str = mpz_get_str(NULL, 10, instance->s);
    printf("s: %s\n", s_str);
    free(s_str);
  }
  else
  {
    printf("No secret generated.\n");
    return;
  }

  if (instance->hasShares == 1)
  {
    char *alpha_str = mpz_get_str(NULL, 10, instance->alpha);
    printf("alpha: %s\n", alpha_str);
    free(alpha_str);

    char *shares_str;
    char *m_str;
    m_str = mpz_get_str(NULL, 10, (instance->m)[0]);
    printf("m0: %s\n", m_str);
    free(m_str);
    for (int i = 0; i < instance->n; i++)
    {
      shares_str = mpz_get_str(NULL, 10, (instance->shares)[i]);
      m_str = mpz_get_str(NULL, 10, (instance->m)[i + 1]);
      printf("Share %d: (share%d,m%d) = (%s, %s)\n", i + 1, i, i + 1, shares_str, m_str);
      free(shares_str);
      free(m_str);
    }
  }
  else
  {
    printf("No shares generated.");
    return;
  }

  return;
}
