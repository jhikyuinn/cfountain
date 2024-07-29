#ifndef RU10_H
#define RU10_H

#include <stdint.h>

typedef struct {
    int numSourceSymbols;
    int symbolAlignmentSize;
} ru10_codec;

// Function prototypes
ru10_codec* create_ru10_codec(int numSourceSymbols, int symbolAlignmentSize);
void destroy_ru10_codec(ru10_codec* codec);

int get_source_blocks(const ru10_codec* codec);
int pick_indices(const ru10_codec* codec, int64_t codeBlockIndex, int* indices, int maxIndices);
void generate_intermediate_blocks(const ru10_codec* codec, const uint8_t* message, size_t message_len, uint8_t** blocks, int* numBlocks);

#endif // RU10_H