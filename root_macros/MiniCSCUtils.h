#ifndef MINICSCUTILS_H_
#define MINICSCUTILS_H_

#include "TCanvas.h"
#include "TF1.h"
#include "TH1.h"

namespace MUtils
{

template <typename T> struct histInfo {
    TLegend* legend;
    std::vector<T*> hist;
};

const static uint16_t colors[] = { 38, 30, 46 };

template <typename T>
void drawStack(TCanvas* cc, const histInfo<T>& histHeader, const char* fitOpt = "");

}

#endif // MINICSCUTILS_H_
