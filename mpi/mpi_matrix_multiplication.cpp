#include <mpi.h>
#include <iostream>
#include <vector>
using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int N = 4;                 // Matrix is N x N
    if (N % size != 0) {
        if (rank == 0) std::cout << "Error: N must be divisible by number of processes.\n";
        MPI_Finalize();
        return 0;
    }

    const int rows_per_rank = N / size;

    int A[N][N], B[N][N], C[N][N];

    // Initialize matrices A and B on root
    if (rank == 0) {
        // Example initialization
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                cin >> A[i][j];       // Simple values for A
            }
        }

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                cin >> B[i][j];          // Simple values for B
            }
        }
    }

    // 1) Broadcast full B to all ranks
    MPI_Bcast(
        &B[0][0],          // buffer
        N * N,             // count (elements)
        MPI_INT,           // datatype
        0,                 // root
        MPI_COMM_WORLD
    );

    // 2) Scatter A rows: each rank gets rows_per_rank * N elements
    int local_A[rows_per_rank][N];
    int local_C[rows_per_rank][N];

    MPI_Scatter(
        rank == 0 ? &A[0][0] : nullptr, // sendbuf (root only)
        rows_per_rank * N,              // sendcount per rank
        MPI_INT,                        // sendtype
        &local_A[0][0],                 // recvbuf
        rows_per_rank * N,              // recvcount
        MPI_INT,                        // recvtype
        0,                              // root
        MPI_COMM_WORLD
    );

    // 3) Local compute: local_C = local_A (rows_per_rank x N) * B (N x N)

    for(int i = 0; i < rows_per_rank; i++) {
        for(int j = 0; j < N; j++) {
            local_C[i][j] = 0;
            for(int k = 0; k < N; k++) {
                local_C[i][j] += local_A[i][k] * B[k][j];
            }
        }
    }

    // 4) Gather local_C rows back into C on root
    MPI_Gather(
        &local_C[0][0],                  // sendbuf (each rank)
        rows_per_rank * N,               // sendcount
        MPI_INT,
        rank == 0 ? &C[0][0] : nullptr,  // recvbuf (root only)
        rows_per_rank * N,               // recvcount per rank
        MPI_INT,
        0,                               // root
        MPI_COMM_WORLD
    );

    // 5) Root prints result matrix C
    if (rank == 0) {
        std::cout << "Result C:\n";
        for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
                std::cout << C[i][j] << " ";
            }
            std::cout << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}
