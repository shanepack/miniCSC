
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
    gSystem->Load("MiniCSCUtils_cpp.so");
    // Automatically compiles utils library in case it changed.
    // gSystem->CompileMacro("MiniCSCUtils.cpp");
    gStyle->SetOptStat(0);

    TCanvas* cc = new TCanvas();
    cc->SetRightMargin(0.1);
    cc->SetLeftMargin(0.12);
    std::vector<MiniCSCData> mDataVect;
    cc->Clear();

    uint16_t colorCounter = 0;

    std::string subDir = "./input/";
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
        std::cout << filePath.Data() << std::endl;
        mDataVect.push_back(MiniCSCData(filePath.Data(), true, true));
    }

    float max     = 0.0f;
    MiniCSCData m = mDataVect[0];

    MUtils::histInfo<TH1D> chgSpct;
    chgSpct.legend = new TLegend(0.75, 0.75, 0.9, 0.9);
    TH1D* hist     = m.ChargeSpectra()[2];
    hist->Rebin(32);
    hist->GetXaxis()->SetRangeUser(100.5, 4000.5);
    hist->SetTitle("Charge Spectra");
    chgSpct.hist.push_back(hist);
    chgSpct.legend->AddEntry(hist, TString::Format("%s:", m.RootFileName()), "l");
    chgSpct.legend->AddEntry((TObject*)0, TString::Format("Entries: %d", (int)hist->GetEntries()), "");

    MiniCSCData m2 = mDataVect[1];
    hist           = m2.ChargeSpectra()[2];
    // hist           = m2.GetGraph<TH1D>("chargesL3");
    hist->Rebin(32);
    hist->GetXaxis()->SetRangeUser(0.5, 4000.5);
    hist->SetTitle("Charge Spectra");
    chgSpct.hist.push_back(hist);
    chgSpct.legend->AddEntry(hist, TString::Format("%s:", m2.RootFileName()), "l");
    chgSpct.legend->AddEntry((TObject*)0, TString::Format("Entries: %d", (int)hist->GetEntries()), "");

    MiniCSCData m3 = mDataVect[2];
    hist           = m3.ChargeSpectra()[2];
    // hist           = m2.GetGraph<TH1D>("chargesL3");
    hist->Rebin(32);
    hist->GetXaxis()->SetRangeUser(0.5, 4000.5);
    hist->SetTitle("Charge Spectra");
    chgSpct.hist.push_back(hist);
    chgSpct.legend->AddEntry(hist, TString::Format("%s:", m3.RootFileName()), "l");
    chgSpct.legend->AddEntry((TObject*)0, TString::Format("Entries: %d", (int)hist->GetEntries()), "");

    MUtils::drawStack<TH1D>(cc, chgSpct);
    cc->Print(pdf);

    // mDataVect[0].ChargeTBinProfile()->Draw();

    cc->Print(pdf1);
}
