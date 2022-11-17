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

// Find the determinant of matrix mod p and return in result. 
void determinant_mod(mpz_t **matrix, mpz_t *result, int n, mpz_t p) {
	int index;
	mpz_t num1, num2, minus1, det;

	mpz_init(num1);
	mpz_init(num2);
	mpz_init_set_ui(det, (unsigned long int) 1);
	mpz_init_set_si(minus1, (long int) -1);
	mpz_set_ui(*result, (unsigned long int) 1);

	// make a copy of matrix because we will change values of matrix otherwise
	mpz_t m_copy[n][n];
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			mpz_init_set(m_copy[i][j], matrix[i][j]); }
	}

	// temp array for storing row
	mpz_t temp[n + 1];
	for (int i = 0; i < n+1; i++) {
		mpz_init(temp[i]);
	}

	// loop traversing diagonal elements
	for (int i = 0; i < n; i++) {
		index = i;

		// finding index with non-zero value
		while (index < n && mpz_sgn(m_copy[index][i]) == 0) {
			index++;
		}
		if (index == n) { // if there is no nonzero element
			continue; // determinant is zero
		}
		if (index != i) {
			// loop for swapping diag element row and index row
			for (int j = 0; j < n; j++) {
				mpz_swap(m_copy[index][j], m_copy[i][j]);
			}

			mpz_t dummy;
			mpz_init(dummy);
			mpz_pow_ui(dummy, minus1, index - 1);
			mpz_mul(det, det, dummy); // det = det * (-1)^(index - 1)
			mpz_clear(dummy);
		}

		// Store values of diagonals
		for (int j = 0; j < n; j++) {
			mpz_set(temp[j],m_copy[i][j]);
		}
		// Traverse each row below diagonal
		for (int j = i + 1; j < n; j++) {
			mpz_set(num1, temp[i]);
			mpz_set(num2, m_copy[j][i]);
			
			// traverse every column of the row and multiply to every row
			for (int k = 0; k < n; k++) {
				mpz_mul(m_copy[j][k], num1, m_copy[j][k]);
				mpz_fdiv_r(m_copy[j][k], m_copy[j][k], p); // mop p
				mpz_submul(m_copy[j][k], num2, temp[k]);
				mpz_fdiv_r(m_copy[j][k], m_copy[j][k], p); // mod p
			}
			mpz_mul(*result, *result, num1);
			mpz_fdiv_r(*result, *result, p); // mod p
		}
	}

	for (int i = 0; i < n; i++) {
		mpz_mul(det, det, m_copy[i][i]);
		mpz_fdiv_r(det, det, p); // ADDED
	}

	int err = mpz_invert(*result, *result, p); // result = result^-1 mod p
	mpz_mul(*result, *result, det); // result = result * det
	mpz_fdiv_r(*result, *result, p); // result = result mod p

	// free m_copy and other big ints
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			mpz_clear(m_copy[i][j]);
		}
	}
	mpz_clear(num1);
	mpz_clear(num2);
	mpz_clear(det);
	mpz_clear(minus1);

	return;
}

		
// Function to get the cofactor of matrix[n][n] at position matrix[x][y] 
// and return as result.
void get_cofactor(mpz_t **matrix, mpz_t *result, int x, int y, int n, mpz_t p) {
	// make cofactor matrix
	mpz_t **cofactor_matrix;
	cofactor_matrix = (mpz_t **) malloc((n-1) * sizeof(mpz_t *));
	for (int i = 0; i < n-1; i++) {
		cofactor_matrix[i] = (mpz_t *) malloc((n-1) * sizeof(mpz_t)); 
	}

	int i = 0;
	int j = 0;

	// Init cofactor matrix
	for (int row = 0; row < n; row++) {
		for (int col = 0; col < n; col++) {
			if (row != x && col != y) {
				mpz_init_set(cofactor_matrix[i][j], matrix[row][col]);
				j++;
				if (j == n - 1) {
					j = 0;
					i++;
				}
			}
		}
	}

	determinant_mod(cofactor_matrix, result, n-1, p);

	// result = result * (-1)^(x+y)
	mpz_t sign;
	mpz_init_set_si(sign, (long int) -1);
	mpz_pow_ui(sign, sign, (x + y));
	mpz_mul(*result, *result, sign);
	mpz_fdiv_r(*result, *result, p);
	mpz_clear(sign);

	// clearing cofactor_matrix
	for (i = 0; i < n-1; i++) {
		for (j = 0; j < n-1; j++) {
			mpz_clear(cofactor_matrix[i][j]);
		}
		free(cofactor_matrix[i]);
	}
	free(cofactor_matrix);

	return;
}


