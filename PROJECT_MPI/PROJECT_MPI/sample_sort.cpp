#include <mpi.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

double startTime_sample=0;
//  ‰›Ì– Sample Sort »«” Œœ«„ MPI
void runSampleSort() {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<int> data;
    int totalSize = 0;

    if (rank == 0) {
        std::string inputFile;
        std::cout << "                     Enter input file path: ";
        std::cin >> inputFile;

        std::ifstream infile(inputFile);
        if (!infile) {
            std::cerr << "                  Error: Cannot open file.\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
         startTime_sample = MPI_Wtime();

        int val;
        while (infile >> val)
            data.push_back(val);
        infile.close();

        totalSize = data.size();
    }

    // ≈—”«· ÕÃ„ «·»Ì«‰« 
    MPI_Bcast(&totalSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int localSize = totalSize / size;
    std::vector<int> localData(localSize);

    //  Ê“Ì⁄ «·»Ì«‰« 
    MPI_Scatter(data.data(), localSize, MPI_INT,
        localData.data(), localSize, MPI_INT,
        0, MPI_COMM_WORLD);

    //  — Ì» «·»Ì«‰«  „Õ·Ì«
    std::sort(localData.begin(), localData.end());

    // «Œ Ì«— «·⁄Ì‰«  „‰ ﬂ· ⁄„·Ì…
    std::vector<int> samples(size);
    for (int i = 0; i < size; ++i) {
        int idx = (i + 1) * localSize / (size + 1);
        samples[i] = localData[idx];
    }

    // Ã„⁄ «·⁄Ì‰«  ›Ì «·⁄„·Ì… 0
    std::vector<int> gatheredSamples;
    if (rank == 0)
        gatheredSamples.resize(size * size);

    MPI_Gather(samples.data(), size, MPI_INT,
        gatheredSamples.data(), size, MPI_INT,
        0, MPI_COMM_WORLD);

    // «Œ Ì«— «·›Ê«’· (pivots)
    std::vector<int> pivots(size - 1);
    if (rank == 0) {
        std::sort(gatheredSamples.begin(), gatheredSamples.end());
        for (int i = 1; i < size; ++i) {
            pivots[i - 1] = gatheredSamples[i * size];
        }
    }

    // ≈—”«· «·›Ê«’· ·ﬂ· «·⁄„·Ì« 
    MPI_Bcast(pivots.data(), size - 1, MPI_INT, 0, MPI_COMM_WORLD);

    //  ﬁ”Ì„ «·»Ì«‰«  Õ”» «·›Ê«’·
    std::vector<std::vector<int>> buckets(size);
    for (int val : localData) {
        int i = 0;
        while (i < size - 1 && val > pivots[i]) ++i;
        buckets[i].push_back(val);
    }

    // ≈—”«· Ê«” ﬁ»«· «·√ÕÃ«„
    std::vector<int> sendCounts(size), recvCounts(size);
    for (int i = 0; i < size; ++i)
        sendCounts[i] = buckets[i].size();

    MPI_Alltoall(sendCounts.data(), 1, MPI_INT,
        recvCounts.data(), 1, MPI_INT,
        MPI_COMM_WORLD);

    //  Õ÷Ì— «·≈—”«· Ê«·«” ﬁ»«·
    std::vector<int> sendData, recvData;
    std::vector<int> sdispls(size), rdispls(size);
    int totalSend = 0, totalRecv = 0;

    for (int i = 0; i < size; ++i) {
        sdispls[i] = totalSend;
        totalSend += sendCounts[i];
    }

    for (const auto& bucket : buckets)
        sendData.insert(sendData.end(), bucket.begin(), bucket.end());

    for (int i = 0; i < size; ++i) {
        rdispls[i] = totalRecv;
        totalRecv += recvCounts[i];
    }

    recvData.resize(totalRecv);

    //  »«œ· «·»Ì«‰«  «·›⁄·Ì…
    MPI_Alltoallv(sendData.data(), sendCounts.data(), sdispls.data(), MPI_INT,
        recvData.data(), recvCounts.data(), rdispls.data(), MPI_INT,
        MPI_COMM_WORLD);

    //  — Ì» «·‰ «∆Ã „Õ·Ì«
    std::sort(recvData.begin(), recvData.end());

    // collect data on processor 0
    std::vector<int> finalSorted;
    if (rank == 0)
        finalSorted.resize(totalSize);

    std::vector<int> recvSizes(size);
    int recvSize = recvData.size();

    MPI_Gather(&recvSize, 1, MPI_INT,
        recvSizes.data(), 1, MPI_INT,
        0, MPI_COMM_WORLD);

    std::vector<int> displs(size);
    if (rank == 0) {
        displs[0] = 0;
        for (int i = 1; i < size; ++i)
            displs[i] = displs[i - 1] + recvSizes[i - 1];
    }

    MPI_Gatherv(recvData.data(), recvData.size(), MPI_INT,
        finalSorted.data(), recvSizes.data(), displs.data(), MPI_INT,
        0, MPI_COMM_WORLD);

    //calc time
    double endTime = MPI_Wtime();
    double duration = endTime - startTime_sample;
    std::cout << "Search time (rank " << rank << "): " << duration << " seconds" << std::endl;

    if (rank == 0) {
        std::ofstream outfile("output_samplesort.txt");
        for (int x : finalSorted)
            outfile << x << " ";
        outfile.close();
        std::cout << "      \n Done # \n   Sorted data saved to output_samplesort.txt\n";
    }
}
