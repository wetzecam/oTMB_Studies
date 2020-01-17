/*
	This file is for version control:
	I am currently unsure how to recreate this GUI in a new directory
		~Cameorn	(06/24/2019)


*/
/*
 * $Id: $
 */

// class header
#include "emu/pc/CSCGEMTestApplication.h"

// Emu includes
#include "emu/utils/Cgi.h"
#include "emu/utils/System.h"
#include "emu/pc/Crate.h"
#include "emu/pc/DAQMB.h"
#include "emu/pc/TMB.h"
#include "emu/pc/CCB.h"
#include "emu/pc/MPC.h"
#include "emu/pc/ALCTController.h"
#include "emu/pc/RAT.h"
#include "emu/pc/VMECC.h"
#include "emu/pc/DDU.h"



// XDAQ includes
#include "cgicc/Cgicc.h"
#include "cgicc/HTMLClasses.h"
#include "toolbox/string.h"

// system includes
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <string>
#include <unistd.h> // for sleep()

// radtest includes
#include "eth_lib.h"
#include "utils.h"
#include "commands.h"

// testing includes
#include <stdio.h>
#include <stdlib.h>

//debugging includes: optional
#include <iostream>

//#include "PatternGen.h"
//#include "emu/pc/Yuriy_CLCT_PatternGen.h"


//parsing
#include "emu/pc/TMBTester.h"

namespace emu {  namespace pc {

using std::endl;
using std::cout;
using std::string;
using emu::utils::bindMemberMethod;
using cw::CLCT;
using cw::Cluster;


//This is the section for the default values displayed on the webpage:

CSCGEMTestApplication::CSCGEMTestApplication(xdaq::ApplicationStub * s)
: EmuPeripheralCrateBase(s)     //: xdaq::WebApplication(s)

{
  //------------------------------------------------------
  // bind methods
  //------------------------------------------------------
  xgi::bind(this, &CSCGEMTestApplication::Default, "Default");
  xgi::bind(this, &CSCGEMTestApplication::GenerateDCFEBData, "GenerateDCFEBData");
  xgi::bind(this, &CSCGEMTestApplication::UploadDCFEBData, "UploadDCFEBData");
  xgi::bind(this, &CSCGEMTestApplication::CheckDCFEBData, "CheckDCFEBData");
  xgi::bind(this, &CSCGEMTestApplication::TransmitDCFEBData, "TransmitDCFEBData");
  xgi::bind(this, &CSCGEMTestApplication::ReadOutTriggerResults, "ReadOutTriggerResults");
  xgi::bind(this, &CSCGEMTestApplication::F0, "F0");
  xgi::bind(this, &CSCGEMTestApplication::GenGEMPattxtFileMounN, "GenGEMPattxtFileMounN");
  xgi::bind(this, &CSCGEMTestApplication::GenGEMPattxtFile, "GenGEMPattxtFile");
  xgi::bind(this, &CSCGEMTestApplication::YuriyPatConvert, "YuriyPatConvert");
  xgi::bind(this, &CSCGEMTestApplication::Yuriy_CLCT_PatternConverter, "Yuriy_CLCT_PatternConverter");
  xgi::bind(this, &CSCGEMTestApplication::GemPatternConverter, "GemPatternConverter");
  xgi::bind(this, &CSCGEMTestApplication::CfebPatternConverter, "CfebPatternConverter");
  xgi::bind(this, &CSCGEMTestApplication::SendFromEmuBoard, "SendFromEmuBoard");
  xgi::bind(this, &CSCGEMTestApplication::ClearEmuBoard, "ClearEmuBoard");
  xgi::bind(this, &CSCGEMTestApplication::OutputCLCTInfo, "OutputCLCTInfo");


  // Pattern Studies Methods
  xgi::bind(this, &CSCGEMTestApplication::AddCLCT, "AddCLCT");
  xgi::bind(this, &CSCGEMTestApplication::AddGEM, "AddGEM");
  xgi::bind(this, &CSCGEMTestApplication::LoadToEmuBoard, "LoadToEmuBoard");
  xgi::bind(this, &CSCGEMTestApplication::SaveCurrentSet, "SaveCurrentSet");	// Outdated! to be REMOVED
  xgi::bind(this, &CSCGEMTestApplication::SaveAsPat, "SaveAsPat");
  xgi::bind(this, &CSCGEMTestApplication::SaveAsTxt, "SaveAsTxt");
  xgi::bind(this, &CSCGEMTestApplication::ClearSet, "ClearSet");
  xgi::bind(this, &CSCGEMTestApplication::RunStudy, "RunStudy");
  xgi::bind(this, &CSCGEMTestApplication::AddCLCTParamScan, "AddCLCTParamScan");
  xgi::bind(this, &CSCGEMTestApplication::RunParamScan, "RunParamScan");


  //parsing section
  XML_or_DB_ = "xml";
  xmlFile_ = "config.xml" ;
  Valid_config_ID="";
  xml_or_db = -1;  /* actual configuration source: 0: xml, 1: db, -1: unknown or error */
  total_crates_=0;
  this_crate_no_=0;
  this->getApplicationInfoSpace()->fireItemAvailable("Standalone", &standalone);
  standalone_ = true;
  thisCCB = 0;
  thisMPC = 0;
  this->getApplicationInfoSpace()->fireItemAvailable("xmlFileName", &xmlFile_);
  parsed = 0;

  startgemtxt = false;
  sprintf(GemtxtMounNchar,"");

  sprintf(DirGen,             "/home/cscdev/Jared/EmulatorBoard/TriDAS_BoardPlus2/emu/emuDCS/CSCGEMTestStand/tmp");
  sprintf(DirUp,              "/home/cscdev/Jared/EmulatorBoard/TriDAS_BoardPlus2/emu/emuDCS/CSCGEMTestStand/tmp");
  sprintf(DirYuriyConvtxt,    "/home/cscdev/Jared/EmulatorBoard/TriDAS_BoardPlus2/emu/emuDCS/CSCGEMTestStand/tmp");
  sprintf(DirYuriyCLCTConvPat,"/home/cscdev/Jared/EmulatorBoard/TriDAS_BoardPlus2/emu/emuDCS/CSCGEMTestStand/tmp");
  sprintf(DirGEMPat,          "/home/cscdev/Jared/EmulatorBoard/TriDAS_BoardPlus2/emu/emuDCS/CSCGEMTestStand/tmp");
  sprintf(DirCfebPat,         "/home/cscdev/Jared/EmulatorBoard/TriDAS_BoardPlus2/emu/emuDCS/CSCGEMTestStand/tmp");
  sprintf(DirGEMtxt,          "/home/cscdev/Jared/EmulatorBoard/TriDAS_BoardPlus2/emu/emuDCS/CSCGEMTestStand/tmp");
  sprintf(DirCfebtxt,         "/home/cscdev/Jared/EmulatorBoard/TriDAS_BoardPlus2/emu/emuDCS/CSCGEMTestStand/tmp");
  sprintf(DirYuriyCLCTConvtxt,"/home/cscdev/Jared/EmulatorBoard/TriDAS_BoardPlus2/emu/emuDCS/CSCGEMTestStand/tmp");
  sprintf(DCFEBN,"1");
  sprintf(HalfStripN,"1");
  sprintf(FiberN,"1");
  sprintf(PatFile,"DCFEB_1.pat");
  sprintf(NumRuns,"1");
  sprintf(GEMtxtFile,"gempads.txt");
  sprintf(YuriyCLCTConvtxtFile,"clctpattern.txt");
  sprintf(CfebtxtFile,"clctpattern.txt");
  sprintf(YuriyConvtxtFile,"Yuriy_pattern.txt");

  // Cameron Addition : Jan. 2020
  sprintf(GetSetPrefix, patternSet.Prefix.c_str());
  sprintf(GetSaveDir, cw::default_dir.c_str());

  EmuBoardLocation = "/dev/schar3";

  for (int i = 0; i < 8; ++i) {
    sprintf(FileNameFiberN[i],"N/A");
  }

  sprintf(Fiber0PatName,"N/A");
  sprintf(Fiber1PatName,"N/A");
  sprintf(Fiber2PatName,"N/A");
  sprintf(Fiber3PatName,"N/A");
  sprintf(Fiber4PatName,"N/A");
  sprintf(Fiber5PatName,"N/A");
  sprintf(Fiber6PatName,"N/A");
  sprintf(Fiber7PatName,"N/A");

  FileLoadedToBoard = false;
  //TMB * thisTMB   = tmbVector[chamber_index];

  for (int i = 0; i < 512; ++i) {
    string Result;          // string which will contain the result
    std::ostringstream convert;   // stream used for the conversion
    convert << i;      // insert the textual representation of 'Number' in the characters in the stream
    Result = convert.str();
    string tempstring = "N" + Result;
    sprintf(testingarray[i],tempstring.c_str());
    sprintf(testingarray[i],"");
  }

  //TMB * thisTMB = tmbVector[1];
  //CLCT0_Counter = thisTMB->GetCounter(cw::tmb_counters[0]);
  //CLCT1_Counter = thisTMB->GetCounter(cw::tmb_counters[1]);
  
  for(int i=0; i < 4; i++){
    std::string str;
    GetCLCT_ps_char.push_back(str);
  }
}

bool CSCGEMTestApplication::ParsingXML(){
  //
	xmlFile_.fromString(emu::utils::performExpansions(xmlFile_));
    std::string config_src, config_key;
    //
    Logger logger_ = getApplicationLogger();
    //
    LOG4CPLUS_INFO(logger_, "EmuPeripheralCrate reloading...");
    //
    config_src = XML_or_DB_.toString();
    // std::cout << "XML_or_DB: " << config_src << std::endl;
    if(config_src == "xml" || config_src == "XML")
    {
       config_key = xmlFile_.toString();
    }
    else if (config_src == "db" || config_src == "DB")
    {
       config_key = Valid_config_ID;
    }
    else
    {
       std::cout << "No valid XML_or_DB found..." << std::endl;
       return false;
    }
    std::cout << "this works up to the CommonParser. CSCGEMTestApplication btw." << std::endl;
    std::cout << "The strings that go into the CommonParser function are as follows:" << std::endl; //jmn
    std::cout << "config_src: " << config_src << std::endl; //jmn
    std::cout << "config_key: " << config_key << std::endl; //jmn
    if(!CommonParser(config_src, config_key)) return false;
    std::cout << "this works past the CommonParser. " << std::endl;
    EmuEndcap *myEndcap = GetEmuEndcap();
    if(myEndcap == NULL) return false;
    myEndcap->NotInDCS();
    xml_or_db = Xml_or_Db();
    crateVector = myEndcap->crates();
    //
    total_crates_=crateVector.size();
    if(total_crates_<=0) return false;
    this_crate_no_=0;

    std::string lab_mode=standalone;
    if(lab_mode=="True" || lab_mode=="TRUE" || lab_mode=="true" || lab_mode=="Yes" || lab_mode=="YES" || lab_mode=="yes")
    {    standalone_ = true;
         std::cout << "PeripheralCrateConfig started in Standalone mode." << std::endl;
    }

    for(unsigned crate_number=0; crate_number< crateVector.size(); crate_number++) {
      //
      SetCurrentCrate(crate_number);
      for(unsigned i=0; i<dmbVector.size();i++) {
	OutputDMBTests[i][current_crate_] << "DMB-CFEB Tests "
					  << thisCrate->GetChamber(dmbVector[i]->slot())->GetLabel().c_str()
					  << " output:" << std::endl;
      }
      for(unsigned i=0; i<tmbVector.size();i++) {
	OutputTMBTests[i][current_crate_] << "TMB-RAT Tests "
					  << thisCrate->GetChamber(tmbVector[i]->slot())->GetLabel().c_str()
					  << " output:" << std::endl;
	ChamberTestsOutput[i][current_crate_] << "Chamber-Crate Phases "
					      << thisCrate->GetChamber(tmbVector[i]->slot())->GetLabel().c_str()
					      << " output:" << std::endl;
      }
      OutputCCBTests[current_crate_] << "CCB Tests output: " << std::endl;
    }
    //
    SetCurrentCrate(this_crate_no_);
    //
    std::cout << "Parser Done" << std::endl ;
    //
    parsed=1;
    return true;
  }

void CSCGEMTestApplication::SetCurrentCrate(int cr)
  {
    if(total_crates_<=0) return;
    thisCrate = crateVector[cr];

    if ( ! thisCrate ) {
      std::cout << "Crate doesn't exist" << std::endl;
      assert(thisCrate);
    }

    ThisCrateID_=thisCrate->GetLabel();
    thisCCB = thisCrate->ccb();
    thisMPC = thisCrate->mpc();
    tmbVector = thisCrate->tmbs();
    dmbVector = thisCrate->daqmbs();
    chamberVector = thisCrate->chambers();
    //dduVector = thisCrate->ddus();
    //
    //tmbTestVector = InitTMBTests(thisCrate);
    //
    for (unsigned int i=0; i<(tmbVector.size()<9?tmbVector.size():9) ; i++) {
      MyTest[i][cr].SetTMB(tmbVector[i]);
      MyTest[i][cr].SetDMB(dmbVector[i]);
      MyTest[i][cr].SetCCB(thisCCB);
      MyTest[i][cr].SetMPC(thisMPC);
      // reset ALCT firmware check value for this crate
      able_to_load_alct[i] = -1;
    }
    //
  // DefineFirmwareFilenames();
    //
    current_crate_ = cr;
  }

void CSCGEMTestApplication::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  using cgicc::br;
  using namespace std;

