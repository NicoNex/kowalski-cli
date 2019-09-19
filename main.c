/*
 * Kowalski
 * Copyright (C) 2019  Nicolò Santamaria
 */

#define _GNU_SOURCE
#define _XOPEN_SOURCE

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/drivers.h"
#include "include/travels.h"


enum choices {
	QUIT,

	PRINT_DRIVERS,
	PRINT_TRAVELS,

	RATE_DRIVER,
	BEST_DRIVERS,
	SEARCH_TRAVEL,
	BOOK_TRAVEL,

	ADD_DRIVER,
	MOD_DRIVER,
	DEL_DRIVER,

	ADD_TRAVEL,
	MOD_TRAVEL,
	DEL_TRAVEL
};

// This function recursively prints all drivers in a list.
void print_drivers(const list_t node) {
	if (node == NULL)
		return;

	char txt[256];
	struct driver *drv = GET_OBJ(node);

	snprintf(txt, 256,
		"\nID: %d\nNome: %s\nEtà: %d\nVeicolo: %s\nValutazione: %d/10\n\n",
		drv->id, drv->name, drv->age, drv->vehicle, drv->rating);
	puts(txt);

	print_drivers(NEXT(node));
}

// This function recursively prints all travels in a list.
void print_travels(const list_t node, const list_t drivers) {
	if (node == NULL)
		return;

	char txt[512];
	struct travel *trv = GET_OBJ(node);
	struct driver *drv = get_driver_by_token(drivers, trv->token);

	snprintf(txt, 512,
		"\nID: %d\nDestinazione: %s\nData: %s\nGuidatore: %s\nValutazione: %d/10\nPrezzo: %.2f €\nPosti: %d\n\n",
		trv->id, trv->destination, trv->date, drv->name, drv->rating, trv->price, trv->seats);
	puts(txt);

	print_travels(NEXT(node), drivers);
}

// Displays the drivers with the highest rating in a list.
void print_best_drivers(const list_t drivers) {
	list_t best_drivers = NULL;
	int hrt = 0;

	// Loop to find the highest rating
	for (list_t node = drivers; node; node = NEXT(node)) {
		struct driver *drv = GET_OBJ(node);

		if (drv->rating > hrt)
			hrt = drv->rating;
	}

	// Loop to find all drivers with the highest rating
	for (list_t node = drivers; node; node = NEXT(node)) {
		struct driver *drv = GET_OBJ(node);

		if (drv->rating == hrt)
			best_drivers = list_add(best_drivers, drv);
	}

	print_drivers(best_drivers);
	dispose_list(best_drivers);
}


// This function prints a string given in input
// and returns an int from stdin.
int get_int_input(char *txt) {
	char buf[64];

	fputs(txt, stdout);
	fgets(buf, 64, stdin);

	return strtol(buf, NULL, 10);
}

// Same as get_int_input but returns a float.
float get_float_input(char *txt) {
	char buf[32];

	fputs(txt, stdout);
	fgets(buf, 32, stdin);

	return strtof(buf, NULL);
}

// Prints msg to stdout and writes n bytes read from
// stdin in dest.
void get_str_input(char *msg, char *dest, int n) {
	fputs(msg, stdout);
	fgets(dest, n, stdin);
}

// Prints msg and returns one character read from stdin.
char get_char_input(char *msg) {
	char c;

	fputs(msg, stdout);
	c = getchar();
	while (getchar() != '\n');
	return c;
}

// This function prints msg and returns the pointer
// to a newly allocated string read from stdin.
char *new_string_input(char *msg) {
	char buf[512];
	size_t ln;
	char *str;

	get_str_input(msg, buf, 512);
	ln = strlen(buf) - 1;

	if (buf[ln] == '\n')
		buf[ln] = '\0';

	ln++;
	str = malloc(ln);
	strncpy(str, buf, ln);

	return str;
}


static struct driver *ask_driver(const list_t drivers) {
	int id;

	print_drivers(drivers);
	id = get_int_input("Scrivi l'ID del guidatore\n\n>>> ");

	return get_driver_by_id(drivers, id);
}


void rate_driver(const list_t drivers) {
	int rating;
	struct driver *drv = ask_driver(drivers);

	if (drv == NULL) {
		puts("ID non valido\n");
		rate_driver(drivers);
	}

	do {
		char buf[128];
		snprintf(buf, 128, "Scrivi la valutazione da dare a %s da 1 a 10\n\n>>> ", drv->name);
		rating = get_int_input(buf);

		if (rating < 1 || rating > 10)
			puts("Valutazione incorretta");

	} while (rating < 1 || rating > 10);

	drv->rating = rating;
	update_drivers_file(drivers);
}

// Sets all the characters in a string to lowercase recursively.
static void lower(char *c) {
	if (*c) {
		*c = tolower(*c);
		lower(++c);
	}
}

