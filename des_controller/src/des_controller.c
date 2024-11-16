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
#include "des.h"
#include <semaphore.h>

// Forward declarations for state handler functions
typedef State (*StateHandler)(Person *p, State current_state, server_response_t *response);

// State handler functions
State handler_init();
State handler_door_scan();
State handler_door_unlocked();
State handler_door_open();
State handler_weight_check();
State handler_door_close();
State handler_door_locked();
State handler_lockdown();

StateHandler state_handlers[] = {
		handler_init,
		handler_door_scan,
		handler_door_unlocked,
		handler_door_open,
		handler_weight_check,
		handler_door_close,
		handler_door_locked,
		handler_lockdown,
		};

State handler_init(Person *p, State current_state, server_response_t *response) {
	// Check if the event is a scan event (LEFT_SCAN_EVT or RIGHT_SCAN_EVT)
	if (p->event == LEFT_SCAN_EVT || p->event == RIGHT_SCAN_EVT) {
			response->status_code = 200;
			return DOOR_SCAN_STATE;
	}
	response->status_code = 400;
	return current_state;
}

State handler_door_scan(Person *p, State current_state, server_response_t *response) {
    // Simulate door scan process
    if (p->event == GUARD_LEFT_UNLOCK_EVT) {
        if (p->previous_event == LEFT_SCAN_EVT) {
            // If the previous event was a left scan, unlock the door
        	response->status_code = 200;
            return DOOR_UNLOCKED_STATE;
        }
    } else if (p->event == GUARD_RIGHT_UNLOCK_EVT) {
        if (p->previous_event == RIGHT_SCAN_EVT) {
            // If the previous event was a right scan, unlock the door
        	response->status_code = 200;
            return DOOR_UNLOCKED_STATE;
        }
    }
   response->status_code = 400; // Bad Request
   return current_state;
}

State handler_door_unlocked(Person *p, State current_state, server_response_t *response) {
	if (p->event == LEFT_DOOR_OPEN_EVT){
		if (p->previous_event == GUARD_LEFT_UNLOCK_EVT){
			response->status_code = 200;
			return DOOR_OPEN_STATE;
		}
	}else if(p->event == RIGHT_DOOR_OPEN_EVT) {
		if (p->previous_event == GUARD_RIGHT_UNLOCK_EVT){
			response->status_code = 200;
			return DOOR_OPEN_STATE;
		}
	}
	 response->status_code = 400;
	return current_state;
}

State handler_door_open(Person *p, State current_state, server_response_t *response) {
	if (p->event == WEIGHT_CHECK_EVT) {
		response->status_code = 200;
		return WEIGHT_CHECK_STATE;
	}
	if (p->event == LEFT_DOOR_CLOSE_EVT) {
		if (p->previous_event == LEFT_DOOR_OPEN_EVT) {
			response->status_code = 200;
			return DOOR_CLOSE_STATE;
		}
	} else if (p->event == RIGHT_DOOR_CLOSE_EVT) {
		if (p->previous_event == RIGHT_DOOR_OPEN_EVT) {
			response->status_code = 200;
			return DOOR_CLOSE_STATE;
		}
	}
	response->status_code = 400;
	return current_state;
}

State handler_weight_check(Person *p, State current_state, server_response_t *response) {
	if (p->event == LEFT_DOOR_CLOSE_EVT || p->event == RIGHT_DOOR_CLOSE_EVT) {
		response->status_code = 200;
		return DOOR_CLOSE_STATE;
	}
	response->status_code = 400;
	return current_state;
}

State handler_door_close(Person *p, State current_state, server_response_t *response) {
	if (p->event == GUARD_LEFT_LOCK_EVT) {
		if (p->previous_event == LEFT_DOOR_CLOSE_EVT) {
			response->status_code = 200;
			return DOOR_LOCKED_STATE;
		}
	} else if (p->event == GUARD_RIGHT_LOCK_EVT) {
		if (p->previous_event == RIGHT_DOOR_CLOSE_EVT) {
			response->status_code = 200;
			return DOOR_LOCKED_STATE;
		}
	}
	response->status_code = 400;
	return current_state;
}

State handler_door_locked(Person *p, State current_state, server_response_t *response) {
	// Simulate locking the door
	if (p->event == GUARD_LEFT_UNLOCK_EVT|| p->event == GUARD_RIGHT_UNLOCK_EVT) {
		response->status_code = 200;
		return  DOOR_UNLOCKED_STATE;
	}
	if (p->event == RIGHT_SCAN_EVT) {
		response->status_code = 200;
		return DOOR_SCAN_STATE;
	}
	if (p->event == EXIT_EVT) {
		response->status_code = 200;
		return SYSTEM_EXIT_STATE;
	}
	response->status_code = 400;
	return current_state;
}

