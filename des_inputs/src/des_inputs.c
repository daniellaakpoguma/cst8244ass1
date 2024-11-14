#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include "../../des_controller/src/des.h"

// Function to get the person's ID
void get_person_id(Person *p) {
    printf("Enter the person ID: ");
    fflush(stdout);
    scanf("%d", &p->person_id);
}

// Function to get the person's weight
void get_weight(Person *p) {
    printf("Enter the weight: ");
    fflush(stdout);
    scanf("%d", &p->weight);
}


int main(int argc, char *argv[]) {
    Person p;
    Display ctr;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_pid>\n", argv[0]);
        return EXIT_FAILURE;
    }


    pid_t server_pid = atoi(argv[1]);
    int coid = ConnectAttach(ND_LOCAL_NODE, server_pid, 1, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) {
        perror("ERROR: ConnectAttach\n");
        return EXIT_FAILURE;
    }

    size_t shm_size = sizeof(Person) * 100;  // Adjust the number of Person objects as needed
    Person *shared_memory = NULL;

    // Open the shared memory region and map it to the address space
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);  // Open the existing shared memory
    if (fd == -1) {
        perror("shm_open failed");
        return EXIT_FAILURE;
    }

    // Map the shared memory to the process's address space
    shared_memory = (Person *)mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return EXIT_FAILURE;
    }

    // Close the file descriptor as it is no longer needed after mapping
    close(fd);

    p.state = INIT_STATE;
    MsgSend(coid, &p, sizeof(p), &ctr, sizeof(ctr));

    while (1) {
        char userInput[20];

        // Display available event choices
        printf("Enter the event type (ls=left scan, rs=right scan, ws=weight scale, lo=left open, "
                       "ro=right open, lc=left closed, rc=right closed, gru=guard right unlock, "
                       "grl=guard right lock, gll=guard left lock, glu=guard left unlock): ");

        scanf("%s", userInput);

        // Convert user input to an event code
        if (strcmp(userInput, "ls") == 0) {
            p.event = LEFT_SCAN_EVT;
            get_person_id(&shared_memory[0]);  //  prompt for the person id
        } else if (strcmp(userInput, "ws") == 0) {
            p.event = WEIGHT_CHECK_EVT;
            get_weight(&shared_memory[0]);  // prompt for the weight
        } else if (strcmp(userInput, "lo") == 0) {
            p.event = LEFT_DOOR_OPEN_EVT;
        } else if (strcmp(userInput, "lc") == 0) {
            p.event = LEFT_DOOR_CLOSE_EVT;
        } else if (strcmp(userInput, "glu") == 0) {
            p.event = GUARD_LEFT_UNLOCK_EVT;
        } else if (strcmp(userInput, "gll") == 0) {
            p.event = GUARD_LEFT_LOCK_EVT;
        } else if (strcmp(userInput, "gru") == 0) {
            p.event = GUARD_RIGHT_UNLOCK_EVT;
        } else if (strcmp(userInput, "grl") == 0) {
            p.event = GUARD_RIGHT_LOCK_EVT;
        } else if (strcmp(userInput, "rs") == 0) {
            p.event = RIGHT_SCAN_EVT;
        } else if (strcmp(userInput, "ro") == 0) {
            p.event = RIGHT_DOOR_OPEN_EVT;
        } else if (strcmp(userInput, "rc") == 0) {
            p.event = RIGHT_DOOR_CLOSE_EVT;
        } else if (strcmp(userInput, "exit") == 0) {
            p.event = EXIT_EVT;
        } else if (strcmp(userInput, "lock") == 0) {
            p.event = LOCK_DOWN_EVT;
        }


        // Send the updated Person struct to the controller
        if (MsgSend(coid, &p, sizeof(p), &ctr, sizeof(ctr)) == -1) {
            perror("ERROR: MsgSend\n");
            ConnectDetach(coid);
            return EXIT_FAILURE;
        }

        // Check and display the controller's response
//        if (ctr.message_index < 0 || ctr.message_index >= NUM_OUTPUTS) {
//            printf("Error: Invalid response from controller\n");
//        } else {
//            printf("Controller response: %s\n", outMessage[ctr.message_index]);
//        }

        // After usage, unmap shared memory and clean up
        // munmap(shared_memory, shm_size);

    }

    // Detach from the connection and exit
    ConnectDetach(coid);
    return EXIT_SUCCESS;
}
