#include "Riostream.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2F.h"
#include "TLegend.h"
#include "TMath.h"
#include "TStyle.h"
#include <THStack.h>
#include <TKey.h>
#include <TList.h>
#include <TMatrixTSym.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <cmath>
#include <iostream>
#include <math.h>
#include <vector>

#include "MiniCSCData.h" // MiniCSCData.h object to read the root file, true for null pointer, false for blank graphs
#include "MiniCSCUtils.h"

//! NOTE: Use '.L MiniCSCUtils.cpp' to load the library (Must be running ROOT in the same directory as MiniCSCUtils.cpp)
R__LOAD_LIBRARY(MiniCSCUtils_cpp.so);

const static char* pdf = "graphs.pdf";
const static char* pdf0 = "graphs.pdf[";
const static char* pdf1 = "graphs.pdf]";

using namespace std;

int rebinFactor = 24;

void clusterChargeRebinWithUtils()
{
    // Automatically compiles utils library in case it changed.
    gSystem->CompileMacro("MiniCSCUtils.cpp");

    // Disable stats
    gStyle->SetOptStat(0);

    TCanvas* canvas = new TCanvas(true);
    canvas->SetRightMargin(0.1);
    canvas->SetLeftMargin(0.12);
    std::vector<MiniCSCData> mDataVect;

    TSystemDirectory dir("rootfiles", "../../rootfiles"); //! NOTE: Replace with your own path to root files.
    TList* files = dir.GetListOfFiles();
    files->Sort();
    canvas->Print(pdf0); // Open PDF

    if (!files) return; // Return if no files found
    TSystemFile* file;
    TString fname;
    TIter next(files); // Iterate through files

    //?--------------------------------------Read Root Files--------------------------------------

    while ((file = (TSystemFile*)next())) {
        fname = file->GetName();
        if (file->IsDirectory() || !fname.EndsWith(".root")) continue;

        TString filePath = TString::Format("../../rootfiles/%s", fname.Data()); //! NOTE: Replace with your own path to root files.
        mDataVect.push_back(MiniCSCData(filePath.Data()));
    }
    //?--------------------------------------Utilize MiniCSCUtils.h & MiniCSCData.h--------------------------------------
    MUtils::histInfo<TH1D> chgSpct;
    chgSpct.legend = new TLegend(0.75, 0.75, 0.9, 0.9);

    for (MiniCSCData& m : mDataVect) {

        TH1D* hist = m.ChargeSpectra()[3];

        hist->Rebin(32);
        hist->GetXaxis()->SetRangeUser(100, 4000.5);
        hist->SetTitle("Charge Spectra");
        chgSpct.hist.push_back(hist);
        chgSpct.legend->AddEntry(hist, TString::Format("%s:", m.RootFileName()), "l");
        chgSpct.legend->AddEntry((TObject*)0, TString::Format("Entries: %d", (int)hist->GetEntries()), "");
    }

    MUtils::drawStack<TH1D>(canvas, chgSpct, "gaus");

    canvas->Print(pdf);
    canvas->Print(pdf1);
}
// TODO: Parse through each charge histogram in the root file and generate histograms for each of