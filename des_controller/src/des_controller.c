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
//		// Check if the person ID exists in the database
//		int index = check_person_in_db(p->person_id);
//		if (index == -1) {
//			// Transition to lock-down state if person ID is not found
//			// printf("Person ID not found. Entering lock-down state.\n");
//			return LOCK_DOWN_STATE;
//		} else {
			printf("Moving to Door Scan state\n");
			return DOOR_SCAN_STATE;
		//}
	}
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
	// Display ctr; 	// Display structure for displaying current state

	client_send_t request;
	server_response_t response;
	response.status_code = 200;  // Success

	// create a channel --- Phase I
	printf("Creating channel...\n");
	chid = ChannelCreate(0);
	if (chid == -1) {
	    perror("Failed to create the channel");
	    printf("Error code: %d\n", errno);
	    exit(EXIT_FAILURE);
	}
	printf("Channel created successfully. Channel ID: %d\n", chid);


	// --- Phase II
	while (1) {

		// receive inputs from des_input containing data and the status
		printf("Waiting for messages on channel ID: %d...\n", chid);
//		rcvid = MsgReceive(chid, &person, sizeof(Person), NULL);
//		if (rcvid == -1) {
//		    perror("MsgReceive failed");
//		    printf("Error code: %d\n", errno);
//		    continue;
//		}

		rcvid = MsgReceive(chid, &request, sizeof(request), NULL);
		if (rcvid == -1) {
			perror("Failed to receive message.");
			continue;
		}
		printf("Message received. rcvid: %d, Person ID: %d, Event: %d, State: %d\n",
		       rcvid, request.person.person_id, request.person.event, request.person.state);

		response.person = request.person;

//		if (person.state < 0 || person.state >= NUM_STATES) {
//		    printf("ERROR: Unknown state received: %d\n", person.state);
//		    break;  // Exit the loop if state is invalid
//		}

		// Assign the updated person state to the display structure
		// ctr.person = person;

		// Check state and pass to appropriate state handler function
			switch (response.person.state) {
			case INIT_STATE:
				response.person.state = state_handlers[INIT_STATE](&response.person,
						response.person.state);
				break;
			case DOOR_SCAN_STATE:
				response.person.state = state_handlers[DOOR_SCAN_STATE](&response.person,
						response.person.state);
				break;
			case DOOR_UNLOCKED_STATE:
				response.person.state = state_handlers[DOOR_UNLOCKED_STATE](&response.person,
						response.person.state);
				break;
			case DOOR_OPEN_STATE:
				response.person.state = state_handlers[DOOR_OPEN_STATE](&response.person,
						response.person.state);
				break;
			case WEIGHT_CHECK_STATE:
				response.person.state = state_handlers[WEIGHT_CHECK_STATE](&response.person,
						response.person.state);
				break;
			case DOOR_CLOSE_STATE:
				response.person.state = state_handlers[DOOR_CLOSE_STATE](&response.person,
						response.person.state);
				break;
			case DOOR_LOCKED_STATE:
				response.person.state = state_handlers[DOOR_LOCKED_STATE](&response.person,
						response.person.state);
				break;
			case SYSTEM_EXIT_STATE:
				response.person.state = SYSTEM_EXIT_STATE;
				break;
			//default:
				//fprintf(stderr, "Unknown state received: %d\n", person.state);
				//break;
			}
			 printf(" Updated State: %d\n", response.person.state);

			// send message to display
			// check for exit condition
			// state. When an exit condition is met, the state handler should return the function pointer for the next state handler


		if (MsgReply(rcvid, EOK, &response, sizeof(response))) {
		    perror("MsgReply failed");
		    break;
		}else{
			printf("Reply sent successfully\n");
			printf("Reply sent: ID=%d, Event=%d, State=%d\n", response.person.person_id, response.person.event, response.person.state);
		}

	}
	ChannelDestroy(chid);
	return EXIT_SUCCESS;
}
