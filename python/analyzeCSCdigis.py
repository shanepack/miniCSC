import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.Eras import eras

# command line arguments
from FWCore.ParameterSet.VarParsing import VarParsing

# from Configuration.AlCa.autoCond import autoCond
from Configuration.AlCa.GlobalTag import GlobalTag as gtCustomise

options = VarParsing("analysis")
options.maxEvents = -1
options.inputFiles = "file:out_numEvent5000.root"
options.register(
    "debug", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool
)
options.parseArguments()
# end command line arguments

process = cms.Process("TEST", eras.Run3)
process.load("Configuration.StandardSequences.GeometryDB_cff")
process.load("CondCore.CondDB.CondDB_cfi")
process.load("Configuration/StandardSequences/RawToDigi_Data_cff")
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

process.GlobalTag = gtCustomise(process.GlobalTag, "auto:run2_data", "")

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))

process.MessageLogger = cms.Service(
    "MessageLogger",
    cerr=cms.untracked.PSet(enable=cms.untracked.bool(False)),
    cout=cms.untracked.PSet(
        WARNING=cms.untracked.PSet(limit=cms.untracked.int32(0)),
        enable=cms.untracked.bool(True),
        threshold=cms.untracked.string("WARNING"),
    ),
    debugModules=cms.untracked.vstring("*"),
)


process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(options.inputFiles),
    # fileNames = cms.untracked.vstring('file:testRF838_27.root')
    # rootFileName = cms.untracked.string(options.outputFile),
)

process.FEVT = cms.OutputModule(
    "PoolOutputModule",
    fileName=cms.untracked.string("testD_27.root"),
    outputCommands=cms.untracked.vstring("keep *"),
)


process.muonCSCDigis.UnpackStatusDigis = True
process.muonCSCDigis.PrintEventNumber = True
process.muonCSCDigis.FormatedEventDump = False

process.muonCSCDigis.UseExaminer = True  # should be true!
process.muonCSCDigis.UseSelectiveUnpacking = True  # should be true!

process.muonCSCDigis.Debug = options.debug


process.test904 = cms.EDAnalyzer(
    "MiniCSC",
    # Data tags
    stripDigiTag=cms.InputTag("muonCSCDigis", "MuonCSCStripDigi"),
    wireDigiTag=cms.InputTag("muonCSCDigis", "MuonCSCWireDigi"),
    clctDigiTag=cms.InputTag("muonCSCDigis", "MuonCSCCLCTDigi"),
    # Configuration tags
    rootFileName=cms.untracked.string("output.root"),
    # Hardcoded width of charge spectra in strips.
    # When plotting change spectra it will use x number of strips in it's analysis.
    stripWidthCharges=cms.uint32(5),
    # Controls threshold needed for valid strip signal.
    # If any timebin - pedestal > threshold then we consider it a valid signal.
    # Real CSCs use 13, experimentation is allowed.
    adcThreshold=cms.uint32(32),
)

# process.p = cms.Path( process.muonCSCDigis * process.csc2DRecHits * process.gif)
process.p = cms.Path(process.muonCSCDigis * process.test904)

process.outpath = cms.EndPath(process.FEVT)
