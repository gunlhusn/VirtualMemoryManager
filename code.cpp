#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <iomanip>

using namespace std;

#define PROGRAM_ADDRESS_SPACE 16
#define PAGE_NUM_BITS 8
#define PAGE_SIZE (1 << PAGE_NUM_BITS)
#define PAGE_COUNT (1 << (PROGRAM_ADDRESS_SPACE - PAGE_NUM_BITS))
#define TLB_SIZE 16
#define FRAME_SIZE 256
#define FRAME_ENTRIES 256
#define PHYSICAL_MEMORY_SIZE (FRAME_SIZE * FRAME_ENTRIES) / 1024 // Memory size, in KB.
#define VALUE_BITS 8

typedef long long ll;

char diskName[] = "disk_sim";
char inputName[] = "addresses.txt";
char outputName[] = "output.txt";

int TLB_Hit, pageFault, RAMPageCounter;
string replacementAlgorithm;
ifstream disk;

//contents of RAM entry, including frame, r bit, and corresponding virtual page number
class entryRAM {
public:
    vector<int> frame;
    int R;
    int pageTableEntry;
};

deque<pair<int, int>> TLB(TLB_SIZE, make_pair(-1, -1));
vector<int> pageTable(PAGE_COUNT, -1);
vector<entryRAM> RAM(FRAME_ENTRIES);

void RAM_Init();
int TLB_Search(int virtualPageNumber);
int readFromDisk(int pageNumber);
void allocateInPageTable(vector<int> page, int virtualPageNumber);
void clockAlgorithm();
void allocateInTLB(int virtualPageNumber, int physicalPageNumber);
void updateTLB(int virtualPageNumber, int physicalPageNumber);
int toSignedConversion(int value);
void startOfFileFormat(string replacementAlgorithm);
void addressesAndValueOutputFormat(ll virtualAddress, ll physicalAddress, int value);
void endOfFileFormat(int TLB_Hit, int pageFault, ll count);


int main() {

    cout << "Enter replacement algorithm: ";
    cin >> replacementAlgorithm;

    transform(replacementAlgorithm.begin(), replacementAlgorithm.end(), replacementAlgorithm.begin(), ::toupper);
    if (replacementAlgorithm != "FIFO" && replacementAlgorithm != "LRU") {
        return -1;
    }

    RAM_Init();

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

    disk.open(diskName, ios::binary | ios::in);
    if (!disk.is_open()) {
        cout << "Failed to open the file" << endl;
        return -1;
    }

    startOfFileFormat(replacementAlgorithm);

    ll virtualAddress, count = 0;
    while (cin >> virtualAddress) {

        count++;
        int virtualPageNumber = virtualAddress / PAGE_SIZE, pageOffset = virtualAddress % PAGE_SIZE;

        int physicalPageNumber = TLB_Search(virtualPageNumber);
        //if virtual page number is not in TLB
        if (physicalPageNumber == -1) {
            physicalPageNumber = pageTable[virtualPageNumber];
            //if virtual page number is not in page table
            if (physicalPageNumber == -1) {
                pageFault++;
                physicalPageNumber = readFromDisk(virtualPageNumber);
                //for cases when physical address space is smaller than virtual address space (for clock algorithm)
                RAMPageCounter %= FRAME_ENTRIES;
            }

            allocateInTLB(virtualPageNumber, physicalPageNumber);
        }
        //set r bit of this page entry (most recently used entry) to 1
        RAM[physicalPageNumber].R = 1;
        int physicalAddress = physicalPageNumber * FRAME_SIZE + pageOffset;
        int value = RAM[physicalPageNumber].frame[pageOffset];

        addressesAndValueOutputFormat(virtualAddress, physicalAddress, value);
    }

    endOfFileFormat(TLB_Hit, pageFault, count);
    fclose(input);
    fclose(output);
    disk.close();
}

void RAM_Init() {
    //initialize RAM
    for (int i = 0; i < FRAME_ENTRIES; i++) {
        RAM[i].frame.resize(FRAME_SIZE, -1);
        RAM[i].R = 0;
        RAM[i].pageTableEntry = -1;
    }
}

int TLB_Search(int virtualPageNumber) {
    //search for a virtual page number in TLB
    for (int i = 0; i < TLB_SIZE; i++) {
        if (TLB[i].first == virtualPageNumber) {
            auto TLBEntry = TLB[i];
            //if replacement algorithm is LRU, then remove the most recently used page and add it to the front
            if (replacementAlgorithm == "LRU") {
                TLB.erase(TLB.begin() + i);
                TLB.push_back(TLBEntry);
            }
            TLB_Hit++;
            //return physical page number
            return TLBEntry.second;
        }
    }
    //if virtual page number is not found in TLB, return -1
    return -1;
}