// This function searches travels whose destination contains a
// string read from stdin.
void search_travels(const list_t travels, const list_t drivers) {
	int seats;
	char text[32];
	list_t matches = NULL;

	get_str_input("Cosa vuoi cercare?\n>>> ", text, 32);
	text[strlen(text)-1] = '\0';
	seats = get_int_input("Quanti posti cerchi?\n>>> ");

	for (list_t node = travels; node; node = NEXT(node)) {
		struct travel *trv = GET_OBJ(node);

		if (strcasestr(trv->destination, text) && trv->seats >= seats)
			matches = list_add(matches, trv);
	}

	matches ? print_travels(matches, drivers) : puts("Nessun viaggio corrisponde alla ricerca :(\n");
}


void book_travel(const list_t travels, const list_t drivers) {
	int id;
	int seats;
	struct travel *trv = NULL;

	print_travels(travels, drivers);

	do {
		id = get_int_input("Scrivi l'ID del viaggio da prenotare\n>>> ");
		trv = get_travel_by_id(travels, id);

		if (trv == NULL)
			puts("Nessun viaggio corrisponde all'ID :(\n");

	} while (trv == NULL);


	char text[64];
	snprintf(text, 64, "Scrivi il numero di posti da prenotare (max %d)\n>>> ", trv->seats);

	do {
		seats = get_int_input(text);

		if (seats < 1 || seats > trv->seats)
			puts("\nNumero di posti non valido :/");

	} while (seats < 1 || seats > trv->seats);

	trv->seats -= seats;
	puts("Viaggio prenotato!");
	print_travels(travels, drivers);
	update_travels_file(travels);
}


// This function modifies a driver in a list.
void mod_driver(const list_t drivers) {
	int id;
	struct driver *drv = NULL;

	print_drivers(drivers);

	do {
		id = get_int_input("Scrivi l'ID del guidatore da modificare o 0 per annullare\n>>> ");
		if (id == 0) {
			puts("Azione annullata");
			return;
		}

		drv = get_driver_by_id(drivers, id);
		if (!drv) puts("Nessun guidatore corrisponde all'ID :/");

	} while (drv == NULL);

	free(drv->name);
	free(drv->vehicle);
	drv->name = new_string_input("Scrivi il nuovo nome del guidatore\n>>> ");
	drv->age = get_int_input("Scrivi l'età del guidatore\n>>> ");
	drv->vehicle = new_string_input("Scrivi il veicolo del guidatore\n>>> ");

	update_drivers_file(drivers);
	print_drivers(drivers);
}


// This function adds a new driver to a list and returns its head.
list_t get_new_driver(const list_t drivers) {
	struct driver *drv = malloc(sizeof(struct driver));

	drv->token = time(NULL);
	drv->name = new_string_input("Immetti il nome del nuovo guidatore\n>>> ");
	drv->age = get_int_input("Immetti l'età del nuovo guidatore\n>>> ");
	drv->vehicle = new_string_input("Immetti il veicolo usato dal guidatore\n>>> ");
	drv->rating = 0;

	return add_driver(drivers, drv);
}


// This function deletes a driver from a list and returns its head.
list_t delete_driver(const list_t drivers) {
	int id;
	struct driver *drv = NULL;

	print_drivers(drivers);

	do {
		id = get_int_input("Scrivi l'ID del guidatore da cancellare o 0 per annullare\n>>> ");
		if (id == 0) {
			puts("Azione annullata");
			return drivers;
		}

		drv = get_driver_by_id(drivers, id);
		if (!drv) puts("Nessun guidatore corrisponde all'ID :/");

	} while (drv == NULL);

	char ans = '\0';
	char txt[80];
	snprintf(txt, 80, "Sicuro di voler rimuovere %s dal sistema? [S/n]\n>>> ", drv->name);


	do {
		ans = get_char_input(txt);
		ans = tolower(ans);
	} while (ans != 's' && ans != 'n');

	switch (ans) {
		case '\n':
		case 's':
			return del_driver(drivers, id);
		case 'n':
			return drivers;
	}

	return drivers;
}

// This function asks the user the date and returns
// it as a string with format DD-MM-YYYY.
static char *get_date() {
	char buf[16];
	time_t epoch;
	struct tm tm;

	memset(&tm, 0, sizeof(struct tm));
	get_str_input("Scrivi la data del viaggio nel formato GG-MM-AAAA\n>>> ", buf, 16);
	strptime(buf, "%d-%m-%Y", &tm);
	epoch = mktime(&tm);

	if (epoch < time(NULL)) {
		puts("Data non valida");
		return get_date();
	}

	size_t len = strlen(buf) - 1;
	buf[len++] = '\0';
	char *str = malloc(len);
	memcpy(str, buf, len);

	return str;
}

