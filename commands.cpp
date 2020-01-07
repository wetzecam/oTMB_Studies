#include "commands.h"

//static int nn=0; // for testing

int write_command(int num, int adr, char* block){
  if(DEBUG>10){
    if(block==0) std::cout<<"DEBUG[commands.cpp]  write_command("<<num<<",0,NULL)"<<std::endl;
    else std::cout<<"DEBUG[commands.cpp]  write_command("<<num<<","<<adr<<",{"<<HEX(0xff&block[0])<<HEX(0xff&block[1])<<"...})"<<DEC()<<std::endl;
  }
  nwdat=12; // so there are no troubles sending a small packet
  char* command = wdat;
  switch(num){
  case 0:
    command[0]=0xf0;
    command[1]=0xf0;
    command[2]=0x00;
    command[3]=0x00;
    break;
  case 1: // pack Sbits into clusters 
    command[0]=0xf1;
    command[1]=0xf1;
    command[2]=0x00;
    command[3]=0x00;
    break;
  case 2:
    command[0]=0xf2;
    command[1]=0xf2;
    command[2]=0x00;
    command[3]=0x00;
    break;
  case 3: // read command
    command[0]=0xf3;
    command[1]=0xf3;
    command[2]=adr&0x00ff;
    command[3]=(adr&0xff00)>>8;
    break;
  case 5:
    command[0]=0xf5;
    command[1]=0xf5;
    command[2]=0x00;
    command[3]=0x00;
    break;
  case 7: // write command
    command[0]=0xf7;
    command[1]=0xf7;
    command[2]=adr&0x00ff;
    command[3]=(adr&0xff00)>>8;
    memcpy((void*)&command[4], (const void*)&block[0], RAMPAGE_SIZE);  // fill 4KB
    nwdat=4+RAMPAGE_SIZE;
    break;
  case 0xe: // send out to otmb (not used now)
    command[0]=0xfe;
    command[1]=0xfe;
    command[2]=adr&0x00ff;
    command[3]=(adr&0xff00)>>8;
    break;
  case 0xd: // send out to otmb
    command[0]=0xfd;
    command[1]=0xfd;
    command[2]=adr&0x00ff;
    command[3]=0x00;
    break;
  default:
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  write_command return -1  <== command number "<<num<<" not recognized"<<std::endl;
    return -1;
  }

  // testing >>>
  //std::cout<<"TESTING: write_command return 0"<<std::endl;
  //return 0;
  // <<< testing
  
  int n=eth_write();
  
  if(n!=nwdat){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  write_command return -2  <== n="<<n<<" != nwdat="<<nwdat<<std::endl;
    return -2;
  }
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  write_command return 0"<<std::endl;
  return 0;
}


int read_command(char** pkt){
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  read_command(*{NULL})"<<std::endl;
  *pkt=0;

  int n=eth_read(0);

  // testing >>>
  //std::cout<<"TESTING: read_command, copy wdat to rdat"<<std::endl;
  //n=24;
  //memcpy(rdat,wdat,4*n);
  //n=RAMPAGE_SIZE+4;
  //memcpy(rdat,(const void*)&wdat[nn],n);
  //rdat[3] = (rdat[3]&0xff) + 0xd0;
  //if(time(0)%10==0) rdat[10]++;
  //if(time(0)%15==0) rdat[2]++;
  //if(time(0)%2==0 && (rdat[0]&0xff) == 0xf3) rdat[2]++;
  // <<< testing


  if(n<=6){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  read_command return -2  <== n="<<n<<" <= 6"<<std::endl;
    return -2;
  }
  *pkt = rdat;  // currently just setting the pointer, but I may want to make a copy (otherwise I can just use rbuf)

  if(DEBUG>20){
    std::cout<<"DEBUG[commands.cpp]  read_command read in packet with "<<n<<" bytes:"<<std::endl;
    dumphex(n,*pkt);
    std::cout<<std::endl;
  }

  int num=0;
  if     ((rdat[0]&0xff) == 0xf0 && (rdat[1]&0xff) == 0xf0) num=0;
  else if((rdat[0]&0xff) == 0xf1 && (rdat[1]&0xff) == 0xf1) num=1;
  else if((rdat[0]&0xff) == 0xf2 && (rdat[1]&0xff) == 0xf2) num=2; 
  else if((rdat[0]&0xff) == 0xf3 && (rdat[1]&0xff) == 0xf3) num=3; // return packet for F3 will now start with f3f3
  else if((rdat[0]&0xff) == 0xf5 && (rdat[1]&0xff) == 0xf5) num=5;
  else if((rdat[0]&0xff) == 0xf7 && (rdat[1]&0xff) == 0xf7) num=7;
  else if((rdat[0]&0xff) == 0xfd && (rdat[1]&0xff) == 0xfd) num=0xd;
  else if((rdat[0]&0xff) == 0xfe && (rdat[1]&0xff) == 0xfe) num=0xe;
  else num=-1;

  if(num==3){ // compare the address word from the header to the first word (-0xa000+0xd000) in the data
    if( (rdat[2]&0xff) != (rdat[4]&0xff) ||
	(rdat[3]&0xff) != ((rdat[5]&0xff) - 0xa0 + 0xd0) ){
      if(DEBUG>10){
	std::cout<<"DEBUG[commands.cpp]  Address words do not agree: "; dumphex(6,rdat); std::cout<<std::endl;
      }
      num = -num;
    }
  }
  
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  read_command return "<<num<<std::endl;
  return num;
}

