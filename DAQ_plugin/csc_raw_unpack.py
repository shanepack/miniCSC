# based on CMSSW_12_0_0_pre2/src/IORawData/CSCCommissioning/test/readFile_b904_Run3.py
import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('analysis')
options.parseArguments()
process = cms.Process("reader")

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cout.threshold = cms.untracked.string('DEBUG')
process.MessageLogger.debugModules = cms.untracked.vstring('*')
process.MessageLogger.cerr.FwkReport.reportEvery = 10000

process.options = cms.untracked.PSet( wantSummary = cms.untracked.bool(True) )


#first run is needed only for running L1Emulator (see p.2 of
#https://indico.cern.ch/event/1046913/contributions/4398858/attachments/2261412/3838507/CSCDPG_SD_20210609.pdf)
#options.register ("firstRun", 341761, VarParsing.multiplicity.singleton, VarParsing.varType.int)

#options.maxEvents = 50000
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.maxEvents) )

process.source = cms.Source("EmptySource",
       #firstRun= cms.untracked.uint32(options.firstRun),
       firstRun= cms.untracked.uint32(1),
       numberEventsInLuminosityBlock = cms.untracked.uint32(2000),
       numberEventsInRun       = cms.untracked.uint32(0)
	  # fileNames = cms.untracked.vstring(options.inputFiles)
 )

# GIF++: FED838 vme1 dmb2 corresponds  to E:1 S:1 R:1 C:1
process.rawDataCollector = cms.EDProducer('CSCFileReader',
    firstEvent  = cms.untracked.int32(0),
    FED838 = cms.untracked.vstring('RUI01'), #ME1/1
	#FED839 = cms.untracked.vstring('RUI01'), #ME2/1
	FED839 = cms.untracked.vstring('RUI01'), #test
   # RUI01  = cms.untracked.vstring(fileNames)
	#RUI01  = cms.untracked.vstring('csc_00000001_EmuRUI01_STEP_40_000_210909_172524_UTC.raw'),
	#RUI07  = cms.untracked.vstring('csc_00000001_EmuRUI01_STEP_40_000_210721_022758_UTC.raw')
	RUI01  = cms.untracked.vstring(options.inputFiles)
)

process.FEVT = cms.OutputModule("PoolOutputModule",
        #fileName = cms.untracked.string("test_21_500.root"),
		fileName = cms.untracked.string(options.outputFile),
        outputCommands = cms.untracked.vstring("keep *")
)

process.p = cms.Path( process.rawDataCollector)

process.outpath = cms.EndPath(process.FEVT)

