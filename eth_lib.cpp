#define RAD_TEST_ETH_LIB_CPP
#include "eth_lib.h"

static int fd_schar;

//// global variables ////
int  eth_open(const char *dev_name);
int  eth_open_device();
int  eth_register_mac();
void eth_close();
int  eth_reset();
int  eth_read(int suppression);
int  eth_write();


/* I've removed the header so w/rpkt and w/rdat are the same, but keep both in case we want to add a header back in later */
char wpkt[4+MAX_DAT_SIZE]; /* full packet to send (header+data) */
char *wdat=wpkt;           /* buffer with data to send */
char rpkt[18+MAX_DAT_SIZE]; /* full packet read (header+data) */
//char *rdat=rpkt;           /* buffer with data read */
char *rdat=rpkt+14;           /* buffer with data read */
int nwdat;                 /* number of data bytes to send */
int nrdat;                 /* number of data bytes read */



/*********************************************************************
  Sending data:
  wdat = the data to send
  (wpkt = the data to send with header, header filled in eth_write())
  nwdat = number of bytes in wdat to send
  eth_write() => send data and return the number of bytes successfully sent (including header)
  
  To receive data:
  eth_read(0) => read data into buffers and return the number of bytes read (including header)
  rpkt = the header+data received
  rdat = the data received (without header)
  nrdat = number of bytes read into rdat
**********************************************************************/


int eth_open(const char *dev_name)
{
  if(DEBUG>10) std::cout<<"DEBUG[eth_lib.cpp]  eth_open(\""<<dev_name<<"\")"<<std::endl;
  //fd_schar = open(dev_name, O_RDWR); // I think all file descriptors for a given device point to the same place so the exact fd doesn't matter much
  fd_schar = open(dev_name, O_RDWR|O_NONBLOCK);
  if (fd_schar == -1) {
    perror("open");
    return -1;
  }
  //printf("Opened network path on device %s\n",dev_name);
  return 0;
}

int eth_open_device()
{
  const char* dev_schar = "/dev/schar3";
  return eth_open(dev_schar);
}

int eth_register_mac()
{
  if(DEBUG>10) std::cout<<"DEBUG[eth_lib.cpp]  eth_register_mac()"<<std::endl;
  /* This used to set the header, but now it does nothing.  Leave it here in case we want to use it in the future. */
  return 0;
}


int eth_reset()
{ 
  if(DEBUG>10) std::cout<<"DEBUG[eth_lib.cpp]  eth_reset()"<<std::endl;
  if(ioctl(fd_schar,SCHAR_RESET)){
    printf("Error in SCHAR_RESET \n");
  }
  //printf("Reset device.\n");
  return 0;
}

int eth_read(int suppression)
{
  if(DEBUG>10) std::cout<<"DEBUG[eth_lib.cpp]  eth_read("<<suppression<<")"<<std::endl;
  /*
   * suppression=0 => pass all packets 
   * suppression=1 => skip packets with 1-6 bytes and start will 0x03
   * suppression=2 => same as 1, plus ignore packets that begin with 0x33 or 0xff (multicast packets)
   */
  
  int lp;
  nrdat=0;
  do {
    //printf(" Receiving...");
    nrdat=read(fd_schar,rpkt, MAX_DAT_SIZE);
    //printf(" nrdat %d \n",nrdat);
    for(lp=0;suppression>0 && nrdat>=0 && nrdat <7 && rpkt[0]==0x03 && lp<100;lp++){ // Will ignore packets with 1-6 bytes and start will 0x03 (???)
      printf("...\n");
      usleep(100);
      nrdat=read(fd_schar,rpkt, MAX_DAT_SIZE);
    }
  } while(suppression>1 && nrdat>6 && (rpkt[0]&1)==1); //ignore multicast packets (ignores packet with first byte that is odd)

  //if(DEBUG>20){
    std::cout<<"DEBUG[eth_lib.cpp]  eth_read read packet of "<<nrdat<<" bytes:"<<std::endl;
    dumphex(nrdat,rdat);
    std::cout<<std::endl;
  //}
  return nrdat;
}

int eth_write()
{
  if(DEBUG>10) std::cout<<"DEBUG[eth_lib.cpp]  eth_write()"<<std::endl;
  if(nwdat>MAX_DAT_SIZE){
    printf("ERROR: nwdat=%d is too large! MAX_DAT_SIZE=%d",nwdat,MAX_DAT_SIZE);
    return 0;
  }
  int n_written;
  //printf(" Creating the packet... nwdat %d ...",nwdat);
  //if(DEBUG>20){
    std::cout<<"DEBUG[eth_lib.cpp]  eth_write sending packet of "<<nwdat<<" bytes:"<<std::endl;
    dumphex(nwdat,wdat);
    std::cout<<std::endl;
  //}
  n_written = write(fd_schar, (const void *)wpkt, nwdat);
  //printf(" n_written %d \n",n_written);
  return n_written;
}

void eth_close()
{
  if(DEBUG>10) std::cout<<"DEBUG[eth_lib.cpp]  eth_close()"<<std::endl;
  close(fd_schar);
  //printf("Closed device\n");
}

