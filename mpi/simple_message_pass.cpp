#include <mpi.h>
#include <iostream>
using namespace std;

int main(int argc, char** argv){

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(size < 2){
        cout << "This program requires at least two processes." << endl;
        MPI_Finalize();
        return 1;
    }

    const int MSG_SIZE = 20;
    if(rank == 0){
        char message[MSG_SIZE] = "Hello, Process 1!";
        MPI_Send(message, MSG_SIZE, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        cout << "Process 0 sent message to Process 1." << endl;
    }
    else if(rank == 1){
        char message[MSG_SIZE];
        MPI_Recv(message, MSG_SIZE, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        cout << "Process 1 received message: " << message << endl;
    }
    
    MPI_Finalize();
    return 0;
}