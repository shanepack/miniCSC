
#include "TCanvas.h"
#include "TF1.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TSystemFile.h"
#include <cstdint>
#include <sys/types.h>

#include "MiniCSCData.h"
#include "MiniCSCUtils.h"

// Loading utils library
R__LOAD_LIBRARY(MiniCSCUtils_cpp.so);

const static char* pdf  = "graphs.pdf";
const static char* pdf0 = "graphs.pdf[";
const static char* pdf1 = "graphs.pdf]";

void mData()
{
    // gSystem->Load("MiniCSCUtils_cpp.so");
    // Automatically compiles utils library in case it changed.
    gSystem->CompileMacro("MiniCSCUtils.cpp");
    gStyle->SetOptStat(0);

    TCanvas* cc = new TCanvas();
    cc->SetRightMargin(0.1);
    cc->SetLeftMargin(0.12);
    std::vector<MiniCSCData> mDataVect;

    uint16_t colorCounter = 0;

    std::string subDir = "./pipe-depth-test/";
    TSystemDirectory dir("pipe-depth-test", subDir.c_str());
    TList* files = dir.GetListOfFiles();
    files->Sort();
    // files->Print();
    cc->Print(pdf0);

    if (!files) return;
    TSystemFile* file;
    TString fname;
    TIter next(files);
    // Get all files in directory
    while ((file = (TSystemFile*)next())) {
        fname = file->GetName();
        if (file->IsDirectory() || !fname.EndsWith(".root")) continue;

        TString filePath = TString::Format((subDir + "%s").c_str(), fname.Data());
        mDataVect.push_back(MiniCSCData(filePath.Data()));
    }

    MUtils::histInfo<TH1D> chgWght;
    MUtils::histInfo<TH1D> chgTBin;
    MUtils::histInfo<TH1D> chgSpct;
    chgWght.legend = new TLegend(0.8, 0.75, 0.9, 0.9);
    chgTBin.legend = new TLegend(0.8, 0.75, 0.9, 0.9);
    chgSpct.legend = new TLegend(0.75, 0.75, 0.9, 0.9);
    for (MiniCSCData& m : mDataVect) {
        TH1D* hist = m.ChargeTBinWeighted()[3];
        chgWght.hist.push_back(hist);
        chgWght.legend->AddEntry(hist, TString::Format("%s:", m.RootFileName()), "l");
        chgWght.legend->AddEntry(
            (TObject*)0, TString::Format("Entries: %d", (int)hist->GetEntries()), "");

        hist = m.ChargeTBin()[3];
        hist->SetTitle("Strip Time Bin Occupancy");
        chgTBin.hist.push_back(hist);
        chgTBin.legend->AddEntry(hist, TString::Format("%s:", m.RootFileName()), "l");
        chgTBin.legend->AddEntry(
            (TObject*)0, TString::Format("Entries: %d", (int)hist->GetEntries()), "");

        hist = m.ChargeSpectra()[3];
        hist->Rebin(32);
        hist->GetXaxis()->SetRangeUser(100, 4000.5);
        hist->SetTitle("Charge Spectra");
        chgSpct.hist.push_back(hist);
        chgSpct.legend->AddEntry(hist, TString::Format("%s:", m.RootFileName()), "l");
        chgSpct.legend->AddEntry(
            (TObject*)0, TString::Format("Entries: %d", (int)hist->GetEntries()), "");
    }
    MUtils::drawStack<TH1D>(cc, chgWght, "gaus");
    // MUtils::drawStack(cc, chgWght, "gaus");
    cc->Print(pdf);

    MUtils::drawStack<TH1D>(cc, chgTBin, "gaus");
    cc->Print(pdf);

    MUtils::drawStack<TH1D>(cc, chgSpct, "gaus");
    cc->Print(pdf);

    cc->Print(pdf1);
}
