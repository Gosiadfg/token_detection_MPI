//mpicc mpi.c -o mpi
//mpirun -np 4 mpi

#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

int main( int argc, char **argv )
{
	int rank, size;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int namelen;
	int x; //token 
	int confirm=2;
	int log[2]; // [last sended token, last received token]
	int resend=0;
	int resend2=0;
	srand(time(NULL));
	int lost;
	int time=0;
	int received =0;

	MPI_Status status;

	MPI_Init( &argc, &argv );

	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Get_processor_name(processor_name,&namelen);

	if (rank==0){
		x=1;
		log[0] = x;
		while(!resend){
			MPI_Send(&x, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
			printf("Node %d -> %d -> Node %d\n", rank, x, rank+1);
			sleep(2);
			MPI_Iprobe(rank+1, 0, MPI_COMM_WORLD, &resend, &status);
		}
		MPI_Recv(&x, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status);
	 	log[1] = x;
		printf("Node %d <- confirm <- Node %d\n", rank, rank+1);
		MPI_Recv(&x, 1, MPI_INT, size-1, 0, MPI_COMM_WORLD, &status);
		MPI_Send(&confirm, 1, MPI_INT, size-1, 0, MPI_COMM_WORLD);
		printf("Node %d <- %d <- Node %d\n", rank, x, size-1);
	}
	else if(rank==size-1){
		while(1){		
			MPI_Iprobe(rank-1, 0, MPI_COMM_WORLD, &resend2, &status);
			if(resend2){
 				MPI_Recv(&x, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status); 
				MPI_Send(&confirm, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
				printf("Node %d <- %d <- Node %d\n", rank, x, rank-1);
			        received=1;
			}
			sleep(1);
			if (received==1)time++;
			if(time==5) break;
		}	
		log[1] = x;
		if(x==0) x=1;
		if(x==1) x=0;
		log[0] = x;
		while(!resend){
			lost=rand()%2;
			if(lost==0)MPI_Send(&x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			printf("Node %d -> %d -> Node %d\n", rank, x, 0);
			MPI_Iprobe(0, 0, MPI_COMM_WORLD, &resend, &status);	
		}
		MPI_Recv(&x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		printf("Node %d <- confirm <- Node %d\n", rank, 0);
	}
	else{
		while(1){		
			MPI_Iprobe(rank-1, 0, MPI_COMM_WORLD, &resend2, &status);
			if(resend2){
				MPI_Recv(&x, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status); 
				MPI_Send(&confirm, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
				printf("Node %d <- %d <- Node %d\n", rank, x, rank-1);
				received=1;
			}
			sleep(1);
			if (received==1)time++;
			if(time==5) break;
		}
		log[1] = x;
		if(x==0) x=1;
		else x=0;
		sleep(1);
		log[0] = x;
		while(!resend){		
			lost=rand()%2;
			if(lost==0)MPI_Send(&x, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
			printf("Node %d -> %d -> Node %d\n", rank, x, rank+1);
			sleep(2);
			MPI_Iprobe(rank+1, 0, MPI_COMM_WORLD, &resend, &status);		
		}
		MPI_Recv(&x, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status);
		printf("Node %d <- confirm <- Node %d\n", rank, rank+1);
	}
	MPI_Finalize();
}
