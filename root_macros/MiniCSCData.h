#ifndef MINICSCDATA_H_
#define MINICSCDATA_H_

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

/// @class MiniCSCData contains graph utilities for root files output by the MiniCSC CMSSW plugin
class MiniCSCData
{
public:
    /// Enumeration for getting a specific graph.
    /// If you want to add a graph just add it to the enum (ideally at the end) and update
    /// MiniCSCData::graphPaths_ with the path within the root file
    enum class Graph {
        kWireOccupancy,      // TH1D, multi-layer, wirgroup occupancy for a layer
        kSimulAnodeHit,      // TH2F, multi-layer, (WIP) simultanious anode hits for a layer
        kFiredTBinAnode,     // TH2F, multi-layer, (WIP) which time bin each wiregroup tends to
                             // fire
        kFiredWireGroup,     // TH1I, all-layers, how many wiregroups fire per event
        kChargeSpectra,      // TH1D, multi-layer, charge spectra on all strips per layer
        kChargeTBin,         // TH1D, multi-layer, (DEPRECIATED) Shows time bin firing occupancy for all
                             // strips
        kChargeTBinWeighted, // TH1D, multi-layer, (DEPRECIATED) Same as Graph::kChargeTBin however
                             // each bin is weighted with the accumulated charge
        kStripTBinADCVal,    // TH2F, multi-layer, time bin firing occupancy per strip
        kStripOccupancy,     // TH1D, multi-layer, strip occupancy (ADC>13) for a layer
        kHalfStripOccupancy, // TH1D, multi-layer, half-strip occupancy for a layer
        kAveragePedestal,    // TH1F, multi-layer, (WIP) average pedestal value for each ADC
        kFirstPedestal,      // TH1F, multi-layer, first sampled pedestal value
        kFiredStrip,         // TH1F, all-layers, how many strips fire per event
        kChargeTBinProfile,  // TProfile, all-layers, better representation of Graph::kChargeTBin,
                             // shows time bin firing occupancy for all strips and layers
        kLAST                // Just for array sizing, no members should be placed after this
    };

    /// Number of layers in a miniCSC, should be 6 unless something very bad happened
    const uint16_t kNumLayers_ = 6;

    /// Root file containing the graphs
    TFile* rootFile_;

    /// Constructor
    /// @param rootFilePath full file path pointing to the root file
    /// @param nullableGraphs If true, changes graph getters to return a nullptr if a graph cannot be found.
    ///                       If false, not found graphs will be empty
    /// @param vectStartZero If true, graph getter vectors will start at zero, so to get a graph for a layer you would
    ///                      need to use layer - 1. If false, graph vectors will start at 1, aligning with miniCSC
    MiniCSCData(const char* rootFilePath, bool nullableGraphs = true, bool vectStartZero = false)
        : nullableGraphs_(nullableGraphs)
        , vectStartZero_(vectStartZero)
    {
        // Allow other open options?
        rootFile_ = new TFile(rootFilePath, "READ");

        // Getting the file name from path for future utilities
        std::stringstream path(rootFilePath);
        std::string segment;
        std::vector<std::string> seglist;

        while (std::getline(path, segment, '/')) {
            seglist.push_back(segment);
        }
        // NOTE: It appears root will free the strdup result at some point, when I tried to do it manually
        //       the interpreter complained about duplicate freeing.
        rootFileName_ = strdup(seglist.back().c_str());

        // Getting anode graphs
        wireOccupancy_  = GetGraphs<TH1D>(Graph::kWireOccupancy);
        simulAnodeHit_  = GetGraphs<TH2F>(Graph::kSimulAnodeHit);
        firedTBinAnode_ = GetGraphs<TH2F>(Graph::kFiredTBinAnode);
        firedWireGroup_ = GetGraph<TH1I>(Graph::kFiredWireGroup);

        // Getting cathode graphs
        chargeSpectra_      = GetGraphs<TH1D>(Graph::kChargeSpectra);
        chargeTBin_         = GetGraphs<TH1D>(Graph::kChargeTBin);
        chargeTBinWeighted_ = GetGraphs<TH1D>(Graph::kChargeTBinWeighted);
        stripTBinADCVal_    = GetGraphs<TH2F>(Graph::kStripTBinADCVal);
        stripOccupancy_     = GetGraphs<TH1D>(Graph::kStripOccupancy);
        halfStripOccupancy_ = GetGraphs<TH1D>(Graph::kHalfStripOccupancy);
        avgPedestal_        = GetGraphs<TH1F>(Graph::kAveragePedestal);
        fstPedestal_        = GetGraphs<TH1F>(Graph::kFirstPedestal);
        firedStrip_         = GetGraph<TH1I>(Graph::kFiredStrip);
        chargeTBinProfile_  = GetGraph<TProfile>(Graph::kchargeTBinProfile);
    }

    // ~MiniCSCData() { std::free(rootFileName_); }

    // Anode Getters ===========================================================

    /// Get wire occupancy graph
    std::vector<TH1D*> WireOccupancy() const { return wireOccupancy_; }
    /// (GRAPH WIP) Get simultanious anode hit graph
    std::vector<TH2F*> SimulAnodeHit() const { return simulAnodeHit_; }
    /// (GRAPH WIP) Get graph showing which time bin occupancy for each wiregroup
    std::vector<TH2F*> FiredTBinAnode() const { return firedTBinAnode_; }
    /// Get graph showing how many wiregroups fire per event
    TH1I* FiredWireGroup() const { return firedWireGroup_; }

