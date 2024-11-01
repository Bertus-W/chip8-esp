#include <TFT_eSPI.h>
#include <bb_captouch.h>
#include <SdFat.h>
#include "chip8.h"
#include "views/view.h"
#include "views/file_view.h"
#include "views/chip8_view.h"
#include <vector>

#define VIEW_COUNT 2

ViewType currentView;

std::vector<View *> views;

TFT_eSPI tft;
BBCapTouch bbct;

void setup()
{
    Serial.begin(115200);

    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    bbct.init(33, 32, 25, 21);
    bbct.setOrientation((tft.rotation * 90), 240, 320);
    Chip8View *chip8View = new Chip8View();
    FileView *fileView = new FileView(chip8View);
    views.push_back(fileView);
    views.push_back(chip8View);

    setCurrentView(ViewType::FileSelect);
}

void loop()
{
    views[currentView]->step(tft, bbct);
}

void setCurrentView(ViewType type)
{
    views[currentView]->reset();
    currentView = type;
    views[currentView]->init();
}
