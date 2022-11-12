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

  // flags
  int passedInit;
  int hasSecret;
  int hasShares;
};

void free_instance(struct blakely *);

struct blakely *init_instance(int, int, int);

void generate_secret(struct blakely *);

void generate_shares(struct blakely *);

int recover_secret(struct blakely *);

void print_instance(struct blakely *);

#endif
