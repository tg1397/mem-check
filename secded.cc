/*
AUTHOR: Trish
DUE DATE: March 9th, 2018
DESC: calculates errors in faulty memory for 1 and 2 bit switches
 */

#include <stdio.h>
#include "mem.h"
#include "secded.h"



unsigned long calcparity(unsigned long data) {
  //returns a long with 0 if even 1s, 1 if odd 1s in 22nd position
  unsigned long count = 0;
  for(size_t i = 0; i <= 20; i++) {
    count = count ^ ((data >> i) & 1);
  }
  return count << 21;
}

unsigned long calccheckbits(unsigned short data) {
  //returns checkbits in locations 1-5
  unsigned long cb16 = 0;
  unsigned long cb8 = 0;
  unsigned long cb4 = 0;
  unsigned long cb2 = 0;
  unsigned long cb1 = 0;
  unsigned long cbits;
  
  //cb16
  for(size_t i = 11; i<=15; i++){
    cb16 = (cb16 ^ ((data >> i) & 1));
  }
  cb16 = cb16<<4;
    
  //cb8
  for(size_t i = 4; i<=10; i++) {
    cb8 = (cb8 ^ (data >> i) & 1);
  }
  cb8 = cb8<<3;
    
  //cb4
  for(size_t i = 1; i <= 3; i++) {
    cb4 = (cb4 ^ (data >> i) & 1);
  }
  for(size_t i = 7; i<=10; i++){
    cb4 = (cb4 ^ (data >> i) & 1);
  }
  cb4 = cb4 ^ (data >> 14) & 1;
  cb4 = cb4 ^ (data >> 15) & 1;
  cb4 = cb4<<2;
    
  //cb2
  cb2 = cb2 ^ (data >> 0) & 1;
  cb2 = cb2 ^ (data >> 2) & 1;
  cb2 = cb2 ^ (data >> 3) & 1;
  cb2 = cb2 ^ (data >> 5) & 1;
  cb2 = cb2 ^ (data >> 6) & 1;
  cb2 = cb2 ^ (data >> 9) & 1;  
  cb2 = cb2 ^ (data >> 10) & 1;
  cb2 = cb2 ^ (data >> 12) & 1;
  cb2 = cb2 ^ (data >> 13) & 1;
  cb2 = cb2<<1;
    
  //cb1
  cb1 = cb1 ^ (data >> 0) & 1;
  cb1 = cb1 ^ (data >> 1) & 1;
  cb1 = cb1 ^ (data >> 3) & 1;
  cb1 = cb1 ^ (data >> 4) & 1;
  cb1 = cb1 ^ (data >> 6) & 1;
  cb1 = cb1 ^ (data >> 7) & 1;
  cb1 = cb1 ^ (data >> 8) & 1;
  cb1 = cb1 ^ (data >> 10) & 1;
  cb1 = cb1 ^ (data >> 11) & 1;
  cb1 = cb1 ^ (data >> 13) & 1;
  cb1 = cb1 ^ (data >> 15) & 1;
  
  cbits = cb16 | cb8 | cb4 | cb2 | cb1;
  return cbits;
}

/*
  secded_store - called by the system to store a word (16-bits) of data.
  Before storing the data in the error-prone memory using store, the 
  function should add check-bits and a parity bit for Single Error
  Correction, Double Error Detection (SECDED). 

  The function store() will take a 22-bit value (since it takes an unsigned
  long (32-bits), the high 10 bits will be cleared.
*/

void secded_store (unsigned int address, unsigned short data) {
  unsigned long data_and_checkbits;
  unsigned long checkbits;
  unsigned long parity;
  //calculates the checkbits/parity, places it in locs 17-22
  data_and_checkbits = calccheckbits(data) << 16;
  data_and_checkbits = data_and_checkbits | data;
  parity = calcparity(data_and_checkbits);
  data_and_checkbits = data_and_checkbits | parity;
  
  store(address, data_and_checkbits);
}

/*
  secded_load - called by the system to load a word (16-bits) of data.
  After loading the data from the error-prone memory using load(), the
  function should verify the validity of the data using the check-bits and
  parity bit.

  The function should return the data in the location pointed to by
  data. The function should return false if there was an uncorrectable error
  that was detected. It should return true if there were no errors or if the
  errors were corrected.
*/

bool secded_load (unsigned int address, unsigned short& data) {
  
  unsigned long unchecked_data;
  unsigned long checked_data;
  unsigned long storeparity;
  unsigned long wrongparity;
  unsigned long checkbits;
  unsigned long uncheckedbits;
  //allows for iteration to swich bits more easily
  unsigned long dchek [16] = {3, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15, 17, 18, 19, 20, 21};
  size_t loc [16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  size_t syndrome;
  
  //using the information from stored memory, calc parity + cbits again
  unchecked_data = load(address);
  wrongparity = calcparity(unchecked_data) >> 21;
  storeparity = unchecked_data >> 21;
  checkbits = calccheckbits(unchecked_data);
  uncheckedbits = unchecked_data >> 16;
  //removes potential parity bit lead
  if (uncheckedbits >> 5)
    uncheckedbits = uncheckedbits ^ (1<<5);
  syndrome = checkbits ^ uncheckedbits;
  
  //if the parity does not match
  if (wrongparity != storeparity) {
    for(size_t i = 0; i < 16; i++) {
      if (syndrome == dchek[i]) {
	data = unchecked_data ^ (1 << loc[i]);
	return true;
      }
    }
    //syndrome is not wrong but parity is wrong - parity has been flipped
    data = unchecked_data;
    return true;
  }
  else {
    //if the parity matches and the checked bits are wrong, 2 bit error
    if (syndrome != 0) {
      return false;
    }
      //syndrome is not wrong, no errors
    else {
      data = unchecked_data;
      return true;
    }
  }
}
