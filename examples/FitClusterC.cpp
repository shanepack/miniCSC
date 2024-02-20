
#include "Riostream.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TH1.h"
#include "TMath.h"
#include "TStyle.h"
#include <TMatrixTSym.h>
#include <TROOT.h>
#include <cmath>
#include <iostream>
#include <math.h>

Double_t Gauss(Double_t* x, Double_t* par)
{
    Double_t arg = 0;
    if (par[2] != 0)
        arg = (x[0] - par[1]) / par[2];
    //	Double_t gauss = par[0] /  (2.506628275*par[2]) *
    // TMath::Exp(-0.5*arg*arg);
    Double_t gauss = par[0] * TMath::Exp(-0.5 * arg * arg);
    return gauss;
}

using namespace std;

void FitClusterC(TString filename = "0")
{

    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasColor(kWhite);
    gStyle->SetFrameBorderMode(0);
    gStyle->SetFrameFillColor(kWhite);
    gStyle->SetPalette(1, 0);
    // gStyle->SetOptStat(000000000);
    gStyle->SetOptStat(1);
    gStyle->SetOptFit(1111);

    TCanvas* cankres1 = new TCanvas("cankres1", "Canvas with fit1", 1200, 800);

    if (filename != "0") {
        cout << "Opening file: " << filename << endl;
        TFile* fin = new TFile(filename, "READ");
    } else {
        cout << "Reading from current file" << endl;
    }

    cout << gFile->GetName() << endl;
    TH1D* hClusterC = (TH1D*)gFile->Get(
        "DQMData/CSC_013_04/h1_CSC_013_04_CFEB_Clusters_Charge_Ly_6");

    hClusterC->Draw();
    ///*
    const int nParams = 3;
    TF1* GFit = new TF1("Gauss fit", Gauss, 0, 3000, nParams);
    GFit->SetParNames("A", "Mean", "Sigma");
    // basic parameter settings
    GFit->SetParLimits(1, 300, 1500);
    GFit->SetParLimits(2, 50, 500);
    GFit->SetParameters(
        hClusterC->GetBinContent(20), 700,
        100); // Setting initial parameter to approx the peak position

    double lowerFitBound = 500; // this can probably be improved, just some
                                // reasonable range for now for Cd
    double upperFitBound = 1200;

    // double lowerFitBound = 300; //this can probably be improved, just some
    // reasonable range for now for Fe double upperFitBound = 850;

    // Setting for no signal
    // GFit->SetParLimits(1, 50, 1500);
    // GFit->SetParLimits(2, 10, 500);
    // GFit->SetParameters(hClusterC->GetBinContent(20), 200, 100); //Setting
    // initial parameter to approx the peak position

    // double lowerFitBound = 150; //this can probably be improved, just some
    // reasonable range for now for Fe double upperFitBound = 300;

    TFitResultPtr FitRes = hClusterC->Fit(GFit, "L S R", "", lowerFitBound, upperFitBound); // O: L

    cout << endl;
    for (int j = 0; j < nParams; j++) {
        cout << setw(8) << GFit->GetParName(j)
             << " has value and error: " << setw(8) << GFit->GetParameter(j)
             << " +- " << GFit->GetParError(j) << endl;
    }
    cout << endl;
    //*/
    TString canSaveName = gFile->GetName();
    canSaveName.Remove((canSaveName.Length() - 5), 5);
    canSaveName += ".png";
    cout << canSaveName << endl;

    cankres1->SaveAs(canSaveName);
}
