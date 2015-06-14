/*
*	Luis Ospina
*	Producer Consumer problem in a Pizzeria! using semaphores.
	   	For this solution, there will be a number of consumers and a number of producers,
	 	the program finishes when all the consumers are satisfied, that is, in order to 
*		establish a finishing point for the algorithm. It could also be implemented with
*		one producer, one costumer and a number of elements needed or this could be made
*		setting the number of producers to 1 and the number of consumers to 1 and just
*		changing the number of pizzas the consumer will ask for and the number of pizzas
*		that could be prepared.
*
*	Compile as:
*		gcc producerConsumer.c -o  producerConsumer -lpthread
*	Execute as:
*		./producerConsumer <Number of pizza producers> <Number of pizza consumers>
*
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define ANSI_COLOR_BASE    "\x1b["
#define ANSI_COLOR_RESET   "\x1b[0m"
//The maximum time that takes a producer to make a pizza
#define MAX_COOKINGTIME 20
//The maximum time that takes a consumer to eat one single pizza
#define MAX_EATINGTIME 10
//The maximum amount of pizzas that a client can ask for PS: There's never too much pizza
#define MAX_PIZZASPERCLIENT 3
//The maximum amount of pizzas that can be prepared and waiting for a client to come at the same time
#define MAX_PIZZASPREPARED 5
//The maximum amount of time that can take a consumer to go to the pizzeria


//pizza is the semaphore holding all the pizza that has been prepared
sem_t pizza;
//tooMuchPizza is the semaphore that tells the producers they shouldn't prepare more pizza PS: Again, there's never too much pizza
sem_t tooMuchPizza;
//A flag for the producer to know if the job has finished yet or no
int finished = 0;

void *consumerMain(void *number){
	//Parse the consumers ID
	int id = *(int *)number;
	int i;
	int value;
	int eatingTime;
	//ANSI color (For formated output)
	int color = 31+rand()%8;
	//How many pizzas will this consumer want for today? (Making sure he'll ask for at least once)
	int numPizzas = rand()%MAX_PIZZASPERCLIENT+1;
	printf(ANSI_COLOR_BASE "%dmConsumer %d arrived to the Pizzeria and will take %d Pizzas!" ANSI_COLOR_RESET "\n",color,id,numPizzas);
	for(i=0;i<numPizzas;i++){
		//Wait for the chiefs to cook the pizzas
		sem_wait(&pizza);
		sem_getvalue(&tooMuchPizza,&value);
		if(value < MAX_PIZZASPREPARED){
			//Tell the producers you got one of the pizzas that is prepared
			sem_post(&tooMuchPizza);
		}
		printf(ANSI_COLOR_BASE "%dmConsumer %d got %d from %d Pizzas!" ANSI_COLOR_RESET "\n",color,id,i+1,numPizzas);
	}
	printf(ANSI_COLOR_BASE "%dmConsumer %d is eating!" ANSI_COLOR_RESET "\n",color,id);
	//The consumer now has to eat all of the pizzas and then can leave the Pizzeria!
	eatingTime = (rand()%MAX_EATINGTIME)*numPizzas;
	sleep(eatingTime);
	printf(ANSI_COLOR_BASE "%dmConsumer %d finished eating and leaves satisfied!" ANSI_COLOR_RESET "\n",color,id);
	return;
}

void *producerMain(void *number){
	//Parse the producer ID
	int id = *(int *)number;
	int value;
	//ANSI color (For formated output)	
	int color = 31+rand()%8;
	while(!finished){
		//The producer will wait until he/she can make a new pizza
		sem_wait(&tooMuchPizza);
		//Check if there are clients left
		if(!finished){
			//Initialize the cooking time for the new pizza
			int cookingTime = rand()%MAX_COOKINGTIME;
			printf(ANSI_COLOR_BASE "%d;1mProducer %d is cooking a new Pizza!"ANSI_COLOR_RESET"\n",color,id);
			sleep(cookingTime);
			sem_post(&pizza);
			sem_getvalue(&pizza,&value);
			printf(ANSI_COLOR_BASE "%d;1mProducer %d delivered a Pizza, there are currently %d!"ANSI_COLOR_RESET"\n",color,id,value);
		}
		//This has to be separated in case there's a producer cooking a pizza when everything ends
		if(finished){
			//Check if there are more pizzas preparated
			sem_getvalue(&pizza,&value);
			if(value > 0){
				//EAT IT!!!!
				sem_wait(&pizza);
				printf(ANSI_COLOR_BASE "%d;1mProducer %d ate a pizza!"ANSI_COLOR_RESET"\n",color,id);
			}
			//The producer can go home now
			printf(ANSI_COLOR_BASE "%d;1mProducer %d is going home now!"ANSI_COLOR_RESET"\n",color,id);
		}
	}	
	return;
	
}

int main(int argc, char *argv[]){
	if (argc != 3){
		printf("Usage: ./producerConsumer <Number of pizza producers> <Number of pizza consumers>\n");
		exit(0);
	}
	int i;
	int numProducers = atoi(argv[1]);
	int numConsumers = atoi(argv[2]);
	int producerID[numProducers];
	int consumerID[numConsumers];

	if(numProducers == 0){
		//If there are no producers today....
		printf("The Mario Pizzeria is closed!!\n");
		exit(0);
	}

	if(numConsumers == 0){
		//If there are no consumers today....
		printf("Luigi my brother, the pizza is ours, call the Peach and the Daisy!\n");
		printf("Do not call the Wario again, or I will tell Mama!\n");
		exit(0);
	}

	pthread_t producers[numProducers];
	pthread_t consumers[numConsumers];

	//There are no pizzas at the start of the day :(
	sem_init(&pizza,0,0);
	sem_init(&tooMuchPizza,0,MAX_PIZZASPREPARED);
	//Initialize the seed for rand numbers
	srand(time(NULL));

	//Initialize all the producers
	for(i = 0 ; i < numProducers ; i++){
		producerID[i] = i+1;
		pthread_create(&producers[i],NULL,producerMain,(void *)&producerID[i]);
	}

	//Initialize all the consumers
	for(i=0;i<numConsumers;i++){
		consumerID[i] = i+1;
		pthread_create(&consumers[i],NULL,consumerMain,(void *)&consumerID[i]);
	}

	//Join all consumers
	for(i=0;i<numConsumers;i++){
		pthread_join(consumers[i],NULL);
	}

	finished = 1;
	//Tell the producers they can eat the pizza left and go home
	for(i=0;i<numProducers;i++){
		sem_post(&tooMuchPizza);
	}
	//Wait for all producers to end
	for(i=0;i<numProducers;i++){
		pthread_join(producers[i],NULL);
	}

	//Destroy the evidence! There has never been pizzas in here!
	sem_destroy(&pizza);
	sem_destroy(&tooMuchPizza);
	return 1;
}
