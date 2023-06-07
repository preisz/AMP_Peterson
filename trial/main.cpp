#include "FilterLock2.cpp"
#include "fair_correct.cpp"

#include <iostream>
#include <fstream>
#include <numeric>
#include <atomic>
#include <chrono>
#include <cassert>
#include <omp.h>
#include <vector>

using autint = std::atomic_uint;
using uint = unsigned int;
/*
//Perform MMM, serial version
std::vector<std::vector<int>> matrixMultiplySerial(const std::vector<std::vector<int>>& matrixA, 
            const std::vector<std::vector<int>>& matrixB) {
    int m = matrixA.size();
    int n = matrixA[0].size();
    int p = matrixB[0].size();
    int dummy = 1;

    std::vector<std::vector<int>> result(m, std::vector<int>(p, 0));

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            for (int k = 0; k < n; ++k) {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }          
        }
    }

    return result;
}


void testCorrectnes(auto &lock){
    std::cout << "....Benchmark correctness...\n" << std::endl;
    int control;
    int me;
    bool iscorrect = true;
    #pragma omp parallel private(me)
    {
        me = omp_get_thread_num();
        for(unsigned i = 0; i < 500; i++)
        {
            lock.lock(me);
            control = me;
            int N = 100;
            std::vector<std::vector<int>> matrixA(N, std::vector<int>(N, 1));
            std::vector<std::vector<int>> matrixB(N, std::vector<int>(N, 1));
            matrixMultiplySerial(matrixA, matrixB); //do some job
            
            if(control != me) { //check control if another thread has joined CS
                iscorrect = false;
            }
            lock.unlock(me);
        }
    }

        if(iscorrect) { std::cout << " Lock fulfills mutual exclusion. " << std::endl;}
        else{ std::cout << " Another thread has entered CS during execution.... " << std::endl; } 
    }*/

int main(int argc, char *argv[]){
    assert( argc == 4 );
    int numthreads = std::atoi(argv[1]);
    const uint max_acquire = std::strtoul(argv[2], nullptr, 10);
    int N = std::atoi(argv[3]);
    autint turns [numthreads];  //how often a certain thread acquired lock 
    for (uint tid =0; tid < numthreads; tid ++){ turns[tid].store(0, std::memory_order_seq_cst);  } 

    std::atomic<double> waited_to_getlock [numthreads]; //how long I waited to get the lock
    for (uint tid =0; tid < numthreads; tid ++){ waited_to_getlock[tid].store(0, std::memory_order_seq_cst);  }

    std::atomic<double> spent_inCS [numthreads]; //how long I spent in CS
    for (uint tid =0; tid < numthreads; tid ++){ spent_inCS[tid].store(0, std::memory_order_seq_cst);  } 
 
    PetersonsFilterLock PFLock(numthreads) ;

    //benchmark code:
    testCorrectnes(PFLock);
    benchmark_throughput(numthreads, N, PFLock );
    return 0;
}
