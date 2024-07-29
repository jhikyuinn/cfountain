#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>
#include <stdbool.h>

// block 구조체: 데이터 블록을 나타냅니다.
typedef struct {
    uint8_t* data;  // 블록의 데이터
    int length;     // 데이터의 길이
    int padding;    // 패딩의 길이
} block;

// sparseMatrix 구조체: 희소 행렬을 나타냅니다.
typedef struct {
    int** coeff;   // 행렬의 계수
    block* v;      // 행렬의 값 (블록 배열)
    int numRows;   // 행렬의 행 수
} sparseMatrix;

// 함수 선언
block* newBlock(int len);
int blockLength(block* b);
bool blockEmpty(block* b);
void blockXor(block* b, block* a);
int** PartitionBytes(uint8_t* in, int inLen, int p, block** longBlocks, int* numLong, block** shortBlocks, int* numShort);
void addEquation(sparseMatrix* m, int* components, int numComponents, block b);
int* xorRow(sparseMatrix *m, int s, int *indices, size_t indices_len, block *b, size_t *new_indices_len);
bool determined(sparseMatrix* m);
void Reduce(sparseMatrix* m);
uint8_t* Reconstruct(sparseMatrix* m, int totalLength, int lenLong, int lenShort, int numLong, int numShort);

#endif // BLOCK_H