int retrieve_VotingErrorCounts(unsigned int* counters){
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_VotingErrorCounts("<<*counters<<")"<<std::endl;
// *** the counters array *must* have places for at least 12 elements
// fill coutners array
// return value!=0 indicates failure

  //// testing >>>
  /*
  if(time(0)%10==0) counters[0]++;
  if(time(0)%10==1) counters[1]++;
  if(time(0)%10==2) counters[2]++;
  if(time(0)%10==3) counters[3]++;
  if(time(0)%10==4) counters[4]++;
  if(time(0)%10==5) counters[5]++;
  if(time(0)%10==3) counters[6]++;
  if(time(0)%10==4) counters[7]++;
  if(time(0)%10==5) counters[8]++;
  if(time(0)%10==6) counters[9]++;
  if(time(0)%10==7) counters[10]++;
  if(time(0)%10==6) counters[11]++;
  return 0;
  */
  // <<< testing

  int commandnum=5;

  eth_reset();  // first, clear the buffers

  if(write_command(commandnum) != 0){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_VotingErrorCounts return -1  <== error sending command"<<std::endl;
    return -1;  // send command
  }
  char* pkt;
  if(read_command(&pkt) != commandnum){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_VotingErrorCounts return -2  <== error reading command"<<std::endl;
    return -2;  // get responce
  }
  if(sizeof(int)!=4){ // make sure int is 4 bytes
    std::cerr<<"ERROR: sizeof(int)="<<sizeof(int)<<"  This code assume sizeof(int)==4!"<<std::endl;
    return -3;
  }
  memcpy(counters, (const void*)&pkt[2], 12*4);  // fill error counts: 12x 4-byte counters
  // Note: this will thrash memory if the counters array does not have 12 places!
  // It is also possible that the bytes in the array are reversed from how they should be in the int, which will take more work to unpack
  for (int i=0; i<12; i++)
  if (DEBUG>0) std::cout <<" retrieve_VotingErrorCounts counter "<< i <<" "<< counters[i] << std::endl;

  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_VotingErrorCounts return 0"<<std::endl;
  return 0;
}


int retrieve_Snap12ErrorCounts(unsigned int* counters){ 
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_Snap12ErrorCounts("<<*counters<<")"<<std::endl;
// *** the counters array *must* have places for at least 8 elements
// fill coutners array
// return value!=0 indicates failure
  
  //// testing >>>
  //counters[0]++;
  //counters[1]++;
  //if(time(0)%20==2) counters[2]++;
  //return 0;
  // <<< testing

  int commandnum=1;
  
  eth_reset();  // first, clear the buffers
  
  if(write_command(commandnum) != 0){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_Snap12ErrorCounts return -1  <== error sending command"<<std::endl;
    return -1;  // send command
  }
  
  char* pkt;
  if(read_command(&pkt) != commandnum){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_Snap12ErrorCounts return -2  <== error reading command"<<std::endl;
    return -2;  // get responce
  }
  
  if(sizeof(int)!=4){ // make sure int is 4 bytes
    std::cerr<<"ERROR: sizeof(int)="<<sizeof(int)<<"  This code assume sizeof(int)==4!"<<std::endl;
    return -3;
  }
  int nbyte_per_counter = 4;
  if (Nfibers > 12) {
	  nbyte_per_counter = 2;
          unsigned short counters_short [Nfibers];
	  memcpy(counters_short, (const void*)&pkt[2], Nfibers*nbyte_per_counter);  // fill error counts: 8x 4-byte counters
	  for (int i=0; i<Nfibers; i++) counters[i] = counters_short[i];
  }
  else 
	  memcpy(counters, (const void*)&pkt[2], Nfibers*nbyte_per_counter);  // fill error counts: 8x 4-byte counters
  // Note: this will thrash memory if the counters array does not have 24 places!
  // It is also possible that the bytes in the array are reversed from how they should be in the int, which will take more work to unpack
  //for (int i=0; i<Nfibers; i++)
  //if (DEBUG>0) std::cout <<" retrieve_Snap12ErrorCounts counter "<< i <<" "<< counters[i]<<" size of counters[i] "<< sizeof(counters[i]) << std::endl;
  
  
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_Snap12ErrorCounts return 0"<<std::endl;
  return 0;  
}


