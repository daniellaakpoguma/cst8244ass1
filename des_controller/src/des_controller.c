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
#include "des.h"

// Forward declarations for state handler functions
typedef State (*StateHandler)(Person *p, State current_state);

// State handler functions
State handler_init();
State handler_door_scan();
State handler_door_unlocked();
State handler_door_open();
State handler_weight_check();
State handler_door_close();
State handler_door_locked();
State handler_system_exit();
State handler_lockdown();

StateHandler state_handlers[] = {
		handler_init,
		handler_door_scan,
		handler_door_unlocked,
		handler_door_open,
		handler_weight_check,
		handler_door_close,
		handler_door_locked,
		handler_system_exit,
		handler_lockdown,
		};

int check_person_in_db(int person_id) {
//	for (int i = 0; i < MAX_PERSONS; i++) {
//	    if (person_db[i].person_id == person_id) {
//			return i;  // Return the index if found
//		}
//	}
	return -1;  // Return -1 if not found
}

State handler_init(Person *p, State current_state) {
	// Check if the event is a scan event (LEFT_SCAN_EVT or RIGHT_SCAN_EVT)
	if (p->event == LEFT_SCAN_EVT || p->event == RIGHT_SCAN_EVT) {
		// Check if the person ID exists in the database
		int index = check_person_in_db(p->person_id);
		if (index == -1) {
			// Transition to lock-down state if person ID is not found
			// printf("Person ID not found. Entering lock-down state.\n");
			return LOCK_DOWN_STATE;
		} else {
			return DOOR_SCAN_STATE;
		}
	}
	return current_state; // If no condition is met, remain in the current state
}

State handler_door_scan(Person *p, State current_state) {
	// Simulate door scan process
	if (p->event == GUARD_LEFT_UNLOCK_EVT
			|| p->event == GUARD_RIGHT_UNLOCK_EVT) {
		// receive the id to be scanned
		return DOOR_UNLOCKED_STATE;
	}
	return current_state;
}

State handler_door_unlocked(Person *p, State current_state) {
	if (p->event == LEFT_DOOR_OPEN_EVT || p->event == RIGHT_DOOR_OPEN_EVT) {
		return DOOR_OPEN_STATE; // Transition to DOOR_OPEN_STATE
	}
	return current_state;
}

State handler_door_open(Person *p, State current_state) {
	if (p->event == WEIGHT_CHECK_EVT) {
		return WEIGHT_CHECK_STATE;
	}
	return current_state;
}

State handler_weight_check(Person *p, State current_state) {
	if (p->event == LEFT_DOOR_CLOSE_EVT || p->event == RIGHT_DOOR_CLOSE_EVT) {
		// Get the person from the database (example assumption, matching person)
		int index = check_person_in_db(p->person_id);

//		if (index != -1 && person_db[index].weight != p->weight) {
//			// Transition to lock-down state if weight mismatch
//			// printf("Weight mismatch detected. Entering lock-down state.\n");
//			return LOCK_DOWN_STATE;
//		} else {
//			//printf("Weight check passed. Proceeding to next action...\n");
//			return DOOR_CLOSE_STATE;
//		}

	}
	return current_state;
}

State handler_door_close(Person *p, State current_state) {
	// Simulate closing the door
	if (p->event == GUARD_LEFT_LOCK_EVT || p->event == GUARD_RIGHT_LOCK_EVT) {
		return DOOR_LOCKED_STATE;
	}
	return current_state;
}

State handler_door_locked(Person *p, State current_state) {
	// Simulate locking the door
	if (p->event == EXIT_EVT) {
		return SYSTEM_EXIT_STATE;
	}
	return current_state;
}

State handler_system_exit(Person *p, State current_state) {
	//  When an exit condition is met, the state handler should
	// return the function pointer for the next state handler
}

State handler_lockdown(Person *p, State current_state){

}

int main(int argc, char *argv[]) {

	// Check the number of command-line arguments
	if (argc != 2) {
		fprintf(stderr, "Missing PID to display\n");
		exit(EXIT_FAILURE);
	}

	// Print process ID
	printf("The controller is running as process_id: %d\n", getpid());

	int rcvid;      // indicates who we should reply to
	int chid;       // the channel ID
	Person person;  // Message of type Person struct
	Display ctr; 	// Display structure for displaying current state

	// create a channel --- Phase I
	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("failed to create the channel.");
		exit(EXIT_FAILURE);
	}

	// --- Phase II
	while (1) {

		// receive inputs from des_input containing data and the status
		rcvid = MsgReceive(chid, &person, sizeof(Person), NULL);
		if (rcvid == -1) {
			perror("MsgReceive failed");
			continue;  // Skip to the next iteration if MsgReceive fails
		}

		if (person.state < 0 || person.state >= NUM_STATES) {
		    printf("ERROR: Unknown state received: %d\n", person.state);
		    break;  // Exit the loop if state is invalid
		}

		// Assign the updated person state to the display structure
		 ctr.person = person;

		// Check state and pass to appropriate state handler function
		while (person.state != SYSTEM_EXIT_STATE) {
			switch (person.state) {
			case INIT_STATE:
				person.state = state_handlers[INIT_STATE](&person,
						person.state);
				break;
			case DOOR_SCAN_STATE:
				person.state = state_handlers[DOOR_SCAN_STATE](&person,
						person.state);
				break;
			case DOOR_UNLOCKED_STATE:
				person.state = state_handlers[DOOR_UNLOCKED_STATE](&person,
						person.state);
				break;
			case DOOR_OPEN_STATE:
				person.state = state_handlers[DOOR_OPEN_STATE](&person,
						person.state);
				break;
			case WEIGHT_CHECK_STATE:
				person.state = state_handlers[WEIGHT_CHECK_STATE](&person,
						person.state);
				break;
			case DOOR_CLOSE_STATE:
				person.state = state_handlers[DOOR_CLOSE_STATE](&person,
						person.state);
				break;
			case DOOR_LOCKED_STATE:
				person.state = state_handlers[DOOR_LOCKED_STATE](&person,
						person.state);
				break;
			case SYSTEM_EXIT_STATE:
				person.state = SYSTEM_EXIT_STATE;
				break;
			//default:
				//fprintf(stderr, "Unknown state received: %d\n", person.state);
				//break;
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
