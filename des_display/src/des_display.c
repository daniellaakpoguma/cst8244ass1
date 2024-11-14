#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <unistd.h>
#include "../../des_controller/src/des.h"

// Function to display the current state
void current_state(Display ctr);

int main(void) {
    int chid, rcvid;
    Display ctr;
    Person p;

    if ((chid = ChannelCreate(0)) == -1) {
        printf("Channel creation failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("Display PID = %d\n", getpid());

    while (1) {
        // Receive message from the controller (it contains Person data)
        if ((rcvid = MsgReceive(chid, &p, sizeof(p), NULL)) == -1) {
            printf("Message receive failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        // Call the function to display the current state based on the received Person data
        current_state(ctr);

        // Set the status code for the reply
        ctr.message_index = EOK;  // assuming a successful state transition

        // If the state is not EXIT_STATE, reply to the message
        if (p.state != SYSTEM_EXIT_STATE) {
            if (MsgReply(rcvid, EOK, &ctr, sizeof(ctr)) == -1) {
                printf("Error while replying to message: %s\n", strerror(errno));
            }
        }

        // If the state is SYSTEM_EXIT_STATE, break out of the loop
        if (p.state == SYSTEM_EXIT_STATE) {
            break;
        }
    }

    // Clean up the channel before exiting
    ChannelDestroy(chid);
    return EXIT_SUCCESS;
}

// Function to display the current state based on the state of the person
void current_state(Display ctr) {
    switch (ctr.person.state) {
        case INIT_STATE:
            printf("System Initializing...\n");
            break;

        case DOOR_SCAN_STATE:
            printf("Person ID scanned, proceeding to door scan...\n");
            break;

        case DOOR_UNLOCKED_STATE:
            printf("Guard unlocking the door...\n");
            break;

        case DOOR_OPEN_STATE:
            printf("Door opened.\n");
            break;

        case WEIGHT_CHECK_STATE:
            printf("Checking weight: %d kg\n", ctr.person.weight);
            break;

        case DOOR_CLOSE_STATE:
            printf("Door closed.\n");
            break;

        case DOOR_LOCKED_STATE:
            printf("Guard locking the door...\n");
            break;

        case SYSTEM_EXIT_STATE:
            printf("Exiting system...\n");
            break;

        default:
            printf("ERROR: Invalid state\n");
            break;
    }
}