State handler_system_exit(Person *p, State current_state, server_response_t *response) {
	//  When an exit condition is met, the state handler should
	// return the function pointer for the next state handler
}

State handler_lockdown(Person *p, State current_state, server_response_t *response){
	if (p->event == LEFT_SCAN_EVT || p->event == RIGHT_SCAN_EVT) {
		printf("Cannot perform scan event while in lock-down state.\n");
		response->status_code = 200;
		return INIT_STATE;
	}
}

int main(int argc, char *argv[]) {

	// Check the number of command-line arguments
	if (argc != 2) {
		fprintf(stderr, "Missing PID to display\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	// Print process ID
	printf("The controller is running as process_id: %d\n", getpid());
	fflush(stdout);

	int rcvid;      // indicates who we should reply to
	int chid;       // the channel ID

	client_send_t request;
	server_response_t response;
	response.status_code = 200;  // Success
    // Semaphore to control access to shared memory
    sem_t *mutex;

	// create a channel for communication with des_inputs
	printf("Creating channel...\n");
	chid = ChannelCreate(0);
	if (chid == -1) {
	    perror("Failed to create the channel");
	    printf("Error code: %d\n", errno);
	    exit(EXIT_FAILURE);
	}
	printf("Channel created successfully. Channel ID: %d\n", chid);

    // Create shared memory for Display
    int shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Map shared memory
    void *ptr = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }


	// --- Phase II
	while (1) {

		// receive inputs from des_input containing data and the status
		rcvid = MsgReceive(chid, &request, sizeof(request), NULL);
		if (rcvid == -1) {
			perror("Failed to receive message.");
			continue;
		}
		printf("Message received. rcvid: %d, Person ID: %d, Event: %d, State: %d\n",
		       rcvid, request.person.person_id, request.person.event, request.person.state);

		response.person = request.person;

		if (request.person.state < 0 || request.person.state >= NUM_STATES) {
			printf("ERROR: Unknown state received: %d\n", request.person.state);
		    break;  // Exit the loop if state is invalid
		}


		// Check state and pass to appropriate state handler function
			switch (response.person.state) {
			case INIT_STATE:
				response.person.state = state_handlers[INIT_STATE](&response.person,
						response.person.state, &response);
				break;
			case DOOR_SCAN_STATE:
				response.person.state = state_handlers[DOOR_SCAN_STATE](&response.person,
						response.person.state, &response);
				break;
			case DOOR_UNLOCKED_STATE:
				response.person.state = state_handlers[DOOR_UNLOCKED_STATE](&response.person,
						response.person.state, &response);
				break;
			case DOOR_OPEN_STATE:
				response.person.state = state_handlers[DOOR_OPEN_STATE](&response.person,
						response.person.state, &response);
				break;
			case WEIGHT_CHECK_STATE:
				response.person.state = state_handlers[WEIGHT_CHECK_STATE](&response.person,
						response.person.state, &response);
				break;
			case DOOR_CLOSE_STATE:
				response.person.state = state_handlers[DOOR_CLOSE_STATE](&response.person,
						response.person.state, &response);
				break;
			case DOOR_LOCKED_STATE:
				response.person.state = state_handlers[DOOR_LOCKED_STATE](&response.person,
						response.person.state, &response);
				break;
			case SYSTEM_EXIT_STATE:
				response.person.state = SYSTEM_EXIT_STATE;
				break;
			}
			 printf(" Updated State: %d\n", response.person.state);

		// Modify the shared Display structure
		Display *shared_display = (Display*) ptr;
		shared_display->person.person_id = response.person.person_id;
		shared_display->person.weight = response.person.weight;
		shared_display->person.state = response.person.state;
		shared_display->person.event = response.person.event;
		shared_display->person.previous_event = response.person.previous_event;

		// Post the semaphore (unlock)
		sem_post(mutex);

		// This replies to des_inputs
		if (MsgReply(rcvid, EOK, &response, sizeof(response))) {
			perror("MsgReply failed");
			break;
		} else {
			printf("Reply sent successfully\n");
			printf("Reply sent: ID=%d, Event=%d, State=%d\n",
					response.person.person_id, response.person.event,
					response.person.state);
		}
	}
    // Cleanup shared memory and semaphore
    if (munmap(ptr, SHARED_MEM_SIZE) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    // Close the shared memory object
    if (close(shm_fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

	ChannelDestroy(chid);
	return EXIT_SUCCESS;
}
