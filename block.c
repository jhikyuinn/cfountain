#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "block.h"

// 새로운 블록을 생성합니다.
block* newBlock(int len) {
    block* b = (block*)malloc(sizeof(block));
    b->data = (uint8_t*)calloc(len, sizeof(uint8_t)); // 패딩으로 초기화
    b->length = 0;
    b->padding = len;
    return b;
}

// 블록의 총 길이를 반환합니다.
int blockLength(block* b) {
    return b->length + b->padding;
}

// 블록이 비어 있는지 확인합니다.
bool blockEmpty(block* b) {
    return blockLength(b) == 0;
}

// 두 블록을 XOR 연산합니다.
void blockXor(block* b, block* a) {
    if (b->length < a->length) {
        b->data = (uint8_t*)realloc(b->data, a->length);
        if (b->padding > a->length - b->length) {
            b->padding -= a->length - b->length;
        } else {
            b->padding = 0;
        }
    }

    for (int i = 0; i < a->length; i++) {
        b->data[i] ^= a->data[i];
    }
}

// 입력 데이터를 블록으로 분할합니다.
void PartitionBytes(uint8_t* in, int inLen, int p, block** longBlocks, int* numLong, block** shortBlocks, int* numShort) {
    int lenLong, lenShort;
    // 길이를 계산합니다 (partition 함수의 구현이 필요합니다)
    partition(inLen, p, &lenLong, &lenShort, numLong, numShort);

    *longBlocks = (block*)malloc((*numLong) * sizeof(block));
    *shortBlocks = (block*)malloc((*numShort) * sizeof(block));

    for (int i = 0; i < *numLong; i++) {
        (*longBlocks)[i].data = (uint8_t*)malloc(lenLong);
        if (inLen >= lenLong) {
            memcpy((*longBlocks)[i].data, in, lenLong);
            (*longBlocks)[i].length = lenLong;
            (*longBlocks)[i].padding = 0;
            in += lenLong;
            inLen -= lenLong;
        } else {
            memcpy((*longBlocks)[i].data, in, inLen);
            (*longBlocks)[i].length = inLen;
            (*longBlocks)[i].padding = lenLong - inLen;
            inLen = 0;
        }
    }

    for (int i = 0; i < *numShort; i++) {
        (*shortBlocks)[i].data = (uint8_t*)malloc(lenShort);
        if (inLen >= lenShort) {
            memcpy((*shortBlocks)[i].data, in, lenShort);
            (*shortBlocks)[i].length = lenShort;
            (*shortBlocks)[i].padding = 0;
            in += lenShort;
            inLen -= lenShort;
        } else {
            memcpy((*shortBlocks)[i].data, in, inLen);
            (*shortBlocks)[i].length = inLen;
            (*shortBlocks)[i].padding = lenShort - inLen;
            inLen = 0;
        }
    }
}

// 행렬에 방정식을 추가합니다.
void addEquation(sparseMatrix* m, int* components, int numComponents, block b) {
    while (numComponents > 0 && m->coeff[components[0]] != NULL) {
        int s = components[0];
        if (numComponents >= sizeof(m->coeff[s]) / sizeof(int)) {
            components = m->xorRow(s, components, numComponents, b, &numComponents, &b);
        } else {
            // 기존 행을 새로운 행으로 교환하고, 기존 행을 줄여서 다른 곳에 배치합니다.
            int* tempComponents = m->coeff[s];
            m->coeff[s] = components;
            components = tempComponents;

            block tempBlock = m->v[s];
            m->v[s] = b;
            b = tempBlock;
        }
    }

    if (numComponents > 0) {
        m->coeff[components[0]] = components;
        m->v[components[0]] = b;
    }
}

// 행렬이 완전히 채워졌는지 확인합니다.
bool matrixDetermined(sparseMatrix* m) {
    for (int i = 0; i < m->numRows; i++) {
        if (m->coeff[i] == NULL || m->coeff[i][0] != i) {
            return false;
        }
    }
    return true;
}

// 행렬의 가우스 소거법을 수행합니다.
void matrixReduce(sparseMatrix* m) {
    for (int i = m->numRows - 1; i >= 0; i--) {
        for (int j = 0; j < i; j++) {
            int* ci = m->coeff[i];
            int* cj = m->coeff[j];
            for (int k = 1; k < sizeof(cj) / sizeof(int); k++) {
                if (cj[k] == ci[0]) {
                    blockXor(&m->v[j], &m->v[i]);
                    break;
                }
            }
        }
        // 모든 행에서 선행 계수를 제외한 모든 항을 제거합니다.
        m->coeff[i] = (int*)realloc(m->coeff[i], sizeof(int));
        m->coeff[i][0] = i;
    }
}

// 재구성된 데이터를 반환합니다.
uint8_t* matrixReconstruct(sparseMatrix* m, int totalLength, int lenLong, int lenShort, int numLong, int numShort) {
    uint8_t* out = (uint8_t*)malloc(totalLength * sizeof(uint8_t));
    int offset = 0;
    
    for (int i = 0; i < numLong; i++) {
        memcpy(out + offset, m->v[i].data, lenLong);
        offset += lenLong;
    }

    for (int i = numLong; i < numLong + numShort; i++) {
        memcpy(out + offset, m->v[i].data, lenShort);
        offset += lenShort;
    }

    return out;
}