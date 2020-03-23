//######################################################
//
// Calculate efficiency of the track reconstruciton
// for the DeadCells module
//
// P.Franchini 2020
//
//######################################################

#include "TCanvas.h"
#include "TFile.h"
#include "TH1F.h"
#include "TStyle.h"
#include "TTree.h"
#include <iostream>

static void show_usage(std::string name)
{
  std::cerr << "Usage: ./efficiency <option(s)>\n"
            << "Options:\n"
            << "\t-i,--input\t\tInput root file\n"
             << std::endl;
}

int main(int argc, char* argv[]){

  std::string input;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "-h") || (arg == "--help")) {
      show_usage(argv[0]);
      return 0;
    }
    if ((arg == "-i") || (arg == "--input"))
      input = argv[i+1];
  }

  // Open the output of the Sensitivity monitor
  TFile *f1 = new TFile(input.c_str());
  TTree *t1 = (TTree*)f1->Get("Sensitivity");

  TCanvas *c1 = new TCanvas("c1","Efficiency",900,600);
  TH1F *efficiency   = new TH1F("efficiency","efficiency", 10, 0, 10);

  Int_t reco_track_count, reco_vertices_in_tracker, reco_calorimeter_hit_count, reco_foil_vertex_count;

  t1->SetBranchAddress("reco.track_count",&reco_track_count);
  t1->SetBranchAddress("reco.vertices_in_tracker",&reco_vertices_in_tracker);
  t1->SetBranchAddress("reco.foil_vertex_count",&reco_foil_vertex_count);
  t1->SetBranchAddress("reco.calorimeter_hit_count",&reco_calorimeter_hit_count);
  
  Long64_t nentries = t1->GetEntries();
  for (Long64_t i=0;i<nentries;i++) {
    t1->GetEntry(i);

    /*    std::cout << i << std::endl;
    std::cout << "Tracks:  " << reco_track_count << std::endl;
    std::cout << "Tracker: " << reco_vertices_in_tracker << std::endl;
    std::cout << "Foil:    " << reco_foil_vertex_count << std::endl;
    std::cout << "Calo:    " << reco_calorimeter_hit_count << std::endl;
    */

    efficiency->Fill(reco_track_count);

  }

  Float_t eff = efficiency->GetBinContent(3)/t1->GetEntries();
  Float_t zero = efficiency->GetBinContent(1)/t1->GetEntries();
  Float_t single = efficiency->GetBinContent(2)/t1->GetEntries();

  std::cout << "Efficiency:           " << eff*100 << " %"<< std::endl;
  std::cout << "Zero_tracks:          " << zero*100 << " %"<< std::endl;
  std::cout << "Single_track:         " << single*100 << " %"<< std::endl;
  std::cout << "More_than_two_tracks: " << 100-(eff+zero+single)*100 << " %"<< std::endl;

  c1->cd();
  /*efficiency->Draw();
  c1->SaveAs("efficiency.png");
  system("display efficiency.png");
  */

  return 0;
}
