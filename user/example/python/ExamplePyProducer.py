#! /usr/bin/env python
# load binary lib/pyeudaq.so
import sys


import pyeudaq
import time
import os
from distutils.util import strtobool
from inspect import currentframe, getframeinfo

eudaq_home_folder               =  "/home/belle2/Documents/github/eudaq/tmp"

udp_run                         =  eudaq_home_folder + "/udp_run.py"
configCSV                       =  eudaq_home_folder + "/roling_register_tb.csv"
configCSV_HWout                 =  eudaq_home_folder + "/roling_register_tb_out_HW.csv"
roling_register_tb_csv_header   =  eudaq_home_folder + "/roling_register_tb_csv_header.txt"
tx_triggerbitmonitor_top_out_HW =  eudaq_home_folder + "/tx_triggerbitsz_top_out_HW.csv"
tx_triggerbitmonitor_top        =  eudaq_home_folder + "/tx_triggerbitsz_top.csv"
tx_triggerbitsz_top_header      =  eudaq_home_folder + "/tx_triggerbitsz_top_header.txt"

def get_content(fileName):
    with open(fileName) as f:
        return f.read()

def set_content(fileName, content):
    with open(fileName, "w") as f:
        return f.write(content)


class fileSynchronizer:
    def __init__(self, fileName, Position, isLast):
        super().__init__()
        self.fileName = fileName
        self.position = Position
        self.isLast = isLast
        set_content(self.fileName,"0")


    def isReady(self):
        if int(get_content(self.fileName)) != self.position:
            return False
        return True
        
    def SetToDone(self):
        if self.isLast:
            set_content(self.fileName, str(0))
        else:
            set_content(self.fileName, str(self.position + 1))
        

        

        
class registerEntry:
    def __init__(self,addr, value, Offset = 0):
        self.addr = addr
        self.value = value 
        self.offset = Offset
            
        

    def __str__(self):
        if self.value is None:
            return ""
        print(self.value, self.offset)
        print(type(self.value).__name__)
        print(type(self.offset).__name__)
        val = int(self.value)+ self.offset
        print(val)
        val = val if val >0 else 0
        
        return "1,0,"+str(self.addr)+","+str(val)+",0\n"
        
def saveConfigFile(fileName, conf):

    with open(fileName, "w") as f:
        f_content = "Sregisterin_m2s_valid,registerin_m2s_last,registerin_m2s_data_address,registerin_m2s_data_value,registerin_m2s_data_clk\n"
        f_content+= "Sregisterin_m2s_valid,registerin_m2s_last,registerin_m2s_data_address,registerin_m2s_data_value,registerin_m2s_data_clk\n"
        f_content += "0,0,0,0,0\n"
        f_content += "0,0,0,0,0\n"
        f_content += "0,0,0,0,0\n"
        for x in conf:
            print("-------")
            print(x)
            print("-------")
            f_content += str(x)

        f_content += "1,1,0,0,0\n"
       
        for _ in range(1003-len(f_content.split("\n"))):
            f_content += "0,0,0,0,0\n"

        
        print(f_content)

        f.write(f_content)
        

def load_data(fileName):
    with open(fileName) as f:
        return f.read()


class TargetXRegisters:

    def __init__(self, ASIC_number, Config):
        super().__init__()
        self.ASIC_number = ASIC_number
        self.conf = []
        prefix = "ASIC_" + str(ASIC_number).zfill(2)

        if ASIC_number == 0:
            prefix = "all"
        
        for i in range(80):
            k =  prefix+ ".register." + str(i) 
        #    print(k)
            self.conf.append(registerEntry( (ASIC_number <<7) + i, Config.GetConfigItemOrDefault(k, None)))

        Offset = int(Config.GetConfigItemOrDefault( "TriggerBitsThreshold" ,"0"))
        #print(Offset)
        for i in range(17):
            k =  prefix+ ".Channel_" + str(i).zfill(2) 
        #    print(k)
            self.conf.append(registerEntry( (ASIC_number <<7) + i*2, Config.GetConfigItemOrDefault(k, None) ,Offset ))



    def __str__(self):
        ret = ""
        for x in self.conf:
            ret += str(x)
        return ret


