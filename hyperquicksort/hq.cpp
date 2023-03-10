#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <functional>
#include <mpi.h>

#include "hq.hpp"


int main(int argc, char* argv[]) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 2) {
        if (rank == 0) std::cout << "usage: " << argv[0] << " n" << std::endl;
        return MPI_Finalize();
    }

    long long int n = atoll(argv[1]);

    long long int loc_n = (n/size);
    if (rank == size - 1) loc_n = n - ((size-1) * loc_n);
    
    std::vector<int> x(loc_n);

    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<int> ui(-100000, 100000);
    // std::uniform_int_distribution<int> ui(0, 10);
    auto rng = bind(ui, g);

    generate(std::begin(x), std::end(x), rng);

    MPI_Barrier(MPI_COMM_WORLD);
    auto t0 = MPI_Wtime();

    hyperquick_sort(x, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    auto t1 = MPI_Wtime();

    if (rank == 0) std::cout << (t1 - t0) << std::endl;

    return MPI_Finalize();
} // main
