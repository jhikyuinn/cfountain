
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "fountain.h"

// Block structure: Internal representation for blocks during encoding/decoding
typedef struct {
    uint8_t* data; // Block data
    size_t length; // Block length
} block;

// SparseMatrix structure: Used for solving equations during decoding
typedef struct {
    int** coeff;   // Coefficients of the equations
    block* v;      // Right-hand side of the equations
    int size;      // Number of equations
} sparseMatrix;

// LubyCodec structure: Implementation of the Luby Transform codec
typedef struct {
    Codec base;      // Base codec interface
    int sourceBlocks; // Number of source blocks
    unsigned int seed; // Random seed
    double* degreeCDF; // Cumulative degree distribution function
    int degreeCDFLength; // Length of degreeCDF array
} LubyCodec;

// LubyDecoder structure: State required for decoding Luby Transform messages
typedef struct {
    Decoder base;    // Base decoder interface
    LubyCodec* codec; // Reference to the codec
    int messageLength; // Length of the original message
    sparseMatrix matrix; // Sparse matrix for solving equations
} LubyDecoder;

// Utility function to pick degree based on CDF
int pickDegree(unsigned int* seed, double* degreeCDF, int degreeCDFLength) {
    double r = (double)rand_r(seed) / RAND_MAX;
    for (int i = 0; i < degreeCDFLength; i++) {
        if (r < degreeCDF[i]) {
            return i + 1;
        }
    }
    return degreeCDFLength;
}

// Utility function to sample indices uniformly
int* sampleUniform(unsigned int* seed, int d, int max, int* outSize) {
    int* indices = (int*)malloc(d * sizeof(int));
    int* available = (int*)malloc(max * sizeof(int));
    for (int i = 0; i < max; i++) {
        available[i] = i;
    }
    for (int i = 0; i < d; i++) {
        int index = rand_r(seed) % (max - i);
        indices[i] = available[index];
        available[index] = available[max - i - 1];
    }
    free(available);
    *outSize = d;
    return indices;
}

// XOR two blocks
void xorBlocks(uint8_t* dest, const uint8_t* src, size_t length) {
    for (size_t i = 0; i < length; i++) {
        dest[i] ^= src[i];
    }
}

// Generate a single LT block from source blocks
block generateLubyTransformBlock(block* source, int* indices, int numIndices) {
    block result;
    result.data = (uint8_t*)malloc(source[0].length * sizeof(uint8_t));
    result.length = source[0].length;
    memset(result.data, 0, result.length);

    for (int i = 0; i < numIndices; i++) {
        xorBlocks(result.data, source[indices[i]].data, result.length);
    }

    return result;
}

// Generate intermediate blocks
void GenerateIntermediateBlocks_Luby(struct Codec* codec, uint8_t* message, size_t messageLength, int numBlocks, uint8_t*** blocks, int* blockLength) {
    LubyCodec* lubyCodec = (LubyCodec*)codec;
    PartitionBytes(message, messageLength, lubyCodec->sourceBlocks, blocks, blockLength);
}

// Pick indices for a code block
int* PickIndices_Luby(struct Codec* codec, int64_t codeBlockIndex, int* outSize) {
    LubyCodec* lubyCodec = (LubyCodec*)codec;
    int degree = pickDegree(&lubyCodec->seed, lubyCodec->degreeCDF, lubyCodec->degreeCDFLength);
    return sampleUniform(&lubyCodec->seed, degree, lubyCodec->sourceBlocks, outSize);
}

// Create a new Luby decoder
Decoder* NewDecoder_Luby(struct Codec* codec, int messageLength) {
    LubyCodec* lubyCodec = (LubyCodec*)codec;
    LubyDecoder* decoder = (LubyDecoder*)malloc(sizeof(LubyDecoder));
    decoder->codec = lubyCodec;
    decoder->messageLength = messageLength;

    decoder->matrix.coeff = (int**)malloc(lubyCodec->sourceBlocks * sizeof(int*));
    for (int i = 0; i < lubyCodec->sourceBlocks; i++) {
        decoder->matrix.coeff[i] = (int*)calloc(lubyCodec->sourceBlocks, sizeof(int));
    }
    decoder->matrix.v = (block*)malloc(lubyCodec->sourceBlocks * sizeof(block));
    decoder->matrix.size = 0;

    return (Decoder*)decoder;
}

// Free the Luby codec
void FreeCodec_Luby(struct Codec* codec) {
    LubyCodec* lubyCodec = (LubyCodec*)codec;
    free(lubyCodec->degreeCDF);
    free(lubyCodec);
}

// Free the Luby decoder
void FreeDecoder_Luby(struct Decoder* decoder) {
    LubyDecoder* lubyDecoder = (LubyDecoder*)decoder;
    for (int i = 0; i < lubyDecoder->codec->sourceBlocks; i++) {
        free(lubyDecoder->matrix.coeff[i]);
    }
    free(lubyDecoder->matrix.coeff);
    free(lubyDecoder->matrix.v);
    free(lubyDecoder);
}

// Add blocks to the decoder
bool AddBlocks_Luby(struct Decoder* decoder, LTBlock* blocks, int numBlocks) {
    LubyDecoder* lubyDecoder = (LubyDecoder*)decoder;
    for (int i = 0; i < numBlocks; i++) {
        int outSize;
        int* indices = lubyDecoder->codec->base.PickIndices(&lubyDecoder->codec->base, blocks[i].blockCode, &outSize);
        
        // Add equation to the sparse matrix
        for (int j = 0; j < outSize; j++) {
            lubyDecoder->matrix.coeff[lubyDecoder->matrix.size][indices[j]] = 1;
        }
        lubyDecoder->matrix.v[lubyDecoder->matrix.size].data = blocks[i].data;
        lubyDecoder->matrix.v[lubyDecoder->matrix.size].length = blocks[i].length;
        lubyDecoder->matrix.size++;

        free(indices);
    }
    return lubyDecoder->matrix.size >= lubyDecoder->codec->sourceBlocks;
}

