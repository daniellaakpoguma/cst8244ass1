#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <unistd.h>
#include "../../des_controller/src/des.h"

// Function to display the current state
void display_state(Display ctr);

int main(void) {
    int chid, rcvid;  // Channel ID and Receive ID
    Display msg;      // Struct to receive messages

    // Phase I: Create the channel
    if ((chid = ChannelCreate(0)) == -1) {
        perror("Channel creation failed");
        return EXIT_FAILURE;
    }

    printf("Display is running as process ID %d\n", getpid());

    // Phase II: Message handling loop
    while (1) {
        // Receive a message from the controller
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        if (rcvid == -1) {
            perror("Message receive failed");
            ChannelDestroy(chid);  // Clean up before exiting
            return EXIT_FAILURE;
        }

        // Display the current state based on the received data
        display_state(msg);

        // Reply to the message
        if (MsgReply(rcvid, EOK, NULL, 0) == -1) {
            perror("Message reply failed");
        }

        // Exit the loop if the system is shutting down
        if (msg.person.state == SYSTEM_EXIT_STATE) {
            break;
        }
    }

    // Phase III: Clean up the channel
    ChannelDestroy(chid);
    return EXIT_SUCCESS;
}

// Function to display the state of the system
void display_state(Display ctr) {
    switch (ctr.person.state) {
        case INIT_STATE:
            printf("System Initializing...\n");
            break;

        case DOOR_SCAN_STATE:
            printf("Person ID scanned: %d\n", ctr.person.person_id);
            break;

        case DOOR_UNLOCKED_STATE:
            printf("Guard unlocking the door...\n");
            break;

        case DOOR_OPEN_STATE:
            printf("Door opened.\n");
            break;

        case WEIGHT_CHECK_STATE:
            printf("Person ID: %d, Weight: %d kg\n", ctr.person.person_id, ctr.person.weight);
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
