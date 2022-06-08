#include <iostream>

#include "Stash.h"
Stash* fillStash();
void save();
void load();
void manipulateTiles();
void manipulateFrames();
void compareFrames();
void compareRegions();

int main() {
    std::cout << "Filling Stash ------------------------------------------" << std::endl;
    // How to fill Stash with reads.
    fillStash();

    std::cout << "Saving Stash ------------------------------------------" << std::endl;
    // How to store and re-store Stash.
    save();
    std::cout << "Loading Stash ------------------------------------------" << std::endl;
    load();

    std::cout << "Stash Tile Manipulation------------------------------------------" << std::endl;
    // Tile Manipulation
    manipulateTiles();

    std::cout << "Stash Frame Manipulation------------------------------------------" << std::endl;
    // Frame Manipulation
    manipulateFrames();

    std::cout << "Comparing Stash Frames------------------------------------------" << std::endl;
    // Frame Comparison
    compareFrames();

    std::cout << "Comparing Stash Regions------------------------------------------" << std::endl;
    // Regions Comparison
    compareRegions();

    return 0;
}

Stash* fillStash(){
    std::vector<std::string> spacedSeeds = {
            "110011",
            "010010"
    };
    std::vector<Read*> reads = {
            new Read("AACCGG"),
            new Read("ACCTTA"),
    };

    Stash* stash = new Stash(16, spacedSeeds, 4, 4);
    stash->fill(reads);
    stash->print();

    for (Read* read:reads){
        delete read;
    }

    return stash;
}

void save(){
    Stash* stash = fillStash();
    stash->save("demo.stash");
    delete stash;
}

void load(){
    Stash stash("demo.stash");
    stash.print();
}

void manipulateTiles(){
    Stash* stash = fillStash();
    stash->writeTile(4, 5, 15);
    std::cout << stash->readTile(4, 5) << std::endl;
}

void manipulateFrames(){
    Stash* stash = fillStash();

    createFrame(frame, stash);
    stash->retrieveFrame("ACCTTA", frame);
    stash->printFrame(frame);
    deleteFrame(frame);
}

void compareFrames(){
    std::vector<std::string> spacedSeeds = {
            "0100110010",
            "1001111001",
            "1101001011",
            "0111001110"
    };

    // Overlapping Reads
    std::vector<Read*> reads = {
            new Read(         "CTGCTGATGCTGAAATAGGGGG"),
            new Read("AACCGGGTGCTGCTGATGCTGAAATAGGGGGCGCAAATCCCCCT"),
            new Read(     "GGTGCTGCTGATGCTGAAATAGGGGGCGCAAAT"),
            new Read("AACCGGGTGCTGCTGATGCTGAAATAGGGGGCGCAAAT"),
    };

    Stash* stash = new Stash(1024, spacedSeeds, 4, 4);
    stash->fill(reads);

    // Related frames with 7x coverage.
    int relatedFramesMatches = stash->countFrameMatches(reads[0]->sequence, reads[0]->sequence + 3);

    // Unrelated frames.
    int unrelatedFramesMatches = stash->countFrameMatches(reads[0]->sequence, "GGGGGGGGGG");

    std::cout << "Number of Matches for Related Frames: " << relatedFramesMatches << std::endl;
    std::cout << "Number of Matches for Unrelated Frames: " << unrelatedFramesMatches << std::endl;

    stash->printFrame(reads[0]->sequence);
    stash->printFrame(reads[0]->sequence + 3);
    stash->printFrame((char *) "GGGGGGGGGG");

    for (Read* read:reads){
        delete read;
    }
}

void compareRegions(){
    std::vector<std::string> spacedSeeds = {
            "0100110010",
            "1001111001",
            "1101001011",
            "0111001110"
    };

    // Overlapping Reads
    std::vector<Read*> reads = {
            new Read(         "CTGCTGATGCTGAAATAGGGGG"),
            new Read("AACCGGGTGCTGCTGATGCTGAAATAGGGGGCGCAAATCCCCCT"),
            new Read(     "GGTGCTGCTGATGCTGAAATAGGGGGCGCAAAT"),
            new Read("AACCGGGTGCTGCTGATGCTGAAATAGGGGGCGCAAAT"),
    };
    Stash* stash = new Stash(1024, spacedSeeds, 4, 4);
    stash->fill(reads, 8);

    Stash::Window window1(reads[0]->sequence, 3, 1);

    Stash::Window window2(reads[0]->sequence + 5, 3, 1);

    // Related frames with 7x coverage.
    int relatedFramesMatches = stash->countWindowMatches(window1, window2);
    std::cout << "Number of Matches: " << relatedFramesMatches << std::endl;
}
