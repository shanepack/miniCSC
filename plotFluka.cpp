// Include necessary libraries
#include "TF1.h"
#include "TF2.h"
#include "TH1.h"
#include "TH2.h"
#include "TRandom.h"
#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TLegend.h>
#include <TPad.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// Define constants
static const int LUMI = 5;
static const int hmaxx = 270; // Maximum value for the x-axis of the histogram
static const char* FILE_NAME = "/home/fwoup/rootcode/RunII_allcharged_cav_ME11.txt";
static const string FILE_DIR = "/home/fwoup/rootcode/";
static const bool SCALING = true;

float R_MIN;
float R_MAX;
int R_BIN_NUM;
float Z_BIN_NUM;
float BIN_WIDTH;

using namespace std;

// Define sensitivity values for different particles
std::map<std::string, double> sensitivity = {
    { "neutrons", 0.001 },
    { "photons", 0.01 },
    { "allcharged", 1.0 },
    { "hcharged", 1.0 },
    { "electrons", 1.0 },
    { "muons", 1.0 }
};

// Define color codes for different particles
std::map<std::string, int> color = {
    { "neutrons", 1 },
    { "photons", 4 },
    { "allcharged", 6 },
    { "hcharged", 595 },
    { "electrons", 881 },
    { "muons", 94 }
};

vector<string> parseFile(string particle)
{
    ifstream file;
    string filename = FILE_DIR + "RunII_" + particle + "_cav_ME11.txt";
    cout << "Reading file: " << filename << endl;
    file.open(filename);
    vector<string> data;
    string line;
    while (getline(file, line)) {
        data.push_back(line);
    }

    // Read and parse lines 7, 8, and 9 to get the binning information
    R_MIN = std::stof(data[6].substr(data[6].find(":") + 1)); // stof converts string to float
    R_MAX = std::stof(data[7].substr(data[7].find(":") + 1));
    R_BIN_NUM = std::stoi(data[8].substr(data[8].find(":") + 1)); // stoi converts string to int
    Z_BIN_NUM = std::stof(data[9].substr(data[9].find(":") + 1));
    // Calculate the bin width
    BIN_WIDTH = (R_MAX - R_MIN) / R_BIN_NUM;
    file.close();
    return data;
}

TGraphErrors* sumGraphs(std::vector<TGraphErrors*>& gr)
{
    int n = gr[0]->GetN();
    double rmin = gr[0]->GetPointX(0);
    double rmax = gr[0]->GetPointX(n - 1);

    std::vector<TGraphErrors*> graphRatio;
    graphRatio.push_back(new TGraphErrors(n));
    for (int i = 1; i < gr.size(); i++) {
        if (!(gr[i]->GetN() == n) || !(gr[i]->GetPointX(0) == rmin) || !(gr[i]->GetPointX(n - 1) == rmax)) {
            std::cout << "sumGraph : not compatible x scale" << std::endl;
            return nullptr;
        }
        graphRatio.push_back(new TGraphErrors(n));
    }

    TGraphErrors* graphSum = new TGraphErrors(n);
    for (int i = 0; i < n; i++) {
        double x = gr[0]->GetPointX(i);
        double y = 0;
        for (int j = 0; j < gr.size(); j++) {
            y += gr[j]->GetPointY(i);
        }
        graphSum->SetPoint(i, x, y);
        graphSum->SetPointError(i, (rmax - rmin) / n / 2, 0);

        for (int j = 0; j < gr.size(); j++) {
            double yy = 0;
            if (y != 0) {
                yy = gr[j]->GetPointY(i);
            }
            graphRatio[j]->SetPoint(i, x, yy);
            graphRatio[j]->SetPointError(i, (rmax - rmin) / n, 0);
        }
    }

    graphSum->SetLineWidth(4);
    graphSum->SetLineColor(2);

    for (int j = 0; j < gr.size(); j++) {
        graphRatio[j]->SetLineColor(gr[j]->GetLineColor());
        graphRatio[j]->SetLineWidth(3);
    }

    return graphSum;
}

