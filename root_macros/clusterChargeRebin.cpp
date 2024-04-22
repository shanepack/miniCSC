#include "TCanvas.h"
#include "TH1.h"
#include "TLegend.h"
#include "TStyle.h"
#include <THStack.h>
#include <TKey.h>
#include <TList.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <iostream>
#include <vector>

#include "MiniCSCData.h" // MiniCSCData.h object to read the root file, true for null pointer, false for blank graphs

//?================================================================================================================================================================
//? 1. The clusterChargeRebin function reads all root files in a defined directory and extracts the cluster charge histograms from them.
//? 2. Then, the function rebins the cluster charge histograms and displays them on the same canvas, alongside a Gaussian fit for each histogram.
//? 3. The histograms & Gaussian fits are colored differently to distinguish them from each other.
//? 4. The histograms are saved as a PDF file in the outputs folder, which can be used to compare the cluster charge distributions.
//?================================================================================================================================================================

using namespace std;

//Change this value to rebin the histograms by a different factor (32 is close to DQM plots)
//! Set to 1 to disable rebinning.
int rebinFactor = 32;

void clusterChargeRebin()
{
    // Disable default stats
    gStyle->SetOptStat(0);

    TCanvas* canvas = new TCanvas("canvas", "Histogram Comparison", 800, 600);
    auto leg = new TLegend(0.7, 0.7, 0.9, 0.9);

    // Counter to vary histogram colors.
    //You can also choose your own colors by using a vector of pre-defined color values.
    int colorCounter = 0;
    // Used to store the maximum value of the histograms for the y-axis range.
    int prevMax = 0;

    //?--------------------------------------Read Root File's Cluster Charge Graphs--------------------------------------

    //! NOTE: Replace with your own path to root files.
    TSystemDirectory dir("rootfiles", "../../rootfiles");
    TList* files = dir.GetListOfFiles();
    files->Sort();
    if (files) {
        TSystemFile* file;
        TString fname;
        TIter next(files);
        while ((file = (TSystemFile*)next())) {
            fname = file->GetName();
            if (!file->IsDirectory() && fname.EndsWith(".root")) {

                //! NOTE: Replace with your own path to root files.
                TString filePath = TString::Format("../../rootfiles/%s", fname.Data());

                // MiniCSCData.h object to read the root file.
                // 1st False for blank histograms (Change to true for null pointer if histogram is not found.)
                // 2nd True for vectStartZero to start the layer number at 0 (Used in hist definition below.)
                MiniCSCData myFile(filePath, false, true);

                // MiniCSCData.h object to define which histogram(s) to read, and what layer to read from.
                // If MiniCSCData's vectStartZero is enabled, the layer number is 1 less than the layer you want to read.
                TH1D* hist = (myFile.ChargeSpectra()[2]);

                //*OPTIONAL: Use 'GetGraph' if you need a specific graph/to read something without a getter (this ^)
                //*OPTIONAL: hist = myFile.GetGraph<TH1D>(MiniCSCData::Graph::kChargeSpectra, 3);

                if (!hist) continue; // Skip if histogram is not found

                //!NOTE: Change the title of the histogram here.
                hist->SetTitle("Dark Rate: Premix, PD = 65 vs. Dynamic, PD = 66");

                //?--------------------------------------Rebin Histograms--------------------------------------

                //!NOTE: Rebin DIVIDES by rebinFactor. (32 is close to DQM plots)
                hist->Rebin(rebinFactor); // Rebin the histogram
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

                // Set the x-axis min and max. (Cluster charge spectra should start at 0.5)
                //? (Use '.5' to align the bin index with the center of the bin.)
                hist->GetXaxis()->SetRangeUser(0.5, 3000.5);

                // Get the highest bin value between all histograms for the y-axis range.
                int max = hist->GetMaximum();
                if (max > prevMax) {
                    prevMax = max;
                }

                //! **Y-Axis Range is set after all histograms are processed.**

                //?--------------------------------------Legend--------------------------------------

                // Add legend entries based off the file name and number of entries each histogram
                leg->AddEntry(hist, TString::Format("%s:", fname.Data()), "l");
                leg->AddEntry((TObject*)0, TString::Format("Entries: %d", (int)hist->GetEntries()), "");

                // Increment color counter for the next file. Also used to determine if the histogram is the first one!
                colorCounter++;
            }
        }
    }

    //?--------------------------------------Y-Axis Sizing--------------------------------------

    // Set the y-axis range of entire canvas using prevMax, adjusting it up by 20%
    if (colorCounter > 0) { // Check if any histograms were found
        ((TH1*)canvas->GetListOfPrimitives()->First())->GetYaxis()->SetRangeUser(0, prevMax * 1.2);
    } else {
        cout << "No histograms found. Check your .root files/directories." << endl;
    }

    //?--------------------------------------Drawing & Saving--------------------------------------

    leg->Draw();
    canvas->Draw();
    canvas->SaveAs("../../outputs/clusterCharge.pdf"); //! NOTE: Change to your designated path to save PDF files.
    canvas->Close(); // Close the canvas after saving the file
}