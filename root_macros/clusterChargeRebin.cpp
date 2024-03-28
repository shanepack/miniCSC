#include "MiniCSCData.h" //! NOTE: Make sure to include the MiniCSCData.h file in the same directory as this script. (It is not a part of ROOT!)
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

//? Shane Pack - 28-3-2024

//?================================================================================================================================================================
//? 1. The clusterChargeRebin function reads all root files in a defined directory and extracts the cluster charge histograms from them.
//? 2. Then, the function rebins the cluster charge histograms and displays them on the same canvas, alongside a Gaussian fit for each histogram.
//? 3. The histograms & Gaussian fits are colored differently to distinguish them from each other.
//? 4. The histograms are saved as a PDF file in the outputs folder, which can be used to compare the cluster charge distributions.
//?================================================================================================================================================================

using namespace std;

const static uint16_t colors[] = { 46, 30, 38 };
int rebinFactor = 24; //? Change this value to rebin the histograms by a different factor (24 is close to DQM plots for cluster charge histograms. Set to 1 for no rebinning.)

void clusterChargeRebin()
{
    // Disable stats
    gStyle->SetOptStat(0);

    TCanvas* canvas = new TCanvas("canvas", "Histogram Comparison", 800, 600);
    auto leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    int colorCounter = 0; // Counter to vary histogram/gaussian fit colors

    //?--------------------------------------Read Root File's Cluster Charge Graphs--------------------------------------

    TSystemDirectory dir("rootfiles", "../rootfiles/"); //! NOTE: Replace with your own path to root files.
    TList* files = dir.GetListOfFiles();
    //files->Sort();
    if (files) {
        TSystemFile* file;
        TString fname;
        TIter next(files);
        while ((file = (TSystemFile*)next())) {
            fname = file->GetName();
            if (!file->IsDirectory() && fname.EndsWith(".root")) {
                TString filePath = TString::Format("../rootfiles/%s", fname.Data()); //! NOTE: Replace with your own path to root files.
                MiniCSCData myFile(filePath, false); // MiniCSCData.h object to read the root file, true for null pointer, false for blank graphs
                TH1D* hist = (myFile.ChargeSpectra()[3]); // MiniCSCData.h object to define which histogram(s) to read, and what layer to read from.

                // ... rest of the code ...
            }
        }
    }

    leg->Draw();
    canvas->Draw();
    canvas->SaveAs("../outputs/clusterCharge.pdf"); //!NOTE: Change to your designated path to save PDF files.
    canvas->Close(); // Close the canvas after saving the file
}

//TODO: Parse through each charge histogram in the root file and generate histograms for each of them. (Bins < 50 will not be displayed)
