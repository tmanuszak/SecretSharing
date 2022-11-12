// Instance of Asmuth-Bloom secret sharing scheme.
#ifndef ASMUTH_BLOOM_HEADER
#define ASMUTH_BLOOM_HEADER

#include <gmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/random.h>

struct asmuth_bloom
{
  int t;      // threshold
  int n;      // number of participants
  int lambda; // security parameter

  // flags for instance state and allocation
  int passedInit;
  int hasSecret;
  int hasM; // flag = 0 if m has not been allocated. Need for freeInstance.
  int hasShares;

  // big ints
  mpz_t s;     // the secret
  mpz_t *m;    // the pairwise relative primes
  mpz_t alpha; // random value
  mpz_t *shares;
};

void free_instance(struct asmuth_bloom *);

struct asmuth_bloom *init_instance(int, int, int);

void generate_secret(struct asmuth_bloom *);

void get_next_prime(mpz_t *, mpz_t, int);

void check_m(struct asmuth_bloom *);

void generate_shares(struct asmuth_bloom *);

int recover_secret(struct asmuth_bloom *);

void print_instance(struct asmuth_bloom *);

#endif
