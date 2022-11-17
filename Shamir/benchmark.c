#include "shamir.h"
#include <stdio.h>
#include <gmp.h>

int main(int argc, char *argv[])
{
  struct shamir *instance;
  int t, n, lambda;

  // Make sure we have parameters t and n such that t <= n
  if (argc > 3)
  {
    t = (int)strtol(argv[1], NULL, 10);
    n = (int)strtol(argv[2], NULL, 10);
    lambda = (int)strtol(argv[3], NULL, 10);
    if (t > n || t < 2 || lambda < 64 || lambda > 512 || n > 1000)
    {
      printf("Shamir (%d,%d) scheme is not valid.\n", t, n);
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    printf("Must input a threshold, number of parties, and security parameter.\n");
    exit(EXIT_FAILURE);
  }

  instance = init_instance(t, n, lambda);
  generate_secret(instance);
  // generate_shares(instance);
  // printf("Secret recovered: %d\n", recover_secret(instance));

  //print_instance(instance);

  free_instance(instance);

  return 0;
}
