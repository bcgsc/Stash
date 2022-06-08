#ifndef STOCHASTIC_TILE_HASHING_READ_H
#define STOCHASTIC_TILE_HASHING_READ_H


#include <cstdint>

class Read {
public:
    int readID = 0;
    char* sequence;
    int length;
    uint64_t idHash1 = 0;
    uint64_t idHash2 = 0;
    Read(char* sequence, int length);
    explicit Read(const char* sequence);
    ~Read();
    void hashReadID(int ID, int B1, int B2);

private:
    uint64_t hash(int bits, char* id) const;
};


#endif //STOCHASTIC_TILE_HASHING_READ_H
