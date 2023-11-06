#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#define NUM_RESOURCES 3
#define NUM_CUSTOMERS 5
#define MAX_NEED 10

pthread_mutex_t mtx;
pthread_cond_t cv;

int available[NUM_RESOURCES];
int maximum[NUM_CUSTOMERS][NUM_RESOURCES];
int allocation[NUM_CUSTOMERS][NUM_RESOURCES];
int need[NUM_CUSTOMERS][NUM_RESOURCES];

bool isSafeState() {
    int finish[NUM_CUSTOMERS] = {0};
    int work[NUM_RESOURCES];

    for (int i = 0; i < NUM_RESOURCES; ++i) {
        work[i] = available[i];
    }

    int completed = 0;

    while (completed < NUM_CUSTOMERS) {
        bool found = false;

        for (int i = 0; i < NUM_CUSTOMERS; ++i) {
            if (!finish[i]) {
                bool canAllocate = true;

                for (int j = 0; j < NUM_RESOURCES; ++j) {
                    if (need[i][j] > work[j]) {
                        canAllocate = false;
                        break;
                    }
                }

                if (canAllocate) {
                    for (int j = 0; j < NUM_RESOURCES; ++j) {
                        work[j] += allocation[i][j];
                    }
                    finish[i] = 1;
                    found = true;
                    completed++;
                }
            }
        }

        if (!found) {
            return false;
        }
    }

    return true;
}
void displaySystemState() {
    printf("Available resources: ");
    for (int i = 0; i < NUM_RESOURCES; ++i) {
        printf("%d ", available[i]);
    }
    printf("\n");

    printf("Resource allocation:\n");
    for (int i = 0; i < NUM_CUSTOMERS; ++i) {
        printf("Customer %d: ", i);
        for (int j = 0; j < NUM_RESOURCES; ++j) {
            printf("%d ", allocation[i][j]);
        }
        printf("\n");
    }

    printf("Resource need:\n");
    for (int i = 0; i < NUM_CUSTOMERS; ++i) {
        printf("Customer %d: ", i);
        for (int j = 0; j < NUM_RESOURCES; ++j) {
            printf("%d ", need[i][j]);
        }
        printf("\n");
    }
}
void requestResource(int customer) {
    pthread_mutex_lock(&mtx);
    int request[NUM_RESOURCES];

    printf("Customer %d is requesting resources: ", customer);

    for (int i = 0; i < NUM_RESOURCES; ++i) {
        request[i] = rand() % (need[customer][i] + 1);
        printf("%d ", request[i]);
    }
    printf("\n");

    bool grantRequest = true;

    for (int i = 0; i < NUM_RESOURCES; ++i) {
        if (request[i] > available[i] || request[i] > need[customer][i]) {
            grantRequest = false;
            break;
        }
    }

    if (grantRequest) {
        for (int i = 0; i < NUM_RESOURCES; ++i) {
            available[i] -= request[i];
            allocation[customer][i] += request[i];
            need[customer][i] -= request[i];
        }
        printf("Request granted. System is in a safe state.\n");

        printf("Available resources: ");
        for (int i = 0; i < NUM_RESOURCES; ++i) {
            printf("%d ", available[i]);
        }
        printf("\n");
        printf("Resource allocation:\n");
        for (int i = 0; i < NUM_CUSTOMERS; ++i) {
            printf("Customer %d: ", i);
            for (int j = 0; j < NUM_RESOURCES; ++j) {
                printf("%d ", allocation[i][j]);
            }
            printf("\n");
        }
    } else {
        printf("Request denied. Granting the request would lead to an unsafe state.\n");

        for (int i = 0; i < NUM_RESOURCES; ++i) {
            available[i] += request[i];
            allocation[customer][i] -= request[i];
            need[customer][i] += request[i];
        }
    }
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&mtx);
    displaySystemState();
}

void releaseResource(int customer) {
    pthread_mutex_lock(&mtx);
    int release[NUM_RESOURCES];

    printf("Customer %d is releasing resources: ", customer);

    for (int i = 0; i < NUM_RESOURCES; ++i) {
        release[i] = rand() % (allocation[customer][i] + 1);
        printf("%d ", release[i]);
    }
    printf("\n");

    for (int i = 0; i < NUM_RESOURCES; ++i) {
        if (release[i] > allocation[customer][i]) {
            printf("Invalid release request.\n");
            pthread_mutex_unlock(&mtx);
            return;
        }
    }

    for (int i = 0; i < NUM_RESOURCES; ++i) {
        available[i] += release[i];
        allocation[customer][i] -= release[i];
        need[customer][i] += release[i];
    }

    printf("Available resources: ");
    for (int i = 0; i < NUM_RESOURCES; ++i) {
        printf("%d ", available[i]);
    }
    printf("\n");
    printf("Resource allocation:\n");
    for (int i = 0; i < NUM_CUSTOMERS; ++i) {
        printf("Customer %d: ", i);
        for (int j = 0; j < NUM_RESOURCES; ++j) {
            printf("%d ", allocation[i][j]);
        }
        printf("\n");
    }

    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&mtx);
    displaySystemState();
}

void* customerThread(void* customer_ptr) {
    int customer = *(int*)customer_ptr;
    int iterations = 10;

    for (int i = 0; i < iterations; ++i) {
        requestResource(customer);
        sleep(1);
        releaseResource(customer);
        sleep(1);
    }
    return NULL;
}

int main() {
    int numIterations = 10;

    printf("Enter the available resources:\n");
    for (int i = 0; i < NUM_RESOURCES; ++i) {
        printf("Resource %d: ", i);
        scanf("%d", &available[i]);
    }

    for (int i = 0; i < NUM_CUSTOMERS; ++i) {
        printf("Enter maximum needs for Customer %d:\n", i);
        for (int j = 0; j < NUM_RESOURCES; ++j) {
            printf("Resource %d: ", j);
            scanf("%d", &maximum[i][j]);
            allocation[i][j] = 0;
            need[i][j] = maximum[i][j];
        }
    }

    pthread_t threads[NUM_CUSTOMERS];
    int customer_ids[NUM_CUSTOMERS];

    for (int i = 0; i < NUM_CUSTOMERS; ++i) {
        customer_ids[i] = i;
        pthread_create(&threads[i], NULL, customerThread, &customer_ids[i]);
        printf("\n----------------------------\n");
    }

    for (int i = 0; i < NUM_CUSTOMERS; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
