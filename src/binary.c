#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "../include/binary.h"
#include "../include/block.h"
#include "../include/luby.h"
#include "util.c"


binaryCodec* NewBinaryCodec(int numSourceBlocks) {
    binaryCodec* codec = (binaryCodec*)malloc(sizeof(binaryCodec));
    codec->numSourceBlocks = numSourceBlocks;
    return codec;
}

int BinaryCodec_SourceBlocks(binaryCodec* c) {
    return c->numSourceBlocks;
}

int* PickIndices(binaryCodec* codec, int64_t codeBlockIndex, int* outLength) {
    srand((unsigned int)codeBlockIndex); // Seed random generator

    int* indices = (int*)malloc(codec->numSourceBlocks * sizeof(int));
    int count = 0;
    for (int b = 0; b < codec->numSourceBlocks; b++) {
        if (rand() % 2 == 1) {
            indices[count++] = b;
        }
    }

    *outLength = count;
    return indices;
}

block* GenerateIntermediateBlocks(binaryCodec* codec, uint8_t* message, int messageLength, int numBlocks) {
    int longBlockSize, shortBlockSize;
    int** partitions = PartitionBytes(message, messageLength, codec->numSourceBlocks, &longBlockSize, &shortBlockSize);
    block* source = equalizeBlockLengths(partitions, partitions + codec->numSourceBlocks, codec->numSourceBlocks, codec->numSourceBlocks, longBlockSize, shortBlockSize);
    
    for (int i = 0; i < codec->numSourceBlocks; i++) {
        free(partitions[i]);
    }
    free(partitions);

    return source;
}

binaryDecoder* NewDecoder(binaryCodec* codec, int messageLength) {
    binaryDecoder* decoder = (binaryDecoder*)malloc(sizeof(binaryDecoder));
    decoder->codec = *codec;
    decoder->messageLength = messageLength;
    decoder->matrix.coeff = (int**)malloc(codec->numSourceBlocks * sizeof(int*));
    decoder->matrix.v = (block*)malloc(codec->numSourceBlocks * sizeof(block));
    decoder->matrix.numRows = codec->numSourceBlocks;
    return decoder;
}

bool AddBlocks(binaryDecoder* decoder, LTBlock* blocks, int numBlocks) {
    for (int i = 0; i < numBlocks; i++) {
        int outLength = 0;
        int* indices = PickIndices(&decoder->codec, blocks[i].blockCode, &outLength);
        
        // Assuming addEquation is a function that needs to be implemented
        // decoder->matrix.addEquation(indices, block{data: blocks[i].Data, length: blocks[i].length});
        
        free(indices);
    }
    return true; 
}

uint8_t* Decode(binaryDecoder* decoder, int* outLength) {
    if (!true) { 
        *outLength = 0;
        return NULL;
    }

    decoder->Reduce();

    int lenLong, lenShort, numLong, numShort;
    partition(decoder->messageLength, decoder->codec.numSourceBlocks, &lenLong, &lenShort, &numLong, &numShort);
    
    return decoder->Reconstruct(decoder->messageLength, lenLong, lenShort, numLong, numShort);

    *outLength = decoder->messageLength; 
    uint8_t* result = (uint8_t*)malloc(decoder->messageLength * sizeof(uint8_t));
    return result;
}