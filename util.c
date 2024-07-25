#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// 작은 소수 배열 (primes) 정의
static const int smallPrimes[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157,
    163, 167, 173, 179, 181, 191
};

// solitonDistribution은 soliton 분포를 위한 CDF를 반환합니다.
void solitonDistribution(int n, double **cdf, int *size) {
    *size = n + 1;
    *cdf = (double *)malloc(*size * sizeof(double));
    double *cdf_array = *cdf;
    cdf_array[1] = 1.0 / n;
    for (int i = 2; i < *size; i++) {
        cdf_array[i] = cdf_array[i - 1] + (1.0 / (i * (i - 1)));
    }
}

// robustSolitonDistribution은 robust soliton 분포를 위한 CDF를 반환합니다.
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

// onlineSolitonDistribution은 온라인 코드에 대한 soliton-like 분포를 반환합니다.
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

// pickDegree는 cdf에서 r보다 큰 가장 작은 인덱스를 반환합니다.
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

// sampleUniform은 [0, max)에서 num 개의 숫자를 균등하게 선택합니다.
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

// partition은 RFC 5053 S.5.3.1.2의 블록 분할 함수입니다.
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

// factorial은 입력 인수 x의 팩토리얼을 계산합니다.
int factorial(int x) {
    int f = 1;
    for (int i = 1; i <= x; i++) {
        f *= i;
    }
    return f;
}

// centerBinomial은 choose(x, ceil(x/2))를 계산합니다.
int centerBinomial(int x) {
    return choose(x, x / 2);
}

// choose는 n choose k를 계산합니다.
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

// bitSet는 x에서 b번째 비트가 설정되어 있는지 반환합니다.
bool bitSet(uint x, uint b) {
    return (x >> b) & 1 == 1;
}

// bitsSet는 x에서 설정된 비트의 수를 반환합니다.
int bitsSet(uint64_t x) {
    x -= (x >> 1) & 0x5555555555555555;
    x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
    x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;
    return (int)((x * 0x0101010101010101) >> 56);
}

// grayCode는 입력 인수의 Gray 코드를 계산합니다.
uint64_t grayCode(uint64_t x) {
    return (x >> 1) ^ x;
}

// buildGraySequence는 정확히 b 비트가 설정된 "length"개의 Gray 번호 시퀀스를 반환합니다.
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

// isPrime는 x가 소수인지 테스트합니다.
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

// smallestPrimeGreaterOrEqual은 x보다 크거나 같은 가장 작은 소수를 반환합니다.
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