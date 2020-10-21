#include "eudaq/StdEventConverter.hh"
#include "eudaq/RawEvent.hh"
#include <iostream>
#include "eudaq/Utils.hh"

class HMB_RawEvent2StdEventConverter: public eudaq::StdEventConverter{
public:
  bool Converting(eudaq::EventSPC d1, eudaq::StdEventSP d2, eudaq::ConfigSPC conf) const override;
  static const uint32_t m_id_factory = eudaq::cstr2hash("HMB_Event");
};

namespace{
  auto dummy0 = eudaq::Factory<eudaq::StdEventConverter>::
    Register<HMB_RawEvent2StdEventConverter>(HMB_RawEvent2StdEventConverter::m_id_factory);
}

void safe_push_on_plane( eudaq::StandardPlane& pl, unsigned x,unsigned y,double charge, int64_t ts = 0){

  if (x >= pl.XSize()){
    x = pl.XSize();
  }

  if (y >= pl.YSize()){
    y = pl.YSize();
  }

  pl.PushPixel( x , y , charge, ts,false);

}


void push_data2plane(eudaq::StandardPlane& pl,  uint32_t y_raw ,  int32_t  data, int64_t ts = 0){
      if(data < 16 && data >0 ) {
      //  std::cout <<  "1 " <<std::endl;
        
        //std::cout << e << "; " << y << "; "<< data.size()<< std::endl;
        safe_push_on_plane(pl, data , y_raw , 1.0,ts);
       // std::cout << "2" <<std::endl;
      }else {
        if( data & 1 ){
          safe_push_on_plane(pl, 1 , y_raw , 1.0,ts);
          safe_push_on_plane(pl, 2 , y_raw , 1.0, ts);
          safe_push_on_plane(pl, 3 , y_raw , 1.0, ts);
          safe_push_on_plane(pl, 4 , y_raw , 1.0, ts);
        } 
       if(data & 2){
          safe_push_on_plane(pl,  5 , y_raw , 1.0,ts);
          safe_push_on_plane(pl,  6 , y_raw , 1.0,ts);
          safe_push_on_plane(pl,  7 , y_raw , 1.0,ts);
          safe_push_on_plane(pl,  8 , y_raw , 1.0,ts);
        } 
        if(data & 4){
          safe_push_on_plane(pl,  9 , y_raw , 1.0,ts);
          safe_push_on_plane(pl,  10 , y_raw , 1.0,ts);
          safe_push_on_plane(pl,  11 , y_raw , 1.0,ts);
          safe_push_on_plane(pl,  12 , y_raw , 1.0,ts);
        } 
        if(data & 8){
          safe_push_on_plane(pl,  13 , y_raw , 1.0,ts);
          safe_push_on_plane(pl,  14 , y_raw , 1.0,ts);
          safe_push_on_plane(pl,  15 , y_raw , 1.0,ts);
          safe_push_on_plane(pl,  16 , y_raw , 1.0,ts);
        }
      }

}
void push_data2plane(eudaq::StandardPlane& pl,   std::vector<int32_t>& data){
      uint32_t y_raw = 0;
     for (const auto& e : data){
      ++y_raw;
      //std::cout << e<< "; ";
      push_data2plane(pl, y_raw, e);
    }
}

