#include <cmath>
#include "Stash.h"
#include <string>
#include <iostream>

Stash::Stash(uint64_t rows, std::vector<std::string> &spacedSeeds, int T1, int T2, int readIDHashTiles) {
    this->rows = rows;
    this->spacedSeeds = spacedSeeds;
    this->readIDHashTiles = readIDHashTiles;
    this->T1 = T1;
    this->T2 = T2;
    computeSecondaryParameters();
    data = new int[rows * intsPerRow];
    for(uint64_t i = 0; i < rows * intsPerRow; i++){
        data[i] = 0;
    }
}

Stash::Stash(const char *stashPath) {
    FILE* file = fopen(stashPath, "rb");
    if(file == nullptr){
        std::cout << "File cannot be opened: " << stashPath << std::endl;
        exit(-1);
    }
    fread(&numOfUsedReads, sizeof(int), 1, file);
    fread(&spacedSeedLength, sizeof(int), 1, file);
    fread(&hashFuncNum, sizeof(int), 1, file);
    char spacedSeedi[200];
    for (int i = 0; i < hashFuncNum; i++){
        fread(spacedSeedi, sizeof(char), spacedSeedLength, file);
        spacedSeedi[spacedSeedLength] = 0;
        spacedSeeds.emplace_back(spacedSeedi);
    }
    fread(&rows, sizeof(uint64_t), 1, file);
    fread(&readIDHashTiles, sizeof(int), 1, file);
    fread(&T1, sizeof(int), 1, file);
    fread(&T2, sizeof(int), 1, file);
    computeSecondaryParameters();
    data = new int[rows * intsPerRow];
    fread(data, sizeof(int), rows * intsPerRow, file);
    fclose(file);
}

void Stash::computeSecondaryParameters(){
    for(int i = 0; i < LOCKS; i++)
        omp_init_lock(&(locks[i]));

    B1 = T1 * readIDHashTiles;
    B2 = T2 * readIDHashTiles;
    stashRowTiles = (int) pow(2, T1);
    int BS = T2 * stashRowTiles;
    intsPerRow = BS /intSize;
    segmentsPerInteger = intSize / T2;
    segmentMax = (int) pow(2, T2);
    spacedSeedLength = (int) spacedSeeds[0].length();
    hashFuncNum = (int) spacedSeeds.size();
    seeds = btllib::parse_seeds(spacedSeeds);
}

Stash::~Stash() {
    delete[] data;
    for(int i = 0; i < LOCKS; i++)
        omp_destroy_lock(&(locks[i]));
}

void Stash::fill(std::vector <Read*> & reads, int threads){
    int* extractedValues = new int[hashFuncNum];
    int* hashReadIDTiles1 = new int[hashFuncNum];
    int* hashReadIDTiles2 = new int[hashFuncNum];

    if(threads == -1)
        omp_set_num_threads(omp_get_max_threads());
    else
        omp_set_num_threads(threads);

    #pragma omp parallel for schedule(dynamic)
    for(size_t i = 0; i < reads.size(); i++) {
        Read *read = reads[i];
        if (read->idHash1 == 0) {
            #pragma omp critical
            {
                read->hashReadID(numOfUsedReads, B1, B2);
                numOfUsedReads += 1;
            }
        }

//        std::cout << omp_get_thread_num();

        btllib::SeedNtHash nt = btllib::SeedNtHash(read->sequence, read->length, seeds, 1, spacedSeedLength);

        while(nt.roll()){
            const uint64_t* hashes = nt.hashes();
            combineHashes(hashes, extractedValues);
            extractHashReadIDTiles(extractedValues, read->idHash1, T1, hashReadIDTiles1);
            extractHashReadIDTiles(extractedValues, read->idHash2, T2, hashReadIDTiles2);

            fill(hashReadIDTiles1, hashReadIDTiles2, hashes);
        }
    }

    delete[](extractedValues);
    delete[](hashReadIDTiles1);
    delete[](hashReadIDTiles2);
}

void Stash::fill(int *hashReadIDTiles1, int *hashReadIDTiles2, const uint64_t *hashes) {
    for(int i = 0; i < hashFuncNum; i++){
        uint64_t stashRow = hashes[i] & (rows - 1);
        writeTile(stashRow, hashReadIDTiles1[i], hashReadIDTiles2[i]);
    }
}

void Stash::save(const char* filePath){
    FILE* file = fopen(filePath, "wb");
    if(file == nullptr){
        std::cout << "File cannot be opened: " << filePath << std::endl;
        exit(-1);
    }
    fwrite(&numOfUsedReads, sizeof(int), 1, file);
    fwrite(&spacedSeedLength, sizeof(int), 1, file);
    fwrite(&hashFuncNum, sizeof(int), 1, file);
    for (int i = 0; i < hashFuncNum; i++){
        fwrite(spacedSeeds[i].c_str(), sizeof(char), spacedSeedLength, file);
    }
    fwrite(&rows, sizeof(uint64_t), 1, file);
    fwrite(&readIDHashTiles, sizeof(int), 1, file);
    fwrite(&T1, sizeof(int), 1, file);
    fwrite(&T2, sizeof(int), 1, file);
    fwrite(data, sizeof(int), rows * intsPerRow, file);
    fclose(file);
}

void Stash::combineHashes(const uint64_t* hashes, int* extractedValues){
    for(int i = 0; i < hashFuncNum; i++){   // columns
        int k = (int) pow(2, hashFuncNum - 2); // the value of the MSB
        int value = 0;

        for(int j = 0; j < hashFuncNum; j++){   // rows
            if(i == j)
                continue;

            value += extractBit(hashes[j], i) * k;
            k /= 2;
        }
        extractedValues[i] = value;
    }
}

