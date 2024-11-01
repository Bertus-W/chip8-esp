#include "file_view.h"
#include "chip8_view.h"

FileView::FileView(Chip8View *view) : chip8View(view)
{
    if (!sd.begin(SD_CONFIG))
    {
        sd.initErrorHalt(&Serial);
        while (1)
            delay(0);
    }
}

void FileView::init()
{
    loadFileList();
}

void FileView::step(TFT_eSPI &tft, BBCapTouch &bbct)
{
    static bool show = true;
    if (show)
    {
        displayFiles(tft);
        show = false;
    }

    if (bbct.getSamples(&ti))
    { // If a touch is detected
        handleTouch(tft, ti.y[0]);
        show = true;
    }
}

void FileView::reset()
{
}

void FileView::loadFileList()
{
    fileCount = 0;                  // Reset the file count
    listFiles(sd.open("/"), 0, 20); // Start at y=20 pixels for file list
}

void FileView::displayFiles(TFT_eSPI &tft)
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    for (int i = 0; i < fileCount; i++)
    {
        int ypos = 40 * i + 20;
        tft.drawLine(0, ypos - 20, TFT_HEIGHT, ypos - 20, TFT_WHITE);
        tft.setCursor(20, ypos);
        fileEntries[i].yPosition = ypos;

        if (fileEntries[i].isDirectory)
        {
            tft.print("[DIR] ");
        }
        tft.print(fileEntries[i].name);
    }
}

int FileView::listFiles(FsFile dir, int numTabs, int y)
{
    while (true)
    {
        FsFile entry;
        if (!entry.openNext(&dir, O_RDONLY) || fileCount >= 20)
        { // Limit to 20 entries for simplicity
            break;
        }
        if (entry.isDir())
        {
            entry.close();
            continue;
        }

        char name[13]; // Buffer to hold the filename
        entry.getName(name, sizeof(name));

        if (String(name).length() == 0 || String(name).startsWith("._"))
            continue;

        fileEntries[fileCount].name = String(name);
        fileEntries[fileCount].yPosition = y;
        fileEntries[fileCount].isDirectory = entry.isDir();
        fileCount++;

        Serial.printf("Found file: %s\n", name);

        y += 10; // Move down 10 pixels for the next line
    }
    return y;
}

void FileView::handleTouch(TFT_eSPI &tft, int y)
{
    for (int i = 0; i < fileCount; i++)
    {
        if (y >= fileEntries[i].yPosition - 20 && y < fileEntries[i].yPosition + 20)
        {
            tft.fillScreen(TFT_BLACK); // Clear the screen
            tft.setCursor(0, 0);
            tft.print("Selected: ");
            tft.println(fileEntries[i].name);

            // Read file content into a char* buffer
            readFileToChar(fileEntries[i].name.c_str(), chip8View->programData, chip8View->programSize);
            chip8View->chip8 = new CHIP8Emulator(chip8View->programData, chip8View->programSize);
            setCurrentView(ViewType::Chip8);
            break;
        }
    }
}

void FileView::readFileToChar(const char *filePath, char *&buffer, size_t &fileSize)
{
    SdFile file;
    if (!file.open(filePath, O_READ))
    {
        Serial.println("Failed to open file.");
        return;
    }

    // Get file size and allocate a buffer
    fileSize = file.fileSize();

    buffer = (char *)malloc(fileSize * sizeof(char));

    // Read file into buffer
    file.read(buffer, fileSize);

    file.close(); // Close the file
    return;
}
