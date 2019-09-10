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


int get_int_input(char *txt) {
	char buf[64];

	fputs(txt, stdout);
	fgets(buf, 64, stdin);

	return strtol(buf, NULL, 10);
}


void get_str_input(char *msg, char *dest, int n) {
	fputs(msg, stdout);
	fgets(dest, n, stdin);
}


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


struct driver *ask_driver(const list_t drivers) {
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

// FIXME
void search_travels(const list_t travels, const list_t drivers) {
	int seats;
	char text[32];
	list_t matches = NULL;

	get_str_input("Cosa vuoi cercare?\n>>> ", text, 32);
	seats = get_int_input("Quanti posti cerchi?\n>>> ");

	for (list_t node = travels; node; node = NEXT(node)) {
		struct travel *trv = GET_OBJ(node);

		// apparently the strcasestr returns always NULL wtf
		if (strcasestr(trv->destination, text) && trv->seats >= seats)
			matches = list_add(matches, trv);
	}

	matches ? print_travels(travels, drivers) : puts("Nessun viaggio corrisponde alla ricerca :(\n");
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
	print_travels(travels, drivers);
	update_travels_file(travels);
}


void mod_driver(const list_t drivers) {
	int id;
	struct driver *drv = NULL;

	print_drivers(drivers);

	do {
		id = get_int_input("Scrivi l'ID del guidatore da modificare\n>>> ");
		drv = get_driver_by_id(drivers, id);

		if (drv == NULL)
			puts("Nessun guidatore corrisponde all'ID :/");

	} while (drv == NULL);

	free(drv->name);
	free(drv->vehicle);
	drv->name = new_string_input("Scrivi il nuovo nome del guidatore\n>>> ");
	drv->age = get_int_input("Scrivi l'età del guidatore\n>>> ");
	drv->vehicle = new_string_input("Scrivi il veicolo del guidatore\n>>> ");

	update_drivers_file(drivers);
	print_drivers(drivers);
}


list_t get_new_driver(const list_t drivers) {
	char buf[128];
	size_t len;
	struct driver *drv = malloc(sizeof(struct driver));

	drv->token = time(NULL);
	drv->name = new_string_input("Immetti il nome del nuovo guidatore\n>>> ");
	drv->age = get_int_input("Immetti l'età del nuovo guidatore\n>>> ");
	drv->vehicle = new_string_input("Immetti il veicolo usato dal guidatore\n>>> ");
	drv->rating = 0;

	return add_driver(drivers, drv);
}


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

	for (;;) {
		print_menu();
		int choice = get_int_input(">>> ");

		switch (choice) {
			case QUIT:
				exit(0);

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

			case ADD_TRAVEL:
			case MOD_TRAVEL:
			case DEL_TRAVEL:

			default:
				puts("Scelta non valida");
				continue;
		}

	}

	return 0;
}