  cgicc::Cgicc cgi(in);

  string urn = getApplicationDescriptor()->getURN();

  emu::utils::headerXdaq(out, this, "GEM-CSC Pattern Studies");

  //*out << "Hello World!";
  //TMB * thisTMB   = tmbVector[chamber_index];
  xmlFile_.fromString("$BUILD_HOME/xml/tamu_crate_config_2tmbs2019_GEM.xml");
  std::string testxmlfilename = xmlFile_.toString();
  std::cout << "This is the xmlFile_ name before the fromString: " << testxmlfilename << std::endl << std::endl;
  xmlFile_.fromString(emu::utils::performExpansions(xmlFile_));
  testxmlfilename = xmlFile_.toString(); //jmn
  std::cout << "This is the xmlFile_ name after the fromString: " << testxmlfilename << std::endl << std::endl;

  if(!parsed) ParsingXML();

  string s1,s2,s3,s4,s5,s6,s7;
  string c1,c2,c3,c4,c5,c6,c7;

  if(xgi::Utils::hasFormElement(cgi, "GemtxtMounNchar")) sprintf(GemtxtMounNchar,cgi["GemtxtMounNchar"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "DirGen")) sprintf(DirGen,cgi["DirGen"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "DirYuriyConvtxt")) sprintf(DirYuriyConvtxt,cgi["DirYuriyConvtxt"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "DirGEMPat")) sprintf(DirGEMPat,cgi["DirGEMPat"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "DirCfebPat")) sprintf(DirCfebPat,cgi["DirCfebPat"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "DirGEMtxt")) sprintf(DirGEMtxt,cgi["DirGEMtxt"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "DirCfebtxt")) sprintf(DirCfebtxt,cgi["DirCfebtxt"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "FiberN")) sprintf(FiberN,cgi["FiberN"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "PatFile")) sprintf(PatFile,cgi["PatFile"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "NumRuns")) sprintf(NumRuns,cgi["NumRuns"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "GEMtxtFile")) sprintf(GEMtxtFile,cgi["GEMtxtFile"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "CfebtxtFile")) sprintf(CfebtxtFile,cgi["CfebtxtFile"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "YuriyConvtxtFile")) sprintf(YuriyConvtxtFile,cgi["YuriyConvtxtFile"]->getValue().c_str());

  for (int i = 0; i < 512; ++i) {
    string number;          // string which will contain the result
    std::ostringstream convert;   // stream used for the conversion
    convert << i;      // insert the textual representation of 'Number' in the characters in the stream
    number = convert.str();
    string tempstring = "GemBx" + number;
    if(xgi::Utils::hasFormElement(cgi,tempstring)) sprintf(testingarray[i],cgi[tempstring]->getValue().c_str());
  }






  ifstream fs;
  string line;
  fs.open("s1.txt"); getline(fs,s1); fs.close();
  fs.open("s2.txt"); getline(fs,s2); fs.close();
  fs.open("s3.txt"); getline(fs,s3); fs.close();
  fs.open("s4.txt"); getline(fs,s4); fs.close();
  fs.open("s5.txt"); getline(fs,s5); fs.close();
  fs.open("s6.txt"); getline(fs,s6); fs.close();
  fs.open("s7.txt"); getline(fs,s7); fs.close();
  fs.open("c1.txt"); getline(fs,c1); fs.close();
  fs.open("c2.txt"); getline(fs,c2); fs.close();
  fs.open("c3.txt"); getline(fs,c3); fs.close();
  fs.open("c4.txt"); getline(fs,c4); fs.close();
  fs.open("c5.txt"); getline(fs,c5); fs.close();
  fs.open("c6.txt"); getline(fs,c6); fs.close();
  fs.open("c7.txt"); getline(fs,c7); fs.close();

