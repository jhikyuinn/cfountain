#ifndef RAPTOR_H
#define RAPTOR_H

#include <stdint.h>

#define MAX_SOURCE_SYMBOLS 8192
#define MAX_BLOCK_SIZE 1024

typedef struct {
    int SymbolAlignmentSize;
    int NumSourceSymbols;
} RaptorCodec;

// 함수 선언
RaptorCodec* create_raptor_codec(int source_blocks, int alignment_size);
void destroy_raptor_codec(RaptorCodec* codec);

int raptor_rand(uint32_t x, uint32_t i, uint32_t m);
int deg(uint32_t v);
void intermediate_symbols(int k, int* L, int* S, int* H);
void triple_generator(int k, uint16_t x, int* d, uint32_t* a, uint32_t* b);
void find_lt_indices(int k, uint16_t x, int* indices, int* num_indices);
void lt_encode(int k, uint16_t x, const uint8_t* c, uint8_t* result);
void raptor_intermediate_blocks(const uint8_t* source, int num_blocks, uint8_t* intermediate_blocks);
void generate_intermediate_blocks(const uint8_t* message, int message_length, int num_blocks, uint8_t* blocks);
void pick_indices(int code_block_index, int* indices, int* num_indices);
void* new_decoder(const RaptorCodec* codec, int message_length);
int add_blocks(void* decoder, const uint8_t* blocks, int num_blocks);
uint8_t* decode(void* decoder, int* out_length);

#endif // RAPTOR_H