/*
*	Luis Ospina
*	Sleeping Barbers Problem with n number of barbers and m number of clients
*	Compile as:
*		gcc sleepingBarber.c -o  sleepingBarber -lpthread
*	Execute as:
*		./sleepingBarber <Number of Barbers> <Number of Clients> <Number of chairs on the waiting room>
* 
* Code based on Grant Braught's solution for the Sleeping Barber
* http://users.dickinson.edu/~braught/courses/cs354s00/classes/code/SleepBarber.src.html
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

#define ANSI_COLOR_BASE    "\x1b["
#define ANSI_BOLD		   "\x1b[1m"
#define MAX_TRAVELTIME 40
#define MAX_CUTTINGTIME 20
//cuttingChair is where the costumers will seat for their hair to be cut off
sem_t cuttingChair;
//sleepingBarber is the barber's chair where they sleep if there aren't current clients 
sem_t sleepingBarber;
//waitingRoom is where the costumers will wait for their hair to be cut off
sem_t waitingRoom;
//barberCloth is the cloth that a costumer needs to have on when his hair is being cut off
sem_t barberCloth;
//A flag for the barbers to know if they have more clients to attend
int costumersLeft = 0;

void *costumerMain(void *number){
	//Parse costumer's ID
	int id = *(int *)number;
	//The time that takes the costumer to arrive at the Barber Shop
	int travelTime = rand()%MAX_TRAVELTIME;
	sleep(travelTime);
	//ANSI color (For formated output)
	int color = 31+rand()%8;

	printf(ANSI_COLOR_BASE"%dmCostumer %d arrived at the Barber Shop" ANSI_COLOR_RESET "\n",color,id);
	//Wait for turn on the waiting room
	sem_wait(&waitingRoom);
	printf(ANSI_COLOR_BASE "%dmCostumer %d entered waiting room" ANSI_COLOR_RESET "\n",color,id);
	//Wait for turn on the cutting Chair
	sem_wait(&cuttingChair);
	printf(ANSI_COLOR_BASE "%dmCostumer %d's hair is being cut" ANSI_COLOR_RESET "\n",color,id);
	//Free the seat on waiting room 
	sem_post(&waitingRoom);
	//Wake up the barber that is actually sleeping
	sem_post(&sleepingBarber);
	//The barber puts the cloth to the costumer
	sem_wait(&barberCloth);
	printf(ANSI_COLOR_BASE "%dmCostumer %d's hair has been cut" ANSI_COLOR_RESET "\n",color,id);
	//The haircut has finished, the costumer leaves the Barber Shop satisfied (Probably)
	sem_post(&cuttingChair);
	printf(ANSI_COLOR_BASE "%dmCostumer %d is leaving the Barber Shop" ANSI_COLOR_RESET "\n",color,id);
}

void *barberMain(void *number){
	//Parse barber's ID
	int id = *(int *)number;
	//The time that will take to the Barber to cut the costumer's hair
	int cuttingTime;
	int color = 31+rand()%8;
	while(!costumersLeft){
		//Barber sleeps 
		printf(ANSI_COLOR_BASE "%d;1mBarber %d is sleeping" ANSI_COLOR_RESET "\n",color,id);
		sem_wait(&sleepingBarber);
		//A costumer wakes the Barber up
		if(!costumersLeft){
			printf(ANSI_COLOR_BASE "%d;1mBarber %d woke up and is cutting hair" ANSI_COLOR_RESET "\n",color,id);
			//Initialize the time that the barber will take to cut the costumer's hair
			cuttingTime = rand()%MAX_CUTTINGTIME;
			//Let's cut that hair
			sleep(cuttingTime);
			printf(ANSI_COLOR_BASE "%d;1mBarber %d has finished cutting hair after %d seconds" ANSI_COLOR_RESET "\n",color,id,cuttingTime);
			//Take off the cloth from the costumer
			sem_post(&barberCloth);
		}else{
			//There are no more clients, the barber will go home and yes, he'll probably sleep there too
			printf(ANSI_COLOR_BASE "%d;1mBarber %d finished his duty for today" ANSI_COLOR_RESET "\n",color,id);
		}
		

	}
}

int main(int argc, char *argv[]){	
	if(argc != 4){
		printf("Usage: ./sleepingBarber <Number of barbers> <Numbers of clients> <Numbers of chair on the waiting room>\n");
		exit(0);
	}
	int i;
	int numBarbers = atoi(argv[1]);
	int numClients = atoi(argv[2]);
	int numChairs = atoi(argv[3]);
	int barberID[numBarbers];
	int clientID[numClients];

	if(numBarbers == 0){
		//If no Barber is on the shop...
		printf("The Barber Shop is closed today, come back other day!\n");
		exit(0);
	}

	if(numChairs == 0){
		//If there are no chairs...
		printf("The Barber shop is actually remodelling the place, come back other day!\n");
		exit(0);
	}
	//Barber and Costumers threads id's
	pthread_t barber[numBarbers];
	pthread_t costumers[numClients];

	//Initialize the semaphores
	//There are as much cutting chairs as barbers on the shop (Each one can be cutting at the same time, it could be less, so the barbers wait for their coworkers to wake up and cut someone's hair)
	sem_init(&cuttingChair,0,numBarbers);
	//Initialize the number of chairs on the waiting room
	sem_init(&waitingRoom,0,numChairs);
	//Initialize both sleepingBarber and barberCloth on 0 so every barber start sleepings and no cloths are been used at the moment
	sem_init(&sleepingBarber,0,0);	
	sem_init(&barberCloth,0,0);

	srand(time(NULL));
	//Initialize all barbers
	for(i = 0;i<numBarbers;i++){
		barberID[i] = i+1;
		pthread_create(&barber[i],NULL,barberMain,(void *)&barberID[i]);
	}
	//Initialize all clients
	for(i = 0; i<numClients;i++){
		clientID[i] = i+1;
		pthread_create(&costumers[i],NULL,costumerMain,(void *)&clientID[i]);
	}
	//Join clients
	for(i = 0;i<numClients;i++){
		pthread_join(costumers[i],NULL);
	}
	costumersLeft = 1;
	//Wake up all Barbers
	for(i = 0;i<numBarbers;i++){
		sem_post(&sleepingBarber);
	}
	//Tell them they can go home now
	for(i = 0;i<numBarbers;i++){
		pthread_join(barber[i],NULL);
	}

}
