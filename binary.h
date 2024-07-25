#ifndef BINARY_H
#define BINARY_H

#include <stdint.h>
#include <stdbool.h>

#include "block.h"
#include "luby.h"


// BinaryCodec structure
typedef struct {
    int numSourceBlocks;
} binaryCodec;

// BinaryDecoder structure
typedef struct {
    binaryCodec codec;
    int messageLength;
    sparseMatrix matrix;
} binaryDecoder;

// Function declarations
binaryCodec* NewBinaryCodec(int numSourceBlocks);
int* PickIndices(binaryCodec* codec, int64_t codeBlockIndex, int* outLength);
block* GenerateIntermediateBlocks(binaryCodec* codec, uint8_t* message, int messageLength, int numBlocks);
binaryDecoder* NewDecoder(binaryCodec* codec, int messageLength);
bool AddBlocks(binaryDecoder* decoder, LTBlock* blocks, int numBlocks);
uint8_t* Decode(binaryDecoder* decoder, int* outLength);

// Helper functions (need to be implemented)
void intermediateSymbols(int k, uint32_t* l);
void tripleGenerator(int k, uint16_t x, uint32_t* d, uint32_t* a, uint32_t* b);
block* equalizeBlockLengths(int** longBlocks, int** shortBlocks, int numLong, int numShort, int longBlockSize, int shortBlockSize);

#endif // BINARY_H