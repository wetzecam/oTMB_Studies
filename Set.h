#include "CLCT.h"


namespace cw {

	// 		Container for all Macro Pattern data (CLCTs and GEM Clusters)
	//				to be used for managing, facilitating and storing results of a single Pattern Test.
	class Set
	{
		std::vector<int>  		Stats;		// 0: Valid			1: Wrong # of LCTs		2: Wrong PID		3: Wrong LAYER_SET		4: Wrong HS
		int 									N_trials; //
		char									Tmb_type;
		std::string 					Firmware;

		public:
		std::string						Prefix;
		std::vector<CLCT> 		CSC;
		std::vector<Cluster> 	GEM;

		Set(void);						//	Creates Empty Set
		Set(std::string&);		//  Creates Set from file located @ arg_path

		bool WritePatterns(std::string opt_path = "");			// Creates (.pat) files
		bool LoadEmuBoard(void);			// Loads set of {.pat} files with prefix match -> EmuBoard

		void DumpTMB(void);						// Dumps Contents of EmuBoard -> oTMB
		void CheckTMB(void);					// Compares oTMB results with this->Set's contents

    void ViewContents(std::ostream&);
		void SaveResults(std::string opt_path = "");				// Saves Results of Current Experiment to a data file

	};
}