int recover_secret(struct blakely *instance)
{
	if (instance->hasShares != 1) {
		printf("Cannot recover secret if instance does not have shares.\n");
		free(instance);
		exit(EXIT_FAILURE);
	}
	
	// 1. Get the determinant of the following square shares matrix:
	// 		[share[0][0], ..., share[0][t-2], -1]
	// 		[share[1][0], ..., share[1][t-2], -1]
	// 		...             ...              ...
	// 		[share[t-1][0], ..., share[t-1][t-2], -1]
	mpz_t **mat;
	mat = (mpz_t **) malloc((instance->t) * sizeof(mpz_t *));
	for (int i = 0; i < (instance->t); i++) {
		mat[i] = (mpz_t *) malloc((instance->t) * sizeof(mpz_t));
		for (int j = 0; j  < (instance->t); j++) {
			if (j != (instance->t) - 1) {
				mpz_init_set(mat[i][j], (instance->shares)[i][j]);
			} else {
				mpz_init_set_si(mat[i][j], (long int) -1);
			}
		}
	}
	mpz_t *inv_det = (mpz_t *) malloc(1 * sizeof(mpz_t));
	mpz_init(*inv_det);
	determinant_mod(mat, inv_det, instance->t, instance->p);
	
	// 2. Find the inverse of the determinant mod p
	mpz_invert(*inv_det, *inv_det, instance->p);

	// 3. Get the top row of the transposed cofactor matrix from the above matrix
	mpz_t *inv_mod_mat_row1;
	inv_mod_mat_row1 = (mpz_t *) malloc((instance->t) * sizeof(mpz_t));
	mpz_t result;
	mpz_init(result);	
	for (int i = 0; i < (instance->t); i++) {
		mpz_init(inv_mod_mat_row1[i]);
		get_cofactor(mat, &(inv_mod_mat_row1[i]), i, 0, instance->t, instance->p);
		
		// 4. Multiply this row vector by the inverse of the determinant found in 2.
		mpz_mul(inv_mod_mat_row1[i], inv_mod_mat_row1[i], *inv_det);	
		
		// 5. mod each element by p
		mpz_fdiv_r(inv_mod_mat_row1[i], inv_mod_mat_row1[i], instance->p);
		
		// 6. Find the dot product of that new row vector with the column vector:
		// 		[-share[0][t-1], -share[1][t-1], ..., -share[t-1][t-1]]
		mpz_submul(result, (instance->shares)[i][(instance->t)-1], inv_mod_mat_row1[i]);
	
		// 7. mod the result by p
		mpz_fdiv_r(result, result, instance->p);
	}


	// 8. check if it is the secret
	int found_secret;
	if (mpz_cmp(result, (instance->s)[0]) == 0) {
		found_secret = 1;
	} else {
		found_secret = 0;
	}

	// free allocated vars
	for (int i = 0; i < (instance->t); i++) {
		for (int j = 0; j < (instance->t); j++) {
			mpz_clear(mat[i][j]);
		}
		free(mat[i]);
		mpz_clear(inv_mod_mat_row1[i]);
	}
	free(mat);
	free(inv_mod_mat_row1);
	mpz_clear(result);
	mpz_clear(*inv_det);
	free(inv_det);


	return found_secret;
}

// print mxn matrix of mpz_t
void print_mpz_matrix (mpz_t **matrix, int m, int n) {
	char *str;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			str = mpz_get_str(NULL, 10, matrix[i][j]);
			if (j == n - 1) {
				printf("%s\n", str);
			} else {
				printf("%s, ", str);
			}
			free(str);
		}
	}
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
