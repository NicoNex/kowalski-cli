/*
 * Kowalski
 * Copyright (C) 2019  Nicolò Santamaria
 */

#ifndef TRAVELS_H_
#define TRAVELS_H_


#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#include "list.h"


#define TRAVELS_FILE "res/travels.json"


struct travel {
	int id;
	char *date;
	float price;
	char *destination;
	int seats;
	int64_t token;
};

// Loads the travels list from a json file.
list_t load_travels();


// Returns the pointer to the struct travel in the list.
struct travel *get_travel_by_id(list_t travels, const int id);


// Adds a travel to the travels list and returns it's head.
list_t add_travel(list_t travels, struct travel *travel);

// Deletes a travel from the list and returns it's head.
list_t del_travel(list_t travels, const int id);

// Deletes all travels with the specified token and returns the list head.
list_t del_travels_with_token(list_t travels, const int64_t token);

// Returns the travel pointer with the corresponding token.
struct travel *get_travel_with_token(list_t travels, const int64_t token);

// Updates the file that stores the information regarding the travels.
void update_travels_file(const list_t travels);


// Frees the memory occupied by the travels list.
void dispose_travels(const list_t travels);



#endif // TRAVELS_H_
