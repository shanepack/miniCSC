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

//?================================================================================================================================================================
//? 1. The clusterChargeRebin function reads all root files in a defined directory and extracts the cluster charge histograms from them.
//? 2. Then, the function rebins the cluster charge histograms and displays them on the same canvas, alongside a Gaussian fit for each histogram.
//? 3. The histograms & Gaussian fits are colored differently to distinguish them from each other.
//? 4. The histograms are saved as a PDF file in the outputs folder, which can be used to compare the cluster charge distributions.
//?================================================================================================================================================================

using namespace std;

int rebinFactor = 24; //? Change this value to rebin the histograms by a different factor (24 is close to DQM plots for cluster charge histograms. Set to 1 for no rebinning.)

void clusterChargeRebin()
{
    // Automatically compiles utils library in case it changed.
    gSystem->CompileMacro("MiniCSCUtils.cpp");

    // Disable stats
    gStyle->SetOptStat(0);

    TCanvas* canvas = new TCanvas("canvas", "Histogram Comparison", 800, 600);
    auto leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    int colorCounter = 0; // Counter to vary histogram/gaussian fit colors

    //?--------------------------------------Read Root File's Cluster Charge Graphs--------------------------------------

    TSystemDirectory dir("rootfiles", "../../rootfiles"); //! NOTE: Replace with your own path to root files.
    TList* files = dir.GetListOfFiles();
    files->Sort();
    if (files) {
        TSystemFile* file;
        TString fname;
        TIter next(files);
        while ((file = (TSystemFile*)next())) {
            fname = file->GetName();
            if (!file->IsDirectory() && fname.EndsWith(".root")) {
                TString filePath = TString::Format("../../rootfiles/%s", fname.Data()); //! NOTE: Replace with your own path to root files.

                // MiniCSCData.h object to read the root file, true for null pointer, false for blank graphs
                MiniCSCData myFile(filePath, false);

                // MiniCSCData.h object to define which histogram(s) to read, and what layer to read from.
                //!(miniCSCs uses [3] and/or [4] ONLY!)
                TH1D* hist = (myFile.ChargeSpectra()[3]);

                //*OPTIONAL: Use 'GetGraph' if you need a specific graph/to read something without a getter (this ^)
                //*OPTIONAL: hist = myFile.GetGraph<TH1D>(MiniCSCData::Graph::kChargeSpectra, 3);

                if (!hist) continue; // Skip if histogram is not found

                hist->SetTitle("Cluster Charge Spectra"); //!NOTE: Change the title of the histogram here.

                //?--------------------------------------Rebin Histograms--------------------------------------

                hist->Rebin(rebinFactor); // Rebin the histogram //!NOTE: Rebin DIVIDES by rebinFactor. (24 is close to DQM plots)
                hist->SetLineColor(colorCounter + 1); // Vary the line color
                hist->SetFillColorAlpha(colorCounter + 1, 0.3); // Vary the fill color with opacity

                // If there is only one histogram, draw it. Otherwise, draw the rest of the histograms on the same canvas
                if (colorCounter == 0) {
                    hist->Draw("HIST");
                } else {
                    hist->Draw("HIST SAME");
                }
                cout << "Drawing histogram for: " << fname << endl;

                //?--------------------------------------Axis Labels--------------------------------------

                // Get the maximum value of the histogram to set the y-axis range
                hist->GetXaxis()->SetRangeUser(0, 5000);
                int max = hist->GetMaximum();

                // Resizes merged histograms to better fit the desired cluster charge data
                hist->GetYaxis()->SetRangeUser(0, max + 100);

                //?--------------------------------------Legend--------------------------------------

                leg->AddEntry(hist, TString::Format("%s:", fname.Data()), "l");
                leg->AddEntry((TObject*)0, TString::Format("Entries: %d", (int)hist->GetEntries()), "");

                colorCounter++; // Increment color counter for the next file. Also used to determine if the histogram is the first one!
            }
        }
    }

    leg->Draw();
    canvas->Draw();
    canvas->SaveAs("../../outputs/clusterCharge.pdf"); //! NOTE: Change to your designated path to save PDF files.
    canvas->Close(); // Close the canvas after saving the file
}

// TODO: Parse through each charge histogram in the root file and generate histograms for each of them. (Bins < 50 will not be displayed)