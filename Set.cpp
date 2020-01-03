#include "Set.h"

namespace cw {
using namespace std;


//~~~~~~~~~ Overall Sets ~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Set::Set(void){
	Stats = {0, 0, 0, 0, 0};
	N_trials = 0;
	Tmb_type = COMPILE_TYPE - 0xa + 'a';
	Firmware = "XXX";
}

Set::Set(std::string& path)
{
	Stats = {0, 0, 0, 0, 0};
	N_trials = 0;
	Tmb_type = COMPILE_TYPE - 0xa + 'a';
	Firmware = "XXX";
	Prefix = ReadTxt(path, CSC, GEM);
  sort(CSC.begin(), CSC.end(), compareCLCT);
}

bool Set::WritePatterns(std::string opt_path)
{
	if(opt_path.empty()){
		return (WritePat(Prefix, CSC)) && (WritePat(Prefix, GEM));
	}
	return (WritePat(opt_path, CSC)) && (WritePat(opt_path, GEM));
}

bool Set::LoadEmuBoard()
{
	return false;		// ****************************************************
}

void Set::DumpTMB()
{
	return;					// ****************************************************
}

void Set::CheckTMB()
{
	N_trials++;
	if(N_trials % 50 == 0){Stats[1]++;}
	else{Stats[0]++;}
	return;					//  ***************************************************
}

void Set::ViewContents(std::ostream& oss)
{
  std::cout << "******************************************************\n"
            << "                   Muon Tracks in Set\n"
            << "******************************************************\n";
  for(CLCT cl : CSC){
    std::cout << '\n';
    PrintCLCT(cl);
  }

  std::cout << "\n******************************************************\n"
            << "******************************************************\n";
  return;
}

void Set::SaveResults(std::string opt_path)
{
	if(opt_path.empty()){
		opt_path = Prefix + ".log";
	}

	std::fstream ofs(opt_path.c_str(), std::ios_base::out);

	// Header
	{	// Record Time of Experiment
		time_t rawtime;
		struct tm * timeinfo;
		time( &rawtime );
		timeinfo = localtime( &rawtime );

		ofs << "Date: " << asctime(timeinfo);
		ofs << "TMB Type: " << Tmb_type << std::endl;
		ofs << "Firmware Vers.: " << Firmware << std::endl << std::endl;
	}

	ofs << "# Trials  : \t\t" << N_trials << std::endl;
	ofs << "# Success : \t\t" << Stats[0] << std::endl;
	ofs << "# Errors  : \t\t" << N_trials - Stats[0] << std::endl << std::endl;

	int N_err = 0;
	for(int i=1; i < Stats.size(); i++){
		ofs << "Error " << i << ": " << Stats[i] << std::endl;
		N_err += Stats[i];
	}

	if(N_err != (N_trials - Stats[0])){
		ofs << "\nSomething Went Wrong!\nCorrect # Errors = " << N_err << std::endl;
	}

	ofs << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	ofs << "~~~~~~~~~~Original Pattern Set~~~~~~~~~~~~~~~~\n";
	ofs << "prefix: " << Prefix << std::endl;
	ofs << "\nCSC\nbx\tkey\tpid\tNhit\t\t{ (bx,ly,hs) }\n";

	for(CLCT cl : CSC){
    cl.RelativeSpaceTime();
		ofs << cl << std::endl;
	}

	ofs << "\nGEM\nbx\troll\tpad\tsize\tlayer\n";
	for(Cluster cl : GEM){
		ofs << cl << std::endl;
	}

	ofs.close();
	return;
}


}
