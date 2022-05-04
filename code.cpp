#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <cstring>

#define PAGE_SIZE 256 // Page size, in bytes.
#define PAGE_NUM_BITS 8 // Page number size, in bits.
#define TLB_SIZE 16 // Max TLB entries.
#define FRAME_SIZE 256 // Frame size, in bytes.
#define FRAME_ENTRIES 256 // Number of frames in physical memory.
#define PHYSICAL_MEMORY_SIZE (FRAME_SIZE * FRAME_ENTRIES) // Mem size, in bytes.
#define BUFFER_SIZE 256

char fileName[] = "disk_sim";

FILE *fp;

int readFromDisk(int pageNum, char *PM, int *OF);

int main() {
}

//НЕ РАБОТАЕТ НО ВСЕЖЕ

//int readFromDisk(int pageNum, char *PM, int *OF) {
//
//    char buffer[BUFFER_SIZE];
//    memset(buffer, 0, sizeof(buffer));
//
//    fp = fopen(fileName, "rb");
//    if (fp == NULL) {
//        printf("Failed to open the file\n");
//        exit(0);
//    }
//
////	printf("offset in fseek: %d\n", pageNum*PHYS_MEM_SIZE);
//
//    if (fseek(fp, pageNum * PHYSICAL_MEMORY_SIZE, SEEK_SET) != 0)
//        printf("fseek error\n");
//
//    if (fread(buffer, sizeof(char), PHYSICAL_MEMORY_SIZE, fp) == 0)
//        printf("fread error\n");
//
//    for (int i = 0; i < PHYSICAL_MEMORY_SIZE; i++) {
//        *((PM + (*OF) * PHYSICAL_MEMORY_SIZE) + i) = buffer[i];
//        printf("buffer[%d]=%d\n", i + pageNum * 256, buffer[i]);
//    }
//
//    (*OF)++;
//
//    fclose(fp);
//
//    return (*OF) - 1;
//}