/*******************************************************************************
 * Copyright (c) The JETSCAPE Collaboration, 2018
 *
 * Modular, task-based framework for simulating all aspects of heavy-ion collisions
 * 
 * For the list of contributors see AUTHORS.
 *
 * Report issues at https://github.com/JETSCAPE/JETSCAPE/issues
 *
 * or via email to bugs.jetscape@gmail.com
 *
 * Distributed under the GNU General Public License 3.0 (GPLv3 or later).
 * See COPYING for details.
 ******************************************************************************/

#include <iostream>
#include <fstream>
#include <memory>
#include <chrono>
#include <thread>

#include "gzstream.h"
#include "PartonShower.h"
#include "JetScapeLogger.h"
#include "JetScapePartonReader.h"
#include "JetScapeBanner.h"
#include "fjcore.hh"

#include <GTL/dfs.h>

using namespace std;
using namespace fjcore;

using namespace Jetscape;

// You could overload here and then simply use ofstream << p;
// ostream & operator<<(ostream & ostr, const fjcore::PseudoJet & jet);


// -------------------------------------

int main(int argc, char** argv)
{
  
  auto reader=make_shared<JetScapeReaderAscii>("../Result/AA_Eloss/AA_martini_parton.dat");
  std::ofstream jet_output ("../Result/AA_Eloss/partonJet_AA_martini.txt"); 
  
  const int nEvents = 10000; 
  double jetpTMin = 10., jetRadius = 0.4, jetrapMax = 2.8; 
  fjcore::JetDefinition jetDef(fjcore::antikt_algorithm, jetRadius); 
  vector <fjcore::PseudoJet> fjInputs; 
  fjcore::Selector select_rapidity = fjcore::SelectorRapMax(jetrapMax); 
  
  vector <double> pTBin{10., 30., 50., 80., 110., 160., 210., 260., 310., 400., 500., 600., 800.}; 
  vector <double> jet_cs(pTBin.size()-1, 0.), err(pTBin.size()-1, 0.); 
  vector <int> sqSum(pTBin.size()-1, 0); 
  
  while (!reader->Finished())
    {
      reader->Next();
      vector <int> jet_ct(pTBin.size()-1, 0); 
      cout << "The number of final partons "<< reader->GetPartonsForFastJet().size() << endl; 

      vector <fjcore::PseudoJet> inclusiveJets, sortedJets; 
      fjcore::ClusterSequence clustSeq(reader->GetPartonsForFastJet(), jetDef);
      inclusiveJets = clustSeq.inclusive_jets(jetpTMin); 
      vector <fjcore::PseudoJet> selected_jets = select_rapidity(inclusiveJets); 
      sortedJets    = fjcore::sorted_by_pt(selected_jets); 
      
      for (unsigned int i = 0; i < sortedJets.size(); i++)
      	for (unsigned int j = 0; j < pTBin.size() - 1; j++)
      		if (sortedJets[i].perp()>pTBin[j] && sortedJets[i].perp()<pTBin[j+1])
		{
			jet_ct[j]++; 
			break; 
		}
      for (unsigned int j = 0; j < pTBin.size() - 1; j++)
        {
  	    jet_cs[j] += jet_ct[j]; 
  	    sqSum[j] += pow(jet_ct[j], 2);
        }
     }
	
  cout<<"Finished!"<<endl; 
  double sigmagen = 11.8282; 
  double sigmainel = 62.3; 
  int Nbin = 1; 
  double sigmapb_weight = sigmagen * 1.0e9 * Nbin / sigmainel / nEvents; 
  for (unsigned int j=0; j<pTBin.size()-1; j++)
  {
	jet_cs[j] *= sigmapb_weight; 
	err[j] = sqrt((sqSum[j]*pow(sigmapb_weight, 2)*nEvents - pow(jet_cs[j], 2)) / nEvents); 
	jet_output << (pTBin[j] + pTBin[j+1]) / 2 << " " << jet_cs[j] / (pTBin[j+1] - pTBin[j]) << " " << err[j] / (pTBin[j+1] - pTBin[j])  << endl; 
  }
  reader->Close(); 
}
