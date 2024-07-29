#ifndef LUBY_H
#define LUBY_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// LTBlock structure: represents a block created using the LT transform.
typedef struct {
    int64_t blockCode; // Block ID used in encoding
    uint8_t* data;     // Encoded block data
    size_t length;     // Length of the data
} LTBlock;

// Codec interface (in C represented as a struct with function pointers)
typedef struct Codec {
    int (*SourceBlocks)(struct Codec*);
    void (*GenerateIntermediateBlocks)(struct Codec*, uint8_t*, size_t, int, uint8_t***, int*);
    int* (*PickIndices)(struct Codec*, int64_t, int*);
    struct Decoder* (*NewDecoder)(struct Codec*, int);
    void (*Free)(struct Codec*); // Free function to deallocate memory
} Codec;

// Decoder interface (in C represented as a struct with function pointers)
typedef struct Decoder {
    bool (*AddBlocks)(struct Decoder*, LTBlock*, int);
    uint8_t* (*Decode)(struct Decoder*, int*);
    void (*Free)(struct Decoder*); // Free function to deallocate memory
} Decoder;

// Function prototypes
Codec* NewLubyCodec(int sourceBlocks, unsigned int seed, double* degreeCDF, int degreeCDFLength);
LTBlock* EncodeLTBlocks(Codec* codec, uint8_t* message, size_t messageLength, int64_t* encodedBlockIDs, int numIDs, int* outSize);

#endif // LUBY_H
