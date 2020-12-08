

#include "eudaq/FileNamer.hh"
#include "eudaq/FileWriter.hh"
#include "eudaq/FileSerializer.hh"
#include "eudaq/StdEventConverter.hh"
#include <iostream>
#include <fstream>


class CSV_FileWriter : public eudaq::FileWriter {
public:
  CSV_FileWriter(const std::string &patt);
  void WriteEvent(eudaq::EventSPC ev) override;
  uint64_t FileBytes() const override;
  
private:
  std::shared_ptr<std::ofstream> out;
  
  std::string m_filepattern;
  uint32_t m_run_n;
  
  std::vector<double> m_id;
  std::vector<double> m_x;
  std::vector<double> m_charge;
  std::vector<double> m_y;
  std::vector<int64_t> m_ts;
  int m_event_nr = 0;

};

namespace{
  auto dummy0 = eudaq::Factory<eudaq::FileWriter>::
    Register<CSV_FileWriter, std::string&>(eudaq::cstr2hash("csv"));
  auto dummy1 = eudaq::Factory<eudaq::FileWriter>::
    Register<CSV_FileWriter, std::string&&>(eudaq::cstr2hash("csv"));
}


CSV_FileWriter::CSV_FileWriter(const std::string &patt){
  m_filepattern = patt;
}
  
void CSV_FileWriter::WriteEvent(eudaq::EventSPC ev) {
  uint32_t run_n = ev->GetRunN();
  if(m_run_n != run_n){
    std::time_t time_now = std::time(nullptr);
    char time_buff[13];
    time_buff[12] = 0;
    std::strftime(time_buff, sizeof(time_buff),
		  "%y%m%d%H%M%S", std::localtime(&time_now));
    std::string time_str(time_buff);
    std::string f_name = eudaq::FileNamer(m_filepattern).
					   Set('X', ".csv").
					   Set('R', run_n).
					   Set('D', time_str);


    std::cout << f_name << std::endl;

    out = std::make_shared<std::ofstream>(f_name);
    *out << "RunNumber;EventNumber;PlaneID;x;y;Charge;TimeStamp\n";
    m_run_n = run_n;

  }
  m_id.clear();
  m_x.clear();
  m_y.clear();
  m_charge.clear();
  m_ts.clear();

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
            m_ts.push_back(pl.GetTimestamp(j,0));

          }


        }

auto eventNR = evstd->GetEventN();
for (int i =0; i < m_id.size() ;++i ){
    *out <<m_run_n <<"; "<< eventNR << "; "<< m_id[i] << "; " << m_x[i] << "; "<< m_y[i] << "; "<< m_charge[i]<< ";  "<< m_ts[i] <<"\n";
}

}
  
uint64_t CSV_FileWriter::FileBytes() const {
  return 0;
}

