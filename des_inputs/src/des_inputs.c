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


//// Function to map user input to an event code
//int getEventCode(const char *event) {
//    if (strncmp(event, LEFT_SCAN_EVT, strlen(LEFT_SCAN_EVT)) == 0) return 0;
//    if (strcmp(event, GUARD_LEFT_UNLOCK_EVT), strlen(GUARD_LEFT_UNLOCK_EVT) == 0) return 1;
//    if (strcmp(event, LEFT_OPEN_EVT), strlen(LEFT_OPEN_EVT) == 0) return 2;
//    if (strcmp(event, WEIGHT_EVT), strlen(WEIGHT_EVT) == 0) return 3;
//    if (strcmp(event, LEFT_CLOSE_EVT), strlen(LEFT_CLOSE_EVT) == 0) return 4;
//    if (strcmp(event, GUARD_LEFT_LOCK_EVT), strlen(GUARD_LEFT_LOCK_EVT) == 0) return 5;
//    if (strcmp(event, GUARD_RIGHT_UNLOCK_EVT), strlen(GUARD_RIGHT_UNLOCK_EVT) == 0) return 6;
//    if (strcmp(event, RIGHT_OPEN_EVT), strlen(RIGHT_OPEN_EVT) == 0) return 7;
//    if (strcmp(event, RIGHT_CLOSE_EVT), strlen(RIGHT_CLOSE_EVT) == 0) return 8;
//    if (strcmp(event, GUARD_RIGHT_LOCK_EVT), strlen(GUARD_RIGHT_LOCK_EVT) == 0) return 9;
//    if (strcmp(event, RIGHT_SCAN_EVT), strlen(RIGHT_SCAN_EVT) == 0) return 10;
//    if (strcmp(event, EXIT_EVT), strlen(EXIT_EVT) == 0) return 11;
//    if (strcmp(event, LOCK_DOWN_EVT), strlen(LOCK_DOWN_EVT) == 0) return 12;
//    return -1;  // Unknown event
// }


int main(int argc, char *argv[]) {
    Person p;
    Display ctr;
    Input input_code;

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

    p.state = INIT_STATE;
    MsgSend(coid, &p, sizeof(p), &ctr, sizeof(ctr));

    while (1) {
        char usrInput[20];

        // Display available event choices
        printf("Enter the event type (ls=left scan, rs=right scan, ws=weight scale, lo=left open, "
                       "ro=right open, lc=left closed, rc=right closed, gru=guard right unlock, "
                       "grl=guard right lock, gll=guard left lock, glu=guard left unlock): ");
        scanf("%s", usrInput);


        // Convert user input to an event code
		if (strcmp(userInput, "ls") == 0)
			p.event = LEFT_SCAN_EVT;
		if (strcmp(userInput, "ws") == 0)
			p.event =  WEIGHT_CHECK_EVT;
		if (strcmp(userInput, "lo") == 0)
			p.event = LEFT_DOOR_OPEN_EVT;
		if (strcmp(userInput, "lc") == 0)
			p.event = LEFT_DOOR_CLOSE_EVT;
		if (strcmp(userInput, "glu") == 0)
			p.event = GUARD_LEFT_UNLOCK_EVT;
		if (strcmp(userInput, "gll") == 0)
			p.event =  GUARD_LEFT_LOCK_EVT;
		if (strcmp(userInput, "gru") == 0)
			p.event =  GUARD_RIGHT_UNLOCK_EVT;
		if (strcmp(userInput, "grl") == 0)
			p.event =  GUARD_RIGHT_LOCK_EVT;
		if (strcmp(userInput, "rs") == 0)
			p.event =  RIGHT_SCAN_EVT;
		if (strcmp(userInput, "ro") == 0)
			p.event =  RIGHT_DOOR_OPEN_EVT;
		if (strcmp(userInput, "rc") == 0)
			p.event =  RIGHT_DOOR_CLOSE_EVT;
		if (strcmp(userInput, "exit") == 0)
			p.event =  EXIT_EVT;
		if (strcmp(userInput, "lock") == 0)
			p.event = LOCK_DOWN_EVT;

        // Send the updated Person struct to the controller
        if (MsgSend(coid, &p, sizeof(p), &ctr, sizeof(ctr)) == -1) {
            perror("ERROR: MsgSend\n");
            ConnectDetach(coid);
            return EXIT_FAILURE;
        }

        // Check and display the controller's response
        if (ctr.message_index < 0 || ctr.message_index >= NUM_OUTPUTS) {
            printf("Error: Invalid response from controller\n");
        } else {
            printf("Controller response: %s\n", outMessage[ctr.message_index]);
        }

        // If exit event is selected, break the loop
        if (p.state == SYSTEM_EXIT_STATE) {
            break;
        }
    }

    // Detach from the connection and exit
    ConnectDetach(coid);
    return EXIT_SUCCESS;
}