class SCROD_registers:
    def __init__(self,Config):
        super().__init__()
        self.conf = []
        self.conf.append(registerEntry( 4180 , Config.GetConfigItemOrDefault( "TriggerBitMask", 4)) )
        self.conf.append(registerEntry( 4181 , Config.GetConfigItemOrDefault( "TriigerBitSwitch", 1)) )
        self.conf.append(registerEntry( 4182 , Config.GetConfigItemOrDefault( "trigger_maxCount", 400)) )

    def __str__(self):
        ret = ""
        for x in self.conf:
            ret += str(x)
        return ret



class DATA_sender:
    def __init__(self,IP,Port,path,header):
        super().__init__()
        self.IP =  IP
        self.Port = Port
        self.path = path
        self.header = header
        

    def send_data(self, inputFile,OutputFile):
        print("udp_run", udp_run)
        print("inputFile", inputFile)
        print("OutputFile", OutputFile)
        print("self.IP",  self.IP)
        print("self.Port", self.Port)
        print("self.header", self.header)

        command = udp_run                  +\
            ' --InputFile ' + inputFile     +\
            ' --OutputFile  '+ OutputFile   +\
            " --IpAddress " + self.IP       +\
            ' --port ' + self.Port          +\
            ' --OutputHeader ' +self.header \
        
        print(command)
        os.system( command)

    def __call__(self, inputFile,OutputFile):
        self.send_data(inputFile,OutputFile)


class ExamplePyProducer(pyeudaq.Producer):
    def __init__(self, name, runctrl):
        pyeudaq.Producer.__init__(self, 'PyProducer', name, runctrl)
        self.is_running = 0
        self.config_sender = DATA_sender(IP="192.168.1.20", Port="2000", path=eudaq_home_folder , header=roling_register_tb_csv_header)
        self.data_sender   = DATA_sender(IP="192.168.1.33", Port="2001", path=eudaq_home_folder , header= "tx_triggerbitmonitor_top_header.txt")
        self.Sync = None
        


        self.Send_RAW_Dump = False
        print ('New instance of ExamplePyProducer')

    def GetConfigItemOrDefault(self, keyName,Default):
        ret =  self.GetConfigItem(keyName)
        if ret:
            return ret 
        return Default

    def DoInitialise(self):        
        print ('DoInitialise')
        configFile  = self.GetConfigItemOrDefault("configFile"  ,"roling_register_tb_csv.csv")
        print("Initilizing with file: ",configFile )
        #os.system( udp_run + ' --InputFile ' + configFile + ' --OutputFile roling_register_tb_csv_out_HW.csv --IpAddress  192.168.1.20 --port 2000 --OutputHeader roling_register_tb_csv_header.txt')

    def DoConfigure(self): 
        

            
        print ('DoConfigure')
        fileName = self.GetConfigItemOrDefault( "fileSynchronizer.fileName",  eudaq_home_folder + "/sync.txt") 
        Position = int(self.GetConfigItemOrDefault( "fileSynchronizer.Position",  "0" ) )
        isLast = int(self.GetConfigItemOrDefault( "fileSynchronizer.isLast",  "1" ) )
        print(fileName, Position,isLast)

        self.Sync = fileSynchronizer(fileName,Position,isLast)

        self.config_sender.IP      =  self.GetConfigItemOrDefault( "config_sender.IP",     "192.168.1.20") 
        self.config_sender.Port    =  self.GetConfigItemOrDefault( "config_sender.Port",   "2000") 
        self.config_sender.path    =  self.GetConfigItemOrDefault( "config_sender.path",   eudaq_home_folder) 
        self.config_sender.header  =  self.GetConfigItemOrDefault( "config_sender.header", eudaq_home_folder + "/roling_register_tb_csv_header.txt") 
        


        self.data_sender.IP      =  self.GetConfigItemOrDefault( "data_sender.IP",     "192.168.1.33") 
        self.data_sender.Port    =  self.GetConfigItemOrDefault( "data_sender.Port",   "2001") 
        self.data_sender.path    =  self.GetConfigItemOrDefault( "data_sender.path",   eudaq_home_folder ) 
        self.data_sender.header  =  self.GetConfigItemOrDefault( "data_sender.header", tx_triggerbitsz_top_header) 



        self.Send_RAW_Dump       =  strtobool(self.GetConfigItemOrDefault( "Send_RAW_Dump", "False"))
        
        allAsics = []
        for i in range(11):
            allAsics.append(TargetXRegisters(i, self))
        
        allAsics.append(SCROD_registers(self))
