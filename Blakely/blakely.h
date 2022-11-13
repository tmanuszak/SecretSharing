#ifndef BLAKELY_HEADER
#define BLAKELY_HEADER

#include <gmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/random.h>

struct blakely {
  int t;
  int n;
  int lambda;
  gmp_randstate_t state; // RNG

  // flags
  int passedInit;
  int hasSecret;
  int hasShares;

  // big ints
  mpz_t *s; // secret is s[0]
  mpz_t p; // prime
  mpz_t **shares; // shares
};

void free_instance(struct blakely *);

struct blakely *init_instance(int, int, int);

void generate_secret(struct blakely *);

void generate_shares(struct blakely *);

int recover_secret(struct blakely *);

void print_instance(struct blakely *);

#endif
