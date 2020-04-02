#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "banker.h"

#define NUM_CUSTOMERS 5
#define NUM_RESOURCES 3

int available[NUM_RESOURCES];

int maximum[NUM_CUSTOMERS][NUM_RESOURCES];

int allocation[NUM_CUSTOMERS][NUM_RESOURCES];

int need[NUM_CUSTOMERS][NUM_RESOURCES];

pthread_mutex_t lock;

void *customerMethod(void *customerNum){

    int customer = *(int*)customerNum;
    bool acquiredResources = false;

    int request [NUM_RESOURCES];
    
    while(1){
        
        //request resources
        for(int i = 0; i < NUM_RESOURCES; i++){
            request[i] = rand() % (maximum[customer][i] + 1); 
        }
        
        //lock
        pthread_mutex_lock(&lock);
        
        //attempt to acquire the resouces and then unlock
        acquiredResources = request_res(customer,request);
        pthread_mutex_unlock(&lock);
        //if resources lock and sleep
        if(acquiredResources){
            sleep((int)rand() % 5 + 1);
            acquiredResources = false;
            pthread_mutex_lock(&lock);
            release_res(customer,request);
            pthread_mutex_unlock(&lock);
            if(rand() % 2 == 0){
                printf("\nCustomer : %d is finished\n", customer);
                for(int i = 0; i < NUM_RESOURCES; i++){
                    need[customer][i] = 0;
                    maximum[customer][i] = 0;
                }
                printf("Maximum Needs After the Customer Finished\n");
                for(int i = 0; i < NUM_CUSTOMERS; i++){
                    for(int j = 0; j < NUM_RESOURCES; j++){
                        printf("%3d ",maximum[i][j]);
                    }
                    puts("");
                }
                sleep(2);
                break;
            }
        }
        //used for readability
        sleep(1);
    }
    return 0;
}

bool isSafe(){

    int work[NUM_RESOURCES];
    for(int i = 0; i < NUM_RESOURCES; i++){
        work[i] = available[i];
    }
    
    bool finish [NUM_CUSTOMERS];
    for(int i = 0; i < NUM_CUSTOMERS; i++){
        finish[i] = false;
    }

    int count = 0;
    int indexFinish = -1;
    int prevFinishIndex = -1;
    bool finishCustomer = true;

    while (count < NUM_CUSTOMERS){
        prevFinishIndex = indexFinish; 
        for(int i = 0; i < NUM_CUSTOMERS; i++){
            if(!finish[i]){
                for(int j = 0; j < NUM_RESOURCES; j++){
                    if (need[i][j] > work[j])
                        finishCustomer = false;
                }
                if(finishCustomer){
                    indexFinish = i;
                    for(int j = 0; j < NUM_RESOURCES; j++){
                        work[j] += allocation[i][j];
                    }
                    finish[i] = true;
                    count++;
                    finishCustomer = true;
                    break; 
                }
            }
        }
        for(int i = 0; i < NUM_CUSTOMERS; i++){
            if(!finish[i])
                break;
        }
        if(prevFinishIndex == indexFinish)
            return false;
    }
    return true;
}

bool request_res(int n_customer, int request[]){
   
    printf("\nCustomer: %d Requesting Resources\n",n_customer);
    for(int i = 0; i < NUM_RESOURCES; i++){
        printf("%d ",request[i]);
    }
    puts("");

    printf("Available Resources Before\n");
    for(int i = 0; i < NUM_RESOURCES; i++){
        printf("%d ",available[i]);
    }
    puts("");
     for(int i = 0; i < NUM_RESOURCES; i++){
        if(request[i] <= need[n_customer][i]){
            if(request[i] > available[i]){
                printf("The system is NOT safe with this request\n\n");
                sleep(1);
                return false;
            }
            else{
                printf("testing taking resources\n");
                for(int k = 0; k < NUM_RESOURCES; k++){
                    allocation[n_customer][k] += request[k];
                    available[k] -= request[k];
                    need[n_customer][k] -= request[k];
                }
                if(isSafe()){
                    printf("The system is safe with this request\nResources granted\n");
                    printf("Available Resources After\n");
                    for(int i = 0; i < NUM_RESOURCES; i++){
                        printf("%d ",available[i]);
                    }
                    puts("\n");
                    sleep(2);
                    return true;
                }
                else{
                    printf("The system is not safe with this request\nResources are not granted\n");
                    for(int k = 0; k < NUM_RESOURCES; k++){
                        available[k] += request[k]; 
                        need[n_customer][k] += request[k]; 
                        allocation[n_customer][k] -= request[k]; 
                    }
                    return false;
                }
            }
        } else{
            printf("Customer: %d is NEEDY\n",n_customer);
            sleep(1);
            return false;
        }

    }
 
    return true;
}

bool release_res(int n_customer, int release[])
{  
    printf("\nCustomer: %d Releasing Resources\n", n_customer);
    for(int i = 0; i < NUM_RESOURCES; i++){
        printf("%d ",release[i]);
        available[i] += release[i]; 
        need[n_customer][i] += release[i]; 
        allocation[n_customer][i] -= release[i];
    }

    puts("");
    printf("Available Resources After\n");
    for(int i = 0; i < NUM_RESOURCES; i++){
        printf("%d ",available[i]);
    }
    puts("");
    sleep(2);
    return true;
}



int main(int argc, char *argv[])
{   
    //create threads and lock
    pthread_t threads[NUM_CUSTOMERS];
    pthread_mutex_init(&lock, NULL);
    
    //initialize the resources
    for (int i =0; i < NUM_RESOURCES; i++) {
        available[i] = atoi(argv[i+1]);
        for(int j=0; j<NUM_CUSTOMERS; j++) {
            maximum[j][i] = rand() % (available[i] + 1);
            need[j][i] = maximum[j][i];
            allocation[j][i] = 0;
        }        
    }
    printf("Maximum Needs\n");
    for(int i = 0; i < NUM_CUSTOMERS; i++){
        for(int j = 0; j < NUM_RESOURCES; j++){
            printf("%3d ",maximum[i][j]);
        }
        puts("");
    }
    
    //create customers and threads
    srand(time(NULL));
    for(int i = 0; i < NUM_CUSTOMERS; i++){
        int *c_num = malloc(sizeof(*c_num));
        if(c_num == NULL){
            printf("couldn't make customer number");
            exit(1);
        }
        *c_num = i;
        pthread_create(&threads[i],NULL,&customerMethod, c_num);
    }
    
    //clean up
    for(int i = 0; i < NUM_CUSTOMERS; i++){
        pthread_join(threads[i],0);
    }
    printf("DONE ALL CUSTOMERS\n");
    pthread_mutex_destroy(&lock);
    
    return EXIT_SUCCESS;
}
