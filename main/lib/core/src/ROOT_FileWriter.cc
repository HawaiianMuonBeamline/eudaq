#ifdef ROOT_USE__

#include "eudaq/FileNamer.hh"
#include "eudaq/FileWriter.hh"
#include "eudaq/FileSerializer.hh"
#include <iostream>
#include "eudaq/StdEventConverter.hh"
#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TString.h"


class ROOT_FileWriter : public eudaq::FileWriter {
public:
  ROOT_FileWriter(const std::string &patt);
  void WriteEvent(eudaq::EventSPC ev) override;
  uint64_t FileBytes() const override;
  ~ROOT_FileWriter(){{
    m_tfile->Write();
    m_tfile->Close();
  }}
private:
  TFile * m_tfile =nullptr; // book the pointer to a file (to store the otuput)
  TTree * outTtreem_ttree; // book the tree (to store the needed event info)
  std::string m_filepattern;
  uint32_t m_run_n;
  
  std::vector<Double_t> m_id;
  std::vector<Double_t> m_x;
  std::vector<Double_t> m_charge;
  std::vector<Double_t> m_y;
  int m_event_nr = 0;

};

namespace{
  auto dummy0 = eudaq::Factory<eudaq::FileWriter>::
    Register<ROOT_FileWriter, std::string&>(eudaq::cstr2hash("TTree"));
  auto dummy1 = eudaq::Factory<eudaq::FileWriter>::
    Register<ROOT_FileWriter, std::string&&>(eudaq::cstr2hash("TTree"));
}


ROOT_FileWriter::ROOT_FileWriter(const std::string &patt){
  m_filepattern = patt;
}
  
void ROOT_FileWriter::WriteEvent(eudaq::EventSPC ev) {
  uint32_t run_n = ev->GetRunN();
  if(m_run_n != run_n){
    std::time_t time_now = std::time(nullptr);
    char time_buff[13];
    time_buff[12] = 0;
    std::strftime(time_buff, sizeof(time_buff),
		  "%y%m%d%H%M%S", std::localtime(&time_now));
    std::string time_str(time_buff);
    std::string f_name = eudaq::FileNamer(m_filepattern).
					   Set('X', ".root").
					   Set('R', run_n).
					   Set('D', time_str);


    std::cout << f_name << std::endl;

    m_tfile = new TFile(f_name.c_str(),"recreate");
    outTtreem_ttree = new TTree("t1","t1");
    outTtreem_ttree->Branch("event_nr", &m_event_nr);
    outTtreem_ttree->Branch("x", &m_x);
    outTtreem_ttree->Branch("y", &m_y);
    outTtreem_ttree->Branch("ID", &m_id);
    outTtreem_ttree->Branch("charge", &m_charge);
    m_run_n = run_n;
  }
  m_id.clear();
  m_x.clear();
  m_y.clear();
  m_charge.clear();

        auto evstd = eudaq::StandardEvent::MakeShared();
        eudaq::StdEventConverter::Convert(ev, evstd, nullptr);
        for (size_t i = 0 ; i < evstd->NumPlanes();++i){
          //std::cout << "plane i =  " << i <<std::endl;
          auto pl = evstd->GetPlane(i);
 //         std::cout << "size x " << pl.XVector().size() <<std::endl;
 //         std::cout << "size y " << pl.YVector().size() <<std::endl;
          
          for (size_t j=0; j <  pl.XVector().size() ; ++j){
            
            m_id.push_back(pl.ID());
            m_x.push_back(pl.XVector()[j]);
            m_y.push_back(pl.YVector()[j]);
            m_charge.push_back(pl.PixVector()[j]);

          }


        }
  outTtreem_ttree->Fill();
}
  
uint64_t ROOT_FileWriter::FileBytes() const {
  return 0;
}

#endif