// Decode the message from the decoder
uint8_t* Decode_Luby(struct Decoder* decoder, int* outSize) {
    LubyDecoder* lubyDecoder = (LubyDecoder*)decoder;
    if (lubyDecoder->matrix.size < lubyDecoder->codec->sourceBlocks) {
        return NULL;
    }

    int messageSize = lubyDecoder->messageLength;
    uint8_t* decodedMessage = (uint8_t*)malloc(messageSize * sizeof(uint8_t));
    memset(decodedMessage, 0, messageSize);

    // Perform Gaussian elimination
    for (int i = 0; i < lubyDecoder->codec->sourceBlocks; i++) {
        if (lubyDecoder->matrix.coeff[i][i] == 0) {
            for (int j = i + 1; j < lubyDecoder->codec->sourceBlocks; j++) {
                if (lubyDecoder->matrix.coeff[j][i] != 0) {
                    int* tempCoeff = lubyDecoder->matrix.coeff[i];
                    lubyDecoder->matrix.coeff[i] = lubyDecoder->matrix.coeff[j];
                    lubyDecoder->matrix.coeff[j] = tempCoeff;

                    block tempV = lubyDecoder->matrix.v[i];
                    lubyDecoder->matrix.v[i] = lubyDecoder->matrix.v[j];
                    lubyDecoder->matrix.v[j] = tempV;
                    break;
                }
            }
        }
        for (int j = i + 1; j < lubyDecoder->codec->sourceBlocks; j++) {
            if (lubyDecoder->matrix.coeff[j][i] != 0) {
                for (int k = 0; k < lubyDecoder->codec->sourceBlocks; k++) {
                    lubyDecoder->matrix.coeff[j][k] ^= lubyDecoder->matrix.coeff[i][k];
                }
                xorBlocks(lubyDecoder->matrix.v[j].data, lubyDecoder->matrix.v[i].data, lubyDecoder->matrix.v[j].length);
            }
        }
    }

    // Back substitution
    for (int i = lubyDecoder->codec->sourceBlocks - 1; i >= 0; i--) {
        for (int j = i - 1; j >= 0; j--) {
            if (lubyDecoder->matrix.coeff[j][i] != 0) {
                xorBlocks(lubyDecoder->matrix.v[j].data, lubyDecoder->matrix.v[i].data, lubyDecoder->matrix.v[j].length);
            }
        }
    }

    // Copy decoded message
    for (int i = 0; i < lubyDecoder->codec->sourceBlocks; i++) {
        memcpy(decodedMessage + i * lubyDecoder->matrix.v[i].length, lubyDecoder->matrix.v[i].data, lubyDecoder->matrix.v[i].length);
    }

    *outSize = messageSize;
    return decodedMessage;
}

// Create a new Luby codec
Codec* NewLubyCodec(int sourceBlocks, unsigned int seed, double* degreeCDF, int degreeCDFLength) {
    LubyCodec* codec = (LubyCodec*)malloc(sizeof(LubyCodec));
    codec->sourceBlocks = sourceBlocks;
    codec->seed = seed;
    codec->degreeCDF = (double*)malloc(degreeCDFLength * sizeof(double));
    memcpy(codec->degreeCDF, degreeCDF, degreeCDFLength * sizeof(double));
    codec->degreeCDFLength = degreeCDFLength;

    codec->base.SourceBlocks = SourceBlocks_Luby;
    codec->base.GenerateIntermediateBlocks = GenerateIntermediateBlocks_Luby;
    codec->base.PickIndices = PickIndices_Luby;
    codec->base.NewDecoder = NewDecoder_Luby;
    codec->base.Free = FreeCodec_Luby;

    return (Codec*)codec;
}

// Encode a message into LT blocks
LTBlock* EncodeLTBlocks(Codec* codec, uint8_t* message, size_t messageLength, int64_t* encodedBlockIDs, int numIDs, int* outSize) {
    int blockLength;
    uint8_t** intermediateBlocks;
    codec->GenerateIntermediateBlocks(codec, message, messageLength, codec->SourceBlocks(codec), &intermediateBlocks, &blockLength);

    LTBlock* ltBlocks = (LTBlock*)malloc(numIDs * sizeof(LTBlock));
    for (int i = 0; i < numIDs; i++) {
        int outSize;
        int* indices = codec->PickIndices(codec, encodedBlockIDs[i], &outSize);
        ltBlocks[i].blockCode = encodedBlockIDs[i];
        block b = generateLubyTransformBlock((block*)intermediateBlocks, indices, outSize);
        ltBlocks[i].data = b.data;
        ltBlocks[i].length = b.length;
        free(indices);
    }

    for (int i = 0; i < codec->SourceBlocks(codec); i++) {
        free(intermediateBlocks[i]);
    }
    free(intermediateBlocks);

    *outSize = numIDs;
    return ltBlocks;
}

// Retrieve the number of source blocks for the Luby codec
int SourceBlocks_Luby(struct Codec* codec) {
    LubyCodec* lubyCodec = (LubyCodec*)codec;
    return lubyCodec->sourceBlocks;
}