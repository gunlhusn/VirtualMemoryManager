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

int TLB_Hit, pageFault;

int RAMPageCounter;

deque <pair<int, int>> TLB(TLB_SIZE, make_pair(-1, -1));
vector<int> pageTable(PAGE_COUNT, -1);
vector <vector<int>> RAM(FRAME_ENTRIES); 

void RAMInit();
int TLBSearch(int virtPageNum);
int readFromDisk(int pageNum, int pageOffset);
void allocateInPageTable(vector<int> page, int virtPageNum);
void allocateInTLB(int virtPageNum, int physicalPageNum);
void FIFO(int virtPageNum, int physicalPageNum);


int main() {

    RAMInit();

    ll virtualAddress;
    while(cin >> virtualAddress){

        int virtPageNum = virtualAddress/PAGE_SIZE, pageOffset = virtualAddress%PAGE_SIZE;

        int value;

        int physicalPageNum = TLBSearch(virtPageNum);

        if (physicalPageNum == -1) {
            physicalPageNum = pageTable[virtPageNum];

            if (physicalPageNum == -1) {
                pageFault++;
                physicalPageNum = readFromDisk(virtPageNum, pageOffset);
            }                          
            else{
                allocateInTLB(virtPageNum, physicalPageNum);   
            }         
        }

        value = RAM[physicalPageNum][pageOffset];

    }
    
    cout << TLB_Hit* 100.0/3000  << "%" << endl << pageFault*100.0/3000 << "%" << endl;
}

void RAMInit(){
    for(int i = 0; i < FRAME_ENTRIES; i++){
        RAM[i].resize(FRAME_SIZE, -1);
    }
}

 
int TLBSearch(int virtPageNum) {

    for (int i = 0; i < TLB_SIZE; i++) {
        if (TLB[i].first == virtPageNum) {
            TLB_Hit++;
            return TLB[i].second;
        }
    }
    return -1;
}

int readFromDisk(int pageNum, int pageOffset) {

//    fp = fopen(fileName, "rb");
//    if (fp == NULL) {
//        cout << "Failed to open the file" << endl;
//        return 1;
//    }
//    auto p = malloc(256);
//    auto *value = &p;
//    int idk = fseek(fp, pageNum * PAGE_SIZE + pageOffset, SEEK_SET);
//    fread(value, 256, 1, fp);

    vector<int> v (256, 0);
    allocateInPageTable(v, pageNum);
    return RAMPageCounter++;
}

void allocateInPageTable(vector<int> page, int virtPageNum){
    
    RAM[RAMPageCounter] = page;
    pageTable[virtPageNum] = RAMPageCounter;
    allocateInTLB(virtPageNum, RAMPageCounter);
}

void allocateInTLB(int virtPageNum, int physicalPageNum){
    for(int i = 0; i < TLB_SIZE; i++){
        if (TLB[i] == make_pair(-1, -1)) {
            TLB[i] = make_pair(virtPageNum, physicalPageNum);
            return;
        }
    }
    FIFO(virtPageNum, physicalPageNum); 
}

void FIFO(int virtPageNum, int physicalPageNum){
    TLB.pop_front();
    TLB.push_back(make_pair(virtPageNum, physicalPageNum));    
}

