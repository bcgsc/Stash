#include <cstring>
#include "Read.h"
#include "city.h"
#include <stdlib.h>
#include <stdio.h>

Read::Read(char *sequence, int length) {
    this->sequence = new char[length + 1];
    strncpy(this->sequence, sequence, length);
    this->sequence[length] = 0;
    this->length = length;
}

Read::Read(const char *sequence) {
    length = (int) strlen(sequence);
    this->sequence = new char[length + 1];
    strncpy(this->sequence, sequence, length);
    this->sequence[length] = 0;
}

Read::~Read() {
    delete[] sequence;
}

void Read::hashReadID(int ID, int B1, int B2) {
    if(length == 0)
        return;

    readID = ID;

    char id1[20];
    char id2[20];

    sprintf(id1, "%d", ID);
    sprintf(id2, " %d", ID);

    idHash1 = hash(B1, id1);
    idHash2 = hash(B2, id2);
}

uint64_t Read::hash(int bits, char* id) const {
    switch(bits){
        case 32:
            return (uint64_t) CityHash32(id, strlen(id));

        case 64:
            return (uint64_t) CityHash64(id, strlen(id));

        default:
            printf("ERROR: ReadID Hash value can either be 32 or 64 bits!\n");
            exit(-1);
    }
}
