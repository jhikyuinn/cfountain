#ifndef FOUNTAIN_H
#define FOUNTAIN_H

#include <stdint.h>
#include <stdbool.h>
#include "raptor.h"

// 함수 선언
RaptorCodec* NewRaptorCodec(int symbols, int alignment);
LTBlock* EncodeLTBlocks(uint8_t* message, int messageLen, int64_t* ids, int numIds, RaptorCodec* codec);
uint8_t* Decode(RaptorCodec* codec, LTBlock* encodedSymbols, int numSymbols, int messageSize, int* decodedSymbolIndex, int* outSize, uint8_t hash[32]);

#endif // FOUNTAIN_H