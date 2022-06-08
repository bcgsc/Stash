#ifndef STOCHASTIC_TILE_HASHING_STASH_H
#define STOCHASTIC_TILE_HASHING_STASH_H

#include <omp.h>
#include <vector>
#include <string>
#include "Read.h"
#include "btllib/nthash.hpp"
#define LOCKS 1000

#define FRAME_SIZE(stash) (stash)->stashRowTiles * (stash)->hashFuncNum
#define createFrame(frame, stash) int* frame = new int[FRAME_SIZE(stash)];
#define deleteFrame(frame) delete[] frame;

#define TILE(frame, i, j) frame[i * stashRowTiles + j]
typedef int* Frame;

class Stash {
public:
    struct Window{
        Window(char* start, int framesNum, int stride){
            this->start = start;
            this->framesNum = framesNum;
            this->stride = stride;
        }
        char* start;
        int framesNum;
        int stride;
        void extractWindowFrames(std::vector<Frame>& frames, Stash& stash);
    };
    void fill(std::vector <Read*> & reads, int threads=-1);
    void save(const char* filePath);
    const uint64_t* computeSequenceHash(const char* sequence);
    void retrieveFrame(const char* sequence, Frame frame);
    int countFrameMatches(const char* sequence1, const char* sequence2);
    int countFrameMatches(Frame frame1, Frame frame2) const;
    int countWindowMatches(Window window1, Window window2);
    void print();
    void printFrame(Frame frame);
    void printFrame(char* sequence);
    void writeTile(uint64_t row, int column, int value);
    int readTile(uint64_t row, int column);
    Stash(uint64_t rows, std::vector<std::string> & spacedSeeds, int T1, int T2, int readIDHashTiles);
    Stash(const char* stashPath);
    ~Stash();
    int numOfUsedReads = 0;
    int stashRowTiles;  // Ns
    int hashFuncNum;

private:
    void combineHashes(const uint64_t* hashes, int* extractedValues);
    static int extractBit(uint64_t hashValue, int index);
    void extractHashReadIDTiles(int* tileAddresses, uint64_t IDHash, int tileBits, int* hashReadIDTiles);
    void fill(int* hashReadIDTiles1, int* hashReadIDTiles2, const uint64_t* hashes);
    void retrieveFrame(const uint64_t* hashes, Frame frame);
    omp_lock_t locks[LOCKS];

private:
    int* data;
    uint64_t rows;
    int intsPerRow, segmentsPerInteger, segmentMax;
    int T1, T2;

    int readIDHashTiles;
    int spacedSeedLength;
    std::vector<btllib::SpacedSeed> seeds;
    std::vector<std::string> spacedSeeds;
    int B1, B2;   // Only supports 32, 64
    const int intSize = sizeof(int) * 8;
    void computeSecondaryParameters();
};


#endif //STOCHASTIC_TILE_HASHING_STASH_H
