#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "constants.c"


void solitonDistribution(int n, double **cdf, int *size) {
    *size = n + 1;
    *cdf = (double *)malloc(*size * sizeof(double));
    double *cdf_array = *cdf;
    cdf_array[1] = 1.0 / n;
    for (int i = 2; i < *size; i++) {
        cdf_array[i] = cdf_array[i - 1] + (1.0 / (i * (i - 1)));
    }
}

void robustSolitonDistribution(int n, int m, double delta, double **cdf, int *size) {
    *size = n + 1;
    *cdf = (double *)malloc(*size * sizeof(double));
    double *pdf = (double *)malloc(*size * sizeof(double));
    double *cdf_array = *cdf;

    pdf[1] = 1.0 / n + 1.0 / m;
    double total = pdf[1];
    for (int i = 2; i < *size; i++) {
        pdf[i] = (1.0 / (i * (i - 1)));
        if (i < m) {
            pdf[i] += 1.0 / (i * m);
        }
        if (i == m) {
            pdf[i] += log(n / (m * delta)) / m;
        }
        total += pdf[i];
    }

    for (int i = 1; i < *size; i++) {
        pdf[i] /= total;
        cdf_array[i] = cdf_array[i - 1] + pdf[i];
    }

    free(pdf);
}

void onlineSolitonDistribution(double eps, double **cdf, int *size) {
    double f = ceil(log(eps * eps / 4) / log(1 - (eps / 2)));
    *size = (int)f + 1;
    *cdf = (double *)malloc(*size * sizeof(double));
    double *cdf_array = *cdf;

    double rho = 1 - ((1 + (1 / f)) / (1 + eps));
    cdf_array[1] = rho;

    for (int i = 2; i <= (int)f; i++) {
        double rhoI = ((1 - rho) * f) / ((f - 1) * (i - 1) * i);
        cdf_array[i] = cdf_array[i - 1] + rhoI;
    }
}

int pickDegree(double r, double *cdf, int size) {
    int low = 1, high = size - 1;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (cdf[mid] >= r) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    return low;
}

void sampleUniform(int num, int max, int **picks, int *size) {
    if (num >= max) {
        *size = max;
        *picks = (int *)malloc(max * sizeof(int));
        for (int i = 0; i < max; i++) {
            (*picks)[i] = i;
        }
        return;
    }

    *size = num;
    *picks = (int *)malloc(num * sizeof(int));
    bool *seen = (bool *)calloc(max, sizeof(bool));

    for (int i = 0; i < num; i++) {
        int p;
        do {
            p = rand() % max;
        } while (seen[p]);
        (*picks)[i] = p;
        seen[p] = true;
    }

    free(seen);
    qsort(*picks, num, sizeof(int), (int (*)(const void *, const void *))strcmp);
}

void partition(int i, int j, int *il, int *is, int *jl, int *js) {
    *il = (int)ceil((double)i / j);
    *is = (int)floor((double)i / j);
    *jl = i - (*is * j);
    *js = j - *jl;

    if (*jl == 0) {
        *il = 0;
    }
    if (*js == 0) {
        *is = 0;
    }
}

int factorial(int x) {
    int f = 1;
    for (int i = 1; i <= x; i++) {
        f *= i;
    }
    return f;
}

int centerBinomial(int x) {
    return choose(x, x / 2);
}

int choose(int n, int k) {
    if (k > n / 2) {
        k = n - k;
    }
    int *numerator = (int *)malloc((n - k) * sizeof(int));
    int *denominator = (int *)malloc((n - k) * sizeof(int));

    for (int i = k + 1, j = 1; i <= n; i++, j++) {
        numerator[j - 1] = i;
        denominator[j - 1] = j;
    }

    for (int j = (n - k) - 1; j > 0; j--) {
        for (int i = (n - k) - 1; i >= 0; i--) {
            if (numerator[i] % denominator[j] == 0) {
                numerator[i] /= denominator[j];
                denominator[j] = 1;
                break;
            }
        }
    }

    int f = 1;
    for (int i = 0; i < (n - k); i++) {
        f *= numerator[i];
    }

    free(numerator);
    free(denominator);
    return f;
}

bool bitSet(uint32_t x, uint32_t b) {
    return (x >> b) & 1 == 1;
}

int bitsSet(uint64_t x) {
    x -= (x >> 1) & 0x5555555555555555;
    x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
    x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;
    return (int)((x * 0x0101010101010101) >> 56);
}

uint64_t grayCode(uint64_t x) {
    return (x >> 1) ^ x;
}

void buildGraySequence(int length, int b, int **sequence, int *size) {
    *size = length;
    *sequence = (int *)malloc(length * sizeof(int));
    int *s = *sequence;
    int i = 0;
    for (uint64_t x = 0; ; x++) {
        uint64_t g = grayCode(x);
        if (bitsSet(g) == b) {
            s[i] = (int)g;
            i++;
            if (i >= length) {
                break;
            }
        }
    }
}

bool isPrime(int x) {
    for (int i = 0; i < sizeof(smallPrimes) / sizeof(smallPrimes[0]); i++) {
        int p = smallPrimes[i];
        if (p * p > x) {
            return true;
        }
        if (x % p == 0) {
            return false;
        }
    }
    return true;
}

int smallestPrimeGreaterOrEqual(int x) {
    int len = sizeof(smallPrimes) / sizeof(smallPrimes[0]);
    if (x <= smallPrimes[len - 1]) {
        for (int i = 0; i < len; i++) {
            if (smallPrimes[i] >= x) {
                return smallPrimes[i];
            }
        }
    }

    while (!isPrime(x)) {
        x++;
    }
    return x;
}