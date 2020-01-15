#include "CLCT.h"
//#include "emu/pc/TMB.h"
//#include "eth_lib.h"
//#include "commands.h"
//#include "utils.h"
//#include "CSCConstants.h"

namespace cw {

        // This is a container for the oTMB's response during a pattern Study
	struct TMBresponse{
                int CLCT_0;     // CLCT0 register contents : HEX
                int CLCT_1;     // CLCT1 register contents : HEX
                int delta_LCT;   // increment of LCT counter: int
		
		int occurCount;	// Tally of how many times this response observed

                TMBresponse(int clct_0, int clct_1, int delta_lct, int N_occr=1); //constructor

		void operator++();
                friend bool operator==(const TMBresponse& a, const TMBresponse& b);
                friend std::ostream& operator<<(std::ostream&, const TMBresponse&);	// To be used in File Writing
                friend std::istream& operator>>(std::istream&, TMBresponse&);		// To be used in File Reading
		
        };
	
	// This serves the same purpose as the previous... experimenting with less compact data storage
	struct TMBresponse_long{
		int CLCT_nhit_0;     // CLCT0 register contents : # Hits
                int CLCT_pid_0;
		int CLCT_key_0;

		int CLCT_nhit_1;     // CLCT1 register contents : # Hits
		int CLCT_pid_1;
		int CLCT_key_1;

                int delta_LCT;   // increment of LCT counter: int
                int occurCount; // Tally of how many times this response observed

                TMBresponse_long(int clct_nhit_0, int clct_pid_0, int clct_key_0, int clct_nhit_1, int clct_pid_1, int clct_key_1, int delta_lct, int N_occr=1); //constructor

                void operator++();
                friend bool operator==(const TMBresponse_long& a, const TMBresponse_long& b);
                friend std::ostream& operator<<(std::ostream&, const TMBresponse_long&);     // To be used in File Writing
                friend std::istream& operator>>(std::istream&, TMBresponse_long&);           // To be used in File Reading
	};
	
	// 		Container for all Macro Pattern data (CLCTs and GEM Clusters)
	//				to be used for managing, facilitating and managing results of a single Pattern Study.
	class Set
	{
		
		char				Tmb_type;
		std::string 			Firmware;

		public:
		int 				N_trials;
		std::string			Prefix;
		std::vector<CLCT> 		CSC;
		std::vector<Cluster> 		GEM;
		std::vector<int>                Stats;          // 0: Valid  1: Wrong # of LCTs  2: Wrong PID  3: Wrong LAYER_SE  4: Wrong HS
		//std::vector<TMBresponse>	Results;		
		std::vector<TMBresponse_long>	Results;	

		Set(void);			//  Creates Empty Set
		Set(std::string&);		//  Creates Set from file located @ arg_path

		bool AddCLCT(CLCT);		// Adds CLCT to pattern Set
		bool AddGEM(Cluster);		// Adds GEM Cluster to pattern Set

		bool RemoveCLCT(int);		// Removes CLCT from set
		bool RemoveGEM(int);		// Removes GEM from set

		bool WritePatterns(std::string opt_path = "/home/cscdev/cameron/pats/tmp/");		// Creates (.pat) files
		bool LoadEmuBoard(std::string opt_path = "/home/cscdev/cameron/pats/tmp/");		// Loads set of {.pat} files with prefix match -> EmuBoard
		void ReadEmuBoard(void);
		void ClearEmuBoard(void);

		void Dump(char opt = 'o');					// Dumps Contents of EmuBoard -> oTMB
		void CheckTMB(void);						// Compares oTMB results with this->Set's contents

		void ClassifyResults(char mode = 'a');

		void ViewContents(std::ostream& oss = std::cout);
		void SaveResults(std::string opt_path = "");			// Saves Results of Current Experiment to a data file

	};
	
}
