#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

const int Line_number = 40; 

int *row;
int rank, size, granularity;

void simulation(int rounds) {
  int i, j;
  int bufferSize = sizeof(int) * granularity;
  int *buff = (int *)malloc(sizeof(int) * granularity);
  int *tempRow = (int *)malloc(sizeof(int) * granularity);
  for (i = 0; i < rounds; i++) {
	int sender = rank - 1;
	force_cal(row, tempRow, rank); 
	if (size > 1) {
	  int n = 0;
	  if (rank == 0){
	MPI_Recv(buff, bufferSize, MPI_BYTE, size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Send(row, bufferSize, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
	  } else {
	MPI_Send(row, bufferSize, MPI_BYTE, (rank + 1) % size, 0, MPI_COMM_WORLD);
	MPI_Recv(buff, bufferSize, MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  }
	  while (n < size - 1) {
	if (sender < 0)
	  sender = size - 1;
	force_cal(buff, tempRow, sender);
	MPI_Send(buff, bufferSize, MPI_BYTE, (rank + 1) % size, 0, MPI_COMM_WORLD);
	MPI_Recv(buff, bufferSize, MPI_BYTE, (rank - 1 + size) % size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	sender--;
	n++;
	  }
	}
	memcpy(row, tempRow, sizeof(int) * granularity); 
  }
  free(buff);
  free(tempRow);
}

void initialize(char *fileName) {
  char buffer[Line_number];
  FILE *file;
  if (rank == 0)
  {
	file = fopen(fileName, "rt");
	if (file == NULL) {
	  printf("File doesn't exist");
	  exit(1);
	}
  }
  int i = 0;
  int n = rank;
  while (n < size) {
	if (rank != 0) {
	   MPI_Recv(&buffer, Line_number, MPI_CHAR, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	} else {
	   fgets(buffer, Line_number, file);
	}
	if (i < granularity) {
	  sscanf(buffer, "%d %d %d", &row[0], &row[1], &row[2]);
	  i = 3;
	} else {
	  MPI_Send(&buffer, Line_number, MPI_CHAR, rank + 1, 0, MPI_COMM_WORLD);
	}
	n++;
  }
  if (rank == 0)
	fclose(file);
} 


void termination() {
  int n = rank;
  int sent = 0;
  while (n < size) { 
	  if (sent == 1) {
	MPI_Recv(row, sizeof(int) * granularity, MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  }
	  if (rank == 0) {
	int i;
	for (i = 0; i < granularity; i++)
	  printf("%d ", row[i]);
	printf("\n");
	  } else {
	   	MPI_Send(row, sizeof(int) * granularity, MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD);
	  }
	  sent = 1;
	  n++;
  }
  free(row);
}

void force_cal(int *other, int *tempRow, int sender) {
  int i, result = 0;
  for (i = 0; i < granularity; i++) { 
	result += other[i] * row[i];
  }
  tempRow[sender] = result;
}

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv); 
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  int rounds;
  char *fileName;
  if (argc == 4) {
	  rounds = atoi(argv[1]);
	  granularity = atoi(argv[2]);
	  fileName = argv[3];
  	}
	// } else {
	//   if (rank ==0)
	// printf("USAGE ./assignment2matrix ROUNDS GRANULARITY FILENAME\n");
	//   MPI_Finalize();
	//   return 1;
	//  }
  row = (int *)malloc(sizeof(int) * granularity);
  double ib = MPI_Wtime();
  initialize(fileName); 
  double ie = MPI_Wtime();
  double sb = MPI_Wtime();
  simulation(rounds);
  double se = MPI_Wtime();
  double gb = MPI_Wtime();
  termination();
  double ge = MPI_Wtime();
 
  MPI_Finalize();  
}


//Simulates the ring communication and recalculates new value of force at each step
//updates positions for each round


//Recalculates new value in the matrix at the sender index



//forwards particle information down and root proc prints them