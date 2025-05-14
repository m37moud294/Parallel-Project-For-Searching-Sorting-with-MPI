#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>

bool isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i <= std::sqrt(n); ++i)
        if (n % i == 0)
            return false;
    return true;
}

void runPrimeFinder() {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int low = 0, high = 0;

    if (rank == 0) {
        std::cout << "                ------------------------------\n";
        std::cout << "                 Prime Number Finder Selected\n";
        std::cout << "                ------------------------------\n";
        std::cout << "                 Enter the range to search for primes (low high):   ";
        std::cin >> low >> high;
        std::cout << " \n";
    }
    double startTime = MPI_Wtime();
    // Broadcast range to all
    MPI_Bcast(&low, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&high, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int range = high - low + 1;
    int chunk = range / size;
    int start = low + rank * chunk;
    int end = (rank == size - 1) ? high : start + chunk - 1;

    std::vector<int> localPrimes;
    for (int i = start; i <= end; ++i) {
        if (isPrime(i)) localPrimes.push_back(i);
    }

    // Gather sizes first
    int localSize = localPrimes.size();
    std::vector<int> allSizes(size);
    MPI_Gather(&localSize, 1, MPI_INT, allSizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Gather results to master
    std::vector<int> displs(size, 0);
    int totalSize = 0;
    if (rank == 0) {
        for (int i = 1; i < size; ++i)
            displs[i] = displs[i - 1] + allSizes[i - 1];
        totalSize = displs[size - 1] + allSizes[size - 1];
    }

    std::vector<int> globalPrimes(totalSize);
    MPI_Gatherv(localPrimes.data(), localSize, MPI_INT,
        globalPrimes.data(), allSizes.data(), displs.data(),
        MPI_INT, 0, MPI_COMM_WORLD);

    // Output
    if (rank == 0) {
        std::cout << "    Primes in range [" << low << ", " << high << "]:\n";
        for (int prime : globalPrimes) {
            std::cout << prime << " ";
        }
        std::cout << "\n";
    }
    double endTime = MPI_Wtime();

    double duration = endTime - startTime;
    std::cout << "Search time (rank " << rank << "): " << duration << " seconds" << std::endl;
}
