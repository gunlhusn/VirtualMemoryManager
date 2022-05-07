#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>

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
char inputName[] = "addresses.txt";
char outputName[] = "output.txt";

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
int toSignedConversion(int value);
void AddressesAndValueOutputFormat(ll virtualAddress, ll physicalAddress, int value);
void startOfFileFormat(string replacementAlgorithm);
void endOfFileFormat(int TLB_Hit, int pageFault, ll count);


int main() {

    string replacementAlgorithm;
    cout << "Enter replacement algorithm: ";
    cin >> replacementAlgorithm;

    transform(replacementAlgorithm.begin(), replacementAlgorithm.end(), replacementAlgorithm.begin(), ::toupper);
    if (replacementAlgorithm != "FIFO" && replacementAlgorithm != "LRU") {
        return -1;
    }

    RAMInit();

    FILE *input = freopen(inputName, "r", stdin);
    if (input == NULL) {
        cout << "Failed to open the file" << endl;
        return -1;
    }

    FILE *output = freopen(outputName, "w", stdout);
    if (output == NULL) {
        cout << "Failed to open the file" << endl;
        return -1;
    }

    startOfFileFormat(replacementAlgorithm);

    ll virtualAddress, count = 0;
    while (cin >> virtualAddress) {

        count++;
        int virtPageNum = virtualAddress / PAGE_SIZE, pageOffset = virtualAddress % PAGE_SIZE;

        int physicalPageNum = TLBSearch(virtPageNum);

        if (physicalPageNum == -1) {
            physicalPageNum = pageTable[virtPageNum];

            if (physicalPageNum == -1) {
                pageFault++;
                physicalPageNum = readFromDisk(virtPageNum, pageOffset);
            }

            allocateInTLB(virtPageNum, physicalPageNum);
        }

        int physicalAddress = physicalPageNum * FRAME_SIZE + pageOffset;
        int value = RAM[physicalPageNum][pageOffset];

        AddressesAndValueOutputFormat(virtualAddress, physicalAddress, value);
    }

    endOfFileFormat(TLB_Hit, pageFault, count);
    fclose(input);
    fclose(output);
}

void startOfFileFormat(string replacementAlgorithm) {
    cout << "Welcome to Team 1's Virtual Memory Simulator!\n\nSystem parameters:\n\n" <<
         "Program address space: 16-bit\nPage size: 2^8 bytes\nTLB capacity: 16 entries\n" <<
         "Number of frames: 256\nPhysical memory size: 64 KB\nReplacement algorithm: " << replacementAlgorithm << endl
         << endl;

    cout << "----------------------------------------------------------------------------------\n\nStarting...\n\n";
}

void endOfFileFormat(int TLB_Hit, int pageFault, ll count) {
    cout << "\nFinished!\n\n"
         << "-----------------------------------------------------------------------------------\n\n"
         << "Statistics:\n\nTranslated addresses: " << count << endl << "Page fault rate: " << pageFault * 100.0 / 3000
         << "%\n"
         << "TLB hit rate: " << TLB_Hit * 100.0 / 3000 << "%\n\n" <<
         "-----------------------------------------------------------------------------------";
}

void AddressesAndValueOutputFormat(ll virtualAddress, ll physicalAddress, int value) {
    cout << "Virtual address: " << virtualAddress << "\t\tPhysical address: " << physicalAddress << "\t\tValue: "
         << value << endl;
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
        file.seekg(pageNum * PAGE_SIZE + i, ios::beg);
        file.read((char *) &val, sizeof(char));
        page.push_back(val);
        if (page[i] > 127) {
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

int toSignedConversion(int value) {
    string valueBin = "";

    for (int i = 0; i < 8; i++) {
        if (1 << i & value) valueBin += "1";
        else valueBin += "0";
    }

    int carry = 1;
    for (int i = 0; i < 8; i++) {
        if (carry && valueBin[i] == '1') {
            carry = 0;
        } else if (!carry) {
            valueBin[i] = (valueBin[i] == '1' ? '0' : '1');
        }
    }

    value = 0;
    for (int i = 0; i < 8; i++) {
        value += (valueBin[i] == '1') * (1 << i);
    }

    return -1 * value;
}
