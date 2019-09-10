/*
 * Kowalski
 * Copyright (C) 2019  Nicolò Santamaria
 */

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


// TODO: fix the segfault here
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

	#pragma omp parallel
	{
		dispose_list(best_drivers);
	}
}


int get_int_input(char *txt) {
	char buf[64];

	fputs(txt, stdout);
	fgets(buf, 64, stdin);

	return strtol(buf, NULL, 10);
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

	#pragma omp parallel
	{
		update_drivers_file(drivers);
	}
}


void print_menu() {

	char *txt = "Menu\n\
	0. Esci\n\n\
	1. Mostra i guidatori\n\
	2. Mostra i viaggi\n\
	3. Valuta un guidatore\n\
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
			case BOOK_TRAVEL:
			case ADD_DRIVER:
			case MOD_DRIVER:
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
