#include "debug.h"
#include "utils.h"

/* Common funtions */

int file_size(FILE *fp){
  /* Returns the size of a file */
  
  int size;
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  return size;
}


int get_pattern(char* filename, char **buf, int verbose){
  /* A funtion to read a pattern file into a buffer.
   * filename = name of a file to read
   * *buf = pointer char array that will be allocated and filled (remeber to free)
   * verbose = a verbosity flag
   * return value: >=0 indicated the size of the pattern
   * return value: <0 indicated an error
   */
  int size = -1;
  
  if(verbose>0) printf("Prepare to read pattern from %s\n",filename);
  FILE *fp = fopen(filename,"r");
  if(!fp){
    if(verbose>0) printf("Could not open input file %s for reading!\n",filename);
    return 1;
  }
  
  size = file_size(fp); /* how many bytes in pattern file? */

  if(size<1){
    if(verbose>0) printf("\nERROR: File size = %d!\n=> Aborting!\n", size);
    return size;
  }
  if(size>MAX_PAT_SIZE){ /* hard limit on pattern size */
    if(verbose>0) printf("\nERROR: Pattern larger than limit %d KB!\n=> Aborting!\n", MAX_PAT_SIZE/1024);
    return -99;
  }
  
  if(verbose>0) printf("Read pattern from %s...",filename);
  
  *buf = 0;
  *buf = (char*)calloc( 1+size, sizeof(char) );
  if( !*buf ){
    printf("ERROR: Could not allocate memory!\n");
    return -3;
  }
  
  int nf = fread( *buf, sizeof( char ), size, fp );
  
  if(nf!=size){
    if(verbose>0) printf("\nERROR:  Only read %d of %d bytes from %s!\n=> Aborting!\n",nf, size, filename);
    return -98;
  }

  if(verbose>0) printf("...read %d bytes\n",size);
  
  return size;
}


void dumphex(int n, char* buf, std::ostream* s){
  int i;
  for(i=0;i<n;i++){
    //printf("%02x ",0xff&buf[i]);
    *s<<HEX(0xff&buf[i])<<" ";
  }
  *s<<std::dec<<std::setfill(' '); // back to default formatting
}

int save_page_to_file(int pageid, char* block){
  using namespace std;
  /* open output file */
  FILE* outfile=0;
  stringstream ss;
  ss<<"rampage"<<pageid<<".hex";
  outfile = fopen(ss.str().c_str(),"w");
  if(!outfile){
    cout<<"Could not open output file "<<ss.str()<<" for write!\n";
    return 1;
  }
  
  /* write pattern to file */
  fwrite( block, sizeof( char ), RAMPAGE_SIZE, outfile );

  /* close file */
  fclose( outfile );
  return 0;
}


int get_sequential_block(int pageid, char* block, int start){
  using namespace std;
  
  block[0] = pageid&0x00ff;
  block[1] = (pageid&0xff00)>>8;
  block[1] = (block[1]&0xff) + 0xa0;
  for(int i=2; i<2+RAMPAGE_SIZE; i++){
    block[i] = (i+start)&0xff;
  }
  return 0;
}

int get_page_from_file(int pageid, char* block){
  using namespace std;

  if(DEBUG==99){ // use sequential numbers for testing
    return get_sequential_block(pageid,block,2);
  }
  
  /* open input file */
  FILE* infile=0;
  vector<string> paths; // give a few possible locations for RAM page files
  paths.push_back("./");
  paths.push_back("./pages/");
  paths.push_back("../pages/");
  stringstream ss;
  ss<<"/rampage"<<pageid<<".hex";
  for(uint i=0; i<paths.size(); i++){
    paths[i]+=ss.str();
    infile = fopen(paths[i].c_str(),"r");
    if(infile) break;
  }
  if(!infile){
    cout<<"Could not find RAM page file.  Tried: "<<endl;
    for(uint i=0; i<paths.size(); i++) cout<<"   "<<paths[i]<<endl;
    return 1;
  }
  
  /* write pattern to file */
  fread( block, sizeof( char ), RAMPAGE_SIZE, infile );

  /* close file */
  fclose( infile );

  return 0;
}

unsigned int bytes2int(char* c){
// little endian conversion of first sizeof(int) bytes in c to an unsigned int
  unsigned int i=0;
  for(int n=sizeof(int); n >= 0; n--)  i = (i << 8) + c[n];
  return i;
}

unsigned short bytes2short(char* c){
// little endian bitwise conversion of first sizeof(short) bytes in c to an unsigned short
  unsigned short i=0;
  for(int n=sizeof(int); n >= 0; n--)  i = (i << 8) + c[n];
  return i;
}

std::string time_string(bool terse){
  time_t ltime; // calendar time
  ltime = time(0); // get current time
  // can format however
  std::string str(asctime(localtime(&ltime)));
  str.erase(str.find("\n"));
  if (terse){
    char tmp[300];
    struct tm *Tm=localtime(&ltime);
    sprintf(tmp, "%04d-%02d-%02d_%02d-%02d-%02d",
            Tm->tm_year+1900,
            Tm->tm_mon+1,
            Tm->tm_mday,
            Tm->tm_hour,
            Tm->tm_min,
            Tm->tm_sec);
    str = tmp;
  }
  return str;
}
