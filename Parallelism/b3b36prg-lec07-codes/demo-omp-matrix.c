#include <omp.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/time.h>

void fill(int n, int m[n][n]);
bool isEqual(int n, int m1[n][n], int m2[n][n]);

void multiply_single(int n, int a[n][n], int b[n][n], int c[n][n]);
void multiply_omp(int n, int a[n][n], int b[n][n], int c[n][n]);

int main(int argc, char *argv[])
{
   int n = argc > 1 ? atoi(argv[1]) : 100;
   int (*a)[n] = malloc(n*n * sizeof(int));
   int (*b)[n] = malloc(n*n * sizeof(int));
   int (*c1)[n] = malloc(n*n * sizeof(int));
   int (*c2)[n] = malloc(n*n * sizeof(int));

   fill(n, a);
   fill(n, b);

   struct timeval t1, t2;
   printf("Size of matrices %d x %d naive multiplication with O(n^3)\n", n, n);
   gettimeofday(&t1, NULL);
   multiply_single(n, a, b, c1);
   gettimeofday(&t2, NULL);
   double dt1 = t2.tv_sec - t1.tv_sec + ((t2.tv_usec - t1.tv_usec) / 1000000.0);

   gettimeofday(&t1, NULL);
   multiply_omp(n, a, b, c2);
   gettimeofday(&t2, NULL);
   double dt2 = t2.tv_sec - t1.tv_sec + ((t2.tv_usec - t1.tv_usec) / 1000000.0);

   printf("c1 == c2: %d\n", isEqual(n, c1, c2));

   printf("Multiplication single core %.2lf sec\n", dt1);
   printf("Multiplication  multi-core %.2lf sec\n", dt2);

   free(a);
   free(b);
   free(c1);
   return 0;
}

// - function -----------------------------------------------------------------
void fill(int n, int m[n][n])
{
   for (int i = 0; i < n; ++i) {
      for(int j = 0; j < n; ++j) {
         m[i][j] = rand();
      }
   }
}

// - function -----------------------------------------------------------------
bool isEqual(int n, int m1[n][n], int m2[n][n])
{
   bool ret = true;
   for (int r = 0; r < n; ++r) {
      for(int c = 0; c < n; ++c) {
         if (m1[r][c] != m2[r][c]) {
            ret = false;
            goto out;
         }
      }
   }
out:
   return ret;
}

// - function -----------------------------------------------------------------
void multiply_omp(int n, int a[n][n], int b[n][n], int c[n][n]) 
{
   int i;
 #pragma omp parallel private(i)
 #pragma omp for schedule (dynamic, 1)
   for (i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
         c[i][j] = 0;
         for (int k = 0; k < n; ++k) {
            c[i][j] += a[i][k] * b[k][j];
         }
      }
   }
}

// - function -----------------------------------------------------------------
void multiply_single(int n, int a[n][n], int b[n][n], int c[n][n]) 
{
   for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
         c[i][j] = 0;
         for (int k = 0; k < n; ++k) {
            c[i][j] += a[i][k] * b[k][j];
         }
      }
   }
}
