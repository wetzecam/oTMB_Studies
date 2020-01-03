/*SCCONSTANTS_H_CSCCONSTANTS_H_
 * Brief : This file contains the functions that calculate the
 * 		local, global and cfeb values from eachother
 *	  **Note : This is used to construct the high level CLCT objects**
 * */
#include "CSCConstants.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <bitset>
#include <iomanip>
#include <cmath>
#include <ctime>
#include <vector>


// Calculates the Global Half Strip from the CFEB and Local Half Strip
template <size_t XS_per_CFEB> int GetOutputXStrip(int cfeb, int input_xstrip, int tmb_compile_type) {

  const int Bad_HS = -1;
  const int Min_CFEB_in_non_me11 = 0;
  const int Max_CFEB_in_non_me11 = 4;
  const int N_CFEB_in_non_me11 = Max_CFEB_in_non_me11 - Min_CFEB_in_non_me11 + 1;
  const int Min_XS_in_non_me11 = 0;
  const int Max_XS_in_non_me11 = N_CFEB_in_non_me11*XS_per_CFEB - 1;
  const int Min_CFEB_in_region_a = 0;
  const int Max_CFEB_in_region_a = 3;
  const int Min_XS_in_region_a = Min_CFEB_in_region_a*XS_per_CFEB;
  const int Max_XS_in_region_a = (Max_CFEB_in_region_a + 1)*XS_per_CFEB - 1;
  const int Min_CFEB_in_region_b = 4;
  const int Max_CFEB_in_region_b = 6;
  const int Min_XS_in_region_b = Min_CFEB_in_region_b*XS_per_CFEB;
  const int Max_XS_in_region_b = (Max_CFEB_in_region_b + 1)*XS_per_CFEB - 1;
  int output_xs = Bad_HS;
  if(tmb_compile_type == 0xa) {
    if(cfeb >= Min_CFEB_in_non_me11 && cfeb <= Max_CFEB_in_non_me11)
      output_xs = Min_XS_in_non_me11 + (XS_per_CFEB*cfeb + input_xstrip);
    if(!(output_xs >= Min_XS_in_non_me11 && output_xs <= Max_XS_in_non_me11))
      output_xs = Bad_HS;
  }
  else if(tmb_compile_type == 0xb) {
    if(cfeb >= Min_CFEB_in_non_me11 && cfeb <= Max_CFEB_in_non_me11)
      output_xs = Max_XS_in_non_me11 - (XS_per_CFEB*cfeb + input_xstrip);
    if(!(output_xs >= Min_XS_in_non_me11 && output_xs <= Max_XS_in_non_me11))
      output_xs = Bad_HS;
  }
  else if(tmb_compile_type == 0xc) {
    if(cfeb >= Min_CFEB_in_region_a && cfeb <= Max_CFEB_in_region_a)
      output_xs = Min_XS_in_region_a + (XS_per_CFEB*(cfeb - Min_CFEB_in_region_a) + input_xstrip);
    else if(cfeb >= Min_CFEB_in_region_b && cfeb <= Max_CFEB_in_region_b)
      output_xs = Max_XS_in_region_b - (XS_per_CFEB*(cfeb - Min_CFEB_in_region_b) + input_xstrip);
    if(!(output_xs >= Min_XS_in_region_a && output_xs <= Max_XS_in_region_b))
      output_xs = Bad_HS;
  }
  else if(tmb_compile_type == 0xd) {
    if(cfeb >= Min_CFEB_in_region_a && cfeb <= Max_CFEB_in_region_a)
      output_xs = Max_XS_in_region_a - (XS_per_CFEB*(cfeb - Min_CFEB_in_region_a) + input_xstrip);
    else if(cfeb >= Min_CFEB_in_region_b && cfeb <= Max_CFEB_in_region_b)
      output_xs = Min_XS_in_region_b + (XS_per_CFEB*(cfeb - Min_CFEB_in_region_b) + input_xstrip);
    if(!(output_xs >= Min_XS_in_region_a && output_xs <= Max_XS_in_region_b))
      output_xs = Bad_HS;
  }

  return output_xs;
}


