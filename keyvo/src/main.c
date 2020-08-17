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
void initialize_symbol_table(struct symbol_table_t* symbol_table) {
    if (symbol_table == NULL) {
        symbol_table = malloc(10 * sizeof (struct key_val_t));

        if (symbol_table == NULL) {
            fprintf(stderr, "%s\n", "Memory-allocation failure.");
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief This function checks whether there is already a
 * version of the process being executed.
 *
 * @details Upon reaching the usual spot where the process
 * would create a file for you, the server simply checks
 * to make sure the file is both readable and writable.
 * 
 * @return true 
 * @return false 
 */
static bool already_running(void) {
    
    /**
     * @brief Test whether the global file lock is in its
     * configured location. Additionally, the server checks
     * to make sure the file lock is both readable and
     * writable, thus preventing multiple processes from
     * running into one another.
     * 
     */
    int fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);

    /** @todo: Complete this - IN PROGRESS */
    return true;
}

/**
 * @brief This function take care of the minutia of losing
 * the physical bonds that tie us to our mortal flesh,
 * transcending objective existence and crossing over into
 * the mystical spirit world of daemon processes, or
 * services.
 * 
 */
void daemonize(void) {
    /**
     * @brief Before we do all of the billions of things
     * required of us before we can become a daemon, let's
     * check to make sure a previous process isn't already
     * there.
     * 
     */
    if (already_running()) {
        
        /**
         * @brief There was a process already running, so
         * there's nothing for us to do. Return void and
         * go back to whence we came.
         * 
         */
        return;
    }

    /**
     * @brief Clear the file creation mask.
     * 
     */
    umask(0);

    /**
     * @brief Clear any flags in errno to be able to handle
     * any errors during the subsequent execution.
     * 
     */
    errno = 0;

    /**
     * @brief Become a session leader by calling fork and
     * exiting from the parent process. The child process
     * will continue the daemonization.
     * 
     */
    pid_t pid = fork();

    /**
     * @brief Make sure the process fork actually proceeded
     * without a hitch.
     * 
     */
    switch (pid) {
        
        /**
         * @brief Check whether the process failed while
         * attempting to duplicate itself.
         * 
         */
        case -1: {
            
            /**
             * @brief The process was unable to fork; check
             * the value of errno to see whether we can
             * figure out what exactly happened.
             *
             * For the moment, we are simply echoing a
             * quick statement.
             * 
             */
            fprintf(stderr, "%s\n", "Error after calling fork()");
            exit(EXIT_FAILURE);
        } break;

        /**
         * @brief This is the newly-spawned child thread.
         * 
         */
        case 0: {
            /**
             * @brief Proceed by breaking out of this loop
             * and continuing the daemonization.
             * 
             */
        } break;

        /**
         * @brief This is the parent thread. Having
         * fulfilled its biological imperative, its mission
         * is now complete; terminate.
         * 
         */
        default: {
            /** @brief Exit with a success status code */
            exit(EXIT_SUCCESS);
        } break;
    }

    /**
     * @brief Create a new session if the calling process is
     * not a group leader. This is part of the reason for
     * all of the security hullabaloo of forking the process
     * just to kill one of them; the child process was a
     * subordinate of the parent from which it forked, and
     * by terminating the parent, we created a process which
     * had terminal access but no sudo privileges of any
     * kind (hopefully).
     * 
     */
    setsid();

    /**
     * @brief Change the current working directory to root
     * so we won't prevent file systems from being mounted.
     * 
     */
    if (chdir("/") < 0) {
        fprintf(stderr, "%s\n", "Failed to change directory");
        exit(EXIT_FAILURE);
    }

    /**
     * @brief Get the resource limits for the current user
     * so we can evaluate the filehandle situation.
     * 
     */
    struct rlimit rl;

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        fprintf(stderr, "%s\n", "Error in call to getrlimit()");
        exit(EXIT_FAILURE);
    }

    /**
     * @brief Cap the number that tells the you how many
     * files a user has open in total; the information was
     * obviously going up.
     * 
     */
    if (rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }

    /**
     * @brief Close all open file descriptors.
     * 
     */
    for (size_t i = 0; i < rl.rlim_max; ++i) {
        close(i);
    }

    /**
     * @brief Attach any remaining open file descriptors to
     * /dev/null.
     * 
     */
    int fd0 = open("dev/null", O_RDWR);
    int fd1 = dup(0);
    int fd2 = dup(0);
        
    /**
     * @brief Ensure future opens will not allocate
     * controlling terminals.
     * 
     */
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        fprintf(stderr, "%s\n", "Fatal error after calling sigaction()");
        exit(EXIT_FAILURE);
    }

    /**
     * @brief Initialize the log file.
     * 
     */
    openlog("keyvo", LOG_CONS, LOG_DAEMON);

    /**
     * @brief Ensure we successfully unset all file
     * descriptors.
     * 
     */
    if ((fd0 != 0) || (fd1 != 1) || (fd2 != 2)) {
        syslog(LOG_ERR, "%s %s %s\n", "Unexpected file descriptors", "Unexpected file descriptors", "Unexpected file descriptors");
        exit(EXIT_FAILURE);
    }
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

    /**
     * @brief Cross over to the spirit world.
     *
     */
    daemonize();

    return EXIT_SUCCESS;
}
