#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

// Define the maximum number of threads and resources
#define MAX_THREADS 10
#define MAX_RESOURCES 10

// Define the mutex and semaphore locks
pthread_mutex_t mutex;
sem_t semaphore;

// Define the data structures for the banker's algorithm
int alloc[MAX_THREADS][MAX_RESOURCES]; // Allocation matrix
int max[MAX_THREADS][MAX_RESOURCES]; // Max matrix
int need[MAX_THREADS][MAX_RESOURCES]; // Need matrix
int avail[MAX_RESOURCES]; // Available resources
int f[MAX_THREADS]; // Finish vector
int n; // Number of threads
int m; // Number of resources

// Function to check if the system is in a safe state
int isSafeState() {
  // Create a copy of the available resources
  int availCopy[MAX_RESOURCES];
  for (int i = 0; i < m; i++) {
    availCopy[i] = avail[i];
  }

  // Mark all threads as unfinished
  for (int i = 0; i < n; i++) {
    f[i] = 0;
  }

  // Iterate over the threads in a sequence
  for (int i = 0; i < n; i++) {
    // If the thread is unfinished
    if (!f[i]) {
      // Check if the thread has all the resources it needs
      int flag = 0;
      for (int j = 0; j < m; j++) {
        if (need[i][j] > availCopy[j]) {
          flag = 1;
          break;
        }
      }

      // If the thread has all the resources it needs, mark it as finished
      if (!flag) {
        f[i] = 1;

        // Update the available resources
        for (int j = 0; j < m; j++) {
          availCopy[j] += alloc[i][j];
        }
      }
    }
  }

  // Free the allocated memory
    int allFinished = 1;
    for (int i = 0; i < n; i++) {
        if (!f[i]) {
            allFinished = 0;
            break;
        }
    }

    if (allFinished) {
        // The system is in a safe state
        return 1;
    } else {
        // The system is in a deadlock state
        printf("Deadlock detected!\n");
        return 0;
    }
}

// Function to simulate a thread requesting resources
void requestResources(int tid, int resources[]) {
  // Acquire the mutex lock
  pthread_mutex_lock(&mutex);

  // Check if the request is safe
  int flag = 0;
  for (int i = 0; i < m; i++) {
    if (resources[i] > need[tid][i]) {
      flag = 1;
      break;
    }
  }

  // Grant the request if it is safe
  if (!flag) {
    for (int i = 0; i < m; i++) {
      alloc[tid][i] += resources[i];
      need[tid][i] -= resources[i];
      avail[i] -= resources[i];
    }
  }

  // Release the mutex lock
  pthread_mutex_unlock(&mutex);

  // Post to the semaphore to indicate that the request has been processed
  sem_post(&semaphore);

  // Wait for the semaphore to be posted again to indicate that the system state has been updated
  sem_wait(&semaphore);
  
    if (!isSafeState()) {
        // Print a message if a deadlock is detected
        printf("Deadlock detected!\n");
    }

    // Display the system state after the allocation
    printf("System state after allocation:\n");
    // ... Print the system state (max, allocation, need, available)
}

// Function to simulate a thread releasing resources
void releaseResources(int tid, int resources[]) {
  // Acquire the mutex lock
  pthread_mutex_lock(&mutex);

  // Release the resources
  for (int i = 0; i < m; i++) {
    alloc[tid][i] -= resources[i];
    need[tid][i] += resources[i];
    avail[i] += resources[i];
  }

  // Release the mutex lock
  pthread_mutex_unlock(&mutex);

  // Post to the semaphore to indicate that the resources have been released
  sem_post(&semaphore);

  // Wait for the semaphore to be posted again to indicate that the system state has been updated
  sem_wait(&semaphore);
	if (!isSafeState()) {
        // Print a message if a deadlock is detected
        printf("Deadlock detected!\n");
    }

    // Display the system state after the release
    printf("System state after release:\n");
    // ... Print the system state (max, allocation, need, available)
}
void *threadFunction(void *thread_id) {
    int tid = *(int *)thread_id;

    // Simulate resource requests for the thread
    int request_resources[MAX_RESOURCES]; // Customize this with actual resource requests

    // Initialize the request_resources array with random values
    for (int i = 0; i < m; i++) {
        request_resources[i] = rand() % (max[tid][i] - alloc[tid][i] + 1); // Generate a random request within the need range
    }

    // Simulate a request for resources
    printf("Thread %d is requesting resources: [", tid);
    for (int i = 0; i < m; i++) {
        printf("%d%s", request_resources[i], (i < m - 1) ? ", " : "]\n");
    }

    requestResources(tid, request_resources);

    // Simulate resource release for the thread
    int release_resources[MAX_RESOURCES]; // Customize this with actual resource releases

    // Initialize the release_resources array with random values
    for (int i = 0; i < m; i++) {
        release_resources[i] = rand() % (alloc[tid][i] + 1); // Generate a random release within the allocated resource range
    }

    // Simulate a release of resources
    printf("Thread %d is releasing resources: [", tid);
    for (int i = 0; i < m; i++) {
        printf("%d%s", release_resources[i], (i < m - 1) ? ", " : "]\n");
    }

    releaseResources(tid, release_resources);

    pthread_exit(NULL);
}

int main(){
	// Initialize the values for n (number of threads) and m (number of resources)
    printf("Enter the number of threads>>\n");
	scanf("%d",&n);
    printf("Enter the number of resources>>\n");
	scanf("%d",&m);

    // Input allocation matrix from the user
    printf("Enter allocation matrix:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            scanf("%d", &alloc[i][j]);
        }
    }

    // Input max matrix from the user
    printf("Enter max matrix:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            scanf("%d", &max[i][j]);
        }
    }

    // Calculate the need matrix based on max and allocation
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            need[i][j] = max[i][j] - alloc[i][j];
        }
    }

    // Input available resources from the user
    printf("Enter available resources:\n");
    for (int i = 0; i < m; i++) {
        scanf("%d", &avail[i]);
    }
    
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        return 1;
    }

    // Initialize the semaphore
    if (sem_init(&semaphore, 0, 0) != 0) {
        perror("Semaphore initialization failed");
        return 1;
    }

    // Create threads
    pthread_t threads[n];
    int thread_ids[n];

    for (int i = 0; i < n; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, threadFunction, &thread_ids[i]) != 0) {
            perror("Thread creation failed");
            return 1;
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    // Clean up
    pthread_mutex_destroy(&mutex);
    sem_destroy(&semaphore);

    return 0;
}
//