int retrieve_TranslatorErrorCounts(unsigned short* counters){
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_TranslatorErrorCounts("<<*counters<<")"<<std::endl;
// *** the counters array *must* have places for at least 24 elements
// fill coutners array
// return value!=0 indicates failure

  //// testing >>>
  //counters[0]++;
  //counters[0]++;
  //if(time(0)%20==11) counters[0]++;
  //return 0;
  // <<< testing

  int commandnum=2;

  eth_reset();  // first, clear all the buffers
  
  if(write_command(commandnum) != 0){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_TranslatorErrorCounts return -1  <== error sending command"<<std::endl;
    return -1;  // send command
  }

  char* pkt;
  if(read_command(&pkt) != commandnum){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_TranslatorErrorCounts return -2  <== error reading command"<<std::endl;
    return -2;  // get responce
  }

  if(sizeof(short)!=2){ // make sure short is 2 bytes
    std::cerr<<"ERROR: sizeof(short)="<<sizeof(short)<<"  This code assume sizeof(short)==2!"<<std::endl;
    return -3;
  }

  memcpy(counters, (const void*)&pkt[2], 24*2);  // fill error counts: 24x 2-byte counters
  // this will thrash memory if the counters array does not have 24 places!
  // Also, bytes in the array are probably reversed from how they should be for a short.  If so, that will have to be fixed!
  
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_TranslatorErrorCounts return 0"<<std::endl;
  return 0;  
}


int send_RAMPage(int pageid, char* block){
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  send_RAMPage("<<pageid<<",{"<<HEX(0xff&block[0])<<HEX(0xff&block[1])<<"...})"<<std::endl;
// *** block must be at least a 4KB buffer
// pageid should be a page number of RAM
// fill block buffer with 4KB block
// return value!=0 indicates failure

  int commandnum=7;
  //nn=0;  // testing

  eth_reset();  // first, clear all the buffers
  
  if(write_command(commandnum, pageid, block) != 0){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  send_RAMPage return -1  <== error sending command"<<std::endl;
    return -1;  // send command
  }
  
  char* pkt;
  int errnum = read_command(&pkt);

  if(errnum == commandnum){
    // a final check that the address that the board said it wrote actually matches what we wanted
    char tmp[2];
    tmp[0]=pageid&0x00ff;
    tmp[1]=(pageid&0xff00)>>8;
    if(DEBUG>20){
      std::cout<<"DEBUG[commands.cpp]  send_RAMPage, compare address bytes:"<<std::endl;
      std::cout<<HEX(tmp[0])<<" "<<HEX(pkt[2]&0xff)<<std::endl;
      std::cout<<HEX(tmp[1])<<" "<<HEX(pkt[3]&0xff - 0xd0)<<std::endl;
      std::cout<<DEC();
    }
    if( (tmp[0]&0xff) != (pkt[2]&0xff) ||
	(tmp[1]&0xff) != ((pkt[3]&0xff) - 0xd0) ){
      if(DEBUG>10){
	std::cout<<"DEBUG[commands.cpp]  Address word does not match pageid("<<HEX(pageid)<<") : "<<HEX(pkt[2]&0xff)<<HEX(pkt[3]&0xff)<<DEC()<<std::endl;
      }
      if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  send_RAMPage return 98"<<std::endl;
      return 98;  
    }
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  send_RAMPage return 0"<<std::endl;
    return 0;  
  }else if(errnum == -commandnum){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  send_RAMPage return 99  <== mismatched address words"<<std::endl;
    return 99;  
  }

  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  send_RAMPage return "<<errnum<<"  <== error reading command"<<std::endl;
  return errnum;  // get responce
}