// This function is responsible for the I/O with the user
// in the case the user wants to add a new travel, returns
// the head of the list.
list_t get_new_travel(const list_t travels, const list_t drivers) {
	struct travel *trv = malloc(sizeof(struct travel));
	struct driver *drv;

	puts("Per prima cosa scrivi l'ID del guidatore collegato al viaggio");
	drv = ask_driver(drivers);
	trv->token = drv->token;
	trv->destination = new_string_input("Scrivi la destinazione del viaggio\n>>> ");
	trv->date = get_date();
	trv->seats = get_int_input("Scrivi i numero di posti disponibili\n>>> ");
	trv->price = get_float_input("Scrivi il prezzo del viaggio\n>>> ");

	return add_travel(travels, trv);
}

// This function is responsible for the I/O with the user
// in case of travel editing.
void mod_travel(const list_t travels) {
	int id;
	struct travel *trv = NULL;

	do {
		id = get_int_input("Scrivi l'ID del viaggio da modificare o 0 per annullare\n>>> ");
		if (id == 0) {
			puts("Azione annullata");
			return;
		}

		trv = get_travel_by_id(travels, id);
		if (!trv) puts("Nessun viaggio corrisponde all'ID");

	} while (trv == NULL);

	free(trv->date);
	free(trv->destination);
	trv->destination = new_string_input("Scrivi la destinazione del viaggio\n>>> ");
	trv->date = get_date();
	trv->seats = get_int_input("Scrivi i numero di posti disponibili\n>>> ");
	trv->price = get_float_input("Scrivi il prezzo del viaggio\n>>> ");

	update_travels_file(travels);
}

// This function is responsible for the I/O with the user
// in case of travel deletion and returns the head of the list.
list_t delete_travel(const list_t travels) {
	int id;
	struct travel *trv = NULL;

	do {
		id = get_int_input("Scrivi l'ID del viaggio da cancellare o 0 per annullare\n>>> ");

		if (!id)
			return travels;

		trv = get_travel_by_id(travels, id);

		if (!trv)
			puts("Nessun viaggio corrisponde all'ID");

	} while (trv == NULL);

	char ans = '\0';
	char txt[80];
	snprintf(txt, 80, "Sicuro di voler rimuovere la destinazione %s dal sistema? [S/n]\n>>> ", trv->destination);

	do {
		ans = get_char_input(txt);
		ans = tolower(ans);
	} while (ans != 's' && ans != 'n');

	switch (ans) {
		case '\n':
		case 's':
			return del_travel(travels, id);
		case 'n':
			return travels;
	}

	return travels;
}

// This function is responsible for printing the menu.
void print_menu() {

	char *txt = "Menu\n\
    0. Esci\n\n\
    1. Mostra i guidatori\n\
    2. Mostra i viaggi\n\
    3. Valuta un guidatore\n\n\
    4. Migliori guidatori\n\
    5. Cerca tra i viaggi\n\
    6. Prenota un viaggio\n\n\
    7. Aggiungi un guidatore\n\
    8. Modifica un guidatore\n\
    9. Cancella un guidatore\n\n\
    10. Aggiungi un viaggio\n\
    11. Modifica un viaggio\n\
    12. Cancella un viaggio\n\n";

	fputs(txt, stdout);
}


int main(void) {
	list_t drivers = load_drivers();
	list_t travels = load_travels();

	// This is a finite state automata.
	for (;;) {
		print_menu();
		int choice = get_int_input(">>> ");

		switch (choice) {
			case QUIT:
				return 0;

			case PRINT_DRIVERS:
				print_drivers(drivers);
				break;

			case PRINT_TRAVELS:
				print_travels(travels, drivers);
				break;

			case RATE_DRIVER:
				rate_driver(drivers);
				break;

			case BEST_DRIVERS:
				print_best_drivers(drivers);
				break;

			case SEARCH_TRAVEL:
				search_travels(travels, drivers);
				break;

			case BOOK_TRAVEL:
				book_travel(travels, drivers);
				break;

			case ADD_DRIVER:
				drivers = get_new_driver(drivers);
				print_drivers(drivers);
				break;

			case MOD_DRIVER:
				mod_driver(drivers);
				break;

			case DEL_DRIVER:
				travels = delete_driver(drivers);
				print_drivers(drivers);
				break;

			case ADD_TRAVEL:
				travels = get_new_travel(travels, drivers);
				print_travels(travels, drivers);
				break;

			case MOD_TRAVEL:
				mod_travel(travels);
				print_travels(travels, drivers);
				break;

			case DEL_TRAVEL:
				print_travels(travels, drivers);
				travels = delete_travel(travels);
				print_travels(travels, drivers);
				break;

			default:
				puts("Scelta non valida");
		}
	}

	return 0;
}
