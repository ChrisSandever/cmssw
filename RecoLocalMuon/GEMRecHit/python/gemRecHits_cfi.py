import FWCore.ParameterSet.Config as cms

# masking is off by default - turn on with 'applyMasking'
# default masking uses DB, to use txt file, set maskFile or deadFile to the relative path of file
 
from RecoLocalMuon.GEMRecHit.gemRecHitsDef_cfi import *
gemRecHits = gemRecHitsDef.clone(
    #applyMasking = False,
    #maskFile = cms.FileInPath("RecoLocalMuon/GEMRecHit/data/maskedStrips.txt"),
    #deadFile = cms.FileInPath("RecoLocalMuon/GEMRecHit/data/deadStrips.txt")
    )

from Configuration.Eras.Modifier_phase2_GEM_cff import phase2_GEM
phase2_GEM.toModify(gemRecHits, gemDigiLabel = "simMuonGEMDigis")