  ofstream ofs; 
  ofs.open("s1.txt"); ofs << "a"; ofs.close();
  ofs.open("s2.txt"); ofs << "a"; ofs.close();
  ofs.open("s3.txt"); ofs << "a"; ofs.close();
  ofs.open("s4.txt"); ofs << "a"; ofs.close();
  ofs.open("s5.txt"); ofs << "a"; ofs.close();
  ofs.open("s6.txt"); ofs << "a"; ofs.close();
  ofs.open("s7.txt"); ofs << "a"; ofs.close();
  ofs.open("c1.txt"); ofs << "a"; ofs.close();
  ofs.open("c2.txt"); ofs << "a"; ofs.close();
  ofs.open("c3.txt"); ofs << "a"; ofs.close();
  ofs.open("c4.txt"); ofs << "a"; ofs.close();
  ofs.open("c5.txt"); ofs << "a"; ofs.close();
  ofs.open("c6.txt"); ofs << "a"; ofs.close();
  ofs.open("c7.txt"); ofs << "a"; ofs.close();
/*
  // unit 1

  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
  *out << cgicc::legend("Generate DCFEB data").set("style", "color:blue") ;
  *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/GenerateDCFEBData" ) << endl;
  *out << "Directory:" << cgicc::input().set("type", "text").set("name", "DirGen").set("size", "110").set("value", DirGen) << endl;
  *out << cgicc::br() << endl;
  *out << cgicc::br() << endl;
  *out << "DCFEB #:" << cgicc::input().set("type", "text").set("name", "DCFEBN").set("size", "2").set("value", DCFEBN) << endl;
  *out << "Half-strip #:" << cgicc::input().set("type", "text").set("name", "HalfStripN").set("size", "2").set("value", HalfStripN) << endl;
  *out << "&nbsp" << endl;
  *out << cgicc::input().set("type", "submit").set("value", "Generate data").set("name", "Generate data") << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::fieldset() << endl;
  *out << cgicc::br() << endl;

  // unit 2

  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
  *out << cgicc::legend("Upload DCFEB data").set("style", "color:blue") ;
  *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/UploadDCFEBData" ) << endl;
  *out << "Directory:" << cgicc::input().set("type", "text").set("name", "DirUp").set("size", "110").set("value", DirUp) << endl;
  *out << cgicc::br() << endl;
  *out << cgicc::br() << endl;
  *out << cgicc::input().set("type", "submit").set("value", "Upload data").set("name", "Upload data") << endl;
  *out << "&nbsp&nbsp" << endl;
  *out << "Status of DCFEB#: " << endl;
  if (s1 == "0") *out << cgicc::input().set("type", "button").set("value", "1").set("style", "color:green") << endl;
  else if (s1 == "1") *out << cgicc::input().set("type", "button").set("value", "1").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "1") << endl;
  if (s2 == "0") *out << cgicc::input().set("type", "button").set("value", "2").set("style", "color:green") << endl;
  else if (s2 == "1") *out << cgicc::input().set("type", "button").set("value", "2").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "2") << endl;
  if (s3 == "0") *out << cgicc::input().set("type", "button").set("value", "3").set("style", "color:green") << endl;
  else if (s3 == "1") *out << cgicc::input().set("type", "button").set("value", "3").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "3") << endl;
  if (s4 == "0") *out << cgicc::input().set("type", "button").set("value", "4").set("style", "color:green") << endl;
  else if (s4 == "1") *out << cgicc::input().set("type", "button").set("value", "4").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "4") << endl;
  if (s5 == "0") *out << cgicc::input().set("type", "button").set("value", "5").set("style", "color:green") << endl;
  else if (s5 == "1") *out << cgicc::input().set("type", "button").set("value", "5").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "5") << endl;
  if (s6 == "0") *out << cgicc::input().set("type", "button").set("value", "6").set("style", "color:green") << endl;
  else if (s6 == "1") *out << cgicc::input().set("type", "button").set("value", "6").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "6") << endl;
  if (s7 == "0") *out << cgicc::input().set("type", "button").set("value", "7").set("style", "color:green") << endl;
  else if (s7 == "1") *out << cgicc::input().set("type", "button").set("value", "7").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "7") << endl;
  *out << cgicc::br() << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/CheckDCFEBData" ) << endl;
  *out << cgicc::input().set("type", "submit").set("value", "Check data").set("name", "Check data") << endl;
  *out << "&nbsp&nbsp&nbsp" << endl;
  *out << "Status of DCFEB#: " << endl;
  if (c1 == "0") *out << cgicc::input().set("type", "button").set("value", "1").set("style", "color:green") << endl;
  else if (c1 == "1") *out << cgicc::input().set("type", "button").set("value", "1").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "1") << endl;
  if (c2 == "0") *out << cgicc::input().set("type", "button").set("value", "2").set("style", "color:green") << endl;
  else if (c2 == "1") *out << cgicc::input().set("type", "button").set("value", "2").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "2") << endl;
  if (c3 == "0") *out << cgicc::input().set("type", "button").set("value", "3").set("style", "color:green") << endl;
  else if (c3 == "1") *out << cgicc::input().set("type", "button").set("value", "3").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "3") << endl;
  if (c4 == "0") *out << cgicc::input().set("type", "button").set("value", "4").set("style", "color:green") << endl;
  else if (c4 == "1") *out << cgicc::input().set("type", "button").set("value", "4").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "4") << endl;
  if (c5 == "0") *out << cgicc::input().set("type", "button").set("value", "5").set("style", "color:green") << endl;
  else if ey HalfStrip : " << cgicc::input().set("type","text").set("name","GetCLCT_key_char").set("size","7").set("value", GetCLCT_key_char) << endl;
  *out << " PiD :(c5 == "1") *out << cgicc::input().set("type", "button").set("value", "5").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "5") << endl;
  if (c6 == "0") *out << cgicc::input().set("type", "button").set("value", "6").set("style", "color:green") << endl;
  else if (c6 == "1") *out << cgicc::input().set("type", "button").set("value", "6").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "6") << endl;
  if (c7 == "0") *out << cgicc::input().set("type", "button").set("value", "7").set("style", "color:green") << endl;
  else if (c7 == "1") *out << cgicc::input().set("type", "button").set("value", "7").set("style", "color:red") << endl;
  else *out << cgicc::input().set("type", "button").set("value", "7") << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::fieldset() << endl;
  *out << cgicc::br() << endl;

  // unit 3

  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
  *out << cgicc::legend("Transmit DCFEB data").set("style", "color:blue") ;
  *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/TransmitDCFEBData" ) << endl;
  *out << cgicc::input().set("type", "submit").set("value", "Transmit data").set("name", "Transmit data") << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::fieldset() << endl;
  *out << cgicc::br() << endl;

  // unit 4

  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
  *out << cgicc::legend("Readout results from OTMB").set("style", "color:blue") ;
  *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/ReadOutTriggerResults" ) << endl;
  *out << cgicc::input().set("type", "submit").set("value", "Readout results").set("name", "Readout results") << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::fieldset() << endl;

  // unit 5
  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
*out << cgicc::legend("F0 Command").set("style", "color:blue") ;
*out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/F0" ) << endl;
*out << cgicc::input().set("type", "submit").set("name", "F0") << endl;
*out << cgicc::form() << endl;
*out << cgicc::fieldset() << endl;
*/

cgicc::form_iterator name = cgi.getElement("tmb");
int tmb;
if(name != cgi.getElements().end()) {
  tmb = cgi["tmb"]->getIntegerValue();
  std::cout << "TMBStatus:  TMB=" << tmb << std::endl;
  TMB_ = tmb;
} else {
  std::cout << "TMBStatus: No TMB" << std::endl ;
  tmb = TMB_;
}


tmb = 1;
TMB * thisTMB = tmbVector[tmb];
std::cout << "the value for tmb is = " << tmb << std::endl;
std::cout << "The tmbVector[tmb] = " << tmbVector[tmb] << std::endl;


  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
  *out << cgicc::legend("Define Pattern Set").set("style", "color:blue") ;
  *out << "CFEB CLCT" << endl;
  *out << cgicc::form().set("method", "GET").set("action","/" + urn + "/AddCLCT") << endl;
  *out << "Bx : " << cgicc::input().set("type","text").set("name","GetCLCT_bx_char").set("size","7").set("value", GetCLCT_bx_char) << endl;
  *out << " Key HalfStrip : " << cgicc::input().set("type","text").set("name","GetCLCT_key_char").set("size","7").set("value", GetCLCT_key_char) << endl;
  *out << " PiD : " << cgicc::input().set("type","text").set("name","GetCLCT_pid_char").set("size","7").set("value", GetCLCT_pid_char) << endl;  
  *out << " N Hits : " << cgicc::input().set("type","text").set("name","GetCLCT_nhit_char").set("size","7").set("value", GetCLCT_nhit_char) << endl;
  *out << cgicc::input().set("type","submit").set("value","Add").set("name", "Add CLCT") << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::br() << endl;
  *out << "GEM Cluster" << endl;
  *out << cgicc::form().set("method", "GET").set("action","/" + urn + "/AddGEM") << endl;
  *out << "Bx : " << cgicc::input().set("type","text").set("name","GetGEM_bx_char").set("size","7").set("value", GetGEM_bx_char) << endl;
  *out << " Roll : " << cgicc::input().set("type","text").set("name","GetGEM_roll_char").set("size","7").set("value", GetGEM_roll_char) << endl;
  *out << " Pad : " << cgicc::input().set("type","text").set("name","GetGEM_pad_char").set("size","7").set("value", GetGEM_pad_char) << endl;
  *out << " Size : " << cgicc::input().set("type","text").set("name","GetGEM_size_char").set("size","7").set("value", GetGEM_size_char) << endl;
  *out << " Layer : " << cgicc::input().set("type","text").set("name","GetGEM_layer_char").set("size","7").set("value", GetGEM_layer_char) << endl;
  *out << cgicc::input().set("type","submit").set("value","Add").set("name", "Add GEM") << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::fieldset() << endl;

  *out << cgicc::fieldset().set("style", "background-color:#FFFFBB; font-size: 11pt") << endl;
  *out << cgicc::legend("Current Pattern Set").set("style","color:blue; font-size: 14pt") << endl;
  *out << cgicc::pre();
  *out << "CSC: " << endl;
  *out << "bx keystrip pattern nhits\t{ (bx,hs,layer) }" << endl;
  for(int i=0; i < patternSet.CSC.size(); i++){
    *out << patternSet.CSC[i] << endl;
  }
  *out << cgicc::br() << endl;
  *out << "GEM: " << endl;
  *out << "bx\troll\tpad\tsize\tlayer" << endl;
  for(int i=0; i < patternSet.GEM.size(); i++){
    *out << patternSet.GEM[i] << endl;
  }
  *out << cgicc::pre();
  *out << cgicc::br() << endl;
  *out << cgicc::form().set("method", "GET").set("action","/" + urn + "/LoadToEmuBoard") << endl;
  *out << cgicc::input().set("type","submit").set("value","Load EmuBoard").set("name", "LoadToEmuBoard") << endl;
  *out << cgicc::form() << endl;  
  *out << cgicc::fieldset();


  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
  *out << cgicc::legend("Save Current Set").set("style", "color:blue; font-size: 14pt") << endl;
  *out << "Prefix : " << endl;
  *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/SaveAsPat") << endl;
  *out << cgicc::input().set("type","text").set("name","GetSetPrefix").set("size","24").set("value",GetSetPrefix) << endl;
  *out << cgicc::br() << endl;
  *out << cgicc::br() << endl;
  *out << "Save Directory : " << endl;
  *out << cgicc::input().set("type","text").set("name","GetSaveDir").set("size","110").set("value",GetSaveDir) << endl;
  *out << cgicc::br() << endl;
  *out << cgicc::br() << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::table().set("border","0") << endl;
  *out << cgicc::td().set("ALIGN", "left") << endl;
  *out << cgicc::form().set("method", "GET").set("action","/" + urn + "/SaveAsPat") << endl;
  *out << cgicc::input().set("type", "submit").set("value","Save as {.pat}").set("name", "SaveAsPat") << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::td() << endl;
  *out << cgicc::td().set("ALIGN", "left") << endl;
  *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/SaveAsTxt") << endl;
  *out << cgicc::input().set("type", "submit").set("value","Save as (.txt)").set("name", "SaveAsTxt") << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::td() << endl;
  *out << cgicc::table() << endl;
  *out << cgicc::fieldset();

  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
  *out << cgicc::legend("Study Pattern Set").set("style", "color:blue; font-size: 14pt") << endl;
  *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/RunStudy" ) << endl;
  *out << "Number of Trials : " << endl;
  *out << cgicc::input().set("type","text").set("name","GetNtrials_char").set("size","10").set("value",GetNtrials_char) << endl;
  *out << cgicc::input().set("type", "submit").set("value", "Run").set("name", "Run Study") << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::br() << endl;
  *out << cgicc::pre();
  thisTMB->RedirectOutput(out);
  thisTMB->GetCounters();
  cout << "TMB Counter # 29 = " << thisTMB->GetCounter(29) << endl;
  cout << "TMB Counter # 30 = " << thisTMB->GetCounter(30) << endl;

