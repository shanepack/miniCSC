#ifndef MINICSCDATA_H_
#define MINICSCDATA_H_

#include <iostream>
#include <string>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"

class MiniCSCData
{
public:
    /// Enumeration for easy graph getting.
    // If you want to add a graph just make sure it goes directly before last and add it to the graphPaths array.
    // TODO: Add graph descriptions
    enum class Graph {
        kWireOccupancy, // TH1D, multi-layer, contains wirgroup occupancy for a layer
        kSimulAnodeHit,
        kFiredTBinAnode,
        kFiredWireGroup,
        kChargeSpectra,
        kChargeTBin,
        kStripTBinADCVal,
        kStripOccupancy,
        kHalfStripOccupancy,
        kFiredStrip,
        kLAST // Just for array sizing
    };
    const unsigned short kNumLayers_ = 6;

    TFile* rootFile;

    MiniCSCData(const char* rootFileName, bool nullableGraphs = true, bool vectStartZero = false)
        : nullableGraphs_(nullableGraphs)
        , vectStartZero_(vectStartZero)
    {
        // Do we want to only work with std::string as well?
        // Allow other open options?
        rootFile = new TFile(rootFileName, "READ");

        wireOccupancy_ = GetGraphs<TH1D>(Graph::kWireOccupancy);
        simulAnodeHit_ = GetGraphs<TH2F>(Graph::kSimulAnodeHit);
        firedTBinAnode_ = GetGraphs<TH2F>(Graph::kFiredTBinAnode);
        firedWireGroup_ = GetGraph<TH1I>(Graph::kFiredWireGroup);

        chargeSpectra_ = GetGraphs<TH1D>(Graph::kChargeSpectra);
        chargeTBin_ = GetGraphs<TH1D>(Graph::kChargeTBin);
        stripTBinADCVal_ = GetGraphs<TH2F>(Graph::kStripTBinADCVal);
        stripOccupancy_ = GetGraphs<TH1D>(Graph::kStripOccupancy);
        halfStripOccupancy_ = GetGraphs<TH1D>(Graph::kHalfStripOccupancy);
        firedStrip_ = GetGraph<TH1I>(Graph::kFiredStrip);
    }

    // ~MiniCSCData() { rootFile->Close(); }

    // Anode Getters ===========================================================

    std::vector<TH1D*> WireOccupancy() const { return wireOccupancy_; }
    std::vector<TH2F*> SimulAnodeHit() const { return simulAnodeHit_; }
    std::vector<TH2F*> FiredTBinAnode() const { return firedTBinAnode_; }
    TH1I* FiredWireGroup() const { return firedWireGroup_; }

    // Cathode Getters =========================================================

    std::vector<TH1D*> ChargeSpectra() const { return chargeSpectra_; }
    std::vector<TH1D*> ChargeTBin() const { return chargeTBin_; }
    std::vector<TH2F*> StripTBinADCVal() const { return stripTBinADCVal_; }
    std::vector<TH1D*> StripOccupancy() const { return stripOccupancy_; }
    std::vector<TH1D*> HalfStripOccupancy() const { return halfStripOccupancy_; }
    TH1I* FiredStrip() const { return firedStrip_; }

    // Generic Getters =========================================================

    const bool NullableGraphs() const { return nullableGraphs_; }
    const bool VectStartZero() const { return vectStartZero_; }

    template <typename T>
    T* GetGraph(const char* graphName) const { return getGraphObject<T>(graphName); }

    /// Get Graph by name and optionally for one layer
    template <typename T>
    T* GetGraph(Graph name, unsigned short layer = kInvalidLayer_) const
    {
        return getGraphObject<T>(getGraphFullPath(name, layer));
    }

    /// Get each layer for one graph
    template <typename T>
    std::vector<T*> GetGraphs(Graph name) const
    {
        std::vector<T*> vect;
        vect.push_back(nullptr);
        for (size_t i = 1; i < kNumLayers_ + 1; i++) {
            vect.push_back(GetGraph<T>(name, i));
        }
        return vect;
    };

    // Generic Setters ========================================================

    void SetNullableGraphs(bool value) { nullableGraphs_ = value; }
    void SetVectStartZero(bool value) { vectStartZero_ = value; }

private:
    const static unsigned short kInvalidLayer_ = 0;
    bool nullableGraphs_;
    bool vectStartZero_;

    // Anode plots
    std::vector<TH1D*> wireOccupancy_;
    std::vector<TH2F*> simulAnodeHit_;
    std::vector<TH2F*> firedTBinAnode_;
    TH1I* firedWireGroup_;

    // Cathode plots
    std::vector<TH1D*> chargeSpectra_;
    std::vector<TH1D*> chargeTBin_;
    std::vector<TH2F*> stripTBinADCVal_;
    std::vector<TH1D*> stripOccupancy_;
    std::vector<TH1D*> halfStripOccupancy_;
    TH1I* firedStrip_;

    const std::string graphPaths_[static_cast<int>(Graph::kLAST)] = {
        "/Anode/wire/wireL",
        "/Anode/simulAnodeHit/simulAnodeHitL",
        "/Anode/firedTBinAnode/firedTBinAnodeL",
        "/Anode/firedWireGroup",
        "/Cathode/charge/chargeL",
        "/Cathode/chargeTBin/chargeTBinL",
        "/Cathode/stripTBinADCVal/stripTBinADCValL",
        "/Cathode/strip/stripL",
        "/Cathode/halfStrip/halfStripL",
        "/Cathode/firedStrip"
    };

    /// Generates full path by adding layer number to end of string from graphPaths if it is a valid layer number.
    inline std::string getGraphFullPath(Graph name, unsigned short layer = kInvalidLayer_) const
    {
        return (layer == kInvalidLayer_ || layer > kNumLayers_) ? graphPaths_[static_cast<int>(name)]
                                                                : graphPaths_[static_cast<int>(name)] + std::to_string(layer);
    }

    template <typename T>
    T* getGraphObject(std::string path) const
    {
        T* hist = nullptr;
        rootFile->GetObject(path.c_str(), hist);
        if (!hist && !nullableGraphs_) hist = new T();

        return hist;
    }
};

#endif // MINICSCDATA_H_