int readFromDisk(int pageNumber) {
    //read values from disk
    int val;
    vector<int> page;

    for (int i = 0; i < 256; i++) {
        //set the position of the stream to the address at specified page number
        disk.seekg(pageNumber * PAGE_SIZE + i, ios::beg);
        //read the entries in the page one by one
        disk.read((char *) &val, sizeof(char));
        page.push_back(val);
        //if number is above 2^7 threshold, then convert it to negative/signed value
        if (page[i] > (1 << (VALUE_BITS - 1)) - 1) {
            page[i] = toSignedConversion(page[i]);
        }
    }

    allocateInPageTable(page, pageNumber);
    //return physical page number (increment is for indicating the oldest page entry)
    return RAMPageCounter++;
}

void allocateInPageTable(vector<int> page, int virtualPageNumber) {
    //allocate page in RAM and update corresponding page table entry
    clockAlgorithm();
    RAM[RAMPageCounter].frame = page;
    RAM[RAMPageCounter].pageTableEntry = virtualPageNumber;
    pageTable[virtualPageNumber] = RAMPageCounter;
}

void clockAlgorithm() {
    //search for the entry, the r bit of which is 0 (old & least recently used)
    while (RAM[RAMPageCounter].R) {
        RAM[RAMPageCounter++].R = 0;
        //imitating circular list
        RAMPageCounter %= FRAME_ENTRIES;
    }
    //for cases when this old virtual address is in TLB
    for (int i = 0; i < TLB_SIZE; i++) {
        if (TLB[i].first != -1 && TLB[i].first == RAM[RAMPageCounter].pageTableEntry) {
            //erase that entry from TLB
            TLB[i] = make_pair(-1, -1);
        }
    }
    //updating page table entry for previous, corresponding virtual page number (deleting physical page number)
    pageTable[RAM[RAMPageCounter].pageTableEntry] = -1;
}

void allocateInTLB(int virtualPageNumber, int physicalPageNumber) {
    //allocate current entry (virtual & physical page numbers) in TLB
    for (int i = 0; i < TLB_SIZE; i++) {
        //search for a free spot in TLB table
        if (TLB[i] == make_pair(-1, -1)) {
            TLB[i] = make_pair(virtualPageNumber, physicalPageNumber);
            return;
        }
    }
    //if there is no free slot in TLB, update it by calling the following function
    updateTLB(virtualPageNumber, physicalPageNumber);
}

void updateTLB(int virtualPageNumber, int physicalPageNumber) {
    //remove the oldest page (in case of LRU: the least recently used page as well) and input the new TLB entry
    TLB.pop_front();
    TLB.push_back(make_pair(virtualPageNumber, physicalPageNumber));
}

int toSignedConversion(int value) {
    string valueBin = "";
    //concert the number to its binary (string) representation
    for (int i = 0; i < VALUE_BITS; i++) {
        if (1 << i & value) valueBin += "1";
        else valueBin += "0";
    }
    //finding two's complement
    int carry = 1;
    for (int i = 0; i < VALUE_BITS; i++) {
        if (carry && valueBin[i] == '1') {
            carry = 0;
        } else if (!carry) {
            valueBin[i] = (valueBin[i] == '1' ? '0' : '1');
        }
    }
    //convert two's complement back to a number
    value = 0;
    for (int i = 0; i < VALUE_BITS; i++) {
        value += (valueBin[i] == '1') * (1 << i);
    }
    //return the negative value
    return -1 * value;
}

void startOfFileFormat(string replacementAlgorithm) {
    cout << "Welcome to Team 1's Virtual Memory Simulator!\n\nSystem parameters:\n\n" <<
         "Program address space: " << PROGRAM_ADDRESS_SPACE << "-bit\nPage size: 2^" << PAGE_NUM_BITS
         << " bytes\nTLB capacity: " << TLB_SIZE << " entries\n" <<
         "Number of frames: " << FRAME_ENTRIES << "\nPhysical memory size: " << PHYSICAL_MEMORY_SIZE
         << " KB\nReplacement algorithm: " << replacementAlgorithm << endl
         << endl;

    cout << "----------------------------------------------------------------------------------\n\nStarting...\n\n";
}

void addressesAndValueOutputFormat(ll virtualAddress, ll physicalAddress, int value) {
    cout << "Virtual address: ";
    cout.width(5);
    cout << left << virtualAddress;
    cout << "\t\tPhysical address: ";
    cout.width(5);
    cout << left << physicalAddress;
    cout << "\t\tValue: " << value << endl;
}

void endOfFileFormat(int TLB_Hit, int pageFault, ll count) {
    cout << "\nFinished!\n\n"
         << "-----------------------------------------------------------------------------------\n\n"
         << "Statistics:\n\nTranslated addresses: " << count << endl << "Page fault rate: " << pageFault * 100.0 / 3000
         << "%\n"
         << "TLB hit rate: " << TLB_Hit * 100.0 / 3000 << "%\n\n" <<
         "-----------------------------------------------------------------------------------";
}