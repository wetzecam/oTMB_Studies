#include "CLCT.h"
#include "TaoFunc.h"
#include "CSCConstants.h"


namespace cw {
using namespace emu;
using namespace pc;

	int layerorder_all[6][6] = { {3,1,5,6,4,2},//layerorder1
						  {4,2,5,6,3,1},
						  {1,3,6,5,2,4},
						  {2,4,6,5,1,3},
						  {4,6,2,1,3,5},
						  {1,2,3,4,5,6}
	};

//~~~~~~~~~ Support Functions ~~~~~~~~~~~~
bool EdgeCheck(int key, int hs)						// used to check if generated hit is within CSC bounds
	{
		if (hs < 0 || hs > CSCConstants::NUM_HS) // falls outside hs range. hs >= 1
			return false;
		else if ((key <= 127 && hs > 127) && (COMPILE_TYPE == 0xc || COMPILE_TYPE == 0xd))	// crosses Edge of ME1/1b
			return false;
		else if ((key >= 128 && hs < 128) && (COMPILE_TYPE == 0xc || COMPILE_TYPE == 0xd))	// crosses Edge of ME1/1a
			return false;
		else if (  (hs < 0 || hs > 159)   && (COMPILE_TYPE == 0xa || COMPILE_TYPE == 0xb))	// out of range for type A & B
			return false;
		else
			return true;
	}

bool Hits_Generator(int Bx, int Key, int Pat, int Nhits, std::vector<Hit>& hits)	// used to Fill Hits vector in CLCT constructor
	{
		std::vector<bool> usedlayers(CSCConstants::NUM_LAYERS, false);				//  used layer accounting

		// Pattern validity check
		if (Pat < 2 || Pat >= CSCConstants::NUM_PATTERNS)
			return true;  				// invalid pattern types
		if (Nhits < 1 || Nhits > CSCConstants::NUM_LAYERS)
			return true;				// invalid number of hits

		std::srand(std::time(0));

		int n = 0;
		while (n != Nhits)
		{
			int layer = std::abs(std::rand()) % CSCConstants::NUM_LAYERS;				// randomly select a layer
			if (usedlayers.at(layer)) continue;
			else usedlayers.at(layer) = true;

			int hs = Key + CSCConstants::CLCTPatterns[Pat][layer];      				// selects halfstrip hit from pattern

			if (EdgeCheck(Key, hs))	hits.push_back(Hit(Bx, hs, layer));

			n++;
		}

		return false;
	}

bool compareHit(Hit h1, Hit h2)
	{
		if(h1.bx == h2.bx)	return (h1.lay < h2.lay);
		else return (h1.bx < h2.bx);
	}
bool compareCLCT(CLCT c1, CLCT c2)
	{
		if(c1.bx == c2.bx)	return (c1.hs < c2.hs);
		else return (c1.bx < c2.bx);
	}

int  GetCFEB(int hs)
	{
		return GetInputCFEBByX<STRIPS_CFEB>(hs, COMPILE_TYPE);
	}

int  GetLocal(int hs)
	{
		return GetInputXStrip<STRIPS_CFEB>(hs, COMPILE_TYPE);
	}


unsigned char GetTriadSeg(Hit hit, int i)
	{
		// Layer Staggering implemented here!
		bool stagger = (COMPILE_TYPE == 0xb) || (COMPILE_TYPE == 0xa);
		int halfStrip = ((hit.lay % 2 != 0) && stagger) ? (hit.hs + 1) : (hit.hs);	// odd layers inc by +1 if staggered
                if (halfStrip < 0) std::cerr<<"wrong halfstrip generated from hit: "<< halfStrip <<" hit "<< hit <<" compile type "<< COMPILE_TYPE << std::endl;
                //if (stagger) std::cout <<"Hit "<< hit <<" add stagger "<< halfStrip << std::endl;
		int localhs = GetLocal(halfStrip);
		unsigned char n = 1 << (localhs / 4);
		bool leftstrip = (localhs % 4 < 2);
		bool lefths = (localhs % 2 == 0);

		switch (i)
		{
		case 0:
			return n;
			break;
		case 1:
			return (leftstrip ? 0x00 : n);
			break;
		case 2:
			return (lefths ? 0x00 : n);
			break;
		default:
			// Throw some kind of error?
			return (0x00);
			break;
		}
	}

//~~~~~~~~~~ Class Structures ~~~~~~~~~~~
Hit::Hit() : bx(0), hs(0), lay(0) {}

Hit::Hit(int Bx, int Hs, int Layer) : bx(Bx), hs(Hs), lay(Layer) {}

std::ostream& operator << (std::ostream& os, const Hit& hit){
    os << '(' << hit.bx << ',' << hit.bx << ',' << hit.lay << ')' ;
    return os;
}

std::istream& operator >> (std::istream& is, Hit& hit){
	char tmp;
	int data;
	is >> tmp >> data;
	hit.bx = data;
	is >> tmp >> data;
	hit.lay = data;
	is >> tmp >> data;
	hit.hs = data;
	return is;
}


CLCT::CLCT(void) : bx(0), hs(0), pat(0), nhits(0) {}

CLCT::CLCT(int Bx, int Key, int Pat, int Nhit) :
	bx(Bx),
	hs(Key),
	pat(Pat),
	nhits(Nhit)
	{
		bool bad = Hits_Generator(bx, hs, pat, nhits, hits);
		sort(hits.begin(), hits.end(), compareHit);
	}

CLCT::CLCT(int Bx, int Key, int Pat, std::vector<Hit>& h_vec) :
	bx(Bx),
	hs(Key),
	pat(Pat)
	{
		for(int i=0; i<h_vec.size(); i++){
			h_vec[i].bx += Bx;
			h_vec[i].hs += Key;
			if(EdgeCheck(Key, h_vec[i].hs)) hits.push_back(h_vec[i]);		// No Check for duplicate laer yet :(
		}
		nhits = hits.size();
		sort(hits.begin(), hits.end(), compareHit);
	}

CLCT::CLCT(const CLCT& cl) :
	bx(cl.bx),
	hs(cl.hs),
	pat(cl.pat),
	nhits(cl.nhits)
	{
		hits.insert( hits.end(), cl.hits.begin(), cl.hits.end() );
		sort(hits.begin(), hits.end(), compareHit);

	}

void CLCT::RelativeSpaceTime(void){
	for(int i=0; i < hits.size(); i++){
		hits[i].bx -= this->bx;
		hits[i].hs -= this->hs;
	}
	return;
}
void CLCT::NormalSpaceTime(void){
	for(int i=0; i < hits.size(); i++){
		hits[i].bx += this->bx;
		hits[i].hs += this->hs;
	}
	return;
}


std::ostream& operator<<(std::ostream& os, const CLCT& cl)
	{
		os << cl.bx << '\t' << cl.hs << '\t'
			<< cl.pat << '\t' << cl.nhits << '\t';

		os << "{ ";
		for(Hit hh : cl.hits){
			os << hh << " ";
		}
		os << "}";

		return os;
	}

std::istream& operator>>(std::istream& is, CLCT& cl)
	{
		Hit hh;
		std::vector<Hit> h_vec;
		int B, K, P, N;
		is >> B >> K >> P >> N;

		char HITS = is.get();
		if(HITS == '{'){
			while(true){
				HITS = is.get();						// THIS SEEMS DANGEROUS
				if(HITS == '}') break;
				else{
					is.putback(HITS);
					is >> hh;
					h_vec.push_back(hh);
				}
			}
			cl = CLCT(B, K, P, h_vec);
		}
		else{
			is.putback(HITS);
			cl = CLCT(B, K, P, N);
		}
		return is;
	}

///~~~~~~~~~~~~ GEM Clusters ~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Cluster::Cluster(void) :
	bx(0), roll(0), pad(0), size(0), layer(0)
	{}

Cluster::Cluster(int Bx, int Roll, int Pad, int Size, int Layer) :
	bx(Bx),
	roll(Roll),
	pad(Pad),
	size(Size),
	layer(Layer)
	{}

Cluster::Cluster(const Cluster& clust) :
	bx(clust.bx),
	roll(clust.roll),
	pad(clust.pad),
	size(clust.size),
	layer(clust.layer)
	{}

std::istream& operator>>(std::istream& is, Cluster& cluster)
	{
		int Bx, Roll, Pad, Size, Layer;
		is >> Bx >> Roll >> Pad >> Size >> Layer;

		cluster = Cluster(Bx, Roll, Pad, Size, Layer);
		return is;
	}

std::ostream& operator<<(std::ostream& os, const Cluster& cl)
	{
		os << cl.bx << '\t' << cl.roll << '\t'
			<< cl.pad << '\t' << cl.size
			<< '\t' << cl.layer;

		return os;
	}

unsigned int Cluster::info(void)
	{
		unsigned int info = 0;
		info = ((size << 11) | (roll << 8) | (pad));	// Do range check before calling!
		return info;
	}


//~~~~~~ GEM Packets ~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gemPacket::gemPacket(void) : num_clusters(0) {}

gemPacket::gemPacket(std::vector<Cluster>& iClu, unsigned int sInd) {
	int Niter = ((iClu.size() - sInd) > 4) ? 4 : (iClu.size() - sInd);
	if(Niter <= 0)
		throw "Something went wrong in gemPack creation!";

	for(int i = 0; i < Niter; i++) {
		this->raw_info.push_back( iClu[sInd + i].info() );
	}

	this->num_clusters = raw_info.size();
	return;
}

std::ostream& operator<<(std::ostream& os, const gemPacket& pack)
	{
		std::vector<unsigned int> pad = pack.raw_info;

		unsigned int empty_pad =  std::pow(2, 14) - 1 ;

		//std::cout << pack.raw_info.size() << std::endl;
		//std::cout << pad.size() << std::endl;

		for (int i = 0; i < (4 - pack.raw_info.size()); i++) {
			pad.push_back( empty_pad );	// i.e. 14 1's or a blank cluster

			//std::cout << pad.size() << std::endl;
		}

		if (pad.size() != 4) return os;

		for (int byte = 0; byte < 8; byte++) {
			switch (byte) {
				case 0:
					os << char( pad[0] >> 6 );
					break;
				case 1:
					os << char( (pad[0] << 2) | (pad[1] >> 12) );
					break;
				case 2:
					os << char( pad[1] >> 4 );
					break;
				case 3:
					os << char( (pad[1] << 4) | (pad[2] >> 10) );
					break;
				case 4:
					os << char( pad[2] >> 2 );
					break;
				case 5:
					os << char( (pad[2] << 6) | (pad[3] >> 8) );
					break;
				case 6:
					os << char( pad[3] );
					break;
				case 7:
					os << char( empty_pad );
					break;
				default:
					break;
			}
			//std::cout << byte << std::endl;
		}

		return os;
	}

//~~~~~~~~~ CLCT Groups ~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Group::Group(void)// :
	//hexdata( std::vector<unsigned char>(CSCConstants::NUM_LAYERS, unsigned char(0)) )
	{
		for(int i=0; i < CSCConstants::NUM_LAYERS; i++)
			{
				hexdata.push_back(char(0));
			}
		//hexdata = std::vector<unsigned char> (CSCConstants::NUM_LAYERS, unsigned char(0));
	}


Group::Group(std::vector<Hit>& hits, int Bx) //:
	//hexdata(std::vector<unsigned char>(CSCConstants::NUM_LAYERS, unsigned char(0)))
{
	for(int i=0;i < CSCConstants::NUM_LAYERS; i++)
		{
			hexdata.push_back(char(0));
		}

	for (int i=0; i < hits.size(); i++)
	{
		int delta_t = Bx - hits[i].bx;
		if(delta_t >= 0 && delta_t < 3)
		{
			hexdata[hits.at(i).lay] = hexdata[hits.at(i).lay] | GetTriadSeg(hits[i], delta_t);
		}
	}
}


std::ostream& operator<<(std::ostream& os, const Group& grp)
	{
		for (int i = 0; i < grp.hexdata.size(); i++)
			{

                            if (COMPILE_TYPE == 0xb)//reverse all layers for type B
				os << grp.hexdata[ 6 - layerorder_all[LAYER_SET-1][i] ];	// Layer Shuffle
                            else
				os << grp.hexdata[ layerorder_all[LAYER_SET-1][i] - 1 ];	// Layer Shuffle
			}

		os << char(0) << char(0);
		return os;
	}

void DumpGroup(Group grp, int Bx)
	{
                if (COMPILE_TYPE == 0xb)
                     std::cout <<"Type B firmware, Also reverse all layers " << std::endl;
		for (int i = 0; i < grp.hexdata.size(); i++)
			{
                            if (COMPILE_TYPE == 0xb)//reverse all layers for type B
				std::cout << std::bitset<8>(grp.hexdata[6 - layerorder_all[LAYER_SET-1][i]]) << std::endl;
                            else
				std::cout << std::bitset<8>(grp.hexdata[layerorder_all[LAYER_SET-1][i] - 1]) << std::endl;
			}
		std::cout << std::bitset<8>(char(0)) << std::endl;
		std::cout << std::bitset<8>(char(0)) << "   Bx: " << Bx << std::endl << std::endl;

		return;
	}

void DumpHit(Hit& hit, int N)
	{
		if(N == -1)	std::cout << "Hit : \n";
		else		std::cout << "Hit : " << N;

		std::cout << '\t' << "bx: " << hit.bx
			  << '\t' << "hs: " << hit.hs
			  << '\t' << "layer: " << hit.lay << '\n';

		for(int i=0; i < 3; i++)	std::cout << "  " << std::bitset<8>(GetTriadSeg(hit,i)) << '\n';

		return;
	}



//~~~~~~~~ Functions ~~~~~~~~~~~~~~~~~~~

void PrintCLCT(CLCT& clct, std::ostream& oss)
	{
		std::string empty_layer = "-----------";
		std::vector<std::string> layers(6,empty_layer);

		for(int i=0; i < clct.hits.size(); i++){
			layers.at(clct.hits.at(i).lay).at(5 + (clct.hits.at(i).hs - clct.hs)) = 'X';
			oss << "Hit (" << i << ')' << "   Bx: " << clct.hits.at(i).bx << "   Hs: " << clct.hits.at(i).hs << "   Layer: " << clct.hits.at(i).lay << "   CFEB: " << GetCFEB(clct.hits.at(i).hs) << std::endl;
		}

		oss << "bx keystrip pattern nhits" << std::endl;
                oss << clct << std::endl;


		for(int ly=0; ly < 6; ly++)	// for each layer
		{
			oss << "ly" << ly;
			if(ly == 2) oss << "  key  ";
			else 	    oss << "       ";

			oss << layers.at(ly) << std::endl;

		}
	}

int ReadTxt(std::string& prefix, std::vector<CLCT>& clcts)
	{
		CLCT cl;
		std::string str; 										// String to be used for parsing purposes

		std::cout << "I MADE IT HERE!" << std::endl;
		std::cout << "I am trying to open file at: " << prefix << ".txt" << std::endl << std::endl;



		std::fstream ifs((prefix + ".txt").c_str(), std::ios_base::in); // Open File to be read;

		// First two lines are header
		std::getline(ifs, prefix);
		std::getline(ifs, str);


		prefix = prefix.substr(8, prefix.length() - 8);

		int n = 0;
		while (ifs >> cl) {
			clcts.push_back(cl);
			n++;
			std::cout << clcts.at(n - 1) << " size :" << n << '\n';
		}

		return n;
	}

std::string ReadTxt(std::string& path, std::vector<CLCT>& clcts, std::vector<Cluster>& clusters)
	{
		CLCT clct;
		Cluster gem;
		char mode = 'c';	// c-CLCT	g-GEM
		std::string str, prefix;

		std::cout << "Trying to open file: " << path << std::endl << std::endl;

		std::fstream ifs(path.c_str(), std::ios_base::in); // Open the file

		std::getline(ifs, prefix);	// first line specify pattern prefix
		prefix = prefix.substr(8, prefix.length()-8);

		do{
			std::getline(ifs, str);

			if(str[0] == 'C'){
				mode = 'c';
				std::cout << "Switching to CLCT Mode" << std::endl;
				std::getline(ifs, str);	// next line is header
				std::cout << str << std::endl;
			}
			else if(str[0] == 'G'){
				mode = 'g';
				std::cout << "Switching to GEM Mode" << std::endl;
				std::getline(ifs, str);	// next line is header
				std::cout << str << std::endl;
			}
			else if(!ifs.eof() && (str.length() != 0)){
				std::stringstream ss(str.c_str());
				switch(mode){
					case 'c':
						ss >> clct;
						clcts.push_back(clct);
						std::cout << clct << std::endl;
						break;
					case 'g':
						ss >> gem;
						clusters.push_back(gem);
						std::cout << gem << std::endl;
						break;
					default:
						break;
				}
			}
		}while(!ifs.eof());

		ifs.close();
		return prefix;
	}

void WriteTxt(std::string& str, std::vector<CLCT>& clcts)
	{
		std::fstream text_file((str + ".txt").c_str(), std::ios_base::out); // Create output file => (input string).txt
		//std::sort(clcts.begin(), clcts.end(), CompareCLCT);	// Sort the CLCT vector in case it's asorted

		// Add header stuff to pattern(.txt) file
		text_file << "prefix:" << str << std::endl;
		text_file << "bx keystrip pattern nhits" << std::endl;

		// print each CLCT to file + "\n" unless last CLCT then No "\n"
		for (int i = 0; i < clcts.size(); i++) {
			text_file << clcts.at(i);
			if (i < (clcts.size() - 1)) {
				text_file << '\n';
			}
		}

		text_file.close();
		return;
	}

void ExtractHits(std::vector<CLCT>& clcts, std::vector<Hit>& hits, int feb)
	{
		if (feb == -1)
		{
			for (int i = 0; i < clcts.size(); i++)
			{
				hits.insert(hits.end(), clcts.at(i).hits.begin(), clcts.at(i).hits.end());
			}
		}
		/// If you want to pull hits from a specific dcfeb
		else if(feb >= 0 && feb < CSCConstants::NUM_DCFEBS)
		{
			for(int i=0; i < clcts.size(); i++)
			{
				for(int j=0; j < clcts.at(i).hits.size(); j++)
				{
					if (GetCFEB(clcts.at(i).hits.at(j).hs) == feb) { hits.push_back(clcts.at(i).hits.at(j)); }
				}
			}
		}
		return;
	}

bool compareCluster(Cluster c1, Cluster c2)
	{
		return(c1.bx < c2.bx);
	}

void CollectClusters(std::vector<Cluster>& pads, std::vector<Cluster>& in_pads, int layer, int gem_fiber = -1)
	{	// Fills pads with Clusters of
		if(gem_fiber == -1)
		{
			for(int i=0; i < in_pads.size(); i++){
				if(in_pads[i].layer == layer) pads.push_back(in_pads[i]);
			}
		}
		else	// Extend if necessary
		{}

		std::sort(pads.begin(), pads.end(), compareCluster);	// sort by Bx
		return;
	}

void fillnbytes(std::vector<std::fstream*> oss, unsigned int n, unsigned int thisfeb)
	{
	for (unsigned int j = 0; j < n; j++)
		(*(oss[thisfeb])) << char(0);

	}

void fillnbytes_allfebs(std::vector<std::fstream*> oss, unsigned int n)
	{
	for (unsigned int i = 0; i < oss.size(); i++)
		for (unsigned int j = 0; j < n; j++)
			(*(oss[i])) << char(0);

	}

void writenbytes(std::fstream* out, unsigned int n, unsigned int x = 255)
	{
		for(unsigned int i=1; i <= n; i++){
			(*out) << char(x);
		}
	}

void writenbxs(std::fstream* out, unsigned int n)
	{
		for(unsigned int i=1; i <=n; i++){
			writenbytes(out, 7);	// default 255
			writenbytes(out, 1, 255);
		}
	}


/// CSC
bool WritePat(std::string & prefix, std::vector<CLCT>& clcts)
	{
		// Prepare output file streams
		std::vector<std::fstream*> oss;
		char tmbtype = COMPILE_TYPE - 0xa + 'a';
		for (int i = 0; i < CSCConstants::NUM_DCFEBS; i++)
		{
			std::stringstream ss;
			ss << prefix << "_cfeb" << i << "_tmb" << tmbtype << ".pat";
			//std::cout << ss.str().c_str() << std::endl;
			oss.push_back(new std::fstream(ss.str().c_str(), std::ios_base::out) );
			//std::cout << "opening file " << ss.str() << std::endl;
		}

		for (int i = 0; i < CSCConstants::NUM_DCFEBS; i++)
		{
			std::cout << "Writing (.pat) file number: " << i << std::endl;	// debug

			std::vector<Hit> hits;		//
			std::vector<int> times;		// list of bx's w/

			ExtractHits(clcts, hits, i);

			// DEBUG PURPOSES~~~~~
			for(int j=0; j < hits.size(); j++){
				DumpHit(hits.at(j), j);
			}
			// ~~~~~~~~~

			if(hits.size() == 0)			// IF : No hits in Pattern
			{
				fillnbytes(oss, RAMPAGE_SIZE, i);
				(*(oss.at(i))) << std::flush;		// Fill & Close file
				delete (oss.at(i));
			}
			else					// ELSE
			{
				// Get times
				for (int j = 0; j < hits.size(); j++)
				{
					bool tf = false;
					for (int k = 0; k < times.size(); k++)
					{
						if (times.at(k) == hits.at(j).bx) tf = true;
					}
					if (!tf) times.push_back(hits.at(j).bx);
				}
				std::sort(times.begin(), times.end());


				int Bx = 0;	// Current Bunch Cross
				int q = 0;	// index var for times vector

				if (times.at(0) != 0)
				{
					Bx = times.at(0);
					fillnbytes(oss, times.at(0)*8, i);
				}


				while (Bx < (times[times.size() -1] + 3) && (RAMPAGE_SIZE - Bx*8) > 0)
				{
					if (Bx == (times.at(q) + 3) && (i + 1) == times.size())
					{
						fillnbytes(oss, RAMPAGE_SIZE - (Bx * 8), i);		/// Double check range here
						break;
					}
					else if (Bx == (times.at(q) + 3))
					{
						q++;
						if(times.at(q) > Bx)
						{
							fillnbytes(oss, (times.at(q) - Bx)*8, i);
							Bx = times.at(q);
							// write Group @ this->Bx
							(*(oss[i])) << Group(hits,Bx);
							DumpGroup(Group(hits, Bx), Bx);	// debug
							Bx++;
						}
						else
						{
							//Write Group @ this->Bx
							(*(oss[i])) << Group(hits, Bx);
							DumpGroup(Group(hits, Bx), Bx);	// debug
							Bx++;
						}
					}
					else
					{
						//Write Group @ this->Bx
						(*(oss[i])) << Group(hits,Bx);
						DumpGroup(Group(hits, Bx), Bx);	// debug
						Bx++;
					}
				}
				if((RAMPAGE_SIZE - Bx*8) > 0)
				{
					fillnbytes(oss, RAMPAGE_SIZE - (Bx * 8), i);		/// Double check range here
				}

				*(oss.at(i)) << std::flush;
				delete (oss.at(i));

			}


		}
		return true;
	}

void pushInfo(std::fstream* oss, unsigned int info, unsigned int& reminfo, unsigned int& rembits, unsigned int iclust)
	{

	}

void closebx(std::fstream* oss, unsigned int& reminfo, unsigned int& rembits, unsigned int icluster)
	{
		unsigned int x = std::pow(2, 8-rembits) - 1;
		(*oss) << char((reminfo << (8 - rembits)) | x);
		writenbytes(oss, (8 - ((icluster*14)/8)));

		reminfo = 0;
		rembits = 0;
		return;
	}
// GEM
bool WritePat(std::string& prefix, std::vector<Cluster>& in_pads)
	{
		// Open (.pat) files
		char tmbtype = COMPILE_TYPE - 0xa + 'a';
		std::vector<std::fstream*> oss;
		for(int gem_fiber = 0; gem_fiber < GEM_FIBERS; gem_fiber++){
			std::stringstream ss;
			ss << prefix << "_GEM" << gem_fiber << "_tmb" << tmbtype << ".pat";
			oss.push_back(new std::fstream(ss.str().c_str(), std::ios_base::out) );
		}

		// Write GEMpads
		for (int layer = 0; layer <= 2; layer += 2) {
			std::cout << "Current Layer " << layer << std::endl;
			std::vector<Cluster> pads;
			if (layer == 0) CollectClusters(pads, in_pads, (1));
			else CollectClusters(pads, in_pads, 2);

			int totbytes;
			unsigned int remainbits = 0;
			unsigned int remaininfo = 0;
			unsigned int icluster = 0;
			unsigned int x;

			unsigned int lastbx = 0;

			for (unsigned int i = 0; i < pads.size(); i++) {
				std::cout << "i " << i << " bx " << pads[i].bx << " cluster bits: " << (std::bitset<14>)pads[i].info() << std::endl;

				if (lastbx < pads[i].bx) {
					if ((icluster >= 0) && (icluster <= 4)) {
						// Finish writing to File 1
						closebx(oss[layer], remaininfo, remainbits, icluster);
						// Fill File 1
						writenbxs(oss[layer], pads[i].bx - 1 - lastbx);
						// Fill File 2
						writenbxs(oss[layer + 1], pads[i].bx - lastbx);	// one more bx since nothing was written for last bx

					}
					else if (icluster > 4 && icluster <= 8) {
						// Finish File 1
						closebx(oss[layer], remaininfo, remainbits, 4);
						// Finish File 2
						closebx(oss[layer + 1], remaininfo, remainbits, (icluster%4));
						// Fill File 1
						writenbxs(oss[layer], pads[i].bx - 1 - lastbx);
						// Fill File 2
						writenbxs(oss[layer + 1], pads[i].bx - 1 - lastbx);
					}
					std::cout <<" lastbx " << lastbx <<" input_pads bx " << pads[i].bx <<" remain n  "<< remainbits << " bits " << (std::bitset<8>)(remaininfo << (8-remainbits)) << std::endl;
					lastbx = pads[i].bx;
					icluster = 0;
					remainbits = 0;
					remaininfo = 0;
				}
				if (lastbx == pads[i].bx) {
					if (icluster >= 8)
						continue;

					switch (icluster) {
						// File 1
					case 0: remainbits = 6;
						x = std::pow(2, remainbits) - 1;
						remaininfo = (pads[i].info() & x);
						(*(oss[layer])) << char(pads[i].info() >> 6);
						break;
					case 1: remainbits = 4;
						(*(oss[layer])) << char((remaininfo << 2) | (pads[i].info() >> 12));
						(*(oss[layer])) << char(pads[i].info() >> 4);
						x = std::pow(2, remainbits) - 1;
						remaininfo = (pads[i].info() & x);
						break;
					case 2: remainbits = 2;
						(*(oss[layer])) << char((remaininfo << 4) | (pads[i].info() >> 10));
						(*(oss[layer])) << char(pads[i].info() >> 2);
						x = std::pow(2, remainbits) - 1;
						remaininfo = (pads[i].info() & x);
						break;
					case 3: remainbits = 0;
						(*(oss[layer])) << char((remaininfo << 6) | (pads[i].info() >> 8));
						(*(oss[layer])) << char(pads[i].info());
						remaininfo = 0;
						//closebx(oss[layer], remaininfo, remainbits, 4);
						break;
						// FILE 2
					case 4: remainbits = 6;
						x = std::pow(2, remainbits) - 1;
						remaininfo = (pads[i].info() & x);
						(*(oss[layer + 1])) << char(pads[i].info() >> 6);
						break;
					case 5: remainbits = 4;
						(*(oss[layer + 1])) << char((remaininfo << 2) | (pads[i].info() >> 12));
						(*(oss[layer + 1])) << char(pads[i].info() >> 4);
						x = std::pow(2, remainbits) - 1;
						remaininfo = (pads[i].info() & x);
						break;
					case 6: remainbits = 2;
						(*(oss[layer + 1])) << char((remaininfo << 4) | (pads[i].info() >> 12));
						(*(oss[layer + 1])) << char(pads[i].info() >> 2);
						x = std::pow(2, remainbits) - 1;
						remaininfo = (pads[i].info() & x);
						break;
					case 7: remainbits = 0;
						(*(oss[layer + 1])) << char((remaininfo << 6) | (pads[i].info() >> 8));
						(*(oss[layer + 1])) << char(pads[i].info());
						remaininfo = 0;
						//closebx(oss[layer + 1], remaininfo, remainbits, 4);
						break;
					defalut:
						std::cout << " error icluster: " << icluster << std::endl;
						break;
					}
				}
				icluster++;
			}// End pads vector loop

			if ((icluster > 0) && (icluster <= 4)) {
				// Finish writing to File 1
				closebx(oss[layer], remaininfo, remainbits, icluster);
				writenbxs(oss[layer + 1], 1);
			}
			else if (icluster > 4 && icluster <= 8) {
				// Finish writing to File 2
				closebx(oss[layer + 1], remaininfo, remainbits, icluster%4);
			}

			// Complete the Files
			writenbxs(oss[layer], RAMPAGE_SIZE / 8 - 1 - lastbx);
			(*(oss[layer])) << std::flush;
			writenbxs(oss[layer + 1], RAMPAGE_SIZE / 8 - 1 - lastbx);
			(*(oss[layer + 1])) << std::flush;

		} // End layer for loop



		return true;
	}


}
