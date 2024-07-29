#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "block.h"
#include "util.c"

// 블록 생성
block* newBlock(int len) {
    block* b = (block*)malloc(sizeof(block));
    b->data = (uint8_t*)calloc(len, sizeof(uint8_t)); 
    b->length = 0;
    b->padding = len;
    return b;
}

int blockLength(block* b) {
    return b->length + b->padding;
}

bool blockEmpty(block* b) {
    return blockLength(b) == 0;
}
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

int** PartitionBytes(uint8_t* in, int inLen, int p) {
    int lenLong, lenShort;

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

void addEquation(sparseMatrix* m, int* components, int numComponents, block b) {
    while (numComponents > 0 && m->coeff[components[0]] != NULL) {
        int s = components[0];
        if (numComponents >= sizeof(m->coeff[s]) / sizeof(int)) {
            components = xorRow(s, components, numComponents, &b, &numComponents, &b);
        } else {
            
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

int* xorRow(sparseMatrix *m, int s, int *indices, block *b, size_t indices_len, size_t *new_indices_len) {

    xor_block(b, &m->v[s]);

    int *newIndices = malloc((indices_len + 100) * sizeof(int)); 
    *new_indices_len = 0;

    int *coeffs = m->coeff[s];
    size_t i = 0, j = 0;
    size_t coeffs_len = 0;
    
    while (coeffs[coeffs_len] != -1) {
        coeffs_len++;
    }

    while (i < coeffs_len && j < indices_len) {
        int index = indices[j];
        if (coeffs[i] == index) {
            i++;
            j++;
        } else if (coeffs[i] < index) {
            newIndices[*new_indices_len] = coeffs[i];
            (*new_indices_len)++;
            i++;
        } else {
            newIndices[*new_indices_len] = index;
            (*new_indices_len)++;
            j++;
        }
    }

    while (i < coeffs_len) {
        newIndices[*new_indices_len] = coeffs[i];
        (*new_indices_len)++;
        i++;
    }

    while (j < indices_len) {
        newIndices[*new_indices_len] = indices[j];
        (*new_indices_len)++;
        j++;
    }

    return newIndices;
}

bool determined(sparseMatrix* m) {
    for (int i = 0; i < m->numRows; i++) {
        if (m->coeff[i] == NULL || m->coeff[i][0] != i) {
            return false;
        }
    }
    return true;
}

void Reduce(sparseMatrix* m) {
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
        m->coeff[i] = (int*)realloc(m->coeff[i], sizeof(int));
        m->coeff[i][0] = i;
    }
}

uint8_t* Reconstruct(sparseMatrix* m, int totalLength, int lenLong, int lenShort, int numLong, int numShort) {
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