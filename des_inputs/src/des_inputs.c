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

// Function to display available event choices to the user
void displayEventChoices() {
    printf("Select an event from the following list:\n");
    printf("\t%s = Left Scan\n", LEFT_SCAN_EVT);
    printf("\t%s = Right Scan\n", RIGHT_SCAN_EVT);
    printf("\t%s = Weight Scale\n", WEIGHT_EVT);
    printf("\t%s = Left Door Open\n", LEFT_OPEN_EVT);
    printf("\t%s = Right Door Open\n", RIGHT_OPEN_EVT);
    printf("\t%s = Left Door Close\n", LEFT_CLOSE_EVT);
    printf("\t%s = Right Door Close\n", RIGHT_CLOSE_EVT);
    printf("\t%s = Guard Right Unlock\n", GUARD_RIGHT_UNLOCK_EVT);
    printf("\t%s = Guard Right Lock\n", GUARD_RIGHT_LOCK_EVT);
    printf("\t%s = Guard Left Lock\n", GUARD_LEFT_LOCK_EVT);
    printf("\t%s = Guard Left Unlock\n", GUARD_LEFT_UNLOCK_EVT);
    printf("\t%s = Exit\n", EXIT_EVT);
}

// Function to map user input to an event code
int getEventCode(const char *event) {
    if (strncmp(event, LEFT_SCAN_EVT, strlen(LEFT_SCAN_EVT)) == 0) return 0;
    if (strcmp(event, GUARD_LEFT_UNLOCK_EVT), strlen(GUARD_LEFT_UNLOCK_EVT) == 0) return 1;
    if (strcmp(event, LEFT_OPEN_EVT), strlen(LEFT_OPEN_EVT) == 0) return 2;
    if (strcmp(event, WEIGHT_EVT), strlen(WEIGHT_EVT) == 0) return 3;
    if (strcmp(event, LEFT_CLOSE_EVT), strlen(LEFT_CLOSE_EVT) == 0) return 4;
    if (strcmp(event, GUARD_LEFT_LOCK_EVT), strlen(GUARD_LEFT_LOCK_EVT) == 0) return 5;
    if (strcmp(event, GUARD_RIGHT_UNLOCK_EVT), strlen(GUARD_RIGHT_UNLOCK_EVT) == 0) return 6;
    if (strcmp(event, RIGHT_OPEN_EVT), strlen(RIGHT_OPEN_EVT) == 0) return 7;
    if (strcmp(event, RIGHT_CLOSE_EVT), strlen(RIGHT_CLOSE_EVT) == 0) return 8;
    if (strcmp(event, GUARD_RIGHT_LOCK_EVT), strlen(GUARD_RIGHT_LOCK_EVT) == 0) return 9;
    if (strcmp(event, RIGHT_SCAN_EVT), strlen(RIGHT_SCAN_EVT) == 0) return 10;
    if (strcmp(event, EXIT_EVT), strlen(EXIT_EVT) == 0) return 11;
    if (strcmp(event, LOCK_DOWN_EVT), strlen(LOCK_DOWN_EVT) == 0) return 12;
    return -1;  // Unknown event
}

// Function to handle each event based on the event code
void handle_event(Person *p, int event_code) {
    switch (event_code) {
        case 0:
            p->state = LEFT_DOOR_SCAN_STATE;
            get_person_id(p);
            break;
        case 1:
            p->state = GUARD_LEFT_UNLOCK_STATE;
            break;
        case 2:
            p->state = LEFT_DOOR_OPEN_STATE;
            break;
        case 3:
            p->state = WEIGHT_CHECK_STATE;
            get_weight(p);
            break;
        case 4:
            p->state = LEFT_DOOR_CLOSE_STATE;
            break;
        case 5:
            p->state = GUARD_LEFT_LOCK_STATE;
            break;
        case 6:
            p->state = GUARD_RIGHT_UNLOCK_STATE;
            break;
        case 7:
            p->state = RIGHT_DOOR_OPEN_STATE;
            break;
        case 8:
            p->state = RIGHT_DOOR_CLOSE_STATE;
            break;
        case 9:
            p->state = GUARD_RIGHT_LOCK_STATE;
            break;
        case 10:
            p->state = RIGHT_DOOR_SCAN_STATE;
            get_person_id(p);
            break;
        case 11:
            p->state = SYSTEM_EXIT_STATE;
            break;
        case 12:
			p->state = LOCK_DOWN_EVT;
			break;
        default:
            printf("Unknown event\n");
    }
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

    p.state = INIT_STATE;
    MsgSend(coid, &p, sizeof(p), &ctr, sizeof(ctr));

    while (1) {
        char usrInput[20];

        // Display available event choices
        displayEventChoices();

        // Get user input
        getUserInput(usrInput);

        // Convert user input to an event code and handle the event
        int event_code = getEventCode(usrInput);
        handle_event(&p, event_code);

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
