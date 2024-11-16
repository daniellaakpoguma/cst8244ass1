#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <unistd.h>
#include "../../des_controller/src/des.h"
#include <semaphore.h>
#include <sys/mman.h>    // for shm_open, mmap
#include <fcntl.h>        // for O_CREAT, O_RDWR
#include <sys/stat.h>     // for S_IRUSR, S_IWUSR


void display_state(Person *person);

int main(void) {
    int chid, rcvid;
    // Semaphore to control access to shared memory
    sem_t *mutex;


    printf("Display is running as process ID %d\n", getpid());

    // Create shared memory for Display
    int shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Set the size of the shared memory segment
    ftruncate(shm_fd, SHARED_MEM_SIZE);

    // Map shared memory
    void *ptr = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory only once, not in the loop
    static int initialized = 0;  // Flag to ensure initialization happens once
    if (!initialized) {
        Display *shared_display = (Display *)ptr;
        shared_display->person.state = INIT_STATE;  // Set the initial state
        initialized = 1;  // Mark that initialization is done
    }

    // Define a flag to track if the state has been printed already
    static int printed_state = 0;
    static int previous_state = -1;

    while (1) {
        // Wait on the semaphore (lock) to avoid race conditions
        sem_wait(mutex);

        // Read the shared Display structure
        Display *shared_display = (Display *)ptr;

        // Check if the state has changed, and print only when necessary
        if (shared_display->person.state != SYSTEM_EXIT_STATE) {
            // Only print if the state hasn't been printed before or if the state has changed
        	if (printed_state == 0){
                display_state(&shared_display->person);
                fflush(stdout);
                printed_state = 1;  // Mark that state has been printed
                previous_state = shared_display->person.state;  // Save the current state
            }
        } else {
            // If the state is SYSTEM_EXIT_STATE, exit the loop
            printf("Exiting the loop. SYSTEM_EXIT_STATE reached.\n");
            break;  // Exit the loop or perform desired action
        }

        // Post the semaphore (unlock)
        sem_post(mutex);

        // Optional: Add a small delay to prevent tight looping
        usleep(100000);  // 100ms delay
    }

    // Cleanup shared memory and semaphore
    if (munmap(ptr, SHARED_MEM_SIZE) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    // Close the shared memory object
    if (close(shm_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    // Optionally unlink the shared memory object
    if (shm_unlink(SHARED_MEM_NAME) == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void display_state(Person *person) {
    switch (person->state) {
        case INIT_STATE:
            printf("System Initializing...\n");
            break;

        case DOOR_SCAN_STATE:
            printf("Person ID scanned: %d\n", person->person_id);
            break;

        case DOOR_UNLOCKED_STATE:
            printf("Guard unlocking the door...\n");
            break;

        case DOOR_OPEN_STATE:
            printf("Door opened.\n");
            break;

        case WEIGHT_CHECK_STATE:
            printf("Person ID: %d, Weight: %d kg\n", person->person_id, person->weight);
            break;

        case DOOR_CLOSE_STATE:
            printf("Door closed.\n");
            break;

        case DOOR_LOCKED_STATE:
            printf("Guard locking the door...\n");
            break;

        case SYSTEM_EXIT_STATE:
            printf("System exiting...\n");
            break;

        case LOCK_DOWN_STATE:
            printf("System in lockdown.\n");
            break;

        default:
            printf("ERROR: Invalid state\n");
            break;
    }

}
