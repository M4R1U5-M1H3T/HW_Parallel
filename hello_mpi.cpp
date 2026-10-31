#include <iostream>
#include <mpi.h>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank = 0;
    int size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char processorName[MPI_MAX_PROCESSOR_NAME];
    int nameLength = 0;
    MPI_Get_processor_name(processorName, &nameLength);

    std::cout << "Hello from process " << rank << " of " << size
              << " on " << processorName << std::endl;

    MPI_Finalize();
    return 0;
}
