#include <stdio.h>
#include <stdlib.h>
#include "des.h"

// this is without proper message passing sent up

// Forward declarations for state handler functions
typedef State (*StateHandler)();

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

// Simulate sending a message to des_display (printing to console for now) - change going down further
void send_to_display(const char *message) {
    printf("Displaying: %s\n", message);
}

int main(void) {
	// Print process id
	printf("Process ID: %d\n", getpid());

    // Start with the INIT_STATE
    State current_state = INIT_STATE;
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
        handler_system_exit
    };

    // Simulate receiving messages from des_inputs
    char message[100];

    // Loop through states and handle events from des_inputs
    while (current_state != SYSTEM_EXIT_STATE) {
        // Simulate receiving an event from des_inputs
        printf("Enter event: ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = 0;  // Remove newline character

        // Process the message and transition to the appropriate state
        if (strcmp(message, "ls") == 0) {
            current_state = state_handlers[WAIT_SCAN_STATE]();  // Handle left scan
        } else if (strcmp(message, "rs") == 0) {
            current_state = state_handlers[DOOR_SCAN_STATE]();  // Handle right scan
        } else if (strcmp(message, "ws") == 0) {
            current_state = state_handlers[WAIT_WEIGHT_STATE]();  // Handle weight scan
        } else if (strcmp(message, "lo") == 0) {
            current_state = state_handlers[DOOR_OPEN_STATE]();  // Handle door open
        } else if (strcmp(message, "ro") == 0) {
            current_state = state_handlers[DOOR_OPEN_STATE]();  // Handle right door open
        } else if (strcmp(message, "lc") == 0) {
            current_state = state_handlers[DOOR_CLOSE_STATE]();  // Handle left door close
        } else if (strcmp(message, "rc") == 0) {
            current_state = state_handlers[DOOR_CLOSE_STATE]();  // Handle right door close
        } else if (strcmp(message, "glu") == 0) {
            current_state = state_handlers[DOOR_UNLOCKED_STATE]();  // Handle guard left unlock
        } else if (strcmp(message, "gll") == 0) {
            current_state = state_handlers[DOOR_LOCKED_STATE]();  // Handle guard left lock
        } else if (strcmp(message, "gru") == 0) {
            current_state = state_handlers[DOOR_UNLOCKED_STATE]();  // Handle guard right unlock
        } else if (strcmp(message, "grl") == 0) {
            current_state = state_handlers[DOOR_LOCKED_STATE]();  // Handle guard right lock
        }

        // Send updated state to des_display
        send_to_display("State updated successfully!");

	return EXIT_SUCCESS;
}


// State handler definitions
State handler_init() {
    printf("Initializing system...\n");
    // Perform initialization actions
    // Transition to WAIT_SCAN_STATE
    return WAIT_SCAN_STATE;
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
