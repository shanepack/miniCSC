#include "MiniCSCUtils.h"

const static uint16_t colors[] = { 38, 30, 46 };

template <typename T> void MUtils::drawStack(TCanvas* cc, const histInfo<T>& histHeader, const char* fitOpt)
{
    cc->Clear();
    cc->cd();

    for (int i = 0; i < histHeader.hist.size(); i++) {
        T* hist = histHeader.hist[i];

        uint16_t color = colors[i];
        hist->SetLineColor(color);
        hist->SetFillColorAlpha(color, 0.4);
        hist->SetLineWidth(1);
        uint64_t max = hist->GetMaximum();
        hist->GetYaxis()->SetRangeUser(0, max * 1.4);
        if (!i)
            hist->Draw("HIST");
        else
            hist->Draw("HIST SAME");

        // NOTE: Tried to keep memory safe with strncmp, length shouldnt matter too much just as
        // long as it tests 1 char
        if (strncmp(fitOpt, "", 1) != 0) {
            double minx = hist->GetXaxis()->GetXmin();
            double maxx = hist->GetXaxis()->GetXmax();
            TF1* fit    = new TF1("fit", "gaus", minx - 1, maxx + 1);
            fit->SetLineColor(color);
            fit->SetLineWidth(3);
            hist->Fit(fit, "RQ");
            fit->Draw("SAME");
        }
    }
    histHeader.legend->Draw();
    cc->Draw();
}
