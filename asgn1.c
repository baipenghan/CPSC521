#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "mpi.h"

#include "mpe.h"
#include "mpe_graphics.h"

#define tag_code 3333
#define grid_size 600




void cal_force(float *data, int size, float* fx, float* fy, float mass);

int main(int argc,char *argv[])
{
	int size, rank, i, j, right, left, rounds, bp;
	static float dt = 1.0;
	
	float mass, vx, vy;
	float fx, fy;
	char* file_name;
	rounds = atoi(argv[1]);
	file_name = argv[2];
	

	
	 
	 MPE_XGraph w;
	
    //initilize the MPI environment
    bp = MPI_Init(&argc,&argv);
    bp = MPI_Comm_size(MPI_COMM_WORLD,&size);
    bp = MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	
	MPI_Status recv_status;
	float data[size * 3];

	right=rank+1;
 	if(right == size) right=0;
  	left=rank-1;
  	if(left == -1) left = size-1;

	
	
	FILE *fp;
	float *p = data;
  
    bp=MPE_Open_graphics(&w,MPI_COMM_WORLD,NULL,0,0,grid_size,grid_size,0);

	//Initialization Part

	if(rank == 0){
		//Store each value in data[]

		fp = fopen(file_name, "r");
	   	for (i = 0; i < size * 3; i++)
	  	{
	  		fscanf(fp, "%f", p);
			p++;
		}
		p = data;
		
		
		bp = MPI_Send(p + 3, (size - rank - 1) * 3, MPI_FLOAT, right, tag_code, MPI_COMM_WORLD);	
	}
	else if(rank == size -1){
		bp = MPI_Recv(data,(size - rank) * 3, MPI_FLOAT, left, tag_code, MPI_COMM_WORLD, &recv_status);
	}
	else{
		bp = MPI_Recv(data,(size - rank) * 3, MPI_FLOAT, left, tag_code, MPI_COMM_WORLD, &recv_status);
		bp = MPI_Send(p + 3, (size - rank - 1) * 3, MPI_FLOAT, right, tag_code, MPI_COMM_WORLD);

	}
	mass = *(p+2);

	//Simulation Part

	for(j = 0; j < rounds; j++){
		p = data;
		for(i = 0; i < size-1; i++){
			bp = MPI_Send(p, 3, MPI_FLOAT, right, tag_code, MPI_COMM_WORLD);
			p += 3;
			bp = MPI_Recv(p, 3, MPI_FLOAT, left, tag_code, MPI_COMM_WORLD, &recv_status);
		}
		cal_force(data, size, &fx, &fy, mass);
		vx += fx*dt/mass;
		vy += fy*dt/mass;
		data[0] += vx*dt;
		data[1] += vy*dt;
		
    	  //make each particle cycle through colour.

  	  MPE_Draw_circle(w, data[0],data[1],3 ,rank+1);
    	  bp=MPE_Update(w);
    	  usleep(100000); //sleep 100ms time

    	//  Delete old particle, by making it white.
    	  if(j < rounds - 1) {
  			  MPE_Draw_circle(w, data[0],data[1],3,MPE_WHITE);
  	  	}
	
		
	}
	
	
	
	
	
	p = data;
	for (i = 0; i < size-1; i++){
		bp = MPI_Send(p, 3, MPI_FLOAT, left, tag_code, MPI_COMM_WORLD);
		p += 3;
		bp = MPI_Recv(p, 3, MPI_FLOAT, right, tag_code, MPI_COMM_WORLD, &recv_status);
	}
	
	//printf("%f\t%f\t%f\n", data[0], data[1], data[2]);
	p = data;
	if(rank == 0){
		for(j = 0; j < size; j++){
			printf("%f\t%f\t%f\n", *p, *(p+1), *(p+2));
			p+=3;
		}
	}
	
    //wait for mouse click before closing the X11 window
    int mouseX,mouseY,button;

    if(rank == 0) {
      do MPE_Get_mouse_press(w,&mouseX,&mouseY,&button);
      while(mouseX==0);
    }

    bp = MPE_Close_graphics(&w);
	
	MPI_FINALIZE();
	
}


void cal_force(float *data, int size, float* fx, float* fy, float mass) {
	int i;
	float xdiff,ydiff,r2;
	float fx_t=0,fy_t=0,F;
	float G=1*pow(10,-2);  //gravitational constant

	for(i=1;i < size;i++) {
	
			xdiff=data[i*3]-data[0];
			ydiff=data[i*3 + 1]-data[1];

			//calculate the force, if r0 is 0, then we have no force.
			r2=pow(xdiff,2)+pow(ydiff,2);
			if(r2!=0) F=G*data[i*3+2]*mass/r2;
			else F=0;

			//output the components of the force, if the x or y difference is 0, we have no force, so ignore.
			if(xdiff!=0 || r2!=0) fx_t += F*xdiff/sqrt(r2);
			if(ydiff!=0 || r2!=0) fy_t += F*ydiff/sqrt(r2);
	
	}

	*fx=fx_t;
	*fy=fy_t;

	return;
}