// Calculates the Local Half Strip from the Global Half Strip
template <size_t XS_per_CFEB> int GetInputXStrip(int output_xstrip, int tmb_compile_type) { // get localhs

  const int Min_CFEB_in_non_me11 = 0;
  const int Max_CFEB_in_non_me11 = 4;
  const int Min_CFEB_in_region_a = 0;
  const int Max_CFEB_in_region_a = 3;
  const int Min_CFEB_in_region_b = 4;
  const int Max_CFEB_in_region_b = 6;
  const int Max_XS_in_CFEB = XS_per_CFEB - 1;

  int region = output_xstrip / XS_per_CFEB;
  int phase = output_xstrip % XS_per_CFEB;
  int input_xs = -1;
  if(tmb_compile_type == 0xa) {
    if(region >= Min_CFEB_in_non_me11 && region <= Max_CFEB_in_non_me11)
      input_xs = phase;
  }
  else if(tmb_compile_type == 0xb) {
    if(region >= Min_CFEB_in_non_me11 && region <= Max_CFEB_in_non_me11)
      input_xs = Max_XS_in_CFEB - phase;
  }
  else if(tmb_compile_type == 0xc) {
    if(region >= Min_CFEB_in_region_a && region <= Max_CFEB_in_region_a) {
      input_xs = phase;
    }
    else if(region >= Min_CFEB_in_region_b && region <= Max_CFEB_in_region_b) {
      input_xs = Max_XS_in_CFEB - phase;
    }
  }
  else if(tmb_compile_type == 0xd) {
    if(region >= Min_CFEB_in_region_a && region <= Max_CFEB_in_region_a) {
      input_xs = Max_XS_in_CFEB - phase;
    }
    else if(region >= Min_CFEB_in_region_b && region <= Max_CFEB_in_region_b) {
      input_xs = phase;
    }
  }
  return input_xs;
}


// Calculates which CFEB from the global Half Strip
// get dcfeb
template <size_t XS_per_CFEB>  int GetInputCFEBByX(int output_xstrip, int tmb_compile_type) {

  const int Min_CFEB_in_non_me11 = 0;
  const int Max_CFEB_in_non_me11 = 4;
  const int Min_CFEB_in_region_a = 0;
  const int Max_CFEB_in_region_a = 3;
  const int Min_CFEB_in_region_b = 4;
  const int Max_CFEB_in_region_b = 6;

  int region = output_xstrip / XS_per_CFEB;
  int cfeb = -1;
  if(tmb_compile_type == 0xa) {
    if(region >= Min_CFEB_in_non_me11 && region <= Max_CFEB_in_non_me11)
      cfeb = region;
  }
  else if(tmb_compile_type == 0xb) {
    if(region >= Min_CFEB_in_non_me11 && region <= Max_CFEB_in_non_me11)
      cfeb = Max_CFEB_in_non_me11 - region;
  }
  else if(tmb_compile_type == 0xc) {
    if(region >= Min_CFEB_in_region_a && region <= Max_CFEB_in_region_a) {
      cfeb = region;
    }
    else if(region >= Min_CFEB_in_region_b && region <= Max_CFEB_in_region_b) {
      cfeb = Min_CFEB_in_region_b + (Max_CFEB_in_region_b - region);
    }
  }
  else if(tmb_compile_type == 0xd) {
    if(region >= Min_CFEB_in_region_a && region <= Max_CFEB_in_region_a) {
      cfeb = Min_CFEB_in_region_a + (Max_CFEB_in_region_a - region);
    }
    else if(region >= Min_CFEB_in_region_b && region <= Max_CFEB_in_region_b) {
      cfeb = region;
    }
  }
  return cfeb;
}