TGraphErrors* getGraph(string particle = "allcharged", bool scale = false)
{
    vector<string> data = parseFile(particle);

    TGraphErrors* gr = new TGraphErrors(R_BIN_NUM);

    int i = 0;
    // Read the data from the file and calculate the flux values
    for (const string& line : data) {
        if (line[0] == '#') continue;
        istringstream iss(line);

        float y = 0.0;
        for (int j = 0; j < Z_BIN_NUM; j++) {
            float ytmp = 0.0;
            iss >> ytmp;
            y += ytmp;
        }

        y = y / 2 * LUMI;
        if (scale) y *= sensitivity[particle];

        // Set the data points for the TGraphErrors object
        gr->SetPoint(i, R_MIN + i * BIN_WIDTH + BIN_WIDTH / 2.0, y);
        gr->SetPointError(i, BIN_WIDTH / 2.0, 0);
        gr->SetLineColor(color[particle]);
        gr->SetLineWidth(3);

        i++;
    }

    return gr;
}

// Function to plot Fluka data
void plotFluka()
{
    // Create a canvas for plotting
    TCanvas* c1 = new TCanvas();
    c1->Divide(1);
    c1->cd(1);
    TVirtualPad* p1 = c1->GetPad(1); // Get the first pad
    p1->SetLogy();

    // Set the title for the plot
    std::string title = "Flux";
    if (SCALING == true) title += " scaled";
    title += ";;Flux [Hz/cm^{2}]";

    // Create a 2D histogram for the plot
    TH2* h1 = new TH2F("h1", title.c_str(), 170, 100, hmaxx, 10000, 1, 1000000);

    h1->Draw();

    // Set the axis labels and offsets
    h1->GetXaxis()->SetTitleOffset(1.2);
    h1->GetXaxis()->SetLabelOffset(1.0);
    h1->GetYaxis()->SetTitleOffset(1.0);
    h1->GetYaxis()->SetTitleSize(0.08);
    h1->GetYaxis()->SetLabelSize(0.08);

    TGraphErrors* gr_allcharged = getGraph("allcharged", true);
    TGraphErrors* gr_neutrons = getGraph("neutrons", true);
    TGraphErrors* gr_photons = getGraph("photons", true);

    vector<TGraphErrors*> graphs = { gr_allcharged, gr_neutrons, gr_photons };
    TGraphErrors* gr_sum = sumGraphs(graphs);

    gr_allcharged->Draw("Psames");
    gr_neutrons->Draw("Psames");
    gr_photons->Draw("Psames");
    gr_sum->Draw("Psames");

    TLegend* l1 = new TLegend(0.20, 0.65, 0.7, 0.87);
    l1->SetHeader("ME11 at 5e34 Hz/cm^{2}");
    l1->SetBorderSize(0);
    l1->SetFillStyle(0);
    l1->AddEntry("", "", "");
    l1->AddEntry(gr_sum, "#splitline{BG RunII FLUKA ("
                         ") scaled}{(neutrons+photons+charged)}",
        "lp");
    l1->Draw();

    TLegend* l2 = new TLegend(0.7, 0.47, 0.9, 0.85);
    l2->SetBorderSize(0);
    l2->SetFillStyle(0);
    l2->AddEntry(gr_neutrons, ("neutron x" + ([](double val) { std::ostringstream ss; ss << std::fixed << std::setprecision(3) << val; return ss.str(); })(sensitivity["neutrons"])).c_str(), "lp");
    l2->AddEntry(gr_photons, ("photons x" + ([](double val) { std::ostringstream ss; ss << std::fixed << std::setprecision(3) << val; return ss.str(); })(sensitivity["photons"])).c_str(), "lp");
    l2->AddEntry(gr_allcharged, ("all charged x" + ([](double val) { std::ostringstream ss; ss << std::fixed << std::setprecision(3) << val; return ss.str(); })(sensitivity["allcharged"])).c_str(), "lp");
    l2->Draw();

    // Update the canvas
    c1->Update();

    // Close the input file
    // file.close();
}
