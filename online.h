#ifndef ONLINE_H
#define ONLINE_H

#include <stdint.h>

typedef struct {
    double epsilon;
    int quality;
    int numSourceBlocks;
    int64_t randomSeed;
    double* cdf; // CDF 배열의 포인터
} online_codec;

// 함수 프로토타입
online_codec* create_online_codec(int sourceBlocks, double epsilon, int quality, int64_t seed);
void destroy_online_codec(online_codec* codec);

int get_source_blocks(const online_codec* codec);
int num_aux_blocks(const online_codec* codec);
int estimate_decode_blocks_needed(const online_codec* codec);
void generate_intermediate_blocks(const online_codec* codec, const uint8_t* message, size_t message_len, uint8_t** src_blocks, uint8_t** aux_blocks);
void encode_online_blocks(const online_codec* codec, const int64_t* ids, size_t num_ids, uint8_t** blocks);

#endif // ONLINE_H