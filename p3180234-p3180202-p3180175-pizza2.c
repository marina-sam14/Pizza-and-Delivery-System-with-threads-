#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "p3180234-p3180202-p3180175-pizza2.h"
#define N 100



//Mutexes
pthread_mutex_t mutex_cookers;
pthread_mutex_t mutex_ovens;
pthread_mutex_t mutex_deliver;
pthread_mutex_t mutex_total_time;

//Conditins
pthread_cond_t bake_cond;
pthread_cond_t ovens_cond;
pthread_cond_t deliver_cond;

//unsigned int sleep(unsigned int seconds);


int id[N];
int Ncust;
unsigned int seed;
int i,rc,next,duration,freeze; //duration = delivery time, freeze: o xronos pou pagwnei i paraggelia,apo ti stigmi pou teleiwnei to bake,mexri na paradwthei
struct timespec start,end;
int numberofPizzas;

double total_time = 0;
double max_time = -1;
double max_freezing = -1;



void *order(void *x){



    int id = *(int *)x; //id pelatwn
    int rc;

	double 	elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec); //xronos pou exei perasei
	numberofPizzas =  rand_r(&seed) %  (Nhigh + 1 - Nlow) + Nlow;




    printf("Hello from order: %d\n",id);


    clock_gettime(CLOCK_REALTIME, &start); //Track starting time.

    //Lock tous cookers
    rc = pthread_mutex_lock(&mutex_cookers);
    if (rc!=0){
        printf("ERROR with cookers' lock %d\n",rc);
        exit(-1);
    }
   while (cookers == 0) {
	printf("Order %d, has not found any available baker. Blocked...\n", id);
	rc = pthread_cond_wait(&bake_cond, &mutex_cookers);
    }


    cookers--;
    sleep(prep*numberofPizzas);//Proetoimasia pizzas
    cookers++; //Release after the end of preparation.


    rc = pthread_cond_signal(&bake_cond);
    rc = pthread_mutex_unlock(&mutex_cookers);



    //Ovens
    rc = pthread_mutex_lock(&mutex_ovens);
    if (rc != 0)
    {
		printf("Error in lock/unlock with code %d\n", rc);
		pthread_exit(&rc);
	}

    while (ovens == 0) {
	printf("There is no ovens for the order %d. Blocked...\n", id);
	rc = pthread_cond_wait(&ovens_cond, &mutex_ovens);
    }
    ovens--;

    sleep(bake);//Xronos psisimatos
    freeze = duration-bake; //diarkeia pagwmatos

    clock_gettime(CLOCK_REALTIME, &end); //Stop ston xrono

    rc = pthread_mutex_unlock(&mutex_ovens);





    //Release ovens
   rc = pthread_mutex_lock(&mutex_ovens);
	if (rc != 0)
    {
		printf("Error in lock/unlock with code %d\n", rc);
		pthread_exit(&rc);
	}
	++ovens;
	pthread_cond_signal(&ovens_cond);
	rc = pthread_mutex_unlock(&mutex_ovens);
	if (rc != 0)
    {
		printf("Error in lock/unlock with code %d\n", rc);
		pthread_exit(&rc);
	}

    //Lock Deliverers
    rc = pthread_mutex_lock(&mutex_deliver);
    if (rc!=0){
        printf("ERROR with bakers'lock %d\n",rc);
        exit(-1);
    }
    while (deliverers == 0) {
	printf("We do not have any deliverer for the order %d. Blocked...\n", id);
	rc = pthread_cond_wait(&deliver_cond, &mutex_deliver);
    }

    deliverers--;

    duration = rand_r(&seed) %  (deliverHigh + 1 - deliverLow) + deliverLow; //Time for delivering
    sleep(2*duration);//o deliveras einai apasxolimenos mexri na paei kai na gyrisei stin pitsaria. opote 2*duration



    rc = pthread_mutex_unlock(&mutex_deliver);
    if (rc != 0)
    {
		printf("Error in lock/unlock with code %d\n", rc);
		pthread_exit(&rc);
    }




    //Release deliverers
   rc = pthread_mutex_lock(&mutex_deliver);
	if (rc != 0)
    {
		printf("Error in lock/unlock with code %d\n", rc);
		pthread_exit(&rc);
	}
	deliverers++;

	printf("Order %d has been delivered in %d minutes while it had been freezing for %d minutes and the deliverer is back! \n", id,duration,freeze);

	pthread_cond_signal(&deliver_cond);
	rc = pthread_mutex_unlock(&mutex_deliver);
	if (rc != 0)
    {
		printf("Error in lock/unlock with code %d\n", rc);
		pthread_exit(&rc);
	}



    //Lock total time
    rc  = pthread_mutex_lock(&mutex_total_time);
	total_time += elapsed_time;
	if (total_time > max_time) {
		max_time =  total_time;
	}

	if (freeze > max_freezing){
        max_freezing = freeze;
	}


    rc  = pthread_mutex_unlock(&mutex_total_time);



    pthread_exit(NULL);
    return 0;
}


int main(int argc,char *argv[])  {

    if (argc < 2) {
		printf("ERROR: the program should take 2 arguments\n");
		exit(-1);
	}
    //i atoi metatrepei to typo pou mas dinoun se string
	 Ncust = atoi(argv[1]);
	 seed = atoi(argv[2]);

	if (Ncust <= 0) {
		printf("ERROR: Clients's number must be positive\n");
		exit(-1);
	}


    pthread_t *threads;

	threads = malloc(Ncust * sizeof(pthread_t));

    //arxikopoihsh mutexes
    pthread_mutex_init(&mutex_cookers, NULL);
    pthread_cond_init(&bake_cond, NULL);
    pthread_mutex_init(&mutex_ovens, NULL);
    pthread_cond_init(&ovens_cond, NULL);
    pthread_mutex_init(&mutex_deliver, NULL);
    pthread_cond_init(&deliver_cond, NULL);
    pthread_mutex_init(&mutex_total_time, NULL);


    //dimiourgia twn threads me 3 lepta diafora i mia paraggelia apo tin alli
    for (int i = 0; i < Ncust; i++) {
        id[i] = i+1;
    	rc = pthread_create(&threads[i], NULL, order, &id[i]);
    	next = rand_r(&seed) %  (orderhigh + 1 - orderlow) + orderlow;
    	sleep(next); //xronos gia tin epomeni paraggelia


    }


    //wait for threads to end
    for (int i = 0; i < Ncust; i++) {
	pthread_join(threads[i], NULL);
    }



    printf("Average order's time: %.2f minutes.\n", total_time/ Ncust);
    printf("Maximum order's time: %d minutes.\n", (int) max_time);
	printf("Average freezing time: %.2d minutes\n", freeze/Ncust);
	printf("Maximum freezing time :%d minutes\n", (int)max_freezing);

    //destroy mutexes
    pthread_mutex_destroy(&mutex_cookers);
    pthread_cond_destroy(&bake_cond);
    pthread_mutex_destroy(&mutex_ovens);
    pthread_cond_destroy(&ovens_cond);
    pthread_mutex_destroy(&mutex_deliver);
    pthread_cond_destroy(&deliver_cond);
    pthread_mutex_destroy(&mutex_total_time);



    //eleutherwsi mnimis pou desmetike me malloc
    free(threads);
    return 0;


    }



