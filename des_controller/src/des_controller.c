#include <stdio.h>
#include <stdlib.h>
#include "des.h"

// Forward declarations for state handler functions
typedef State (*StateHandler)(Input event, State current_state);


// State handler functions
State handler_init();
State handler_wait_scan();
State handler_door_scan();
State handler_door_unlocked();
State handler_door_open();
State handler_wait_weight();
State handler_weight_check();
State handler_door_close();
State handler_door_locked();
State handler_system_exit();


StateHandler state_handlers[] = {
		handler_init,
		handler_wait_scan,
		handler_door_scan,
		handler_door_unlocked,
		handler_door_open,
		handler_wait_weight,
		handler_weight_check,
		handler_door_close,
		handler_door_locked,
		handler_system_exit };


State handler_init(Input event, State current_state) {
	// Check if the event is a scan event (LEFT_SCAN_EVT or RIGHT_SCAN_EVT)
	if (event == LEFT_SCAN_EVT || event == RIGHT_SCAN_EVT) {
		return WAIT_SCAN_STATE;
	}
	return current_state; // If no condition is met, remain in the current state
}

State handler_wait_scan() {
	printf("Waiting for scan...\n");
	// Simulate scan input (e.g., card scan)
	int person_id = 12345;  // Example person ID
	printf("Person scanned: %d\n", person_id);
	// Transition to DOOR_SCAN_STATE
	return DOOR_SCAN_STATE;
}

State handler_door_scan() {
	printf("Processing door scan...\n");
	// Simulate door scan process
	// Transition to DOOR_UNLOCKED_STATE
	return DOOR_UNLOCKED_STATE;
}

State handler_door_unlocked() {
	printf("Guard unlocking door...\n");
	// Simulate unlocking the door
	// Transition to DOOR_OPEN_STATE
	return DOOR_OPEN_STATE;
}

State handler_door_open() {
	printf("Door is open...\n");
	// Wait for the person to step through
	// Transition to WAIT_WEIGHT_STATE
	return WAIT_WEIGHT_STATE;
}

State handler_wait_weight() {
	printf("Waiting for weight scan...\n");
	// Simulate weight measurement
	float weight = 75.5;  // Example weight
	printf("Person weight: %.2f kg\n", weight);
	// Transition to WEIGHT_CHECK_STATE
	return WEIGHT_CHECK_STATE;
}

State handler_weight_check() {
	printf("Checking weight...\n");
	// Perform any weight check logic
	// Transition to DOOR_CLOSE_STATE
	return DOOR_CLOSE_STATE;
}

State handler_door_close() {
	printf("Closing the door...\n");
	// Simulate closing the door
	// Transition to DOOR_LOCKED_STATE
	return DOOR_LOCKED_STATE;
}

State handler_door_locked() {
	printf("Guard locking door...\n");
	// Simulate locking the door
	// Transition to WAIT_SCAN_STATE (or SYSTEM_EXIT_STATE to exit)
	return WAIT_SCAN_STATE;
}

State handler_system_exit() {
	printf("System exiting...\n");
	// Exit the system
	exit(0);
}

int main(int argc, char *argv[]) {
	// Check the number of command-line arguments
	if (argc != 2) {
		fprintf(stderr, "Missing PID to display\n");
		exit(EXIT_FAILURE);
	}

	// Print process ID
	printf("Process ID: %d\n", getpid());

	int rcvid;        // indicates who we should reply to
	int chid;         // the channel ID
	Person person; // Message of type `Person` struct

	// create a channel --- Phase I
	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("failed to create the channel.");
		exit(EXIT_FAILURE);
	}

	int current_state;
	// --- Phase II
	while (1) {

		// receive inputs from des_input containing data and the status
		rcvid = MsgReceive(chid, &person, sizeof(Person), NULL);
		if (rcvid == -1) {
			perror("MsgReceive failed");
			continue;  // Skip to the next iteration if MsgReceive fails
		}

		// Check state and pass to appropriate state handler function
		while (person.state != SYSTEM_EXIT_STATE) {
			switch (person.state) {
			case INIT_STATE:
				person.state = state_handlers[INIT_STATE](person.usrInput, person.state);
				break;
			case WAIT_SCAN_STATE:
				person.state = state_handlers[WAIT_SCAN_STATE](person.usrInput, person.state);
				break;
			case DOOR_SCAN_STATE:
				person.state = state_handlers[DOOR_SCAN_STATE]();
				break;
			case DOOR_UNLOCKED_STATE:
				person.state = state_handlers[DOOR_UNLOCKED_STATE]();
				break;
			case DOOR_OPEN_STATE:
				person.state = state_handlers[DOOR_OPEN_STATE]();
				break;
			case WAIT_WEIGHT_STATE:
				person.state = state_handlers[WAIT_WEIGHT_STATE]();
				break;
			case WEIGHT_CHECK_STATE:
				person.state = state_handlers[WEIGHT_CHECK_STATE]();
				break;
			case DOOR_CLOSE_STATE:
				person.state = state_handlers[DOOR_CLOSE_STATE]();
				break;
			case DOOR_LOCKED_STATE:
				person.state = state_handlers[DOOR_LOCKED_STATE]();
				break;
			case SYSTEM_EXIT_STATE:
				person.state = SYSTEM_EXIT_STATE;
				break;
			default:
				fprintf(stderr, "Unknown state received: %d\n", person.state);
				break;
			}

			// send message to display
			// check for exit condition
			// state. When an exit condition is met, the state handler should return the function pointer for the next state handler

			// now, prepare the reply.  We reuse "message"
			strcpy((char*) &person, "This is the reply");
			MsgReply(rcvid, EOK, &person, sizeof(Person));
		}
	}

	// destroy the channel when done --- Phase III
	ChannelDestroy(chid);
	return EXIT_SUCCESS;
}