    // Cathode Getters =========================================================

    /// Get graph of charge spectra on all strips per layer
    std::vector<TH1D*> ChargeSpectra() const { return chargeSpectra_; }
    /// (GRAPH DEPRECIATED) Get graph of time bin firing occupancy for all strips
    std::vector<TH1D*> ChargeTBin() const { return chargeTBin_; }
    /// (GRAPH DEPRECIATED) Get graph of time bin firing occupancy weighted with the accumulated charge
    std::vector<TH1D*> ChargeTBinWeighted() const { return chargeTBinWeighted_; }
    /// Get graph of time bin firing occupancy per strip
    std::vector<TH2F*> StripTBinADCVal() const { return stripTBinADCVal_; }
    /// Get graph of strip occupancy (>13ADC)
    std::vector<TH1D*> StripOccupancy() const { return stripOccupancy_; }
    /// Get graph of half-strip occupancy
    std::vector<TH1D*> HalfStripOccupancy() const { return halfStripOccupancy_; }
    /// (GRAPH WIP) Get graph of average pedestal for each ADC
    std::vector<TH1F*> AveragePedestal() const { return avgPedestal_; }
    /// Get graph of first sampled pedestal value for each ADC
    std::vector<TH1F*> FirstPedestal() const { return fstPedestal_; }
    /// Get graph of how many strips fire per event
    TH1I* FiredStrip() const { return firedStrip_; }
    /// Get graph of time bin firing occupancy for all strips and layers
    TProfile* ChargeTBinProfile() const { return chargeTBinProfile_; }

    // Generic Getters =========================================================

    /// Check if unlocated graphs will return nullptr or empty graph
    const bool NullableGraphs() const { return nullableGraphs_; }
    /// Check if graph vectors start at zero or one when indexing layers
    const bool VectStartZero() const { return vectStartZero_; }
    /// Get root file name read at construction time
    const char* RootFileName() const { return rootFileName_; }

    /// Get Graph by name
    /// @tparam T type of graph to find
    /// @param graphName full path within the root file pointing to the graph
    /// @return pointer to graph object
    template <typename T> T* GetGraph(const char* graphName) const { return getGraphObject<T>(graphName); }

    /// Get Graph by name and optionally for one layer
    /// @tparam T type of graph to find
    /// @param name name of graph to find
    /// @param layer MiniCSC layer that the graph represents
    /// @return pointer to graph object
    template <typename T> T* GetGraph(Graph name, uint16_t layer = kInvalidLayer_) const
    {
        return getGraphObject<T>(getGraphFullPath(name, layer));
    }

    /// Get each layer for one graph
    /// @tparam T type of graph to find
    /// @param name name of graph to find
    /// @return vector of pointers to graphs for each layer
    template <typename T> std::vector<T*> GetGraphs(Graph name) const
    {
        std::vector<T*> vect;
        vect.push_back(nullptr);
        for (size_t i = 1; i < kNumLayers_ + 1; i++) {
            vect.push_back(GetGraph<T>(name, i));
        }
        return vect;
    };

    // Generic Setters ========================================================

    /// Set if unlocated graphs return nullptr or empty graph
    void SetNullableGraphs(bool value) { nullableGraphs_ = value; }
    /// Set if graph vectors start at zero when indexing layers
    void SetVectStartZero(bool value) { vectStartZero_ = value; }

private:
    /// Value used to check for default layer numbers
    const static uint16_t kInvalidLayer_ = 65535;
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
    TProfile* chargeTBinProfile_;

    /// Array containing root file paths for each graph. Index in MiniCSCData::Graph should be the same as desired path
    /// in this array
    const std::array<std::string, static_cast<int>(Graph::kLAST)> graphPaths_
        = { "/Anode/wire/wireL", "/Anode/simulAnodeHit/simulAnodeHitL", "/Anode/firedTBinAnode/firedTBinAnodeL",
              "/Anode/firedWireGroup", "/Cathode/charge/chargeL", "/Cathode/chargeTBin/chargeTBinL",
              "Cathode/chargeTBinWeighted/chargeTBinWeightedL", "/Cathode/stripTBinADCVal/stripTBinADCValL",
              "/Cathode/strip/stripL", "/Cathode/halfStrip/halfStripL", "/Cathode/avgPedestal/avgPedestalL",
              "/Cathode/fstPedestal/fstPedestalL", "/Cathode/firedStrip", "/Cathode/chargeTBinProfile" };

    /// Generates full path by adding layer number to end of string from graphPaths if it is a valid
    /// layer number.
    inline std::string getGraphFullPath(Graph name, unsigned short layer = kInvalidLayer_) const
    {
        return (layer == kInvalidLayer_ || layer > kNumLayers_)
                   ? graphPaths_[static_cast<int>(name)]
                   : graphPaths_[static_cast<int>(name)] + std::to_string(layer);
    }

    /// Gets TObject or Graph from rootFile
    template <typename T> T* getGraphObject(std::string path) const
    {
        T* hist = nullptr;
        rootFile_->GetObject(path.c_str(), hist);
        if (!hist && !nullableGraphs_) hist = new T();

        return hist;
    }
};

#endif // MINICSCDATA_H_
