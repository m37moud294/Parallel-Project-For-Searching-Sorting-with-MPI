#include <mpi.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>

//  Bitonic Sequence
void bitonicMerge(std::vector<int>& arr, int low, int cnt, bool dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        for (int i = low; i < low + k; i++) {
            if (dir == (arr[i] > arr[i + k])) {
                std::swap(arr[i], arr[i + k]);
            }
        }
        bitonicMerge(arr, low, k, dir);
        bitonicMerge(arr, low + k, k, dir);
    }
}

//  Bitonic Sort
void bitonicSort(std::vector<int>& arr, int low, int cnt, bool dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        bitonicSort(arr, low, k, true);
        bitonicSort(arr, low + k, k, false);
        bitonicMerge(arr, low, cnt, dir);
    }
}
double startTime_bitonic =0;
//  Bitonic Sort using MPI
void runBitonicSort() {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> data;
    int totalSize = 0;

    if (rank == 0) {
        std::string inputFile;
        std::cout << "               Enter input file path: ";
        std::cin >> inputFile;

        std::ifstream infile(inputFile);
        if (!infile) {
            std::cerr << "             Error: Cannot open file.\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        startTime_bitonic = MPI_Wtime();
        
        int val;
        while (infile >> val)
            data.push_back(val);
        infile.close();

        // ensure data is 2 power n
        totalSize = data.size();
        int nextPow2 = std::pow(2, std::ceil(std::log2(totalSize)));

        while (data.size() < nextPow2)
            data.push_back(999999); 

        totalSize = data.size();
    }

    // ‰‘— ÕÃ„ «·»Ì«‰« 
    MPI_Bcast(&totalSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int localSize = totalSize / size;
    std::vector<int> localData(localSize);

    // distribute data
    MPI_Scatter(data.data(), localSize, MPI_INT,
        localData.data(), localSize, MPI_INT,
        0, MPI_COMM_WORLD);

    // Bitonic Sort in each local process
    bitonicSort(localData, 0, localSize, true);

    // collect data to rank 0
    std::vector<int> sortedData(totalSize);
    MPI_Gather(localData.data(), localSize, MPI_INT,
        sortedData.data(), localSize, MPI_INT,
        0, MPI_COMM_WORLD);

    if (rank == 0) {
        // «· — Ì» «·‰Â«∆Ì ›Ì «·⁄„·Ì… 0
        bitonicSort(sortedData, 0, totalSize, true);

        // ≈“«·… «·Õ‘Ê (999999)
        while (!sortedData.empty() && sortedData.back() == 999999)
            sortedData.pop_back();

        //calc time
        double endTime = MPI_Wtime();
        double duration = endTime - startTime_bitonic;
        std::cout << "Search time (rank " << rank << "): " << duration << " seconds" << std::endl;
        // save data to file
        std::ofstream outfile("output_bitonic.txt");
        for (int x : sortedData)
            outfile << x << " ";
        outfile.close();

        std::cout << "      \n Done # \n   Sorted data saved to output_bitonic.txt\n";
    }
}
