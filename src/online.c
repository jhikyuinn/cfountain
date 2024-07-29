#include "online.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

// CDF 계산 함수 (샘플 함수)
double* calculate_cdf(double epsilon) {
    // CDF 계산 로직 구현
    return NULL;
}

online_codec* create_online_codec(int sourceBlocks, double epsilon, int quality, int64_t seed) {
    online_codec* codec = (online_codec*)malloc(sizeof(online_codec));
    codec->epsilon = epsilon;
    codec->quality = quality;
    codec->numSourceBlocks = sourceBlocks;
    codec->randomSeed = seed;
    codec->cdf = calculate_cdf(epsilon);
    return codec;
}

void destroy_online_codec(online_codec* codec) {
    if (codec != NULL) {
        free(codec->cdf);
        free(codec);
    }
}

int get_source_blocks(const online_codec* codec) {
    return codec->numSourceBlocks;
}

int num_aux_blocks(const online_codec* codec) {
    return (int)ceil(0.55 * codec->quality * codec->epsilon * codec->numSourceBlocks);
}

int estimate_decode_blocks_needed(const online_codec* codec) {
    return (int)ceil((1 + codec->epsilon) * (codec->numSourceBlocks + num_aux_blocks(codec)));
}

// 여기에 `generate_intermediate_blocks`와 `encode_online_blocks` 구현 추가
