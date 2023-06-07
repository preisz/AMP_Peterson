#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <omp.h> //Compile with -fopenmp to enable OpenMP.

using autint = std::atomic_uint;
using uint = unsigned int;

class PetersonsFilterLock {
private:
    uint nthreads;
    autint* level;
    autint *victim;

public:
  PetersonsFilterLock( uint nthreads);
  void lock( int dummy );
  void unlock( int dummy );
};

PetersonsFilterLock::PetersonsFilterLock( uint num_threads) {
  this->nthreads = num_threads;
  level = new autint[nthreads];
  victim = new autint[nthreads];
  for(uint i=0; i<nthreads; i++){ level[i].store(0, std::memory_order_seq_cst);}
}

void PetersonsFilterLock::lock( int dummy  ) {
  uint tid = uint ( omp_get_thread_num() ); //who am I?
  for(uint i=1; i<nthreads; i++){ //try to enter lvl j
    level[tid].store( i, std::memory_order_seq_cst ); //seq. consistent
    victim[i].store( tid, std::memory_order_seq_cst );
    for(size_t k=0; k<nthreads; k++){
        while ( k!=tid && level[k].load() >=i 
                && victim[i].load() ==tid) {} //spin
    }
  }
}

void PetersonsFilterLock::unlock( int dummy ) { //neds an argument
  uint tid = uint ( omp_get_thread_num() );
  level[tid].store( 0, std::memory_order_seq_cst );
}