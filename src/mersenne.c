#include <stdint.h>
#include <stdbool.h>

// 32-bit Mersenne Twister MT19937

typedef struct {
    uint32_t mt[624];
    int index;
    bool initialized;
} MersenneTwister;

// Function prototypes
void initializeMT(MersenneTwister* t, uint32_t seed);
void generateUntemperedMT(MersenneTwister* t);
uint32_t uint32(MersenneTwister* t);
int64_t int63(MersenneTwister* t);
void seedMT(MersenneTwister* t, int64_t seed);
MersenneTwister newMersenneTwister(int64_t seed);

MersenneTwister newMersenneTwister(int64_t seed) {
    MersenneTwister t;
    seedMT(&t, seed);
    return t;
}

void seedMT(MersenneTwister* t, int64_t seed) {
    initializeMT(t, (uint32_t)(((seed >> 32) ^ seed) & 0xFFFFFFFF));
}

int64_t int63(MersenneTwister* t) {
    uint32_t a = uint32(t);
    uint32_t b = uint32(t);
    return ((int64_t)a << 31) ^ (int64_t)b;
}

uint32_t uint32(MersenneTwister* t) {
    if (!t->initialized) {
        initializeMT(t, 4357);  // Default seed from original paper
    }

    if (t->index == 0) {
        generateUntemperedMT(t);
    }

    uint32_t y = t->mt[t->index];
    t->index++;
    if (t->index >= 624) {
        t->index = 0;
    }
    y ^= y >> 11;
    y ^= (y << 7) & 0x9d2c5680;
    y ^= (y << 15) & 0xefc60000;
    y ^= y >> 18;

    return y;
}

void initializeMT(MersenneTwister* t, uint32_t seed) {
    t->index = 0;
    t->mt[0] = seed;

    for (int i = 1; i < 624; i++) {
        t->mt[i] = (1812433253U * (t->mt[i - 1] ^ (t->mt[i - 1] >> 30)) + i) & 0xFFFFFFFF;
    }
    t->initialized = true;
}

void generateUntemperedMT(MersenneTwister* t) {
    static const uint32_t mag01[2] = {0x0, 0x9908b0df};
    for (int i = 0; i < 624; i++) {
        uint32_t y = (t->mt[i] & 0x80000000) | (t->mt[(i + 1) % 624] & 0x7fffffff);
        t->mt[i] = t->mt[(i + 397) % 624] ^ (y >> 1) ^ mag01[y & 0x1];
    }
}

// 64-bit Mersenne Twister MT19937-64

typedef struct {
    uint64_t mt[312];
    int index;
    bool initialized;
} MersenneTwister64;

// Function prototypes
void initializeMT64(MersenneTwister64* t, uint64_t seed);
void generateUntemperedMT64(MersenneTwister64* t);
uint64_t uint64(MersenneTwister64* t);
int64_t int63_64(MersenneTwister64* t);
void seedMT64(MersenneTwister64* t, int64_t seed);
MersenneTwister64 newMersenneTwister64(int64_t seed);
void seedSliceMT64(MersenneTwister64* t, const uint64_t* seed, int seedLength);

MersenneTwister64 newMersenneTwister64(int64_t seed) {
    MersenneTwister64 t;
    seedMT64(&t, seed);
    return t;
}

void seedMT64(MersenneTwister64* t, int64_t seed) {
    initializeMT64(t, (uint64_t)seed);
}

int64_t int63_64(MersenneTwister64* t) {
    return (int64_t)(uint64(t) & 0x7FFFFFFFFFFFFFFF);
}

uint64_t uint64(MersenneTwister64* t) {
    if (!t->initialized) {
        initializeMT64(t, 5489);  // Default seed
    }

    if (t->index == 0) {
        generateUntemperedMT64(t);
    }

    uint64_t y = t->mt[t->index];
    t->index++;
    if (t->index >= 312) {
        t->index = 0;
    }
    y ^= (y >> 29) & 0x5555555555555555ULL;
    y ^= (y << 17) & 0x71d67fffeda60000ULL;
    y ^= (y << 37) & 0xfff7eee000000000ULL;
    y ^= y >> 43;

    return y;
}

void initializeMT64(MersenneTwister64* t, uint64_t seed) {
    t->index = 0;
    t->mt[0] = seed;

    for (int i = 1; i < 312; i++) {
        t->mt[i] = 6364136223846793005ULL * (t->mt[i - 1] ^ (t->mt[i - 1] >> 62)) + i;
    }
    t->initialized = true;
}

void seedSliceMT64(MersenneTwister64* t, const uint64_t* seed, int seedLength) {
    initializeMT64(t, 19650218ULL);

    int length = seedLength;
    if (312 > length) {
        length = 312;
    }

    int i = 1, j = 0;
    for (int k = 0; k < length; k++) {
        t->mt[i] = (t->mt[i] ^ ((t->mt[i - 1] ^ (t->mt[i - 1] >> 62)) * 3935559000370003845ULL)) + seed[j] + j;
        i++;
        j++;
        if (i >= 312) {
            t->mt[0] = t->mt[311];
            i = 1;
        }
        if (j >= seedLength) {
            j = 0;
        }
    }

    for (int k = 0; k < 311; k++) {
        t->mt[i] = (t->mt[i] ^ ((t->mt[i - 1] ^ (t->mt[i - 1] >> 62)) * 2862933555777941757ULL)) - i;
        i++;
        if (i >= 312) {
            t->mt[0] = t->mt[311];
            i = 1;
        }
    }

    t->mt[0] = 1ULL << 63;
}

void generateUntemperedMT64(MersenneTwister64* t) {
    static const uint64_t mag01[2] = {0x0ULL, 0xb5026f5aa96619e9ULL};
    for (int i = 0; i < 312; i++) {
        uint64_t y = (t->mt[i] & 0xFFFFFFFF80000000ULL) | (t->mt[(i + 1) % 312] & 0x7FFFFFFFULL);
        t->mt[i] = t->mt[(i + 156) % 312] ^ (y >> 1) ^ mag01[y & 0x1];
    }
}