  thisTMB->PrintCounters(29);
  *out << endl;
  thisTMB->PrintCounters(30);
  thisTMB->RedirectOutput(&std::cout);
  *out << cgicc::pre();
  *out << cgicc::br() << endl;
  *out << cgicc::pre();
  *out << "Statistics : " << endl;
  *out << "# Trials = " << patternSet.N_trials << endl;
  for(int i=0; i < patternSet.Results.size(); i++){
    *out << patternSet.Results[i] << endl;
  }
  *out << cgicc::pre();
  *out << cgicc::fieldset();

  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
  *out << cgicc::legend("Clear Current Set").set("style", "color:blue; font-size: 14pt") << endl; 
  *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/ClearSet" ) << endl;
  *out << cgicc::input().set("type", "submit").set("value", "Clear Current Set").set("name", "Clear Current Set") << " Deletes *.pat in (default_dir), and resets patternSet object" << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::fieldset();

  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
  *out << cgicc::legend("Clear the Emulator Board").set("style", "color:blue") ;
  *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/ClearEmuBoard" ) << endl;
  *out << cgicc::input().set("type", "submit").set("value", "Clear Emulator Board").set("name", "Clear Emulator Board") << " Loads Empty patterns to Emulator Board" << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::fieldset() << endl;


// Automation Section!!
  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
  *out << cgicc::legend("Parameter Scan").set("style", "color:blue; font-size:14pt");
  *out << cgicc::form().set("method", "GET").set("action","/" + urn + "/AddCLCTParamScan") << endl;
  *out << "Bx : " << cgicc::input().set("type","text").set("name","GetCLCT_bx_ps_char").set("size","7").set("value", GetCLCT_bx_ps_char) << endl;
  *out << " Key HalfStrip : " << cgicc::input().set("type","text").set("name","GetCLCT_key_ps_char").set("size","7").set("value", GetCLCT_key_ps_char) << endl;
  *out << " PiD : " << cgicc::input().set("type","text").set("name","GetCLCT_pid_ps_char").set("size","7").set("value", GetCLCT_pid_ps_char) << endl;
  *out << " N Hits : " << cgicc::input().set("type","text").set("name","GetCLCT_nhit_ps_char").set("size","7").set("value", GetCLCT_nhit_ps_char) << endl;
  *out << cgicc::input().set("type","submit").set("value","Add").set("name", "Add CLCT Param Scan") << endl;
  *out << cgicc::form() << endl;
  
  *out << cgicc::form().set("method", "GET").set("action","/" + urn + "RunParamScan") << endl;
  *out << cgicc::table().set("border", "1") << endl;
  *out << cgicc::td().set("ALIGN", "Center") << endl;
  *out << cgicc::textarea().set("name", "ParamScanMuonList").set("rows","5").set("cols","150").set("WRAP","OFF");
  *out << "Muons in Study : " << CLCT_ps_vec.size() << endl;
  for(int i=0; i < patternSet.CSC.size(); i++){
	*out << patternSet.CSC[i] << endl;
  } 
  *out << cgicc::textarea() << endl; 
  *out << cgicc::td() << endl;

  *out << cgicc::td().set("ALIGN", "Center") << endl;
  *out << cgicc::textarea().set("name", "ParamScanMuonList").set("rows","5").set("cols","150").set("WRAP","OFF");
  *out << "Free Parameters : " << Free_params.size() << endl; 
  *out << cgicc::textarea() << endl;
  *out << cgicc::td() << endl;
  *out << cgicc::table() << endl;
  *out << cgicc::br();
  *out << "Trials / Step  " << cgicc::input().set("type","text").set("name","TrialsPerStep_char").set("size","7").set("value",TrialsPerStep_char) << endl;
  *out << cgicc::br();
  *out << "Output File  " << cgicc::input().set("type","text").set("name","ParamScanOutFile").set("size","20").set("value",ParamScanOutFile) << endl;
  *out << cgicc::input().set("type","submit").set("value","Run").set("name", "Run Param Scan") << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::fieldset() << endl;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  LOG4CPLUS_INFO(getApplicationLogger(), "Start PrintCounters");
  thisTMB->RedirectOutput(&OutputStringTMBStatus[tmb]);
  thisTMB->GetCounters();
  thisTMB->PrintCounters();
  thisTMB->PrintGemCounters();
  thisTMB->RedirectOutput(&std::cout);
  LOG4CPLUS_INFO(getApplicationLogger(), "Done PrintCounters");




*out << cgicc::fieldset();

  *out << cgicc::form().set("method","GET") << std::endl ;
  *out << cgicc::pre();
  *out << cgicc::textarea().set("name","CrateTestTMBOutput")
    .set("rows","50")
    .set("cols","150")
    .set("WRAP","OFF");
  *out << OutputStringTMBStatus[tmb].str() << std::endl ;
  *out << cgicc::textarea();
  OutputStringTMBStatus[tmb].str("");
  *out << cgicc::pre();
  *out << cgicc::form() << std::endl ;


*out << cgicc::legend("CLCT Info").set("style","color:blue") << std::endl ;
*out << cgicc::pre();
thisTMB->RedirectOutput(out);
thisTMB->DecodeCLCT();
thisTMB->PrintCLCT();
thisTMB->RedirectOutput(&std::cout);
*out << cgicc::pre();
*out << cgicc::fieldset();



/*
*out << cgicc::fieldset();
*out << cgicc::legend("CLCT Info").set("style","color:blue") << std::endl ;
*out << cgicc::pre();
thisTMB->RedirectOutput(out);
thisTMB->DecodeCLCT();
thisTMB->PrintCLCT();
thisTMB->RedirectOutput(&std::cout);
*out << cgicc::pre();
*out << cgicc::fieldset();
*/

