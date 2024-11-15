#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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
    }else{
    	printf("Succesfully conencted to controller");
    }

    p.person_id = 0; // Initialize person ID to 0
    p.weight = 0;    // Initialize weight to 0
    p.event = 0;     // Initialize event to 0
    p.state = INIT_STATE;
    char userInput[20];


    while (1) {

        // Display available event choices
        printf("Enter the event type (ls=left scan, rs=right scan, ws=weight scale, lo=left open, "
                       "ro=right open, lc=left closed, rc=right closed, gru=guard right unlock, "
                       "grl=guard right lock, gll=guard left lock, glu=guard left unlock): ");
        scanf("%s", userInput);

        // Convert user input to an event code
        if (strcmp(userInput, "ls") == 0) {
            p.event = LEFT_SCAN_EVT;
            get_person_id(&p);
            printf("Event: %d\n", p.event);
            printf("State: %d\n", p.state);
        } else if (strcmp(userInput, "ws") == 0) {
            p.event = WEIGHT_CHECK_EVT;
            get_weight(&p);  // prompt for the weight
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
              printf("Exiting...\n");
              break;
        } else if (strcmp(userInput, "lock") == 0) {
            p.event = LOCK_DOWN_EVT;
        }

        // Send the updated Person struct to the controller
        if (MsgSend(coid, &p, sizeof(p), &ctr, sizeof(ctr)) == -1) {
            perror("ERROR: MsgSend failed");
            printf("Person struct details: ID=%d, Event=%d, State=%d\n", p.person_id, p.event, p.state);
            ConnectDetach(coid);
        } else {
            printf("Message sent successfully. Event: %d, State: %d\n", p.event, p.state);
        }


        printf("Reply received: ID=%d, Event=%d, State=%d\n", p.person_id, p.event, p.state);
        // Check and display the controller's response
    }

    // Detach from the connection and exit
    ConnectDetach(coid);
    return EXIT_SUCCESS;
}
