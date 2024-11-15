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

    client_send_t request;
    server_response_t response;
    request.person.person_id = 0;
    request.person.weight = 0;
	request.person.event = 0;
	request.person.state = INIT_STATE;

    char userInput[20];


    while (1) {

        // Display available event choices
        printf("Enter the event type (ls=left scan, rs=right scan, ws=weight scale, lo=left open, "
                       "ro=right open, lc=left closed, rc=right closed, gru=guard right unlock, "
                       "grl=guard right lock, gll=guard left lock, glu=guard left unlock): ");
        fflush(stdout);
        scanf("%s", userInput);

//        // Convert user input to an event code
        if (strcmp(userInput, "ls") == 0) {
        	request.person.event = LEFT_SCAN_EVT;
            get_person_id(&request.person);
            printf("Event: %d\n", request.person.event);
            printf("State: %d\n", request.person.state);
            fflush(stdout);
        } else if (strcmp(userInput, "ws") == 0) {
        	request.person.event = WEIGHT_CHECK_EVT;
            get_weight(&request.person);  // prompt for the weight
        } else if (strcmp(userInput, "lo") == 0) {
        	request.person.event = LEFT_DOOR_OPEN_EVT;
        } else if (strcmp(userInput, "lc") == 0) {
        	request.person.event = LEFT_DOOR_CLOSE_EVT;
        } else if (strcmp(userInput, "glu") == 0) {
        	request.person.event = GUARD_LEFT_UNLOCK_EVT;
        } else if (strcmp(userInput, "gll") == 0) {
        	request.person.event = GUARD_LEFT_LOCK_EVT;
        } else if (strcmp(userInput, "gru") == 0) {
        	request.person.event = GUARD_RIGHT_UNLOCK_EVT;
        } else if (strcmp(userInput, "grl") == 0) {
        	request.person.event = GUARD_RIGHT_LOCK_EVT;
        } else if (strcmp(userInput, "rs") == 0) {
        	request.person.event = RIGHT_SCAN_EVT;
        } else if (strcmp(userInput, "ro") == 0) {
        	request.person.event = RIGHT_DOOR_OPEN_EVT;
        } else if (strcmp(userInput, "rc") == 0) {
        	request.person.event = RIGHT_DOOR_CLOSE_EVT;
        } else if (strcmp(userInput, "exit") == 0) {
        	request.person.event = EXIT_EVT;
            printf("Exiting...\n");
            break;
        } else if (strcmp(userInput, "lock") == 0) {
        	request.person.event = LOCK_DOWN_EVT;
        }

        // Send the updated Person struct to the controller
        if (MsgSend(coid, &request, sizeof(request), &response, sizeof(response)) == -1)
        {
            perror("Error during MsgSend");
            ConnectDetach(coid);
            return EXIT_FAILURE;
        }

    	// Receive Reply
        printf("Reply received: ID=%d, Event=%d, State=%d\n", response.person.person_id, response.person.event, response.person.state);
        printf("After receiving response, prompting again...\n");
        fflush(stdout);

    }

    // Detach from the connection and exit
    ConnectDetach(coid);
    return EXIT_SUCCESS;
}