int retrieve_RAMPage(int pageid, char* block, char* hdr){
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_RAMPage("<<pageid<<",{"<<HEX(0xff&block[0])<<HEX(0xff&block[1])<<"...})"<<DEC()<<std::endl;
// *** block must be at least a 4KB buffer
// pageid should be a page number of RAM
// fill block buffer with 4KB block
// return value!=0 indicates failure

  int commandnum=3;
  //nn=4; //testing
  
  eth_reset();  // first, clear all the buffers
  
  if(write_command(commandnum, pageid) != 0){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_RAMPage return -1  <== error sending command"<<std::endl;
    return -1;  // send command
  }

  char* pkt;
  int errnum = read_command(&pkt);
  
  //memcpy(block, (const void*)&pkt[0], RAMPAGE_SIZE);  // fill 4KB
  //memcpy(block, (const void*)&pkt[10], RAMPAGE_SIZE);  // return packet now starts with f3f3, then 8 byes of 0, and then the page block
  memcpy(hdr, (const void*)&pkt[0], 4);  // return packet should start with f3f3 then an address word
  memcpy(block, (const void*)&pkt[4], RAMPAGE_SIZE);  // then the page block
  // this will thrash memory if the block array is <4KB buffer
  
  if(errnum == commandnum){
    // a final check that the address that the board said it got actually matches what we wanted
    char tmp[2];
    tmp[0]=pageid&0x00ff;
    tmp[1]=(pageid&0xff00)>>8;
    if(DEBUG>20){
      std::cout<<"DEBUG[commands.cpp]  retrieve_RAMPage, compare address bytes:"<<std::endl;
      std::cout<<HEX(tmp[0])<<" "<<HEX(pkt[2]&0xff)<<std::endl;
      std::cout<<HEX(tmp[1])<<" "<<HEX(pkt[3]&0xff - 0xd0)<<std::endl;
      std::cout<<DEC();
    }
    if( (tmp[0]&0xff) != (pkt[2]&0xff) ||
	(tmp[1]&0xff) != ((pkt[3]&0xff) - 0xd0) ){
      if(DEBUG>10){
	std::cout<<"DEBUG[commands.cpp]  Address word does not match pageid("<<HEX(pageid)<<") : "<<HEX(pkt[2]&0xff)<<HEX(pkt[3]&0xff)<<DEC()<<std::endl;
      }
      if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_RAMPage return 98"<<std::endl;
      return 98;  
    }
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_RAMPage return 0"<<std::endl;
    return 0;  
  }else if(errnum == -commandnum){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_RAMPage return 99  <== mismatched address words"<<std::endl;
    return 99;
  }
  
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_RAMPage return "<<errnum<<"  <== error reading command"<<std::endl;
  return errnum;  // get responce
}


int retrieve_PacketID(int* id){
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_PacketID("<<*id<<")"<<std::endl;
// set id to packet id (page number)
// return value!=0 indicates failure
  
  char* pkt;
  if(read_command(&pkt) == -2){
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_PacketID return -2  <== error reading command"<<std::endl;
    return -2;  // get packet, anything but -2 is fine
  }
  // above line must be uncommented (i.e. running) for the actual running
  
  // testing >>>
  //   pkt=rdat;
  //   nn+=2;
  //   if(nn%256==20) nn+=2; // skip
  //   pkt[1] = 0xff&nn;
  // if(nn%256==44) pkt[1]=0xff; // bad
  // <<< testing
  
  id[0] = 0;
  memcpy(id, (const void*)&pkt[1], 1);  // copy packet id byte to id[0]
  //std::cout<<id[0]<<std::endl;
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  retrieve_PacketID return 0"<<std::endl;
  return 0;  
}



int get_EthErrorCounts(unsigned int* counters){
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  get_EthErrorCounts("<<*counters<<")"<<std::endl;
// *** the counters array *must* have places for at least 8 elements
// fill counters array
// return value!=0 indicates failure

  using namespace std;

  ifstream ifinfo("/proc/net/dev");
  if(ifinfo.is_open()){
    string line;
    while(getline(ifinfo, line)){
      istringstream ss(line);
      string tmps;
      int tmpi;
      ss>>tmps;
      if(tmps.find("eth1")!=string::npos){ // got line for eth1, fill counters
	if(tmps.size()<8){ // first field was not merged with the second field
	  ss>>tmpi>>tmpi>>counters[0]>>counters[1]>>counters[2]>>counters[3]>>tmpi>>tmpi>>tmpi>>tmpi>>counters[4]>>counters[5]>>counters[6]>>counters[7];
	}else{ // first field must have been merged with the second field (byte count)
	  ss>>tmpi>>counters[0]>>counters[1]>>counters[2]>>counters[3]>>tmpi>>tmpi>>tmpi>>tmpi>>counters[4]>>counters[5]>>counters[6]>>counters[7];
	}
	if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  get_EthErrorCounts return 0"<<std::endl;
	return 0;
      }

      // testing >>>
      // following lines for testing only!  remove them!
      // cout<<"line:"<<line<<endl;
      // if(tmps.find("lo")!=string::npos){ // testing with other interfaces
      // 	ss>>counters[0]>>counters[1]>>counters[2]>>counters[3]>>tmpi>>tmpi>>tmpi>>tmpi>>counters[4]>>counters[5]>>counters[6]>>counters[7];
      // 	for(int i=0; i<8; i++) cout<<"  counters["<<i<<"]="<<counters[i]<<endl;
      // 	return 0;
      // }
      // <<< testing

    }
    if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  get_EthErrorCounts return -2  <== did not find eth1"<<std::endl;
    return -2; // did not find eth1
  }
  if(DEBUG>10) std::cout<<"DEBUG[commands.cpp]  get_EthErrorCounts return -1  <== could not open /proc/net/dev"<<std::endl;
  return -1; // could not open /proc/net/dev
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
