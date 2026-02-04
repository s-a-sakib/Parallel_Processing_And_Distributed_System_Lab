#include <mpi.h>
#include <iostream>
#include <vector>
using namespace std;

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int chunk = 4;
    int n = chunk * size;

    vector <int> global_data(n);

    for(int i = 0; i < n; i++) {
        global_data[i] = i;
    }
    vector <int> local_data(chunk);

    MPI_Scatter(global_data.data(), chunk, MPI_INT, local_data.data(), chunk, MPI_INT, 0, MPI_COMM_WORLD);  

    int local_sum = 0;
    for(int i = 0; i < chunk; i++) {
        local_sum += local_data[i];
    }

    MPI_Gather(&local_sum, 1, MPI_INT, global_data.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(rank == 0) {
        int total_sum = 0;
        for(int i = 0; i < size; i++) {
            cout << i << ": " << global_data[i] << endl;
            total_sum += global_data[i];
        }
        cout << "Total Sum: " << total_sum << endl;
    }

    MPI_Finalize();
    return 0;
}