bool convert_string(eudaq::EventSPC d1, eudaq::StdEventSP d2, eudaq::ConfigSPC conf){
  auto ev = std::dynamic_pointer_cast<const eudaq::RawEvent>(d1);
  size_t nblocks= ev->NumBlocks();
  auto block_n_list = ev->GetBlockNumList();
  //std::cout << "sub_name; nblocks = " << nblocks<<  std::endl;
  if( ev->NumBlocks() == 1){
    return true;
  }

  auto b = ev->GetBlock(0);
  std::string block (b.begin() ,b.end() );
  //std::cout << block << std::endl;

  eudaq::StandardPlane plane(0, "HMB", "HMB");
  eudaq::StandardPlane plane_raw(1, "HMB", "HMB");
  plane.SetSizeZS(20, 12, 0);
  plane_raw.SetSizeZS(20, 12, 0);
  
  auto lines = eudaq::split(block,"\n",true);
  //std::cout << lines[0] << std::endl;
  std::vector<int32_t> data;
  std::vector<int32_t> data_raw;
  int counter = 0;
  for (const auto& e : lines){
    if (++counter < 3){
      continue;
    }
    data.clear();
    data_raw.clear();
    //std::cout << e << std::endl;
    auto colums = eudaq::split(e,";",true);
    if (colums.size() < 30 ){
    //  std::cout << "to short\n";
      continue;
    }
    //std::cout << colums[11]<< "; "<<  colums[12] << "; " <<  colums[13] << std::endl;
    data_raw.push_back(std::stoll(colums[11]));
    data_raw.push_back(std::stoll(colums[12]));
    data_raw.push_back(std::stoll(colums[13]));
    data_raw.push_back(std::stoll(colums[14]));
    data_raw.push_back(std::stoll(colums[15]));
    data_raw.push_back(std::stoll(colums[16]));
    data_raw.push_back(std::stoll(colums[17]));
    data_raw.push_back(std::stoll(colums[18]));
    data_raw.push_back(std::stoll(colums[19]));

    push_data2plane(plane_raw ,data_raw);
    


    data.push_back(std::stoll(colums[20]));
    data.push_back(std::stoll(colums[21]));
    data.push_back(std::stoll(colums[22]));
    data.push_back(std::stoll(colums[23]));
    data.push_back(std::stoll(colums[24]));
    data.push_back(std::stoll(colums[25]));
    data.push_back(std::stoll(colums[26]));
    data.push_back(std::stoll(colums[27]));
    data.push_back(std::stoll(colums[28]));

    push_data2plane(plane ,data);

    
    

  }

  d2->AddPlane(plane);
  d2->AddPlane(plane_raw);

  

}

std::map<int,std::vector<int16_t>> g_old_data;


void push2plane(const  std::vector<int16_t>& block, eudaq::StandardPlane& plane_raw, eudaq::StandardPlane& plane ){
    int64_t ts_high = 0;
    int64_t ts_low = 0;
    int64_t ts_full = 0;
    for (int i =0 ; i  < block.size() ; ++i){
      if (block[i] == 7000){
        ts_high = *( reinterpret_cast<const int64_t*> ( block.data() + i + 1));
      }
      if (block[i] == 7001){
        ts_low = *( reinterpret_cast<const int64_t*> ( block.data() + i + 1));
      }
      ts_full = (ts_high<<32) + ts_low;
    //std::cout << block[i] << "; ";

    if (5000  <=  block[i] && block[i] < 5099  ){
      //data_raw.push_back(block[i+1]);
      
      push_data2plane(plane_raw, block[i] -5000 +1, block[i+1], ts_full);
  //    std::cout << block[i] << "; " << block[i+1] << "\n";
    }
  
    if (6000  <=  block[i] && block[i] < 6099  ){
      //data_raw.push_back(block[i+1]);
      
      push_data2plane(plane, block[i] -6000 +1, block[i+1], ts_full);
      //std::cout << block[i] << "; " << block[i+1] << "\n";
    }
  

  }
}
bool Convert_bit(eudaq::EventSPC d1, eudaq::StdEventSP d2, eudaq::ConfigSPC conf){
  auto ev = std::dynamic_pointer_cast<const eudaq::RawEvent>(d1);
  size_t nblocks= ev->NumBlocks();
  auto block_n_list = ev->GetBlockNumList();
  //std::cout << "sub_name; nblocks = " << nblocks<<  std::endl;
  auto b = ev->GetBlock(1);

  int16_t* block = reinterpret_cast<int16_t*>( b.data());
  auto currentData  = std::vector<int16_t>(block, block + b.size()/2 );
  auto producerID = block[1];

  eudaq::StandardPlane plane(2+10*producerID, "HMB", "HMB");
  eudaq::StandardPlane plane_raw(3+10*producerID, "HMB", "HMB");
  plane.SetSizeZS(20, 12, 0);
  plane_raw.SetSizeZS(20, 12, 0);


  


  push2plane(currentData, plane_raw, plane);
  push2plane(g_old_data[producerID], plane_raw, plane);



  d2->AddPlane(plane);
  d2->AddPlane(plane_raw);
 
  g_old_data[producerID] = currentData;

  return true;

}
bool HMB_RawEvent2StdEventConverter::Converting(eudaq::EventSPC d1, eudaq::StdEventSP d2, eudaq::ConfigSPC conf) const{
  

    convert_string(d1,d2, conf);

    Convert_bit(d1,d2, conf);

  return true;
}
