//#include "stdafx.h" 
#include "mpi.h" 
#include <stdio.h> 
#include <stdlib.h> 
int main( int argc, char *argv[] ) 
{ 
   int errs = 0, i, k, neighbourNumber,j; 
   int wsize = 5;
   int topo_type;
   int *index, *edges, *outindex, *outedges,*neighbours;
   MPI_Comm comm1, comm2;
   MPI_Init( &argc, &argv );    //preparation of environment of MPI  
   MPI_Comm_size( MPI_COMM_WORLD, &wsize );  // Get the number of Processors
   if (wsize >= 3) { // If Processor number is more than 3 we can make a graph.
      index = (int*)malloc(wsize * sizeof(int) );
      edges = (int*)malloc(wsize * 2 * sizeof(int) );
           // allocate memory for arrays 
      if (!index || !edges) {   
         printf( "Unable to allocate %d words for index or edges\n", 3 * wsize ); //Error Control if we cannot allocate memory
         fflush(stdout);//buffer ı boşaltır 
         MPI_Abort( MPI_COMM_WORLD, 1 );
      }
      index[0] =2; // We are filling index values of the graph
      index[1]=5; 
      index[2]=6; 
      index[3]=8; 
      index[4]=10;
      edges[0]=1; // We are filling edge values of the graph
      edges[1]=4; 
      edges[2]=0; 
      edges[3]=2; 
      edges[4]=3; 
      edges[5]=1; 
      edges[6]=1; 
      edges[7]=4; 
      edges[8]=0; 
      edges[9]=3;
      
      MPI_Graph_create( MPI_COMM_WORLD, wsize, index, edges, 0, &comm1 );  
      //We are creating our graph
      //MPI_COMM_WORLD is the communicators group we are going to use.  
      // wsize is the number of processors 
      // index and edges are the arrays that we are creating our graphs with. 
      // 0 used if we don’t want to order processes in the group. 
      //comm1 is the communicator which represents the graph. 
      MPI_Comm_dup( comm1, &comm2 );
      // We duplicated our graph. 
      MPI_Topo_test( comm2, &topo_type );
      // Get the type of Toplogy we are using. 
      printf( "The Topology Type of Graphs is %s" , &topo_type);
          
      
      if (topo_type != MPI_GRAPH) { // If Topology type is not graph stop process.
         errs++; 
         printf( "Topo type of duped graph was not graph\n" ); 
         fflush(stdout); 
      } 
      else {// If Topology type  is graph continue our program
         int nnodes, nedges; 
         MPI_Graphdims_get( comm2, &nnodes, &nedges );
      // With using Graphdims we are getting dimensions of index array and edge array.
         if (nnodes != wsize) {
      // And we are controlling if Node number obtained from graphdims same with the number 
      // of processors 
            errs++; 
            printf( "Nnodes = %d, should be %d\n", nnodes, wsize ); 
            fflush(stdout); 
            } 
         if (nedges != 2*wsize) {
            errs++; 
            printf( "Nedges = %d, should be %d\n", nedges, 2*wsize ); 
            fflush(stdout); 
            }
      
      //We are going to obtain arrays that we created graph with. We will use Graphget functions. 
         outindex = (int*)malloc(wsize * sizeof(int) ); //allocate memory for arrays 
         outedges = (int*)malloc(wsize * 2 * sizeof(int)); //allocate memory for arrays 
         MPI_Graph_get( comm2, wsize, 2*wsize, outindex, outedges );  
         // Comm2 is the Communicator we will get arrays from
         // wsize and 2* wsize are the lenghts of arrays. 
         // outindex and outedges are the arrays to write graph information
         for (i=0; i<wsize; i++) {
               // We are controlling arrays we obtained with Graph_get if they are same with input arrays.
            if (index[i] != outindex[i]) { 
               printf( "%d = index[%d] != outindex[%d] = %d\n", index[i], i, i, outindex[i] ); 
               fflush(stdout); 
               errs++; 
            }   
         } 
         for (i=0; i<2*wsize; i++) { 
            if (edges[i] != outedges[i]) { 
               printf( "%d = edges[%d] != outedges[%d] = %d\n", edges[i], i, i, outedges[i] ); 
               fflush(stdout); 
               errs++; 
               } 
         }     
         
         
         
         printf( "\n The node count of graph that obtained with MPI_Graphdims_get   Function : " ); 
         printf( "%d", nnodes ); 
         printf( "\n Edge count of graph that obtained with MPI_Graphdims_get Function : " ); 
         printf( "%d", nedges ); 
         printf( "\n-------------------------------------\n"); 
         printf( "Array of indexes that obtained with MPI_Graph_get Function : " );
                               // We are printing arrays obtained with Graph_Get function.
         for (i=0;i<wsize;i++) { 
            printf( "%d ,", outindex[i] ); 
         } 
         printf( "\nArray of Edges that obtained with MPI_Graph_get Function :" ); 
         for ( i=0;i<wsize;i++) 
         { 
            printf( "%d ,", outedges[i] ); 
         } 
         free( outindex );//returns the memory which was allocated for outindex to system.
         free( outedges ); //returns the memory which was allocated for outedges to system.
         printf( "\n-------------------------------------\n");  
         
         
         for(i=0;i<wsize;i++) // We are going to print each Nodes and their  
         // neighbours with using arrays that we obtained.
         { 
            int temp; 
            if(i==0) 
               temp=0; 
            else 
               temp=index[i-1]; 
         
            neighbourNumber=index[i]-temp; //Get each node’s neighbour number.
            printf( "\nMy node no is = %d  and I have %d neighbours", i,neighbourNumber); 
            printf( "\nMy neighbours are : "); 
            for( j=temp; j<index[i];j++) 
            { 
               printf("%s,",edges[j]); 
            } 
            printf("\n"); 
         }
         
         
         printf( "With Using MPI Commands"); 
         printf( "\n-------------------------------------\n"); 
         
         for( k=0;k<wsize;k++) 
         { 
            MPI_Graph_neighbors_count(comm2,k,&neighbourNumber);
         //comm2 is the communicator we get graph’s info.  
         //k is the node number. 
         // neighbourNumber is number of neighbour of “k”;
            MPI_Graph_neighbors(comm2,k,neighbourNumber,neighbours); 
         //k is the node number. 
         // neighbourNumber is number of neighbour of “k”. 
         // neighbour is the array neighbours of k will be write.
            printf( "My node no is = %d and I have %d neighbours\n", k,neighbourNumber); 
            printf( "My neıghbours are : "); 
            for(i=0;i<neighbourNumber;i++) 
            { 
               printf("%s,",neighbours[i]); 
            } 
         }
         
         free( index ); //return the allocated memory to the system.
         free( edges ); // return the allocated memory to the system.
         MPI_Comm_free( &comm2 ); // Empty comm2 and give to system.
         MPI_Comm_free( &comm1 ); //Empty comm1 and give to system.
         } 
      MPI_Finalize(); //Finish MPI
      return 0; 
   }
}
