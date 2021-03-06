#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "mpi.h"

#define tag_code 3333


void cal_force(float *data, int size, int gran, int k, float* fx, float* fy, float mass);

int main(int argc,char *argv[])
{
	int size, rank, i, j, k,right, left, rounds, gran;
	static float dt = 1.0;
	
	float mass, vx, vy;
	float fx, fy;
	char* file_name;
	rounds = atoi(argv[1]);
	gran = atoi(argv[2]);
	file_name = argv[3];

	
    //initilize the MPI environment
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	
	MPI_Status recv_status;
	int bodies = size * gran;
	float data[bodies * 3];
	float temp[gran * 3];

	right = rank + 1;
 	if(right == size) right=0;
  	left=rank-1;
  	if(left == -1) left = size-1;

	
	
	
	FILE *fp;
	float *p = data;

	//Initialization Part

		//Store each value in data[]

		fp = fopen(file_name, "r");
	   	for (i = 0; i < bodies * 3 ; i++)
	  	{
	  		fscanf(fp, "%f", p);
			p++;
		}
		p = data;
		
		
	

	//Simulation Part
	
	for(k = 0; k < gran*3; k++){
		temp[k] = data[k];
	}
	
	double timestamp1 = MPI_Wtime();
	printf("Simulate start time: %lf\n", timestamp1);
	
	for(j = 0; j < rounds; j++){
		p = data;
		
		for(k = 0; k < gran; k++){
			mass = *(p + k*3 + 2);
			cal_force(data, size, gran, k, &fx, &fy, mass);
			vx += fx*dt/mass;
			vy += fy*dt/mass;
			temp[k*3] = data[k*3] + vx*dt;
			temp[1 + k*3] = data[k*3 +1] + vy*dt;
		}
		for(k = 0; k < gran; k++){
			data[k*3] = temp[k*3];
			data[k*3 + 1] = temp[k*3+1];
		}
	}
	
	timestamp1 = MPI_Wtime();
	printf("Simulate end time: %lf\n", timestamp1);
	
	
	//Gathering Part
	
	p = data;
	if(rank == 0){
		for(j = 0; j < bodies; j++){
			printf("%f\t%f\t%f\n", *p, *(p+1), *(p+2));
			p+=3;
		}
	}
	
	MPI_FINALIZE();
	
}


void cal_force(float *data, int size, int gran, int k, float* fx, float* fy, float mass) {
	int i;
	float xdiff,ydiff,r2;
	float fx_t=0,fy_t=0,F;
	float G=6.67384*pow(10,-11);  //gravitational constant

	for(i = 0; i < size * gran; i++) {
		if(i!=k){
			xdiff=data[i*3]-data[k*3];
			ydiff=data[i*3 + 1]-data[k*3 + 1];

			r2=pow(xdiff,2)+pow(ydiff,2);
			if(r2!=0) F=G*data[i*3+2]*mass/r2;
			else F=0;

			if(xdiff!=0 || r2!=0) fx_t += F*xdiff/sqrt(r2);
			if(ydiff!=0 || r2!=0) fy_t += F*ydiff/sqrt(r2);
		}
	}

	*fx=fx_t;
	*fy=fy_t;

	return;
}
