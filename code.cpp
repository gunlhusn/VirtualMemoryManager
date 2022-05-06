#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>

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

char fileName[] = R"(\Users\GH\source\repos\VirtualMemory\disk_sim)";
char inputName[] = R"(\Users\GH\source\repos\VirtualMemory\addresses.txt)";
char outputName[] = R"(\Users\GH\source\repos\VirtualMemory\output.txt)";

int TLB_Hit, pageFault, RAMPageCounter;

deque<pair<int, int>> TLB(TLB_SIZE, make_pair(-1, -1));
vector<int> pageTable(PAGE_COUNT, -1);
vector<vector<int>> RAM(FRAME_ENTRIES);

void RAMInit();

int TLBSearch(int virtPageNum);

int readFromDisk(int pageNum, int pageOffset);

void allocateInPageTable(vector<int> page, int virtPageNum);

void allocateInTLB(int virtPageNum, int physicalPageNum);

void FIFO(int virtPageNum, int physicalPageNum);

int toSignedConversion (int value);


int main() {

    RAMInit();

    freopen(inputName, "r", stdin);
    freopen(outputName, "w", stdout);

    ll virtualAddress;
    while (cin >> virtualAddress){

        int virtPageNum = virtualAddress / PAGE_SIZE, pageOffset = virtualAddress % PAGE_SIZE;

        int physicalPageNum = TLBSearch(virtPageNum);

        if (physicalPageNum == -1) {
            physicalPageNum = pageTable[virtPageNum];

            if (physicalPageNum == -1) {
                pageFault++;
                physicalPageNum = readFromDisk(virtPageNum, pageOffset);

                //if file failed to open
                if (physicalPageNum == -1){
                    return 0;
                }
            } 

            allocateInTLB(virtPageNum, physicalPageNum);
        }

        int physicalAddress = physicalPageNum * FRAME_SIZE + pageOffset;
        int value = RAM[physicalPageNum][pageOffset];

        cout << virtualAddress << " " << physicalAddress << " " << value << endl;
    }

    cout << TLB_Hit * 100.0 / 3000 << "%" << endl << pageFault * 100.0 / 3000 << "%" << endl;
}

void RAMInit() {
    for (int i = 0; i < FRAME_ENTRIES; i++) {
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

    ifstream file;
    file.open(fileName, ios::binary | ios::in); 
    if (!file.is_open()) {
        cout << "Failed to open the file" << endl;
        return -1;
    }
        
    int val;
    vector<int> page;

    for (int i = 0; i < 256; i++) {
        file.seekg(pageNum * PAGE_SIZE+ i, ios::beg);
        file.read((char *)&val, sizeof(char));
        page.push_back(val);
        if(page[i] > 127){
            page[i] = toSignedConversion(page[i]);
        }
    }

        allocateInPageTable(page, pageNum);
        return RAMPageCounter++;
}

void allocateInPageTable(vector<int> page, int virtPageNum) {

    RAM[RAMPageCounter] = page;
    pageTable[virtPageNum] = RAMPageCounter;
}

void allocateInTLB(int virtPageNum, int physicalPageNum) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (TLB[i] == make_pair(-1, -1)) {
            TLB[i] = make_pair(virtPageNum, physicalPageNum);
            return;
        }
    }
    FIFO(virtPageNum, physicalPageNum);
}

void FIFO(int virtPageNum, int physicalPageNum) {
    TLB.pop_front();
    TLB.push_back(make_pair(virtPageNum, physicalPageNum));
}

int toSignedConversion (int value) {
    string valueBin = "";

    for (int i = 0; i < 8; i++) {
        if (1 << i & value) valueBin += "1";
        else valueBin += "0";
    }

    int carry = 1;
    for (int i = 0; i < 8; i++) {
        if(carry && valueBin[i] == '1'){
            carry = 0;
        }
        else if(!carry) {
            valueBin[i] = (valueBin[i] == '1'? '0' : '1');
        }
    }

    value = 0;
    for (int i = 0; i < 8; i++) {
        value += (valueBin[i] == '1') * (1 << i);
    }

    return -1*value;
}
