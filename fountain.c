#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fountain.h"
#include "raptor.h"

// Raptor 코덱 생성
RaptorCodec* NewRaptorCodec(int symbols, int alignment) {
    RaptorCodec* codec = (RaptorCodec*)malloc(sizeof(RaptorCodec));
    codec->symbols = symbols;
    codec->alignment = alignment;
    return codec;
}

// 메시지를 LT 블록으로 인코딩합니다.
LTBlock* EncodeLTBlocks(uint8_t* message, int messageLen, int64_t* ids, int numIds, RaptorCodec* codec) {
    LTBlock* encodedBlocks = (LTBlock*)malloc(numIds * sizeof(LTBlock));
    
    // 여기에 실제 인코딩 논리를 구현해야 합니다. 임시로 데이터를 직접 복사합니다.
    for (int i = 0; i < numIds; i++) {
        encodedBlocks[i].data = (uint8_t*)malloc(messageLen * sizeof(uint8_t));
        memcpy(encodedBlocks[i].data, message, messageLen);
        encodedBlocks[i].length = messageLen;
        encodedBlocks[i].id = (int)ids[i];
    }
    
    return encodedBlocks;
}

// 메시지를 인코딩합니다.
LTBlock* Encode(uint8_t* message, int messageLen, int symbols, int alignment, int encBlocks, int* outSize) {
    RaptorCodec* codec = NewRaptorCodec(symbols, alignment);
    int64_t* ids = (int64_t*)malloc(encBlocks * sizeof(int64_t));

    // 시드 설정
    const char* seedMode = getenv("TIME_VARIED_SEED");
    srand(seedMode && strstr(seedMode, "true") ? time(NULL) : 8923489);

    for (int i = 0; i < encBlocks; i++) {
        ids[i] = rand() % 60000;
    }

    // 메시지 복사
    uint8_t* messageCopy = (uint8_t*)malloc(messageLen * sizeof(uint8_t));
    memcpy(messageCopy, message, messageLen);

    // LT 블록 인코딩
    LTBlock* encodedBlocks = EncodeLTBlocks(messageCopy, messageLen, ids, encBlocks, codec);

    // 결과 설정
    *outSize = encBlocks;

    // 메모리 해제
    free(messageCopy);
    free(ids);

    return encodedBlocks;
}

// 메시지를 디코딩합니다.
uint8_t* Decode(RaptorCodec* codec, LTBlock* encodedSymbols, int numSymbols, int messageSize, int* decodedSymbolIndex, int* outSize, uint8_t hash[32]) {
    raptorDecoder* decoder = newRaptorDecoder(codec, messageSize);
    uint8_t* output = NULL;

    for (int i = 0; i < numSymbols; i++) {
        AddBlocksToDecoder(decoder, &encodedSymbols[i], 1);
        if (decoder->matrix.determined()) {
            output = decoder->Decode(outSize);
            *decodedSymbolIndex = i + 1;
            break;
        }
    }

    if (!output) {
        *decodedSymbolIndex = -1;
    }

    // 디코더 해제 (사용자 정의 함수 필요)
    freeRaptorDecoder(decoder);

    return output;
}