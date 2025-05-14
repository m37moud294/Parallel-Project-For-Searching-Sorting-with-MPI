#include <mpi.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

// bring the biggest value
int getMax(const std::vector<int>& arr) {
    return *std::max_element(arr.begin(), arr.end());
}


void countSort(std::vector<int>& arr, int exp) {
    int n = arr.size();
    std::vector<int> output(n);
    int count[10] = { 0 };

    for (int i = 0; i < n; i++)
        count[(arr[i] / exp) % 10]++;

    for (int i = 1; i < 10; i++)
        count[i] += count[i - 1];

    for (int i = n - 1; i >= 0; i--) {
        int digit = (arr[i] / exp) % 10;
        output[count[digit] - 1] = arr[i];
        count[digit]--;
    }

    arr = output;
}
double startTime_radix=0;
// Radix Sort
void runRadixSort() {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> data;
    int totalSize = 0;

    // processor reads the input file
    if (rank == 0) {
        std::string inputFile;
        std::cout << "                Enter input file path: ";
        std::cin >> inputFile;

        std::ifstream infile(inputFile);
        if (!infile) {
            std::cerr << "             Error: File not found.\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
         startTime_radix = MPI_Wtime();

        int val;
        while (infile >> val)
            data.push_back(val);
        infile.close();

        totalSize = data.size();
    }


    MPI_Bcast(&totalSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int localSize = totalSize / size;
    std::vector<int> localData(localSize);

    // distribute data on processes
    MPI_Scatter(data.data(), localSize, MPI_INT,
        localData.data(), localSize, MPI_INT,
        0, MPI_COMM_WORLD);

    // find the biggest value in processes
    int localMax = getMax(localData);
    int maxVal = 0;
    MPI_Allreduce(&localMax, &maxVal, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

    // implement radix to each process
    for (int exp = 1; maxVal / exp > 0; exp *= 10)
        countSort(localData, exp);

    // master proccesor collect results
    std::vector<int> sortedData(totalSize);
    MPI_Gather(localData.data(), localSize, MPI_INT,
        sortedData.data(), localSize, MPI_INT,
        0, MPI_COMM_WORLD);

    //final sort
    if (rank == 0) {
        std::sort(sortedData.begin(), sortedData.end());

        double endTime = MPI_Wtime();

        double duration = endTime - startTime_radix;
        std::cout << "Search time (rank " << rank << "): " << duration << " seconds" << std::endl;
        std::ofstream outfile("output_radixsort.txt");
        for (int x : sortedData)
            outfile << x << " ";
        outfile.close();

        std::cout << "      \n Done # \n   Sorted data saved to output_radixsort.txt\n";
    }
}
