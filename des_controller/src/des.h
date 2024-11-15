/*
 * des.h
 *
 *  Created on: Feb 14, 2021
 *      Author: hurdleg
 */

#ifndef DES_H_
#define DES_H_

#define NUM_STATES 9 // Number of states in FSM


typedef enum {
    INIT_STATE = 0,           // Initializing state
    DOOR_SCAN_STATE = 1,      // Person id scanned door state
    DOOR_UNLOCKED_STATE = 2,  // Guard unlock door state
    DOOR_OPEN_STATE = 3,      // Door open state
	WEIGHT_CHECK_STATE = 4,   // Weight check state
    DOOR_CLOSE_STATE = 5,     // Door closed state
    DOOR_LOCKED_STATE = 6,    // Guard lock door state
	SYSTEM_EXIT_STATE = 7,   // Exit state
	LOCK_DOWN_STATE = 8, 	// Lock-down state
} State;


#define NUM_INPUTS 13 // Number of input commands
typedef enum {
    LEFT_SCAN_EVT = 0,         // Left scan event
    GUARD_LEFT_UNLOCK_EVT = 1, // Guard left unlock event
    LEFT_DOOR_OPEN_EVT = 2,    // Left door open event
    WEIGHT_CHECK_EVT = 3,      // Weight check event
    LEFT_DOOR_CLOSE_EVT = 4,   // Left door close event
    GUARD_LEFT_LOCK_EVT = 5,   // Guard left lock event
    GUARD_RIGHT_UNLOCK_EVT = 6, // Guard right unlock event
    RIGHT_DOOR_OPEN_EVT = 7,   // Right door open event
    RIGHT_DOOR_CLOSE_EVT = 8,  // Right door close event
    GUARD_RIGHT_LOCK_EVT = 9,  // Guard right lock event
    RIGHT_SCAN_EVT = 10,       // Right scan event
    EXIT_EVT = 11,             // Exit event
    LOCK_DOWN_EVT = 12,        // Lock-down event
} Input;


const char *inMessage[NUM_INPUTS] = {
    "Left Scan",
    "Guard Left Unlock",
    "Left Door Open",
    "Weight Check",
    "Left Door Close",
    "Guard Left Lock",
    "Guard Right Unlock",
    "Right Door Open",
    "Right Door Close",
    "Guard Right Lock",
    "Right Scan",
    "Exit",
    "Lock-Down",
};

#define NUM_OUTPUTS 14 // Number of output messages
typedef enum {
    INIT_MSG = 0,              // Initializing message
    LEFT_SCAN_MSG = 1,         // Left scan message
    GUARD_LEFT_UNLOCK_MSG = 2, // Guard left unlock message
    LEFT_DOOR_OPEN_MSG = 3,    // Left door open message
    WEIGHT_CHECK_MSG = 4,      // Weight check message
    LEFT_DOOR_CLOSE_MSG = 5,   // Left door close message
    GUARD_LEFT_LOCK_MSG = 6,   // Guard left lock message
    GUARD_RIGHT_UNLOCK_MSG = 7, // Guard right unlock message
    RIGHT_DOOR_OPEN_MSG = 8,   // Right door open message
    RIGHT_DOOR_CLOSE_MSG = 9,  // Right door close message
    GUARD_RIGHT_LOCK_MSG = 10,  // Guard right lock message
    EXIT_MSG = 11,             // Exit message
    LOCK_DOWN_MSG = 12,        // Lock-down message
} Output;

const char *outMessage[NUM_OUTPUTS] = {
    "System Initializing",
    "Scanning Left Door",
    "Guard Unlocking Left Door",
    "Left Door Opened",
    "Checking Person's Weight",
    "Left Door Closed",
    "Guard Locking Left Door",
    "Guard Unlocking Right Door",
    "Right Door Opened",
    "Right Door Closed",
    "Guard Locking Right Door",
    "Exiting System",
    "Person Quarantined (Lock-Down)",
};

// Person struct for storing person-related information
typedef struct {
    int person_id;     // Person's unique ID
    int weight;        // Person's weight
    int state;         // Person's current state
    Input event;
} Person;

// Display struct for storing output messages and the associated person
typedef struct {
    int message_index; // Index into outMessages array
    Person person;     // Person's information
} Display;

void current_state(Display ctr);

#endif  /* DES_H_ */
