// PROJECT_MPI.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <mpi.h>
#include <iostream>

#include "quick_search.h"
#include "prime_finder.h"
#include "bitonic_sort.h"
#include "radix_sort.h"
#include "sample_sort.h"



void runQuickSearch();
void runPrimeFinder();
void runBitonicSort();
void runRadixSort();
void runSampleSort();


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char again = 'Y';
    while (again == 'Y' || again == 'y') {
        if (rank == 0) {
            std::cout << "================================================================================================\n";
            std::cout << "                        Welcome to Parallel Algorithm Simulation with MPI\n";
            std::cout << "================================================================================================\n";
            std::cout << "  \n     Please choose an algorithm to execute:\n";
            std::cout << "            1 - Quick Search\n";
            std::cout << "            2 - Prime Number Finding\n";
            std::cout << "            3 - Bitonic Sort\n";
            std::cout << "            4 - Radix Sort\n";
            std::cout << "            5 - Sample Sort\n \n";
            std::cout << "                 Enter the number of the algorithm to run: ";
        }

        int choice;
        if (rank == 0) std::cin >> choice;
        MPI_Bcast(&choice, 1, MPI_INT, 0, MPI_COMM_WORLD);

        switch (choice) {
        case 1: runQuickSearch(); break;
        case 2: runPrimeFinder(); break;
        case 3: runBitonicSort(); break;
        case 4: runRadixSort(); break;
        case 5: runSampleSort();break;
        

        default: if (rank == 0) std::cout << " Invalid choice !!!!.\n"; break;
        }

        if (rank == 0) {
            std::cout << "***********************************************************************************************& \n ";
            std::cout << "           \n Want to try another algorithm? (Y/N): ";
            std::cin >> again;
        }

        MPI_Bcast(&again, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
