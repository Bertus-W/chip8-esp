#include <TFT_eSPI.h>
#include <bb_captouch.h>
#include <SdFat.h>
#include "../chip8.h"

#ifndef VIEWS_H
#define VIEWS_H

enum ViewType
{
    FileSelect,
    Chip8
};

extern void setCurrentView(ViewType type);

class View
{
public:
    virtual void init() = 0;
    virtual void step(TFT_eSPI &, BBCapTouch &) = 0;
    virtual void reset() = 0;
};

#endif