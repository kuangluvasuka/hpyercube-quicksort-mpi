#include <iostream>
#include <cstdlib>
#include <vector>
#include <time.h>
#include <math.h>
#include <mpi.h>

using namespace std;
const int kMax =  99;
const int kElementPerNode = 4;

int GetPivot(vector<int> &vec);
int QuickSort(vector<int> &vec, int pivot);
void QuickSort(vector<int> &vec, int left, int right);

int main(int argc, char **argv)
{
  int world_rank, local_rank, num_procs, color, proc_per_cube;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Status status;

  /* Generate random integers for the list */
  int dim = log2(num_procs);
  vector<int> list, list_final;

  cout << "rank is: " << world_rank << ", the random list is:";
  srand(time(NULL) * ((unsigned)world_rank+1));
  for (int i = 0; i < kElementPerNode; i++) {
    list.push_back(rand() % kMax);
    cout << " " << list.back();
  }
  cout << endl;

  /* Hybercube quicksort */
  int bitvalue = num_procs >> 1, mask = num_procs - 1;

  for (int i = dim; i > 0; i--) {
    proc_per_cube = 1 << i;
    color = world_rank / proc_per_cube;

    MPI_Comm new_comm;
    MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &new_comm);
    MPI_Comm_rank(new_comm, &local_rank);

    int pivot;
    if (local_rank == 0) {
      pivot = GetPivot(list);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&pivot, 1, MPI_INT, 0, new_comm);

    int pos = QuickSort(list, pivot);
    int partner = world_rank ^ bitvalue, send_size, recv_size;

    if ((world_rank & bitvalue) == 0) {
      send_size = list.size() - pos;
      MPI_Send(&send_size, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
      MPI_Recv(&recv_size, 1, MPI_INT, partner, 1, MPI_COMM_WORLD, &status);

      if (send_size != 0) {
        MPI_Send(&list[pos], send_size, MPI_INT, partner, 2, MPI_COMM_WORLD);
        list.erase(list.begin()+pos, list.end());
      }

      if (recv_size != 0) {
        vector<int> buf(recv_size);
        MPI_Recv(buf.data(), recv_size, MPI_INT, partner, 3, MPI_COMM_WORLD, &status);
        list.insert(list.end(), buf.begin(), buf.end());
      }

    } else {
      send_size = pos;
      MPI_Send(&send_size, 1, MPI_INT, partner, 1, MPI_COMM_WORLD);
      MPI_Recv(&recv_size, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);

      if (send_size != 0) {
        MPI_Send(list.data(), send_size, MPI_INT, partner, 3, MPI_COMM_WORLD);
        list.erase(list.begin(), list.begin()+send_size);
      }

      if (recv_size != 0) {
        vector<int> buf(recv_size);
        MPI_Recv(buf.data(), recv_size, MPI_INT, partner, 2, MPI_COMM_WORLD, &status);
        list.swap(buf);
        list.insert(list.end(), buf.begin(), buf.end());
      }
    }
    
    mask ^= bitvalue;
    bitvalue /= 2;
    MPI_Comm_free(&new_comm);
  }

  /* Sequential quicksort */
  if (list.size() != 0) {
    QuickSort(list, 0, list.size() - 1);
  }

  cout << "~~~rank is: " << world_rank << ", the sorted list is: ";
  for (vector<int>::iterator it = list.begin(); it != list.end(); it++) {
    cout << *it << " ";
  }
  cout << endl;

  MPI_Finalize();

  return 0;
}

/* utility func */
int GetPivot(vector<int> &vec)
{
  int i = vec.size() / 2;
  return vec[i];
}

int QuickSort(vector<int> &vec, int pivot)
{
  int i = 0, j = vec.size() - 1;
  while (i <= j) {
    while (vec[i] < pivot && i < vec.size()) {
      i++;
    }
    while (vec[j] > pivot && j >= 0) {
      j--;
    }
    if (i <= j) {
      swap(vec[i], vec[j]);
      i++;
      j--;
    }
  }

  return i;
}

void QuickSort(vector<int> &vec, int left, int right)
{
  int pivot = vec[(left + right) / 2];
  int i = left, j = right;
  while (i <= j) {
    while (vec[i] < pivot && i <= right) {
      i++;
    }
    while (vec[j] > pivot && j >= left) {
      j--;
    }
    if (i <= j) {
      swap(vec[i], vec[j]);
      i++;
      j--;
    }
  }

  if (left < j) {
    QuickSort(vec, left, j);
  }
  if (i < right) {
    QuickSort(vec, i, right);
  }

}

