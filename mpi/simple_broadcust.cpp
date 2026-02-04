#include <mpi.h>
#include <iostream>
#include <cstring>
using namespace std;

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int const MSG_SIZE = 50;
    char message[MSG_SIZE];
    if(rank == 0){
        strcpy(message, "Broadcast message from Process 0");
    }

    MPI_Bcast(message, MSG_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
    cout << "Process " << rank << " received message: " << message << endl;

    MPI_Finalize();
    return 0;
}