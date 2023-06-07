#include <iostream>
#include <fstream>
#include <numeric>
#include <atomic>
#include <chrono>
#include <cassert>
#include <omp.h>
#include <vector>


void write_to_file(unsigned tid, double time_waited, double time_CS, unsigned turns, const char *filename) { //write the times of acquired locks
    // Column ordering:
    // ThreadID, cs_entered,  waited(avg) , time in cs (avg), time in cs (total)
    std :: ofstream ofs;

    ofs.open(filename, std :: ofstream :: out | std:: ofstream :: app);
    ofs << tid << ", " << turns << ", " <<
    time_waited/turns << ", " << time_CS/turns << ", "<< time_CS << std :: endl;
    ofs.close();
}

void write_to_file(int nthreads, std::vector<double> executiontime, double serialtime, 
                int problemsize , const char *filename) {
    // Column ordering:
    // numthreads, executiontime Lock/CS/OpenMP, parallel speedup -||- , problemsize
    std :: ofstream ofs;
    ofs.open(filename, std :: ofstream :: out | std:: ofstream :: app);
    std::vector<double> speedup(3, 0);
    for (int i = 0; i<3; i++) { speedup[i] = serialtime/executiontime[i];  }
    ofs << nthreads << ", " << executiontime[0] << ", " << executiontime[1] << ", " 
    << executiontime[2] << ", " << 
    speedup[0] << ", " << speedup[1] << ", " 
    << speedup[2] << ", "<< problemsize <<std :: endl;
    ofs.close();
}

// Function to perform matrix multiplication
std::vector<std::vector<int>> matrixMultiply(const std::vector<std::vector<int>>& matrixA, 
            const std::vector<std::vector<int>>& matrixB, int nthreads) {
    int m = matrixA.size();
    int n = matrixA[0].size();
    int p = matrixB[0].size();

    std::vector<std::vector<int>> result(m, std::vector<int>(p, 0));

    #pragma omp parallel for num_threads(nthreads)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            for (int k = 0; k < n; ++k) {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }

    return result;
}

//Perform MMM with critical section with #omp critical
std::vector<std::vector<int>> matrixMultiplyCritical(const std::vector<std::vector<int>>& matrixA, 
            const std::vector<std::vector<int>>& matrixB, int nthreads) {
    int m = matrixA.size();
    int n = matrixA[0].size();
    int p = matrixB[0].size();
    int dummy = 1;

    std::vector<std::vector<int>> result(m, std::vector<int>(p, 0));

    #pragma omp parallel for num_threads(nthreads)
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < p; ++j) {
            for (int k = 0; k < n; ++k) {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }
            #pragma omp critical
                dummy = dummy * 1; //look how critical affects time. 
        }
    }

    return result;
}

//Perform MMM with critical section with lock(). Should be applicable for every lock with attribute lock()
std::vector<std::vector<int>> matrixMultiplyLock(const std::vector<std::vector<int>>& matrixA, 
            const std::vector<std::vector<int>>& matrixB, auto& PFLock ,int nthreads) {
    int m = matrixA.size();
    int n = matrixA[0].size();
    int p = matrixB[0].size();
    int dummy = 1;

    std::vector<std::vector<int>> result(m, std::vector<int>(p, 0));
    //PetersonsFilterLock PFLock(nthreads) ;

    #pragma omp parallel for num_threads(nthreads)
    for (int i = 0; i < m; ++i) {
        int me = omp_get_thread_num();
        for (int j = 0; j < p; ++j) {
            for (int k = 0; k < n; ++k) {
                result[i][j] += matrixA[i][k] * matrixB[k][j];
            }
            PFLock.lock( me ); //wait until I get the lock
            dummy = dummy * 1; //look how critical affects time. Executed O(N^2) times for NxN matrix
            PFLock.unlock( me ); //release it
                
        }
    }

    return result;
}

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


void benchmark_throughput(int numthreads, int N, auto& PFLock ){
    std::cout << "\n... Benchmark throughput with Matrix-Matrix multiplication, N =" << N << " ....\n"; 
    // Example matrices
    int problemsize = N*N*N; //problem size of MMM
    std::vector<std::vector<int>> matrixA(N, std::vector<int>(N, 1));
    std::vector<std::vector<int>> matrixB(N, std::vector<int>(N, 1));
    
    std::vector<double> times(3, 0.0);
    double time_Serial = 0;

    //MMM with self-made Lock
    {auto start = omp_get_wtime ();
    std::vector<std::vector<int>> result = matrixMultiplyLock(matrixA, matrixB, PFLock ,numthreads);
    auto end = omp_get_wtime ();  double time_lockCS = end - start; times[0] = time_lockCS;
    std::cout << "Lock MMM: " << time_lockCS << std::endl;}
     
    //MMM with #omp critical
    {auto start = omp_get_wtime ();
    std::vector<std::vector<int>> result = matrixMultiplyCritical(matrixA, matrixB, numthreads);
    auto end = omp_get_wtime ();  double time_ompCrit = end - start; times[1] = time_ompCrit;
    std::cout << "Critical MMM: " << time_ompCrit << std::endl;}

    // Perform matrix multiplication. Normal OpenMP, no cs
    {auto start = omp_get_wtime ();
    std::vector<std::vector<int>> result = matrixMultiply(matrixA, matrixB, numthreads);
    auto end = omp_get_wtime ();  double time_noCS = end - start; times[2] = time_noCS;
    std::cout << "OMP MMM: " << time_noCS << std::endl;}
    
    // Perform matrix multiplication. Serial
    {auto start = omp_get_wtime ();
    std::vector<std::vector<int>> result = matrixMultiplySerial(matrixA, matrixB);
    auto end = omp_get_wtime ();  time_Serial = end - start;}
    std::cout << "Serial MMM: " << time_Serial << std::endl;

    const char* filename = "ExTimeFilterLock.csv";
    write_to_file(numthreads, times, time_Serial, problemsize, filename);



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
    }