 // *out << cgicc::table().set("border","0");
  //*out << cgicc::h2("System Utilities")<< std::endl;
  //
  //  *out << cgicc::td();
  //

//  *out << cgicc::br() << endl;
//  *out << cgicc::br() << endl;
/*
  *out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
  *out << cgicc::legend("number of mouns to include in txt pattern file (up to 512)").set("style", "color:blue") ;
  *out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/GenGEMPattxtFileMounN" ) << endl;
  *out << "Number of mouns to be in the gem pattern file:" << cgicc::input().set("type", "text").set("name", "GemtxtMounNchar").set("size", "7").set("value", GemtxtMounNchar) << endl;
  *out << cgicc::input().set("type", "submit").set("value", "Start Gem Txt File Creation").set("name", "Start Gem Txt File Creation") << endl;
  *out << cgicc::form() << endl;
  *out << cgicc::fieldset() << endl;


///////////////////////////////////////////////////////////////
//	Cameron 06/24/2019 Comment Test

// the following can be removed without harm

*out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFAABB") << endl;
*out << cgicc::legend("Cameron Pattern Converter").set("style", "color:green") ;
*out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/GenGEMPattxtFileMounN" ) << endl;
*out << "Number of mouns to be in the gem pattern file:" << cgicc::input().set("type", "text").set("name", "GemtxtMounNchar").set("size", "10").set("value", GemtxtMounNchar) << endl;
*out << cgicc::input().set("type", "submit").set("value", "Start Cameron's Pattern Creation").set("name", "Start Gem Txt File Creation") << endl;
*out << cgicc::form() << endl;
*out << cgicc::fieldset() << endl;




///

*out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
*out << cgicc::legend("Testing and have no Idea if this will work or not").set("style", "color:blue") ;
*out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/GenGEMPattxtFile" ) << endl;
for (int i=0; i<10; i++){
string number;          // string which will contain the result
std::ostringstream convert;   // stream used for the conversion
convert << i;      // insert the textual representation of 'Number' in the characters in the stream
number = convert.str();
string tempstring = "GemBx" + number;
*out << "the input for i == " << i << " is: " << cgicc::input().set("type", "text").set("name", tempstring).set("size", "20").set("value", testingarray[i]) << endl;
*out << cgicc::br() << endl;
}
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << "GEM txt File Name : " << cgicc::input().set("type", "text").set("name", "YuriyConvtxtFile").set("size", "60").set("value", YuriyConvtxtFile) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << cgicc::input().set("type", "submit").set("value", "Convert Yuriy txt File").set("name", "Convert Yuriy txt File") << endl;
*out << cgicc::form() << endl;
*out << cgicc::fieldset() << endl;
*/
///////////////////////////////////////////////////////////////////////////



/*
//unit 3 n
*out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
*out << cgicc::legend("Converter Yuriy's style txt file to Cfeb Pat File").set("style", "color:blue") ;
*out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/YuriyPatConvert" ) << endl;
*out << "Yuriy Style txt File Directory : " << cgicc::input().set("type", "text").set("name", "DirYuriyConvtxt").set("size", "110").set("value", DirGEMtxt) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << "GEM txt File Name : " << cgicc::input().set("type", "text").set("name", "YuriyConvtxtFile").set("size", "60").set("value", YuriyConvtxtFile) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << cgicc::input().set("type", "submit").set("value", "Convert Yuriy txt File").set("name", "Convert Yuriy txt File") << endl;
*out << cgicc::form() << endl;
*out << cgicc::fieldset() << endl;

// unit 3.5 n
*out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
*out << cgicc::legend("Converter Human Readable Cfeb txt file to Yuriy Style txt File").set("style", "color:blue") ;
*out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/Yuriy_CLCT_PatternConverter" ) << endl;
*out << "Cfeb txt File Directory : " << cgicc::input().set("type", "text").set("name", "DirYuriyCLCTConvtxt").set("size", "110").set("value", DirYuriyCLCTConvtxt) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << "Cfeb txt File Name : " << cgicc::input().set("type", "text").set("name", "YuriyCLCTConvtxtFile").set("size", "60").set("value", YuriyCLCTConvtxtFile) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << "File Directory For New Yuriy Style txt File: " << cgicc::input().set("type", "text").set("name", "DirYuriyCLCTConvPat").set("size", "110").set("value", DirYuriyCLCTConvPat) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << cgicc::input().set("type", "submit").set("value", "Convert Cfeb txt to Yuriy txt File").set("name", "Convert Cfeb txt to Yuriy txt File") << endl;
*out << cgicc::form() << endl;
*out << cgicc::fieldset() << endl;

// unit 4 n
*out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
*out << cgicc::legend("Converter Human Readable GEM txt file to GEM Pat File ").set("style", "color:blue") ;
*out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/GemPatternConverter" ) << endl;
*out << "GEM txt File Directory : " << cgicc::input().set("type", "text").set("name", "DirGEMtxt").set("size", "110").set("value", DirGEMtxt) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << "GEM txt File Name : " << cgicc::input().set("type", "text").set("name", "GEMtxtFile").set("size", "60").set("value", GEMtxtFile) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << "File Directory For New GEM Pat File: " << cgicc::input().set("type", "text").set("name", "DirGEMPat").set("size", "110").set("value", DirGEMPat) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << cgicc::input().set("type", "submit").set("value", "Convert GEM txt File").set("name", "Convert GEM txt File") << endl;
*out << cgicc::form() << endl;
*out << cgicc::fieldset() << endl;

// unit 5 n
*out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
*out << cgicc::legend("Converter Human Readable Cfeb txt file to Cfeb Pat File").set("style", "color:blue") ;
*out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/CfebPatternConverter" ) << endl;
*out << "Cfeb txt File Directory : " << cgicc::input().set("type", "text").set("name", "DirCfebtxt").set("size", "110").set("value", DirCfebtxt) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << "Cfeb txt File Name : " << cgicc::input().set("type", "text").set("name", "CfebtxtFile").set("size", "60").set("value", CfebtxtFile) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << "File Directory For New Cfeb Pat File: " << cgicc::input().set("type", "text").set("name", "DirCfebPat").set("size", "110").set("value", DirCfebPat) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << cgicc::input().set("type", "submit").set("value", "Convert Cfeb txt File").set("name", "Convert Cfeb txt File") << endl;
*out << cgicc::form() << endl;
*out << cgicc::fieldset() << endl;




// unit 6
*out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
*out << cgicc::legend("Loading the File to Emulator Board").set("style", "color:blue") ;
*out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/LoadToEmuBoard" ) << endl;
*out << "File Directory :" << cgicc::input().set("type", "text").set("name", "DirGen").set("size", "110").set("value", DirGen) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << "Pattern File Name :" << cgicc::input().set("type", "text").set("name", "PatFile").set("size", "60").set("value", PatFile) << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << "Fiber Number :" << cgicc::input().set("type", "text").set("name", "FiberN").set("size", "2").set("value", FiberN) << endl;
*out << "Note: Fiber 0 is for the GEM" << endl;
*out << cgicc::br() << endl;
*out << cgicc::br() << endl;
*out << cgicc::input().set("type", "submit").set("value", "Load to Emulator Board").set("name", "Load to Emulator Board") << endl;
if(FileLoadedToBoard){
  if (PatLoadError){
    *out<< cgicc::span().set("style","color:red");
    *out<< "Error: The pattern file did not load correctly! ";
  }
  else {
    *out<< cgicc::span().set("style","color:green");
    *out<< "The pattern file loaded to the emulator board successfully.";
  }
  *out << cgicc::span();
}
*out << cgicc::form() << endl;
*out << cgicc::fieldset() << endl;

FileLoadedToBoard = false;

//info about the current file loaded
*out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
*out << cgicc::legend("Latest Files Loaded to Emulator Board").set("style", "color:blue") ;
*out << cgicc::pre();
for(int i=0; i<8; ++i){
  *out << "For fiber number " << i << ": " << FileNameFiberN[i] << endl;
}
*out << cgicc::pre();
*out << cgicc::fieldset() << endl;


// unit 7
*out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
*out << cgicc::legend("Sending the File from the Emulator Board").set("style", "color:blue") ;
*out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/SendFromEmuBoard" ) << endl;
*out << cgicc::input().set("type", "submit").set("value", "Send From Emulator Board").set("name", "Send From Emulator Board") << endl;
*out << "Number of times to send :" << cgicc::input().set("type", "text").set("name", "NumRuns").set("size", "7").set("value", NumRuns) << endl;
*out << cgicc::form() << endl;
*out << cgicc::fieldset() << endl;

// unit 8

*out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
*out << cgicc::legend("Clear the Emulator Board").set("style", "color:blue") ;
*out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/ClearEmuBoard" ) << endl;
*out << cgicc::input().set("type", "submit").set("value", "Clear Emulator Board").set("name", "Clear Emulator Board") << endl;
*out << cgicc::form() << endl;
*/
//*out << cgicc::fieldset() << endl;

/*
//unit 9
*out << cgicc::fieldset().set("style", "font-size: 11pt; background-color:#FFFFBB") << endl;
*out << cgicc::legend("Output the CLCT info").set("style", "color:blue") ;
*out << cgicc::form().set("method", "GET").set("action", "/" + urn + "/OutputCLCTInfo" ) << endl;
*out << cgicc::input().set("type", "submit").set("value", "Output CLCT Info").set("name", "Output CLCT Info") << endl;
*out << cgicc::form() << endl;
*out << cgicc::fieldset() << endl;
*/
//

/*
cgicc::form_iterator name = cgi.getElement("tmb");
int tmb;
if(name != cgi.getElements().end()) {
  tmb = cgi["tmb"]->getIntegerValue();
  std::cout << "TMBStatus:  TMB=" << tmb << std::endl;
  TMB_ = tmb;
} else {
  std::cout << "TMBStatus: No TMB" << std::endl ;
  tmb = TMB_;
}

tmb = 1;
TMB * thisTMB = tmbVector[tmb];
std::cout << "the value for tmb is = " << tmb << std::endl;
std::cout << "The tmbVector[tmb] = " << tmbVector[tmb] << std::endl;


*out << cgicc::fieldset();
*out << cgicc::legend("CLCT Info").set("style","color:blue") << std::endl ;
*out << cgicc::pre();
thisTMB->RedirectOutput(out);
thisTMB->DecodeCLCT();
thisTMB->PrintCLCT();
thisTMB->RedirectOutput(&std::cout);
*out << cgicc::pre();
*out << cgicc::fieldset();

*/

/*
  stringstream ss;
  thisTMB->RedirectOutput(&ss);
  thisTMB->DecodeCLCT();
  thisTMB->PrintCLCT();
  thisTMB->RedirectOutput(&std::cout);
  cout << ss.str() << endl;
*/


//
/*
if(thisTMB->GetHardwareVersion() >= 2) {
  *out << cgicc::fieldset();
  *out
    << cgicc::legend("LCT Info: Frames Sent to MPC").set("style", "color:red")
    << std::endl;
  *out << cgicc::pre();
  thisTMB->RedirectOutput(out);
  thisTMB->DecodeMPCFrames(); // Decode MPC frames for LAST trigger. VME registers: 0x88, 0x8a, 0x8c, 0x8e
  thisTMB->PrintMPCFrames();  // Print  MPC frames for LAST trigger. VME registers: 0x88, 0x8a, 0x8c, 0x8e
  //
  thisTMB->DecodeMPCFramesFromFIFO(); // Decode MPC frames for ONE trigger from FIFO. VME registers: 0x17C, 0x17E, 0x180, 0x182
  thisTMB->PrintMPCFramesFromFIFO();  // Print  MPC frames for ONE trigger from FIFO. VME registers: 0x17C, 0x17E, 0x180, 0x182
  //
  thisTMB->DecodeAndPrintMPCFrames(10); // Decode and print MPC frames for both cases:
                    //   1. LAST trigger. VME registers: 0x88, 0x8a, 0x8c, 0x8e
                    //   2. ONE trigger from FIFO. VME registers: 0x17C, 0x17E, 0x180, 0x182
  thisTMB->RedirectOutput(&std::cout);
  *out << cgicc::pre();
  *out << cgicc::fieldset();
}
*/

