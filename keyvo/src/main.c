/**
 *  Keyvo - Key-Value Caching Server
 *  Copyright (C) Jose Fernando Lopez Fernandez, 2020.
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include "keyvo.h"

/**
 * @brief This struct contains two mutable char pointers
 * which will point at a dynamic configuration setting.
 */
struct key_val_t {
    char* key;
    char* val;
};

/**
 * @brief This is the primary datastructure in the server,
 * as a collection of key-value pairs is the definition of
 * a configuration.
 * 
 */
struct symbol_table_t {
    struct key_val_t* key_vals;
};

/**
 * @brief This is the root node of our symbol table.
 *
 * @details For the moment, we are implementing the symbol
 * table as a dynamic array through which we much iterate
 * in order to find a keyval. This is being done for the
 * purposes of prototyping, and not because it's actually
 * not a terrible idea.
 *
 * @author Jose Fernando Lopez Fernandez
 * 
 */
struct symbol_table_t* symbol_table = NULL;

/**
 * @brief This function is meant to be called one and only
 * one time, simply for the purposes of allocating the
 * initial memory to the symbol table. After that, the
 * subroutine which will handle adding elements to the
 * table will handle the necessary baggage to dynamically
 * allocate memory for the symbol table.
 * 
 * @param symbol_table 
 * @return int 
 */
int initialize_symbol_table(struct symbol_table_t* symbol_table) {
    if (symbol_table == NULL) {
        symbol_table = malloc(10 * sizeof (struct key_val_t));

        if (symbol_table == NULL) {
            fprintf(stderr, "%s\n", "Memory-allocation failure.");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < 10; ++i) {
            symbol_table[i] = NULL;
        }
    }

    return symbol_table;
}

/**
 * @brief This is the entry point of the server execution
 * process.
 *
 * @details In order, the server interprets command-line
 * arguments, reads the configuration file, and daemonizes
 * before beginning to actually deal with its first queries.
 * 
 * @param argc 
 * @param argv 
 * @return int
 */
int main(int argc, char *argv[])
{
    // TODO: Read command-line line args
    // TODO: Read configuration file
    // TODO: Daemonize
    // TODO: Accept commands: ( DEFINE | UPDATE | DROP )

    return EXIT_SUCCESS;
}
