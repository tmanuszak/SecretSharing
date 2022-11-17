#ifndef SHAMIR_HEADER
#define SHAMIR_HEADER

#include <gmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/random.h>

struct shamir {
  int t;
  int n;
  int lambda;
  gmp_randstate_t state; // RNG

  // flags
  int passedInit;
  int hasSecret;
  int hasShares;

  // big ints
};

void free_instance(struct shamir *);

struct shamir *init_instance(int, int, int);

void generate_secret(struct shamir *);

void generate_shares(struct shamir *);

int recover_secret(struct shamir *);

void print_instance(struct shamir *);

#endif