const uint64_t* Stash::computeSequenceHash(const char* sequence){
    btllib::SeedNtHash nt = btllib::SeedNtHash(sequence, spacedSeedLength, seeds, 1, spacedSeedLength);
    nt.roll();
    return nt.hashes();
}

void Stash::retrieveFrame(const char* sequence, Frame frame){
    retrieveFrame(computeSequenceHash(sequence), frame);
}

void Stash::retrieveFrame(const uint64_t* hashes, Frame frame){
    for(int i = 0; i < hashFuncNum; i++){
        uint64_t memoryRow = hashes[i] & (rows - 1);

        for(int j = 0; j < stashRowTiles; j++)
            frame[i * stashRowTiles + j] = readTile(memoryRow, j);
    }
}

int Stash::countFrameMatches(const char* sequence1, const char* sequence2){
    createFrame(frame1, this);
    createFrame(frame2, this);
    retrieveFrame(sequence1, frame1);
    retrieveFrame(sequence2, frame2);
    int matches = countFrameMatches(frame1, frame2);

    deleteFrame(frame1);
    deleteFrame(frame2);
    return matches;
}

int Stash::countFrameMatches(Frame frame1, Frame frame2) const{
    int count = 0;
    for(int j = 0; j < stashRowTiles; j++)
        for(int i = 0; i < hashFuncNum; i++){
            if(TILE(frame1, i, j) == 0)
                continue;

            bool check = false;
            for(int m = i + 1; m < hashFuncNum; m++){
                if(TILE(frame1, m, j) == TILE(frame1, i, j)){
                    check = true;
                    break;
                }
            }
            if(check)
                continue;
            for(int k = 0; k < hashFuncNum; k++)
                if(TILE(frame1, i, j) == TILE(frame2, k, j)){
                    count++;
                    break;
                }
        }

    return count;
}

void Stash::Window::extractWindowFrames(std::vector<Frame>& frames, Stash& stash){
    int offset = 0;
    for (int i = 0; i < framesNum; i ++){
        Frame frame = new int[stash.stashRowTiles * stash.hashFuncNum];
        stash.retrieveFrame(start + offset, frame);
        frames.push_back(frame);
        offset += stride;
    }
}

int Stash::countWindowMatches(Window window1, Window window2){
    std::vector<Frame> frameSet1;
    window1.extractWindowFrames(frameSet1, *this);
    std::vector<Frame> frameSet2;
    window2.extractWindowFrames(frameSet2, *this);

    int max = 0;
    for(size_t i = 0; i < frameSet1.size(); i++) {
        for(size_t j = 0; j < frameSet2.size(); j++) {
            int matches = countFrameMatches(frameSet1[i], frameSet2[j]);
            if(matches > max)
                max = matches;
        }
    }
    return max;
}

inline int Stash::extractBit(uint64_t hashValue, int index) {
    return (hashValue & (1 << index)) >> index;
}

void Stash::extractHashReadIDTiles(int* tileAddresses, uint64_t IDHash, int tileBits, int* hashReadIDTiles){
    for (int i = 0; i < hashFuncNum; i++) {
        int starting = tileBits * tileAddresses[i];
        int t = IDHash >> starting;
        // We want the right T1 or T2 bits
        int tile = t & ((int) pow(2, tileBits) - 1);
        hashReadIDTiles[i] = tile;
    }
}

void Stash::writeTile(uint64_t row, int column, int value) {
    if(readTile(row, column) != 0){
        return;
    }

    int lockIndex = row % LOCKS;
    omp_set_lock(&(locks[lockIndex]));

    uint64_t intIndex = row * intsPerRow + column / segmentsPerInteger;
    int intNumber = data[intIndex];

    int segmentIndex = column & (segmentsPerInteger - 1);
    int bitOffset = (segmentsPerInteger - 1 - segmentIndex) * T2;
    int temp = ~((segmentMax - 1) << bitOffset);
    intNumber &= temp;
    intNumber |= value << bitOffset;
    data[intIndex] = intNumber;
    omp_unset_lock(&(locks[lockIndex]));
}

int Stash::readTile(uint64_t row, int column) {
    int lockIndex = row % LOCKS;
    omp_set_lock(&(locks[lockIndex]));
    int intNumber = data[row * intsPerRow + column / segmentsPerInteger];
    omp_unset_lock(&(locks[lockIndex]));
    int segmentIndex = column & (segmentsPerInteger - 1);  // column % segmentsPerInteger
    int shiftedIntNumber = intNumber >> ((segmentsPerInteger - 1 - segmentIndex) * T2);
    return shiftedIntNumber & (segmentMax - 1); // shifted_int_number % segmentMax get the T2 right bits
}

void Stash::print() {
    std::cout << "Stash:" << std::endl;

    for (uint64_t i = 0; i < rows; i++){
        for(int j = 0; j < stashRowTiles; j++){
            std::cout << readTile(i, j) << "\t";
        }
        std::cout << std::endl;
    }
}

void Stash::printFrame(Frame frame) {
    std::cout << "Frame:" << std::endl;

    for (int i = 0; i < hashFuncNum; i++){
        for(int j = 0; j < stashRowTiles; j++){
            std::cout << TILE(frame, i , j) << "\t";
        }
        std::cout << std::endl;
    }
}

void Stash::printFrame(char *sequence) {
    createFrame(frame1, this);
    retrieveFrame(sequence, frame1);
    printFrame(frame1);
    deleteFrame(frame1);
}


