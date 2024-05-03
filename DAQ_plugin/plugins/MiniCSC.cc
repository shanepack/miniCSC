// -*- C++ -*-
//
// Package:    MiniCSC/MiniCSC
// Class:      MiniCSC
//
/**\class MiniCSC MiniCSC.cc MiniCSC/MiniCSC/plugins/MiniCSC.cc

 Description: Analyzes Unpacked MiniCSC Digis

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Katerina Kuznetsova
//         Created:  Tue, 27 Feb 2024 14:21:40 GMT
// Modified By:  Dylan Parks
//
//

// system include files
#include <iostream>
#include <cstring>
#include <cstdint>
#include <cstring>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include <FWCore/Framework/interface/ESHandle.h>
#include <Geometry/Records/interface/MuonGeometryRecord.h>
#include <RtypesCore.h>

#include "DataFormats/MuonDetId/interface/CSCDetId.h"

#include "DataFormats/CSCDigi/interface/CSCCLCTDigi.h"
#include "DataFormats/CSCDigi/interface/CSCCLCTDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCStripDigi.h"
#include "DataFormats/CSCDigi/interface/CSCStripDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCWireDigi.h"
#include "DataFormats/CSCDigi/interface/CSCWireDigiCollection.h"

// Root includes
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

class MiniCSC : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit MiniCSC(const edm::ParameterSet &);
  // ~MiniCSC() override;

  // Tags and tokens
  edm::InputTag stripDigiTag;
  edm::InputTag wireDigiTag;
  edm::InputTag clctDigiTag;
  edm::EDGetTokenT<CSCStripDigiCollection> cscStripToken;
  edm::EDGetTokenT<CSCWireDigiCollection> cscWireToken;
  edm::EDGetTokenT<CSCCLCTDigiCollection> cscCLCTToken;

private:
  // Basic Fields ===================================================

  /// Stores various values for histogram sizing
  float wiregroupHigh, wiregroupLow, numWiregroup, stripHigh, stripLow, numStrip, numHalfStrip;

  /// Defines how many strips to plot for charge spectra. 3-5 works best. Higher values allow in more noise from strips that were not actually hit.
  uint32_t stripWidthChg_;
  /// How much higher than pedestal must a strip time bin be to be valid signal
  uint32_t adcThres_;

  // Constants

  /// Number of layers in a standard CSC
  static const uint16_t numLayers = 6;

  // Used mainly for debugging

  /// Number of empty wiregroups
  uint32_t numEmpty = 0;
  /// Total number of events processed
  uint64_t numEventsProc = 0;

  // Output Fields ==================================================

  // Output filenames
  std::string theRootFileName;
  /// Output root file
  TFile *fout;

  // Histograms =====================================================
  // TODO: Some of these do not need to be floats/doubles. Also some of these graphs are really bad and warrant removal.

  // Anode Histograms

  /// Wiregroup occupancy for each layer
  TH1D *wire[numLayers];
  /// number of "simultaneous" hits from a WG (first WG is assigned to be actual one)
  TH2F *h2dNofAhitWG[numLayers];
  /// Anode vs time bin for each layer, representing when in an event the anodes fire
  TH2F *anodeFiredTimeBins[numLayers];
  /// How many wiregroups fire during an event
  TH1I *firedWireGroups;

  // Cathode Histograms

  /// Strip occupancy for each layer
  TH1D *strip[numLayers];
  /// Halfstrip occupancy for each layer
  TH1D *halfStrip[numLayers];
  /// Absolute ADC values for each layer, representing when in an event a strip fires
  TH2F *absADCVal[numLayers];
  /// Average ADC pedestal value across entire run. This could be a good use for TProfiles
  TH1F *avgPedestals[numLayers];
  /// First sample of pedestal for entire run
  TH1F *fstPedestal[numLayers];
  /// Charge spectra for each layer
  TH1D *charges[numLayers];
  /// How many strips fire during an event
  TH1I *firedStrips;
  /// Time bin distribution for all strips in all layers. Shows where in each event a strip fires
  TProfile *chargeTBinProfile;
  /// Maybe represents average charge for fired strip width. Data wasn't super useful but you can have this graph now :)
  TProfile *firedStripsADC;

  // Methods ======================================================

  void beginJob() override;

  // Called for every event
  void analyze(const edm::Event &, const edm::EventSetup &) override;
  // Called after entire run has been analyzed
  void endJob() override;
  /// Handles all anode analysis
  void handleAnodes(const edm::Handle<CSCWireDigiCollection> wires);
  /// Handles all cathode analysis (strip and halfstrip collections)
  void handleCathodes(const edm::Handle<CSCStripDigiCollection> strips, const edm::Handle<CSCCLCTDigiCollection> clct);
};

// Constructor only grabs config options from the caller. Initialization happens in MiniCSC::beginJob.
MiniCSC::MiniCSC(const edm::ParameterSet &iConfig) {
  // Reading all event tags from the python config
  stripDigiTag = iConfig.getParameter<edm::InputTag>("stripDigiTag");
  wireDigiTag = iConfig.getParameter<edm::InputTag>("wireDigiTag");
  clctDigiTag = iConfig.getParameter<edm::InputTag>("clctDigiTag");
  cscWireToken = consumes<CSCWireDigiCollection>(wireDigiTag);
  cscStripToken = (consumes<CSCStripDigiCollection>(stripDigiTag));
  cscCLCTToken = consumes<CSCCLCTDigiCollection>(clctDigiTag);

  // Getting extra config options, these allow for more dynamic analysis without re-compiling.
  theRootFileName = iConfig.getUntrackedParameter<std::string>("rootFileName");
  std::cout << "Output filename: " << theRootFileName << std::endl;
  stripWidthChg_ = iConfig.getParameter<uint32_t>("stripWidthCharges");
  std::cout << "Charge Spectra Strip Width: " << stripWidthChg_ << std::endl;
  adcThres_ = iConfig.getParameter<uint32_t>("adcThreshold");
  std::cout << "ADC Threshold: " << adcThres_ << std::endl;

  // Set up histogram bounds
  // TODO: Use config for these?
  wiregroupHigh = 120.5;
  wiregroupLow = 0.5;
  numWiregroup = wiregroupHigh - wiregroupLow;
  stripHigh = 120.5;
  stripLow = 0.5;
  numStrip = stripHigh - stripLow;
  numHalfStrip = 225;
}

// ------------ method called once each job just before starting event loop
// ------------
void MiniCSC::beginJob() {
  // Title and name buffers
  char t1[250], t2[250];
  // Setup root file
  fout = new TFile(theRootFileName.c_str(), "RECREATE");
  fout->cd();

  // Creating folder layout within the root file
  // Anode Dirs
  fout->mkdir("Anode/");
  fout->mkdir("Anode/wire/");
  fout->mkdir("Anode/simulAnodeHit/");
  fout->mkdir("Anode/firedTBinAnode/");
  // Cathode Dirs
  fout->mkdir("Cathode/");
  fout->mkdir("Cathode/charge/");
  fout->mkdir("Cathode/stripTBinADCVal/");
  fout->mkdir("Cathode/strip/");
  fout->mkdir("Cathode/halfStrip/");
  fout->mkdir("Cathode/avgPedestal/");
  fout->mkdir("Cathode/fstPedestal/");

  // Plots for each layer
  for (int i = 0; i < numLayers; i++) {
    // Anode plots
    sprintf(t1, "wireL%d", i + 1);
    sprintf(t2, "Wiregroup Occupancy for Layer = %d;Anode Wiregroup;Number of events", i + 1);
    wire[i] = new TH1D(t1, t2, numWiregroup, wiregroupLow, wiregroupHigh);

    // Cathode plots
    sprintf(t1, "chargeL%d", i + 1);
    sprintf(t2, "Charge Spectra for Layer = %d;Charge in adc channels;Number of events", i + 1);
    // 4096 is the maximum adc value, 1 ADC for each bin,
    // NOTE: I changed numbinsx from 3 * 4096 to stripWidthChg_ * 4096 on the last day.
    // Should be correct but if things are acting weird revert the change and test further.
    charges[i] = new TH1D(t1, t2, stripWidthChg_ * 4096, 0.5, stripWidthChg_ * 4096 + 0.5);  // 4096, 4095

    sprintf(t1, "stripL%d", i + 1);
    sprintf(t2, "Strip Occupancy for Layer = %d;Strip;Number of events", i + 1);
    strip[i] = new TH1D(t1, t2, numStrip, stripLow, stripHigh);

    sprintf(t1, "halfStripL%d", i + 1);
    sprintf(t2, "HalfStrip Occupancy for Layer = %d;Cathode HalfStrip;Number of events", i + 1);
    halfStrip[i] = new TH1D(t1, t2, numHalfStrip, 0.5, numHalfStrip + 0.5);

    sprintf(t1, "stripTBinADCValL%d", i + 1);
    sprintf(t2, "Layer = %d;Time bin;Strip number", i + 1);
    absADCVal[i] = new TH2F(t1, t2, 8, -0.5, 7.5, numStrip, stripLow, stripHigh);

    // TODO: Make RMS pedestal value graph
    sprintf(t1, "avgPedestalL%d", i + 1);
    sprintf(t2, "Layer = %d;Strip Number;Average value", i + 1);
    avgPedestals[i] = new TH1F(t1, t2, numStrip, stripLow, stripHigh);

    sprintf(t1, "fstPedestalL%d", i + 1);
    sprintf(t2, "Layer = %d;Strip number;First sampled value", i + 1);
    fstPedestal[i] = new TH1F(t1, t2, numStrip, stripLow, stripHigh);

    // Not sure these are useful
    sprintf(t1, "simulAnodeHitL%d", i + 1);
    sprintf(t2, "Layer = %d;Wiregroup;Number of Wiregroups Hit(?)", i + 1);
    h2dNofAhitWG[i] = new TH2F(t1, t2, numWiregroup, wiregroupLow, wiregroupHigh, 11, -1.5, 9);

    sprintf(t1, "firedTBinAnodeL%d", i + 1);
    sprintf(t2, "Layer = %d;Wiregroup;Time bin", i + 1);
    anodeFiredTimeBins[i] = new TH2F(t1, t2, numWiregroup, wiregroupLow, wiregroupHigh, 16, 0, 16);
  };

  sprintf(t1, "chargeTBinProfile");
  sprintf(t2, "MiniCSC average strip signal (>%d ADC) by time for all layers (Qi-(Q0+Q1)/2);Time bin;ADC", adcThres_);
  chargeTBinProfile = new TProfile(t1, t2, 8, -0.5, 7.5);

  firedWireGroups =
      new TH1I("firedWireGroup", "Number of Fired Wire Groups;Wiregroups;Number of events", 20, 0.5, 20.5);

  sprintf(t2, "Number of Fired Strips, ADC Threshold = %d;Number of Strips;Number of events", adcThres_);
  firedStrips = new TH1I("firedStrip", t2, 20, 0.5, 20.5);

  firedStripsADC = new TProfile("firedStripsADC", "Average Charge per Strip Width;Number of Strips;ADC", 20, 0.5, 20.5);
}

// MiniCSC::~MiniCSC() {}

// ------------ method called for each event  ------------
void MiniCSC::analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup) {
  using namespace edm;

  // Analyze Anodes
  // First we get the event by the token recieved from the python config.
  edm::Handle<CSCWireDigiCollection> wires;
  iEvent.getByToken(cscWireToken, wires);
  // Then we pass it through to our anode analyzer.
  handleAnodes(wires);

  // Analyze Cathodes
  edm::Handle<CSCStripDigiCollection> strips;
  edm::Handle<CSCCLCTDigiCollection> clct;
  iEvent.getByToken(cscStripToken, strips);
  iEvent.getByToken(cscCLCTToken, clct);
  handleCathodes(strips, clct);

  numEventsProc++;
}

// Contains some commented out code that was originally used for debug purposes. I'm leaving it for future reference if someone needs to do similar debugging.
void MiniCSC::handleAnodes(const edm::Handle<CSCWireDigiCollection> wires) {
  // Check for empty collection
  if (wires->begin() == wires->end()) {
    numEmpty++;
    return;
  }

  // std::cout << "--------" << iEvent.id() << std::endl;
  // Iterate through each layer
  for (CSCWireDigiCollection::DigiRangeIterator wi = wires->begin(); wi != wires->end(); wi++) {
    CSCDetId id = (CSCDetId)(*wi).first;
    const uint16_t currLayer = id.layer() - 1;

    // int endcap;
    // if(id.endcap()==2)endcap=-1;
    //  std::cout << id.endcap() << "  " << id.station() << "  " << id.ring() <<
    //  " " << id.chamber() << "  " << id.layer()
    //  << std::endl;
    std::vector<CSCWireDigi>::const_iterator wireIt = (*wi).second.first;
    std::vector<CSCWireDigi>::const_iterator lastWire = (*wi).second.second;

    // Iterate through all wiregroups
    while (wireIt != lastWire) {
      // std::cout << "\t" << wireIt->getWireGroup() << "\t" << wireIt->getTimeBin() << std::endl;
      // station_ring[0]->Fill(endcap * id.station(), id.ring());  //get station and ring

      // Fill time bin occupancy
      std::vector<int> timeBins = wireIt->getTimeBinsOn();
      for (int &bin : timeBins) {
        anodeFiredTimeBins[currLayer]->Fill(wireIt->getWireGroup(), bin);
      }
      wire[currLayer]->Fill(wireIt->getWireGroup());  // Fill wires by layer

      uint16_t nWGh = 0;   // number of consecutive WGs found
      bool nextWG = true;  // check for consecutive WG

      const int currentWGN = wireIt->getWireGroup();

      // looking for consecutive hits
      std::vector<CSCWireDigi>::const_iterator nwgIt = wireIt + 1;
      while (nextWG) {
        nWGh++;

        const int WGN = wireIt->getWireGroup();

        if (nwgIt != lastWire) {
          nextWG = ((nwgIt->getWireGroup() - WGN) == 1);
          ++nwgIt;
        } else {
          nextWG = false;
        }
        ++wireIt;
      }  // end checking consecutive hits
      firedWireGroups->Fill(nWGh);
      h2dNofAhitWG[currLayer]->Fill(currentWGN, nWGh);

    };  // all wires
  }     // all layers for wires
}

void MiniCSC::handleCathodes(const edm::Handle<CSCStripDigiCollection> strips,
                             const edm::Handle<CSCCLCTDigiCollection> clct) {
  // All layers for strips
  for (CSCStripDigiCollection::DigiRangeIterator si = strips->begin(); si != strips->end(); si++) {
    CSCDetId id = (CSCDetId)(*si).first;
    std::vector<CSCStripDigi>::const_iterator stripIt = (*si).second.first;
    std::vector<CSCStripDigi>::const_iterator lastStrip = (*si).second.second;

    const uint16_t currLayer = id.layer() - 1;

    // Total charge per strip in layer
    std::vector<float> chgPerStrip;

    // Each strip in layer
    while (stripIt != lastStrip) {
      uint16_t nStriph = 0;   // number of consecutive Strips found
      bool nextStrip = true;  // check for consecutive Strip

      // looking for consecutive hits
      std::vector<CSCStripDigi>::const_iterator nStripIt = stripIt + 1;
      bool was_signal = false;
      while (nextStrip) {
        // getADCCounts() returns a vector containing the adc value for each time bin. The position in the vector is equal to the time bin.
        // Here is an example, I used made up numbers however the general shape should be similar (bell curve ish):
        // Time Bin    0    1    2    3    4    5    6    7
        // ADC Value 1023 1025 1126 1354 1232 1158 1089 1025
        std::vector<int> ADCVals = stripIt->getADCCounts();
        int strNum = stripIt->getStrip();  // 1->16

        // NOTE: This is copied from other CSC code. This does not really change the output other than shift graphs over.
        if (id.ring() == 4) {
          strNum = strNum + 64;
        }

        // Get pedestal. The pedestal is the "zero" for the ADC, ideally this is 1024 however due to noise this may fluctuate.
        // Any time bin above this pedestal are considered valid signals.
        // The pedestal is commonly (including the line below) implemented as (TBin0 + TBin1) / 2.
        const float ped = stripIt->pedestal();
        // Fill pedestal graphs
        avgPedestals[currLayer]->Fill(strNum, ped);
        if (fstPedestal[currLayer]->GetBinContent(strNum) == 0) {
          fstPedestal[currLayer]->Fill(strNum, ped);
        }

        // Determines if the strip has fired by checking if any time bin is <adcThres_> ADC greater than the pedestal
        was_signal = false;
        for (size_t k = 0; k < ADCVals.size(); k++) {
          if ((ADCVals[k] - ped) > adcThres_) {
            was_signal = true;
            break;
          }
        }

        if (was_signal) {
          nStriph++;
          // Total charge from all time bins for strip
          float sumChargesStrip = 0.0f;

          // Iterate through all time bins
          for (size_t i = 0; i < ADCVals.size(); i++) {
            const float charge = ADCVals[i] - ped;

            // Make sure we have valid signal in time bin
            // NOTE: Alexey said to plot all tbins
            // if (charge < adcThres_)
            //   continue;

            absADCVal[currLayer]->Fill(i, strNum, charge);
            chargeTBinProfile->Fill(i, charge);

            // This makes sure the pedestal is not plotted with the charge spectra since that theoretically adds noise to the plot.
            // This shouldn't be needed for the TBin plots.
            // if (i > 1) {
            //   sumChargesStrip += charge;
            // }
            sumChargesStrip += charge;
          }
          // Add strip charge to collection
          chgPerStrip.push_back(sumChargesStrip);

          // Fill strip occupancy
          strip[currLayer]->Fill(strNum);

        }  // was signal
        // Logic to continue checking consecutive strips
        if (nStripIt != lastStrip) {
          nextStrip = was_signal;
          ++nStripIt;
        } else {
          nextStrip = false;
        }
        ++stripIt;
      }  // End consecutive strips
      firedStrips->Fill(nStriph);
    }  // all strips

    float sumCharges = 0.0f;
    int width = 0;
    // Sort the charges from greatest to least
    sort(chgPerStrip.begin(), chgPerStrip.end(), std::greater<float>());
    // Only fill top 3-5 stips (depends on config), also makes sure we don't go past the end of the vector
    for (size_t i = 0; i < chgPerStrip.size() && i < static_cast<size_t>(stripWidthChg_); i++) {
      sumCharges += chgPerStrip[i];
      width++;
    }
    // This checks that we have a valid charge level, removes extra entries at 0.
    if (sumCharges > 0.0f) {
      charges[currLayer]->Fill(sumCharges);
      firedStripsADC->Fill(width, sumCharges);
    }
  }  // strip collection

  // Halfstrips
  for (CSCCLCTDigiCollection::DigiRangeIterator ci = clct->begin(); ci != clct->end(); ci++) {
    CSCDetId id = (CSCDetId)(*ci).first;
    std::vector<CSCCLCTDigi>::const_iterator clctIt = (*ci).second.first;
    std::vector<CSCCLCTDigi>::const_iterator lastCLCT = (*ci).second.second;

    for (; clctIt != lastCLCT; ++clctIt) {
      short unsigned int cfebId = clctIt->getCFEB();
      // HACK: cfebId - 2 makes things line up with correct strip layers, unsure of edge cases
      float hstrNum = clctIt->getKeyStrip();
      if (id.ring() == 4) {
        hstrNum += 128;
      }
      halfStrip[cfebId - 2]->Fill(hstrNum);
    }  // All clct
  }    // clct collection
}

// ------------ method called once each job just after ending the event loop
// ------------
void MiniCSC::endJob() {
  std::cout << "Events Processed: " << numEventsProc << std::endl;
  std::cout << "Num events spectra: " << charges[2]->GetEntries() << std::endl;
  std::cout << "Number of empty wire collections: " << numEmpty << std::endl;
  std::cout << "Writing to root file" << std::endl;

  // Normalize avgPedestals
  for (uint16_t i = 0; i < numLayers; i++) {
    // NOTE: 120 is number of xbins
    for (uint16_t j = 0; j < 120; j++) {
      avgPedestals[i]->SetBinContent(j, avgPedestals[i]->GetBinContent(j) / numEventsProc);
      avgPedestals[i]->SetBinError(j, avgPedestals[i]->GetBinError(j) / numEventsProc);
      if (fstPedestal[i]->GetBinContent(j) != 0) {
        fstPedestal[i]->SetBinError(j, 1024 - fstPedestal[i]->GetBinContent(j));
      }
    }
  }

  fout->cd();

  // Write Anode histograms
  for (uint16_t i = 0; i < numLayers; i++) {
    if (wire[i]->GetEntries() != 0) {
      fout->cd("/Anode/wire/");
      wire[i]->Write();
    }
    if (h2dNofAhitWG[i]->GetEntries() != 0) {
      fout->cd("/Anode/simulAnodeHit/");
      h2dNofAhitWG[i]->Write();
    }
    if (anodeFiredTimeBins[i]->GetEntries() != 0) {
      fout->cd("/Anode/firedTBinAnode/");
      anodeFiredTimeBins[i]->Write();
    }
  }
  fout->cd("/Anode/");
  firedWireGroups->Write();

  // Write Cathode histograms
  fout->cd("/Cathode/");
  for (uint16_t i = 0; i < numLayers; i++) {
    // Plot everything relating to strip occupancy
    if (strip[i]->GetEntries() != 0) {
      fout->cd("/Cathode/strip/");
      strip[i]->Write();
      fout->cd("/Cathode/halfStrip/");
      halfStrip[i]->Write();
    }

    // Plot everything related to charge spectra
    if (charges[i]->GetEntries() != 0) {
      fout->cd("/Cathode/charge/");
      charges[i]->Write();
      fout->cd("/Cathode/stripTBinADCVal/");
      absADCVal[i]->Write();
      fout->cd("/Cathode/avgPedestal/");
      avgPedestals[i]->Write();
      fout->cd("/Cathode/fstPedestal/");
      fstPedestal[i]->Write();
    }
  }
  fout->cd("/Cathode/");
  firedStrips->Write();
  firedStripsADC->Write();
  chargeTBinProfile->Write();
  fout->Close();

  // Delete all histograms

  for (uint16_t i = 0; i < numLayers; i++) {
    wire[i]->Delete();
    h2dNofAhitWG[i]->Delete();
    anodeFiredTimeBins[i]->Delete();

    charges[i]->Delete();
    absADCVal[i]->Delete();
    avgPedestals[i]->Delete();
    fstPedestal[i]->Delete();
  }
  firedWireGroups->Delete();
  firedStrips->Delete();
  chargeTBinProfile->Delete();
  firedStripsADC->Delete();
}

/*
// ------------ method fills 'descriptions' with the allowed parameters for the
module  ------------ void
MiniCSC::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no
validation
  // Please change this to state exactly what you do use, even if it is no
parameters edm::ParameterSetDescription desc; desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //edm::ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks",
edm::InputTag("ctfWithMaterialTracks"));
  //descriptions.addWithDefaultLabel(desc);
}
*/
// define this as a plug-in
DEFINE_FWK_MODULE(MiniCSC);
