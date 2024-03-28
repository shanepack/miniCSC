#ifndef MINICSCDATA_H_
#define MINICSCDATA_H_

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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
        kChargeTBin, // TH1D
        kChargeTBinWeighted, // TH1D
        kStripTBinADCVal,
        kStripOccupancy,
        kHalfStripOccupancy,
        kAveragePedestal,
        kFirstPedestal, // TH1F
        kFiredStrip, // TH1F
        kLAST // Just for array sizing
    };
    const unsigned short kNumLayers_ = 6;

    TFile* rootFile;

    MiniCSCData(const char* rootFilePath, bool nullableGraphs = true, bool vectStartZero = false)
        : nullableGraphs_(nullableGraphs)
        , vectStartZero_(vectStartZero)
    {
        // Allow other open options?
        rootFile = new TFile(rootFilePath, "READ");
        // Getting the file name from path
        std::stringstream path(rootFilePath);
        std::string segment;
        std::vector<std::string> seglist;

        while (std::getline(path, segment, '/')) {
            seglist.push_back(segment);
        }
        // NOTE: It appears root will free this at some point, when I tried to do it manually
        //       the interpreter complained about duplicate freeing.
        rootFileName_ = strdup(seglist.back().c_str());

        wireOccupancy_  = GetGraphs<TH1D>(Graph::kWireOccupancy);
        simulAnodeHit_  = GetGraphs<TH2F>(Graph::kSimulAnodeHit);
        firedTBinAnode_ = GetGraphs<TH2F>(Graph::kFiredTBinAnode);
        firedWireGroup_ = GetGraph<TH1I>(Graph::kFiredWireGroup);

        chargeSpectra_      = GetGraphs<TH1D>(Graph::kChargeSpectra);
        chargeTBin_         = GetGraphs<TH1D>(Graph::kChargeTBin);
        chargeTBinWeighted_ = GetGraphs<TH1D>(Graph::kChargeTBinWeighted);
        stripTBinADCVal_    = GetGraphs<TH2F>(Graph::kStripTBinADCVal);
        stripOccupancy_     = GetGraphs<TH1D>(Graph::kStripOccupancy);
        halfStripOccupancy_ = GetGraphs<TH1D>(Graph::kHalfStripOccupancy);
        avgPedestal_        = GetGraphs<TH1F>(Graph::kAveragePedestal);
        fstPedestal_        = GetGraphs<TH1F>(Graph::kFirstPedestal);
        firedStrip_         = GetGraph<TH1I>(Graph::kFiredStrip);
    }

    // ~MiniCSCData() { std::free(rootFileName_); }

    // Anode Getters ===========================================================

    std::vector<TH1D*> WireOccupancy() const { return wireOccupancy_; }
    std::vector<TH2F*> SimulAnodeHit() const { return simulAnodeHit_; }
    std::vector<TH2F*> FiredTBinAnode() const { return firedTBinAnode_; }
    TH1I* FiredWireGroup() const { return firedWireGroup_; }

    // Cathode Getters =========================================================

    std::vector<TH1D*> ChargeSpectra() const { return chargeSpectra_; }
    std::vector<TH1D*> ChargeTBin() const { return chargeTBin_; }
    std::vector<TH1D*> ChargeTBinWeighted() const { return chargeTBinWeighted_; }
    std::vector<TH2F*> StripTBinADCVal() const { return stripTBinADCVal_; }
    std::vector<TH1D*> StripOccupancy() const { return stripOccupancy_; }
    std::vector<TH1D*> HalfStripOccupancy() const { return halfStripOccupancy_; }
    std::vector<TH1F*> AveragePedestal() const { return avgPedestal_; }
    std::vector<TH1F*> FirstPedestal() const { return fstPedestal_; }
    TH1I* FiredStrip() const { return firedStrip_; }

    // Generic Getters =========================================================

    const bool NullableGraphs() const { return nullableGraphs_; }
    const bool VectStartZero() const { return vectStartZero_; }
    const char* RootFileName() const { return rootFileName_; }

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
    char* rootFileName_;
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
    std::vector<TH1D*> chargeTBinWeighted_;
    std::vector<TH2F*> stripTBinADCVal_;
    std::vector<TH1D*> stripOccupancy_;
    std::vector<TH1D*> halfStripOccupancy_;
    std::vector<TH1F*> avgPedestal_;
    std::vector<TH1F*> fstPedestal_;
    TH1I* firedStrip_;

    const std::string graphPaths_[static_cast<int>(Graph::kLAST)] = {
        "/Anode/wire/wireL",
        "/Anode/simulAnodeHit/simulAnodeHitL",
        "/Anode/firedTBinAnode/firedTBinAnodeL",
        "/Anode/firedWireGroup",
        "/Cathode/charge/chargeL",
        "/Cathode/chargeTBin/chargeTBinL",
        "Cathode/chargeTBinWeighted/chargeTBinWeightedL",
        "/Cathode/stripTBinADCVal/stripTBinADCValL",
        "/Cathode/strip/stripL",
        "/Cathode/halfStrip/halfStripL",
        "/Cathode/avgPedestal/avgPedestalL",
        "/Cathode/fstPedestal/fstPedestalL",
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
