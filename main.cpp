
#include <assert.h>
#include <iostream>
#include <cstddef>
#include <omp.h>
#include <sstream>
#include <vector>
#include <chrono>  // for high_resolution_clock
#include <string>


#include "Ptree.hpp"

void testCorrectnes(auto &lock){

    unsigned control;
    unsigned id;
    #pragma omp parallel private(id) shared(lock, control) 
    {
        id = omp_get_thread_num();
        for(unsigned i = 0; i < 500; i++)
        {
            lock.lock(id);
            //critical section
            control = id;
            //simulate computation time in critical section for uneven threads, memory intensive... 
            if(id%2)
            {
                const int max = 100000;
                std::vector<unsigned> a(max);

                for(unsigned i = 1; i != a.size(); ++i) {
                    for (unsigned j = 2 * i; j < a.size(); j += i) {
                        a[j] += i;
                    }
                }
                for (unsigned i = 1; i != a.size(); ++i) {
                    if(a[i] == i) {
                        //std::cout << i << " is a perfect number!" << std::endl;
                    }
                }
            }
            
            //check control variable after intensive work to see if another thread has joined CS
            if(control != id)
            {
                std::cout << "Attention, not alone in critical section..." << std::endl;
            }
            lock.unlock(id);
        }
        
    }
    std::cout << "___________________________________________________________________" << std::endl;
    std::cout << std::endl;
}


int main(int argc, char *argv[]) {

    //variables
    unsigned numthreads;
    
    std::cout << "Correctnes benchmark started! Measure if lock works" << std::endl;
    std::cout << "Maximum number of threads for this system: " << omp_get_max_threads() << std::endl;

    //parse command line arguments
    assert(argc == 2);
    {
        std::istringstream tmp(argv[1]);
        tmp >> numthreads;
    }
    
    assert(numthreads >= 1);
    if(numthreads > (unsigned)omp_get_max_threads())
    {
        std::cout << "Number of threads too high. Reduced to maximum number of threads! " << std::endl;
        numthreads = omp_get_max_threads(); 
    }

    std::cout << "Benchmark started! Number of threads: " << numthreads << std::endl;
    std::cout << std::endl;

    omp_set_dynamic(0);     // Explicitly disable dynamic teams to have full control over amount of threads
	omp_set_num_threads(numthreads); // Fixed amount of threads used for all consecutive parallel regions


    TournamentTree tournamentlock(numthreads);
    std::cout << "Testing correctness of Petersons Tournament tree lock..." << std::endl;
    testCorrectnes(tournamentlock);

    return EXIT_SUCCESS;
}