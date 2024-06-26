#include "MiniCSCData.h"
#include <Fit/BinData.h>
#include <Fit/FitData.h>
#include <Fit/Fitter.h>
#include <HFitInterface.h>
#include <Math/WrappedMultiTF1.h>
#include <RtypesCore.h>
#include <TF1.h>
#include <TH1.h>
#include <TStyle.h>
#include <iostream>

/// minimum x value for peak1, also acts as lower bound for summed fit, just give it a good guess
static const size_t peak1_min = 800;
/// maximum x value for peak1, just give it a good guess
static const size_t peak1_max = 2900;
/// minimum x value for peak2, just give it a good guess
static const size_t peak2_min = 3200;
/// maximum x value for peak2, also acts as upper bound for summed fit, just give it a good guess
static const size_t peak2_max = 8000;
/// true if cadmium (has two peaks)
static const bool two_peaks = true;

/// Use this to fit a charge spectra
void chargeFit()
{
    MiniCSCData mdata("./r78_thres32.root");

    TH1D* chg = mdata.ChargeSpectra()[2];
    chg->Rebin(32);
    chg->SetFillColor(38);

    // Define the parameter array for the total function.
    double par[9];

    // Three TF1 objects are created, one for each subrange.
    TF1* g1 = new TF1("g1", "gaus", peak1_min, peak1_max);
    g1->SetLineWidth(3);
    TF1* g2 = new TF1("g2", "gaus", peak2_min, peak2_max);
    g2->SetLineWidth(3);

    // The total is the sum of the three, each has three parameters.
    TF1* total = new TF1("total", "gaus(0)+gaus(3)", peak1_min, peak2_max);
    total->SetLineColor(2);
    total->SetLineWidth(3);

    // Fit each function and add it to the list of functions. By default,
    // TH1::Fit() fits the function on the defined histogram range. You can
    // specify the "R" option in the second parameter of TH1::Fit() to restrict
    // the fit to the range specified in the TF1 constructor. Alternatively, you
    // can also specify the range in the call to TH1::Fit(), which we demonstrate
    // here with the 3rd Gaussian. The "+" option needs to be added to the later
    // fits to not replace existing fitted functions in the histogram.
    chg->Fit(g1, "R");
    if (two_peaks) chg->Fit(g2, "R+");

    // Get the parameters from the fit.
    g1->GetParameters(&par[0]);
    g2->GetParameters(&par[3]);

    // Use the parameters on the sum.
    total->SetParameters(par);
    chg->Fit(total, "R");
    gStyle->SetOptFit(1111);
    chg->Draw();
    chg->GetXaxis()->SetRangeUser(0.5, 10000.5);
}
