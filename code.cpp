#include <iostream>
#include <iomanip>
#include <cstdio>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <stack>
#include <queue>
#include <deque>
#include <bitset>
#include <chrono>
#include <functional>
#include <numeric>
#include <sstream>

using namespace std;

#define PAGE_SIZE 256 // Page size, in bytes.
#define PAGE_COUNT 256
#define PAGE_NUM_BITS 8 // Page number size, in bits.
#define TLB_SIZE 16 // Max TLB entries.
#define FRAME_SIZE 256 // Frame size, in bytes.
#define FRAME_ENTRIES 256 // Number of frames in physical memory.
#define PHYSICAL_MEMORY_SIZE (FRAME_SIZE * FRAME_ENTRIES) // Mem size, in bytes.
#define BUFFER_SIZE 256

typedef long long ll;

char fileName[] = "disk_sim";

FILE *fp;

int TLB_Hit, TLB_Miss, pageFault;

vector<pair<int, int>> TLB(TLB_SIZE, make_pair(-1, -1));
vector<int> pageTable(PAGE_COUNT, -1);
vector<map<int, int>> RAM(FRAME_ENTRIES); //mapsize


int readFromDisk(int pageNum, char *PM, int *OF);
pair<int, int> getVirtPageNumAndPageOffset(ll virtAddress);
int TLBSearch(int virtPageNum);



int main() {

    //input

    ll virtualAddress=30338;
    auto virtPageInfo = getVirtPageNumAndPageOffset(virtualAddress);
    int virtPageNum =  virtPageInfo.first, pageOffset = virtPageInfo.second;

    int physicalAddress, value;

    int physicalPageNum = TLBSearch(virtPageNum);

    if (physicalPageNum == -1) {
        physicalPageNum = pageTable[virtPageNum];

        if (physicalPageNum == -1) {
            pageFault++;
            //read from disk
            //continue;
        }
        
    }

    value = RAM[physicalPageNum][pageOffset];
}


pair<int, int> getVirtPageNumAndPageOffset(ll virtAddress){
    string virtAddressBin = "";

    for(int i = 0; i < 16 ; i++){
        if(1 << i & virtAddress) virtAddressBin += "1";
        else virtAddressBin += "0";
    }

    string pageOffsetBin = virtAddressBin.substr(0, 8), virtPageNumBin = virtAddressBin.substr(8, 8);

    int virtPageNumInt = 0, pageOffsetInt = 0;

    for(int i = 0; i < 8; i++){
        virtPageNumInt += (virtPageNumBin[i]=='1') * (1 << i);
        pageOffsetInt += (pageOffsetBin[i] == '1') * (1 << i);
    }

    return make_pair(virtPageNumInt,  pageOffsetInt);
}



int TLBSearch(int virtPageNum){

    for(int i = 0; i < 16; i++){
        if (TLB[i].first == virtPageNum) {
            TLB_Hit++;
            return TLB[i].second;
        }
    }

    TLB_Miss++;
    return -1;
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