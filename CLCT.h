#ifndef CLCT_H
#define CLCT_H
#include <cstdlib>
#include <fstream>
#include <string.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <bitset>
#include <iomanip>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <vector>
//#include "TaoFunc.h"
//#include "CSCConstants.h"
/// TODO
//		--.pat output function
//		--Add Overlapping Hit Checking

#define COMPILE_TYPE 0xc
#define LAYERS 6
#define NUM_CFEB 7
#define STRIPS_CFEB 32			// number halfstrips in cfeb
#define RAMPAGE_SIZE 4*1024
#define LAYER_SET 1
#define CLUSTER_BITS 14
#define GEM_FIBERS 4


namespace cw {


	//	Structures & Classes
	struct Hit
	{
		public:
		// Data Members
		int bx;
		int hs;
		int lay;

		// Constructors
		Hit(void);
		Hit(int Bx, int Hs, int Layer);
    friend std::ostream& operator << (std::ostream& os, const Hit& hit);
		friend std::istream& operator >> (std::istream& is, Hit& hit);
	};
	//  CSC Macro Object
	class CLCT
	{
		public:
		// Data Members
		int bx;
		int hs;
		int pat;
		int nhits;
		std::vector<Hit> hits;

		// Constructors
		CLCT(void);
		CLCT(int, int, int, int);
		CLCT(int, int, int, std::vector<Hit>&);
		CLCT(const CLCT& clct);

		void RelativeSpaceTime(void);		// Sets bx, hs params of <hits> relative (+-) this->bx
		void NormalSpaceTime(void);			// 	undo the above operation

		// Operators
		//CLCT& operator = (const CLCT&);
		friend std::ostream& operator << (std::ostream&, const CLCT&);
		friend std::istream& operator >> (std::istream&, CLCT&);
	};
	//   CSC object for writing (.pat)
	class Group
	{
		public:
		std::vector<unsigned char> hexdata;

		Group(void);
		Group(std::vector<Hit>&, int);

		void addHit(Hit&, int);
		friend std::ostream& operator << (std::ostream&, const Group&);
	};
	//   GEM Macro object
	class Cluster
	{
		public:
		// Data Members	  Range
		int bx;		// 0-500
		int roll;	// 1-8
		int pad;	// 0-383
		int size;	// 1-8
		int layer;	// 1-2

		Cluster(void);
		Cluster(int, int, int, int, int);
		Cluster(const Cluster&);

		friend std::ostream& operator<<(std::ostream&, const Cluster&);
		friend std::istream& operator>>(std::istream&, Cluster&);

		unsigned int info(void);
	};
	//    GEM object for writing (.pat)
	struct gemPacket {
		int				num_clusters;
		std::vector<unsigned int>	raw_info;

		gemPacket(void);
		gemPacket(std::vector<Cluster>& iClu,	unsigned int sInd = 0);

		friend std::ostream& operator<<(std::ostream&, const gemPacket&);	// writes 8 bytes (i.e. 1 bx of gem data for a single channel)
	};




	// Positioning Functions
	int GetCFEB(int hs);		//	Out:	Cfeb given half strip
	int GetLocal(int hs);		//	Out:	Halfstrip relative to CFEB

	// Text File Parsers / Writers (human readable)
	int ReadTxt(std::string&, std::vector<CLCT>&);					// input : file prefix ONLY
	std::string ReadTxt(std::string&, std::vector<CLCT>&, std::vector<Cluster>&);	// input : (file prefix) + ".txt"	GEM Capable!!

	void WriteTxt(std::string&, std::vector<CLCT>&);

	// Writes Patterns (.pat) to be loaded to EmuBoard
	bool WritePat(std::string&, std::vector<CLCT>&);	// CSC
	bool WritePat(std::string&, std::vector<Cluster>&);	// GEM

	// For Sorting purposes
	bool compareHit(Hit, Hit);
	bool compareCLCT(CLCT, CLCT);

	void ExtractHits(std::vector<CLCT>& clcts, std::vector<Hit>& hits, int feb = -1);
	void CollectClusters(std::vector<Cluster>& pads, std::vector<Cluster>* in_pads, int layer, int gem_fiber = -1);

	// Print object data to console
	void DumpGroup(Group grp, int Bx);
	void DumpHit(Hit&);

	void PrintCLCT(CLCT&, std::ostream& oss = (std::cout));

}
#endif
