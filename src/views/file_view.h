#pragma once
#include "view.h"

#define SPI_CLOCK SD_SCK_MHZ(20)
#define SD_CONFIG SdSpiConfig(SD_CS, DEDICATED_SPI, SPI_CLOCK)

struct FileEntry
{
    String name;
    int yPosition;
    bool isDirectory;
};

class Chip8View;
class FileView : public View
{
public:
    FileView(Chip8View *);
    void init();
    void step(TFT_eSPI &, BBCapTouch &);
    void reset();

    void readFileToChar(const char *, char *&, size_t &);
    void displayFiles(TFT_eSPI &);
    int listFiles(FsFile, int, int);
    void handleTouch(TFT_eSPI &tft, int y);
    void loadFileList();

private:
    TOUCHINFO ti;
    SdFat sd;
    FileEntry fileEntries[20]; // Array to hold file entries, max 20 for simplicity
    int fileCount = 0;
    Chip8View *chip8View;
};