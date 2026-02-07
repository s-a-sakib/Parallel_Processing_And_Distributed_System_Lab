#include <mpi.h>
#include <iostream>
#include <bits/stdc++.h>
using namespace std;

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    const int N = 50;

    if(N % size != 0){
        if(rank == 0) cout << "Error: N must be divisible by number of processes.\n";
        MPI_Finalize();
        return 0;
    }

    const int rows_per_rank = N / size;

    int A[N][N], B[N][N], C[N][N];

    if(rank == 0){
        for(int i = 0; i < N; i++){
            for(int j = 0; j < N; j++){
                A[i][j] = rand() % 10;
                B[i][j] = rand() % 10;
            }
        }
    }
    MPI_Bcast(&B[0][0], N * N, MPI_INT, 0, MPI_COMM_WORLD);

    int local_A[rows_per_rank][N], local_C[rows_per_rank][N];

    MPI_Scatter(rank == 0? &A[0][0] : nullptr,
        rows_per_rank * N,
        MPI_INT,
        &local_A[0][0],
        rows_per_rank * N,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    double start = MPI_Wtime();
    for(int i = 0; i < rows_per_rank; i++){
        for(int j = 0; j < N; j++){
            local_C[i][j] = 0;
            for(int k = 0; k < N; k++){
                local_C[i][j] += local_A[i][k] * B[k][j];
            }
        }
    }

    double end = MPI_Wtime();

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

    if (rank == 0) {
        cout << "C[10][10] : " << C[10][10] << endl;
        cout << "Time to execute : " << (end - start) * 1000 << "ms" << endl;
    }

    MPI_Finalize();
    return 0;
}