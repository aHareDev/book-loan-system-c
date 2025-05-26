#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "buffer.h"
#include "entities.h"

void *auxiliar1(void *arg) {
	Library *lb = (Library *)arg;
	Request req;

	while (*(lb->ejecutando)) {
		buffer_consume(lb->buf, &req);

		if (req.operation != 'D') {
			for (int i = 0; i < MAX_BOOKS; i++) {
				if (lb->books[i].isbn != req.isbn) continue;

				for (int j = 0; j < lb->books[i].totalCopies; j++) {
					Copy *c = &lb->books[i].copies[j];
					if (c->status == 'P') {
						c->status = 'D';

						time_t t = time(NULL);
						struct tm *now = localtime(&t);
						snprintf(c->date, sizeof(c->date), "%02d-%02d-%04d", 
							now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
						break;
					}
				}
				break;
			}
		}
	}
	return NULL;
}