  emu::utils::footer(out);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//					Pattern Studies Functions (Jan. 2020)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSCGEMTestApplication::AddCLCT(xgi::Input * in, xgi::Output * out)
{ 
  using namespace std;

  cgicc::Cgicc cgi(in);

  if(xgi::Utils::hasFormElement(cgi,"GetCLCT_bx_char")) sprintf(GetCLCT_bx_char,cgi["GetCLCT_bx_char"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi,"GetCLCT_key_char")) sprintf(GetCLCT_key_char,cgi["GetCLCT_key_char"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi,"GetCLCT_pid_char")) sprintf(GetCLCT_pid_char,cgi["GetCLCT_pid_char"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi,"GetCLCT_nhit_char")) sprintf(GetCLCT_nhit_char,cgi["GetCLCT_nhit_char"]->getValue().c_str());

  GetCLCT_bx_int = atoi(GetCLCT_bx_char);
  GetCLCT_key_int = atoi(GetCLCT_key_char);
  GetCLCT_pid_int = atoi(GetCLCT_pid_char);
  GetCLCT_nhit_int = atoi(GetCLCT_nhit_char);

  cout << "READ IN CFEB : \n";
  cout << "Bx : " << GetCLCT_bx_int << endl;
  cout << "Key : " << GetCLCT_key_int << endl;
  cout << "PiD : " << GetCLCT_pid_int << endl;
  cout << "N Hits : " << GetCLCT_nhit_int  << endl;

  patternSet.AddCLCT( CLCT(GetCLCT_bx_int, GetCLCT_key_int, GetCLCT_pid_int, GetCLCT_nhit_int) );

  this->Default(in, out);
  return; 
}

void CSCGEMTestApplication::AddGEM(xgi::Input * in, xgi::Output * out)
{
cgicc::Cgicc cgi(in);

  if(xgi::Utils::hasFormElement(cgi,"GetGEM_bx_char")) sprintf(GetGEM_bx_char,cgi["GetGEM_bx_char"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi,"GetGEM_roll_char")) sprintf(GetGEM_roll_char,cgi["GetGEM_roll_char"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi,"GetGEM_pad_char")) sprintf(GetGEM_pad_char,cgi["GetGEM_pad_char"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi,"GetGEM_size_char")) sprintf(GetGEM_size_char,cgi["GetGEM_size_char"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi,"GetGEM_layer_char")) sprintf(GetGEM_layer_char,cgi["GetGEM_layer_char"]->getValue().c_str());

  GetGEM_bx_int = atoi(GetGEM_bx_char);
  GetGEM_roll_int = atoi(GetGEM_roll_char);
  GetGEM_pad_int = atoi(GetGEM_pad_char);
  GetGEM_size_int = atoi(GetGEM_size_char);
  GetGEM_layer_int = atoi(GetGEM_layer_char);

  cout << "READ IN GEM : \n";
  cout << "Bx : " << GetGEM_bx_int << endl;
  cout << "Roll : " << GetGEM_roll_int << endl;
  cout << "Pad : " << GetGEM_pad_int << endl;
  cout << "Size : " << GetGEM_size_int  << endl;
  cout << "Layer : " << GetGEM_layer_int << endl;

  patternSet.AddGEM( Cluster(GetGEM_bx_int, GetGEM_roll_int, GetGEM_pad_int, GetGEM_size_int, GetGEM_layer_int) );

  this->Default(in, out); 
  return; 
}

void CSCGEMTestApplication::SaveAsPat(xgi::Input * in, xgi::Output * out)
{
  using namespace std;
  cgicc::Cgicc cgi(in);

  if(xgi::Utils::hasFormElement(cgi,"GetSetPrefix")) sprintf(GetSetPrefix,cgi["GetSetPrefix"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi,"GetSaveDir")) sprintf(GetSaveDir,cgi["GetSaveDir"]->getValue().c_str());

  patternSet.Prefix = GetSetPrefix;
  patternSet.WritePatterns(GetSaveDir);
  cout << "\n TRYING TO WRITE PATS \n";

  cout << "Input prefix = " << GetSetPrefix << endl;
  cout << "Save prefix = " << patternSet.Prefix << endl;
  cout << "Save Dir = " << GetSaveDir << endl;

  patternSet.Prefix = "default";	// return to default settings
  this->Default(in, out);
  return;
}

void CSCGEMTestApplication::SaveAsTxt(xgi::Input * in, xgi::Output * out)
{
  this->Default(in, out);
  return;
}


void CSCGEMTestApplication::SaveCurrentSet(xgi::Input * in, xgi::Output * out)
{
  this->Default(in, out);
  return;
}

void CSCGEMTestApplication::ClearEmuBoard(xgi::Input * in, xgi::Output * out)
{
    using namespace std;
    cgicc::Cgicc cgi(in);
    cw::Set emptySet;
    if(emptySet.WritePatterns()){
        if( emptySet.LoadEmuBoard() ){
                cout << "Successfully Loaded to EmuBoard!\n";
        }
        else{
                cout << "ERROR: FAILED TO LOAD EmuBoard!!!!!!!\n";
        }
    }
    
    patternSet.DeleteCurrentSet();

    this->Default(in, out);
    return;
}

void CSCGEMTestApplication::ClearSet(xgi::Input * in, xgi::Output * out)
{
  patternSet = cw::Set();
  patternSet.DeleteCurrentSet();

  sprintf(GetSetPrefix, patternSet.Prefix.c_str());
  sprintf(GetSaveDir, cw::default_dir.c_str());

  this->Default(in,out);
  return;
}


void CSCGEMTestApplication::RunStudy(xgi::Input * in, xgi::Output * out)
{
  cgicc::Cgicc cgi(in);

  if(xgi::Utils::hasFormElement(cgi,"GetNtrials_char")) sprintf(GetNtrials_char,cgi["GetNtrials_char"]->getValue().c_str());

  int clct0_lsbs, clct1_lsbs, clct_msbs;
  int CLCT0_data_, CLCT1_data_;
  TMB * thisTMB = tmbVector[1];				/// THIS SHOULD BE EDITED IF USING DIFFERENT TMB SETTINGS !!!!!!!!

  GetNtrials_int = atoi(GetNtrials_char);
  patternSet.N_trials += GetNtrials_int;
  for(int i=0; i < GetNtrials_int; i++){
    patternSet.Dump();
    clct0_lsbs = thisTMB->ReadRegister(seq_clct0_adr);
    clct1_lsbs = thisTMB->ReadRegister(seq_clct1_adr);
    clct_msbs  = thisTMB->ReadRegister(seq_clctm_adr);
  
    CLCT0_data_ = ( (clct_msbs & 0xf) << 16 ) | (clct0_lsbs & 0xffff);
    CLCT1_data_ = ( (clct_msbs & 0xf) << 16 ) | (clct1_lsbs & 0xffff);
    
    int CLCT0_nhit = thisTMB->ExtractValueFromData(CLCT0_data_, CLCT0_nhit_bitlo, CLCT0_nhit_bithi);
    int CLCT0_pid = thisTMB->ExtractValueFromData(CLCT0_data_, CLCT0_pattern_bitlo, CLCT0_pattern_bithi);
    int CLCT0_key = thisTMB->ExtractValueFromData(CLCT0_data_, CLCT0_keyHalfStrip_bitlo, CLCT0_keyHalfStrip_bithi);
    
    std::cout << std::endl << "CLCT 0 Decode:" << std::endl;
    std::cout << "N Hit = " << CLCT0_nhit << std::endl;
    std::cout << "Pid   = " << CLCT0_pid << std::endl;
    std::cout << "KeyStr= " << CLCT0_key << std::endl;

    thisTMB->GetCounters();

    int CLCT1_nhit = thisTMB->ExtractValueFromData(CLCT1_data_, CLCT1_nhit_bitlo, CLCT1_nhit_bithi);
    int CLCT1_pid = thisTMB->ExtractValueFromData(CLCT1_data_, CLCT1_pattern_bitlo, CLCT1_pattern_bithi);
    int CLCT1_key = thisTMB->ExtractValueFromData(CLCT1_data_, CLCT1_keyHalfStrip_bitlo, CLCT1_keyHalfStrip_bithi);

    int clct0_inc = thisTMB->GetCounter(cw::tmb_counters[0]) - CLCT0_Counter;
    int clct1_inc = thisTMB->GetCounter(cw::tmb_counters[1]) - CLCT1_Counter;

    std::cout << "CLCT 1 Decode:" << std::endl;
    std::cout << "N Hit = " << CLCT1_nhit << std::endl;
    std::cout << "Pid   = " << CLCT1_pid << std::endl;
    std::cout << "KeyStr= " << CLCT1_key << std::endl;

    std::cout << "Increment on CLCT0 = " << clct0_inc << "    " << CLCT0_Counter << "   " << thisTMB->GetCounter(cw::tmb_counters[0]) << std::endl;
    std::cout << "Increment on CLCT1 = " << clct1_inc << "    " << CLCT1_Counter << "   " << thisTMB->GetCounter(cw::tmb_counters[1]) << std::endl;

    CLCT0_Counter = thisTMB->GetCounter(cw::tmb_counters[0]);//thisTMB->GetCounter(cw::tmb_counters[0]);
    CLCT1_Counter = thisTMB->GetCounter(cw::tmb_counters[1]);//thisTMB->GetCounter(cw::tmb_counters[1]);


    //cw::TMBresponse thisTrial = cw::TMBresponse(CLCT0_data_, CLCT1_data_, 0);
    cw::TMBresponse_long thisTrial = cw::TMBresponse_long(CLCT0_nhit, CLCT0_pid, CLCT0_key, CLCT1_nhit, CLCT1_pid, CLCT1_key, clct0_inc, clct1_inc);
    bool match = false;
    for(int i=0; i < patternSet.Results.size(); i++){
      if(thisTrial == patternSet.Results[i]){
         ++patternSet.Results[i];
	 match = true;
         break;
      }
    }
    if(!match){ patternSet.Results.push_back(thisTrial); }
    
    //thisTMB->ResetCounters();
  }

  std::cout << "Took this many TMB Dumps: " << GetNtrials_int << std::endl;
	
  this->Default(in,out);
  return;
}

void CSCGEMTestApplication::AddCLCTParamScan(xgi::Input * in, xgi::Output * out)
{
  cgicc::Cgicc cgi(in);

  if(xgi::Utils::hasFormElement(cgi,"GetCLCT_bx_ps_char")) sprintf(GetCLCT_bx_ps_char,cgi["GetCLCT_bx_ps_char"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi,"GetCLCT_key_ps_char")) sprintf(GetCLCT_key_ps_char,cgi["GetCLCT_key_ps_char"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi,"GetCLCT_pid_ps_char")) sprintf(GetCLCT_pid_ps_char,cgi["GetCLCT_pid_ps_char"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi,"GetCLCT_nhit_ps_char")) sprintf(GetCLCT_nhit_ps_char,cgi["GetCLCT_nhit_ps_char"]->getValue().c_str());

  GetCLCT_ps_char[0] = GetCLCT_bx_ps_char;
  GetCLCT_ps_char[1] = GetCLCT_key_ps_char;
  GetCLCT_ps_char[2] = GetCLCT_pid_ps_char;
  GetCLCT_ps_char[3] = GetCLCT_nhit_ps_char;

  std::vector<int> clct_info;

  for(int i=0; i < GetCLCT_ps_char.size(); i++){
    std::istringstream ss;
    ss.str(GetCLCT_ps_char[i]);

    cw::RangeParam range;
    range.param = i;
    range.clct = CLCT_ps_vec.size();
    range.step_size = 1;

    if(ss.str().empty()){ 
        cout << "empty" << endl;
        clct_info.push_back(0);
    }
    else if(ss.str().at(0) == '['){  
      cout << "We are reading a RANGED VALUE" << endl << endl; 
      int a,b;
      char tmp;
      ss >> tmp >> a >> tmp >> b >> tmp;
      cout << "These are the vals a= " << a << " b= " << b << endl << endl;
      range.min = a;
      range.max = b;
      Free_params.push_back(range);
      clct_info.push_back(a);
    }
    else{
      cout << "We are reading a SINGLE VALUE" << endl << endl;
      int a;
      ss >> a;
      clct_info.push_back(a);
      cout << "The Value is a = " << a << endl << endl;
    }
  }

  CLCT_ps_vec.push_back(cw::CLCT(clct_info[0],clct_info[1],clct_info[2],clct_info[3]));

  this->Default(in,out);
  return;
}

void CSCGEMTestApplication::RunParamScan(xgi::Input * in, xgi::Output * out)
{
  this->Default(in,out);
  return;
} 


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSCGEMTestApplication::GenerateDCFEBData(xgi::Input * in, xgi::Output * out )
{
    cgicc::Cgicc cgi(in);

    char DirGen[200];
    char DCFEBN[10];
    char HalfStripN[10];

    //bits = (char*)calloc(size,sizeof(char));
    char bits[4096];

    if (xgi::Utils::hasFormElement(cgi, "DirGen")) sprintf(DirGen,cgi["DirGen"]->getValue().c_str());
    if(xgi::Utils::hasFormElement(cgi, "DCFEBN")) sprintf(DCFEBN,cgi["DCFEBN"]->getValue().c_str());
    if(xgi::Utils::hasFormElement(cgi, "HalfStripN")) sprintf(HalfStripN,cgi["HalfStripN"]->getValue().c_str());

    int HalfStrip = atoi(HalfStripN);
    int diStrip = (HalfStrip-1)/4+1;
    int strip = (HalfStrip-(diStrip-1)*4-1)/2;
    int halfStrip = HalfStrip-(diStrip-1)*4-strip*2-1;

    for (int j = 1; j < 8; j++) {
        char filename[200];
        char strj[10];
        sprintf(strj, "%d", j);
        strcpy(filename,DirGen);
        strcat(filename,"/DCFEB_");
        strcat(filename,strj);
        strcat(filename,".pat");
    
        FILE *outfile = fopen(filename,"w");

        if (j == atoi(DCFEBN)) {
            for (int i = 0; i < 6; i++) {
                bits[i] = 0xff & (int)pow(2,8-diStrip);
            }
            for (int i = 6; i < 12; i++) {
                bits[i] = 0xff & (int)pow(2,8-diStrip)*strip;
            }
            for (int i = 13; i < 18; i++) {
                bits[i] = 0xff & (int)pow(2,8-diStrip)*halfStrip;
            }
        }
        else {
            for (int i = 0; i < 18; i++) {
                bits[i] = 0x00;
            }
        }
        for (int i = 18; i < 4096; i++) {
            bits[i] = 0x00;
        }
        fwrite(bits, sizeof(char), sizeof(bits), outfile);
        fclose(outfile);
    }
    this->Default(in, out);
}

void CSCGEMTestApplication::UploadDCFEBData(xgi::Input * in, xgi::Output * out )
{
    using namespace std;



    cgicc::Cgicc cgi(in);
    //char DirUp[200];
    int pbase = 2816;

    if(xgi::Utils::hasFormElement(cgi, "DirUp")) sprintf(DirUp,cgi["DirUp"]->getValue().c_str());

    ofstream fs;
    fs.open("s1.txt"); fs << "a"; fs.close();
    fs.open("s2.txt"); fs << "a"; fs.close();
    fs.open("s3.txt"); fs << "a"; fs.close();
    fs.open("s4.txt"); fs << "a"; fs.close();
    fs.open("s5.txt"); fs << "a"; fs.close();
    fs.open("s6.txt"); fs << "a"; fs.close();
    fs.open("s7.txt"); fs << "a"; fs.close();

    for (int j = 1; j < 8; j++) {
        char filename[200];
        char strj[10];
        sprintf(strj, "%d", j);
        strcpy(filename,DirUp);
        strcat(filename,"/DCFEB_");
        strcat(filename,strj);
        strcat(filename,".pat");

        char block[RAMPAGE_SIZE];
        fread(block, sizeof(char), RAMPAGE_SIZE, fopen(filename,"r"));
        memcpy(wdat,block,RAMPAGE_SIZE);

        eth_open(EmuBoardLocation);
        eth_reset();

        int e1 = write_command(7,pbase+j, block);
        char* pkt;
        int e2 = read_command(&pkt);

        char out[10];

        strcpy(out,"s");
        strcat(out,strj);
        strcat(out,".txt");
        fs.open(out);
        if (e1 == 0 && e2 == 7) fs << "0";
        else fs << "1";
        fs.close();

        eth_close();
    }


    
    this->Default(in, out);
}

void CSCGEMTestApplication::CheckDCFEBData(xgi::Input * in, xgi::Output * out )
{
    using namespace std;

    int pbase = 2816;

    ofstream fs;
    fs.open("c1.txt"); fs << "a"; fs.close();
    fs.open("c2.txt"); fs << "a"; fs.close();
    fs.open("c3.txt"); fs << "a"; fs.close();
    fs.open("c4.txt"); fs << "a"; fs.close();
    fs.open("c5.txt"); fs << "a"; fs.close();
    fs.open("c6.txt"); fs << "a"; fs.close();
    fs.open("c7.txt"); fs << "a"; fs.close();

    for (int j = 1; j < 8; j++) {
        char strj[10];
        sprintf(strj, "%d", j);

        eth_open(EmuBoardLocation);
        eth_reset();
        int e1 = write_command(3,pbase+j);
        char* pkt;
        int e2 = read_command(&pkt);

        char out[10];
        strcpy(out,"c");
        strcat(out,strj);
        strcat(out,".txt");
        fs.open(out);
        if (e1 == 0 && e2 == -3) fs << "0";
        else fs << "1";
        fs.close();

        eth_close(); 
    }

    this->Default(in, out);
}

void CSCGEMTestApplication::TransmitDCFEBData(xgi::Input * in, xgi::Output * out )
{
    this->Default(in, out);
}

void CSCGEMTestApplication::ReadOutTriggerResults(xgi::Input * in, xgi::Output * out )
{
    this->Default(in, out);
}

void CSCGEMTestApplication::F0(xgi::Input * in, xgi::Output * out )
{
	eth_open("/dev/schar1");
	eth_reset();
	int e1 = write_command(0);
	char* pkt;
	int e2 = read_command(&pkt);
	eth_close();

	if(e2 > 0)
		dumphex(e2, pkt, &std::cout);

    this->Default(in, out);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CSCGEMTestApplication::GenGEMPattxtFileMounN(xgi::Input * in, xgi::Output * out )
{
  using namespace std;

  cgicc::Cgicc cgi(in);
  if(xgi::Utils::hasFormElement(cgi,"GemtxtMounNchar")) sprintf(GemtxtMounNchar,cgi["GemtxtMounNchar"]->getValue().c_str());
  int num_mouns = atoi(GemtxtMounNchar);
  std::cout << "------------this is the num_mouns var output: " << num_mouns << "\n";
  std::cout << "------------this is the num_mouns char output: " << GemtxtMounNchar << "\n";

  this->Default(in, out);
}

void CSCGEMTestApplication::GenGEMPattxtFile(xgi::Input * in, xgi::Output * out )
{
  using namespace std;
  cgicc::Cgicc cgi(in);

  for (int i = 0; i < 512; ++i) {
    string number;          // string which will contain the result
    std::ostringstream convert;   // stream used for the conversion
    convert << i;      // insert the textual representation of 'Number' in the characters in the stream
    number = convert.str();
    string tempstring = "GemBx" + number;
    if(xgi::Utils::hasFormElement(cgi,tempstring)) sprintf(testingarray[i],cgi[tempstring]->getValue().c_str());
  }
  this->Default(in, out);
}

void CSCGEMTestApplication::YuriyPatConvert(xgi::Input * in, xgi::Output * out )
{
  using namespace std;
  cgicc::Cgicc cgi(in);

  if(xgi::Utils::hasFormElement(cgi, "DirYuriyConvtxt")) sprintf(DirYuriyConvtxt,cgi["DirYuriyConvtxt"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "YuriyConvtxtFile")) sprintf(GEMtxtFile,cgi["YuriyConvtxtFile"]->getValue().c_str());
  char filename[400];

  strcpy(filename,DirYuriyConvtxt);
  strcat(filename,"/");
  strcat(filename,YuriyConvtxtFile);
  std::cout << "this is the location of the GEM txt file " << filename << endl;
  std::cout << "this is the location of the GEM Pat Dir " << DirYuriyConvtxt << endl;
//  PatternConvert PatConv;
//  PatConv.Pattern_Converter(filename);
  std::cout << "the pattern file has been created." << std::endl;

  this->Default(in, out);
}

void CSCGEMTestApplication::Yuriy_CLCT_PatternConverter(xgi::Input * in, xgi::Output * out )
{
  using namespace std;
  cgicc::Cgicc cgi(in);


  if(xgi::Utils::hasFormElement(cgi, "DirYuriyCLCTConvtxt")) sprintf(DirYuriyCLCTConvtxt,cgi["DirYuriyCLCTConvtxt"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "DirYuriyCLCTConvPat")) sprintf(DirYuriyCLCTConvPat,cgi["DirYuriyCLCTConvPat"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "YuriyCLCTConvtxtFile")) sprintf(YuriyCLCTConvtxtFile,cgi["YuriyCLCTConvtxtFile"]->getValue().c_str());

  char filename[400];

  strcpy(filename,DirYuriyCLCTConvtxt);
  strcat(filename,"/");
  strcat(filename,YuriyCLCTConvtxtFile);

  std::cout << "this is the location of the Yuriy CLCT txt file " << filename << endl;
  std::cout << "this is the location of the Yuriy CLCT Pat Dir " << DirYuriyCLCTConvPat << endl;

//  Yuriy_CLCT_PatternGen YuriyCLCTPat;
  std::cout << "at least it will create the object \n";
//  YuriyCLCTPat.Yuriy_CLCT_PatternGenerator(filename,DirYuriyCLCTConvPat);
  std::cout << "the Yuriy CLCT Patter has been created." << std::endl;

  this->Default(in, out);
}

void CSCGEMTestApplication::GemPatternConverter(xgi::Input * in, xgi::Output * out )
{
    using namespace std;
    cgicc::Cgicc cgi(in);

   	if(xgi::Utils::hasFormElement(cgi, "DirGEMPat")) sprintf(DirGEMPat,cgi["DirGEMPat"]->getValue().c_str());
   	if(xgi::Utils::hasFormElement(cgi, "DirGEMtxt")) sprintf(DirGEMtxt,cgi["DirGEMtxt"]->getValue().c_str());
  	if(xgi::Utils::hasFormElement(cgi, "GEMtxtFile")) sprintf(GEMtxtFile,cgi["GEMtxtFile"]->getValue().c_str());

   	char filename[400];

    strcpy(filename,DirGEMtxt);
    strcat(filename,"/");
    strcat(filename,GEMtxtFile);

	std::cout << "this is the location of the GEM txt file " << filename << endl;
	std::cout << "this is the location of the GEM Pat Dir " << DirGEMPat << endl;

//    GEMPatternGen GemPat;
//    GemPat.GEMPatternGenerator(filename,DirGEMPat);
    std::cout << "at least it works up to here TrolololOLOLOLOOlol9oLOLoLOlol \n";

    std::cout << "the GEM Patter has been created." << std::endl;


  this->Default(in, out);



}


void CSCGEMTestApplication::CfebPatternConverter(xgi::Input * in, xgi::Output * out )
{
  using namespace std;
  cgicc::Cgicc cgi(in);

  if(xgi::Utils::hasFormElement(cgi, "DirCfebPat")) sprintf(DirCfebPat,cgi["DirCfebPat"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "DirCfebtxt")) sprintf(DirCfebtxt,cgi["DirCfebtxt"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "CfebPatFile")) sprintf(CfebPatFile,cgi["CfebPatFile"]->getValue().c_str());
  if(xgi::Utils::hasFormElement(cgi, "CfebtxtFile")) sprintf(CfebtxtFile,cgi["CfebtxtFile"]->getValue().c_str());

  char filename[400];

  strcpy(filename,DirCfebtxt);
  strcat(filename,"/");
  strcat(filename,CfebtxtFile);

  std::cout << "this is the location of the Cfeb txt file " << filename << endl;
  std::cout << "this is the location of the Cfeb Pat Dir " << DirGEMPat << endl;

  //PatternGen CfebPat;
  std::cout << "at least it will create the object \n";
  //CfebPat.PatternGenerator(filename,DirCfebPat);
  std::cout << "the Cfeb Patter has been created." << std::endl;

  this->Default(in, out);
}

void CSCGEMTestApplication::LoadToEmuBoard(xgi::Input * in, xgi::Output * out )
{
    using namespace std;
    cgicc::Cgicc cgi(in);

    patternSet.DeleteCurrentSet();

    if(patternSet.WritePatterns()){
	if( patternSet.LoadEmuBoard() ){
		cout << "Successfully Loaded to EmuBoard!\n";
	}
	else{
		cout << "ERROR: FAILED TO LOAD EmuBoard!!!!!!!\n";
	}
    }


  TMB * thisTMB = tmbVector[1];
  thisTMB->ResetCounters();
  CLCT0_Counter = 0;
  CLCT1_Counter = 0;
  this->Default(in, out);



}

void CSCGEMTestApplication::SendFromEmuBoard(xgi::Input * in, xgi::Output * out )
{

    using namespace std;
    cgicc::Cgicc cgi(in);

    //char DirGen[200];
   	//char PatFile[200];
   	//char FiberN[10];
   	//char NumRuns[10];

   	if(xgi::Utils::hasFormElement(cgi, "DirGen")) sprintf(DirGen,cgi["DirGen"]->getValue().c_str());
   	if(xgi::Utils::hasFormElement(cgi, "PatFile")) sprintf(PatFile,cgi["PatFile"]->getValue().c_str());
   	if(xgi::Utils::hasFormElement(cgi, "FiberN")) sprintf(FiberN,cgi["FiberN"]->getValue().c_str());
   	if(xgi::Utils::hasFormElement(cgi, "NumRuns")) sprintf(NumRuns,cgi["NumRuns"]->getValue().c_str());




	int Runs = atoi(NumRuns);

	eth_open(EmuBoardLocation);
	int e;
	for(int i = 0; i< Runs; ++i){
		eth_reset();
        e = write_command(0xd); //send the patterns
        usleep(30);
	}
	char* pkt;
	e = read_command(&pkt);
	eth_close();


  this->Default(in, out);
}
/*
void CSCGEMTestApplication::ClearEmuBoard(xgi::Input * in, xgi::Output * out )
{
	cgicc::Cgicc cgi(in);

	//char DirGen[200];
	//char PatFile[200];
	//char FiberN[10];
	//char NumRuns[10];

	if(xgi::Utils::hasFormElement(cgi, "DirGen")) sprintf(DirGen,cgi["DirGen"]->getValue().c_str());
	if(xgi::Utils::hasFormElement(cgi, "PatFile")) sprintf(PatFile,cgi["PatFile"]->getValue().c_str());
	if(xgi::Utils::hasFormElement(cgi, "FiberN")) sprintf(FiberN,cgi["FiberN"]->getValue().c_str());
	if(xgi::Utils::hasFormElement(cgi, "NumRuns")) sprintf(NumRuns,cgi["NumRuns"]->getValue().c_str());

	char filename[200];
	sprintf(filename,"/home/cscdev/Jared/EmulatorBoard/TriDAS_BoardPlus/emu/emuDCS/CSCGEMTestStand/tmp/Empty.pat");

	for (int pageid = 0; pageid < 8; ++pageid) {
		char block[RAMPAGE_SIZE];
		FILE* infile = fopen(filename,"r");
		fread(block, sizeof(char), RAMPAGE_SIZE, infile);
		memcpy(wdat,block,RAMPAGE_SIZE);
		eth_open(EmuBoardLocation);
		eth_reset();
		int e = write_command(7,pageid, block);
		eth_close();
	}
	for (int i = 0; i < 8; ++i) {
	    sprintf(FileNameFiberN[i],"/home/cscdev/Jared/EmulatorBoard/TriDAS_BoardPlus/emu/emuDCS/CSCGEMTestStand/tmp/Empty.pat");
	}

  this->Default(in, out);
}
*/
void CSCGEMTestApplication::OutputCLCTInfo(xgi::Input * in, xgi::Output * out )
{
/*

cgicc::Cgicc cgi(in);
//
cgicc::form_iterator name = cgi.getElement("tmb");
int tmb;
if(name != cgi.getElements().end()) {
  tmb = cgi["tmb"]->getIntegerValue();
  std::cout << "TMBStatus:  TMB=" << tmb << std::endl;
  TMB_ = tmb;
} else {
  std::cout << "TMBStatus: No TMB" << std::endl ;
  tmb = TMB_;
}
//
TMB * thisTMB = tmbVector[tmb];
std::cout << "the value for tmb is = " << tmb << std::endl;
std::cout << "The tmbVector[tmb] = " << tmbVector[tmb] << std::endl;
*/

this->Default(in, out);
}

void CSCGEMTestApplication::OutputMPCFifo(xgi::Input * in, xgi::Output * out )
{
/*

cgicc::Cgicc cgi(in);
//
cgicc::form_iterator name = cgi.getElement("tmb");
int tmb;
if(name != cgi.getElements().end()) {
  tmb = cgi["tmb"]->getIntegerValue();
  std::cout << "TMBStatus:  TMB=" << tmb << std::endl;
  TMB_ = tmb;
} else {
  std::cout << "TMBStatus: No TMB" << std::endl ;
  tmb = TMB_;
}
//
TMB * thisTMB = tmbVector[tmb];
std::cout << "the value for tmb is = " << tmb << std::endl;
std::cout << "The tmbVector[tmb] = " << tmbVector[tmb] << std::endl;
*/

this->Default(in, out);
}
/*// refrence
void EmuPeripheralCrateConfig::TMBPrintCounters(xgi::Input * in, xgi::Output * out )
  throw (xgi::exception::Exception) {
  //
  std::cout << "TMBPrintCounters "<< std::endl;
  //
  cgicc::Cgicc cgi(in);
  //
  cgicc::form_iterator name = cgi.getElement("tmb");
  //
  int tmb=0;
  if(name != cgi.getElements().end()) {
    tmb = cgi["tmb"]->getIntegerValue();
    std::cout << "TMB " << tmb << std::endl;
    TMB_ = tmb;
  }
  //
  TMB * thisTMB = tmbVector[tmb];
  //
  LOG4CPLUS_INFO(getApplicationLogger(), "Start PrintCounters");
  thisTMB->RedirectOutput(&OutputStringTMBStatus[tmb]);
  thisTMB->GetCounters();
  thisTMB->PrintCounters();
  thisTMB->RedirectOutput(&std::cout);
  LOG4CPLUS_INFO(getApplicationLogger(), "Done PrintCounters");
  //
  this->TMBUtils(in,out);
  //
}


  //
  *out << cgicc::td().set("ALIGN","left");
  std::string TMBPrintCounters = toolbox::toString("/%s/TMBPrintCounters",getApplicationDescriptor()->getURN().c_str());
  *out << cgicc::form().set("method","GET").set("action",TMBPrintCounters) ;
  *out << cgicc::input().set("type","submit").set("value","Print TMB Counters") ;
  sprintf(buf,"%d",tmb);
  *out << cgicc::input().set("type","hidden").set("value",buf).set("name","tmb");
  *out << cgicc::form() ;
  *out << cgicc::td();
  //



*/


xdaq::Application * CSCGEMTestApplication::instantiate(xdaq::ApplicationStub * stub )
throw (xdaq::exception::Exception)
{
  return new CSCGEMTestApplication(stub);
}

}}  // namespaces

// factory instantion of XDAQ application
//XDAQ_INSTANTIATOR_IMPL(emu::pc::CSCGEMTestApplication)












