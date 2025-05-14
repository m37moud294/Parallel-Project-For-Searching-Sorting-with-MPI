#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>


void runQuickSearch() {
    

    int rank, size;    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::string inputFile;
    int target;

    std::vector<int> data;

    if (rank == 0) {
        std::cout << "               ------------------------------\n";
        std::cout << "                Quick Search Selected\n";
        std::cout << "               ------------------------------\n";
        std::cout << "                Please enter the path to the input file: ";
        std::cin >> inputFile;
        std::cout << " \n";

        std::ifstream infile(inputFile);
        if (!infile) {
            std::cerr << "      Sorry! \n  Cannot open file: " << inputFile << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        int val;
        while (infile >> val) {
            data.push_back(val);
        }
        infile.close();

        std::cout << "          Enter Search Target: ";
        std::cin >> target;
        std::cout << "          Reading data from file...\n";
        std::cout << "          Distributing data across processes...\n";
    }
    double startTime = MPI_Wtime();
    // Broadcast size of array and target
    int totalSize = data.size();
    MPI_Bcast(&totalSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&target, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Resize for workers
    if (rank != 0) data.resize(totalSize);

    // Broadcast data to all processes
    MPI_Bcast(data.data(), totalSize, MPI_INT, 0, MPI_COMM_WORLD);

    // Define chunk
    int chunkSize = totalSize / size;
    int startIdx = rank * chunkSize;
    int endIdx = (rank == size - 1) ? totalSize : startIdx + chunkSize;

    // Each process searches its part
    //double t1 = MPI_Wtime();
    int localIndex = -1;
    for (int i = startIdx; i < endIdx; ++i) {
        if (data[i] == target) {
            localIndex = i;
            break;
        }
    } //double t2 = MPI_Wtime();

    // Master gathers results
    int globalIndex;
    MPI_Reduce(&localIndex, &globalIndex, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        if (globalIndex != -1)
            std::cout << "       Result: Value " << target << "   found at index " << globalIndex << "\n";
        else        
            std::cout << "       Result: Value " << target << "   not found.\n";
    }
 double endTime = MPI_Wtime();

double duration = endTime - startTime;
std::cout << "Search time (rank " << rank << "): " << duration << " seconds" << std::endl;

}