#        for x in allAsics:
#            print(str(x))
        while not self.Sync.isReady():
            time.sleep(1)
            

        saveConfigFile(configCSV, allAsics)
        self.config_sender(configCSV,configCSV_HWout )
        frameinfo = getframeinfo(currentframe())
        print(fileName, Position,isLast)
        self.Sync.SetToDone()
         

        #os.system( udp_run + ' --InputFile ' + configCSV +' --OutputFile roling_register_tb_csv_out_HW.csv --IpAddress  192.168.1.20 --port 2000 --OutputHeader '+roling_register_tb_csv_header )
        

    def DoStartRun(self):
        print ('DoStartRun')
        self.is_running = 1
        
    def DoStopRun(self):        
        print ('DoStopRun')
        self.is_running = 0

    def DoReset(self):        
        print ('DoReset')
        self.is_running = 0

    def RunLoop(self):
        try:
            frameinfo = getframeinfo(currentframe())
            print ("Start 123")
             
            print ("Start of RunLoop in ExamplePyProducer")
            print ("Start 222")
             
            print ("Start 333")
            trigger_n = 0;
             
            while(self.is_running):
 
                 
                if not self.Sync.isReady():
                    time.sleep(1)
                    continue
                print ("Start 555")
                 
                self.data_sender(tx_triggerbitmonitor_top, tx_triggerbitmonitor_top_out_HW)

                #os.system( udp_run + ' --InputFile tx_triggerbitmonitor_top.csv --OutputFile tx_triggerbitmonitor_top_out_HW.csv --IpAddress  192.168.1.33 --port 2001 --OutputHeader tx_triggerbitmonitor_top_header.txt')
                data = load_data(tx_triggerbitmonitor_top_out_HW)


                 
                #print(data)
                ev = pyeudaq.Event("RawEvent", "sub_name")
                ev.SetTriggerN(trigger_n)
                #block = bytes(r'raw_data_string')
                #ev.AddBlock(0, block)
                #print ev
                # Mengqing:
                #datastr = 'raw_data_string'
                 
                if self.Send_RAW_Dump:
                    block = bytes(data, 'utf-8')
                    ev.AddBlock(0, block)

                print("start convering")
                b_rows = data.split("\n")
         
                block2 = (1234).to_bytes(2, byteorder='little')
                block2 += (self.Sync.position).to_bytes(2, byteorder='little')
 
                row_index = 0
                 
                for row in b_rows[2:]:
             
                    row_index += 1
                    block2 += (4321).to_bytes(2, byteorder='little')
                    block2 += (row_index).to_bytes(2, byteorder='little')
       

                    colloumns = row.split(';')
                    if len(colloumns)< 28:
                        break
                    print(666)
                    block2 += (7000).to_bytes(2, byteorder='little')
                    print(667)
                    data_int = int(colloumns[10])
                    print(668)
                    block2 += (data_int).to_bytes(8, byteorder='little')
                    print(669)
                    block2 += (7001).to_bytes(2, byteorder='little')
                    print(670)
                    data_int = int(colloumns[11])
                    print(671)
                    block2 += (data_int).to_bytes(8, byteorder='little')
                    print(672)
                    for i in range(12,20):
                        #raw
                        print(673, colloumns[i])
                        data_int = int(colloumns[i])
                        if data_int > 0:
                            block2 += int(5000 +i  - 11 ).to_bytes(2, byteorder='little')
                            block2 += int(data_int).to_bytes(2, byteorder='little')
                    print(777)

                    for i in range(20,29):
                        #edge Detected
                        
                        data_int = int(colloumns[i])
                        if data_int > 0:
                            block2 += int(6000 +i - 20 ).to_bytes(2, byteorder='little')
                            block2 += int(data_int).to_bytes(2, byteorder='little')
                    print(888)


                 
#                print(block2)
                print ("Start 1060")
                block2 += (4444).to_bytes(2, byteorder='little')
                ev.AddBlock(1, block2)
                print ("Start 1070")
                #print(ev)
                
                self.SendEvent(ev)
                trigger_n += 1
                self.Sync.SetToDone()
                #time.sleep(1)
            print ("End of RunLoop in ExamplePyProducer")

        except e:
            print ("Start 1080")
            print(e)
        except:
            print ("Start 1090")

import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='KLM producer')
    parser.add_argument('--name', help='Name of the KLM producer',default="klmproducer1")
    args = parser.parse_args()

    myproducer = ExamplePyProducer(args.name, "tcp://localhost:44000")
    print ("connecting to runcontrol in localhost:44000", )
    myproducer.Connect()
    time.sleep(2)
    while(myproducer.IsConnected()):
        time.sleep(1)
