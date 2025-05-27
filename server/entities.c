#include <stdio.h>
#include <string.h>
#include "entities.h"

// Agrega un nuevo reporte al historial de la biblioteca.
void library_addReport(Library *library, char status, const char *title, int isbn, int idCopy, const char *date) {
    static int reportIndex = 0;
    if (reportIndex >= NAME_SIZE) return;

    Report *r = &library->reports[reportIndex++];
    r->status = status;
    strncpy(r->bookName, title, MAX_CHARACTERS);
    r->isbn = isbn;
    r->idCopy = idCopy;
    strncpy(r->date, date, sizeof(r->date));
}

// Busca un libro por su ISBN dentro de la biblioteca.
Book *library_findBookByISBN(Library *lb, int isbn) {
    if (!lb) return NULL;

    for (int i = 0; i < MAX_BOOKS; i++) {
        if (lb->books[i].isbn == isbn) {
            return &lb->books[i];
        }
    }
    return NULL;
}

// Retorna una copia disponible ('D') del libro especificado por ISBN.
Copy *library_findAvailableCopy(Library *lb, int isbn) {
    Book *book = library_findBookByISBN(lb, isbn);
    if (!book) return NULL;

    for (int i = 0; i < book->totalCopies; i++) {
        if (book->copies[i].status == 'D') {
            return &book->copies[i];
        }
    }
    return NULL;
}

// Retorna una copia prestada ('P') del libro especificado por ISBN.
Copy *library_findBorrowedCopy(Library *lb, int isbn) {
    Book *book = library_findBookByISBN(lb, isbn);
    if (!book) return NULL;

    for (int i = 0; i < book->totalCopies; i++) {
        if (book->copies[i].status == 'P') {
            return &book->copies[i];
        }
    }
    return NULL;
}
