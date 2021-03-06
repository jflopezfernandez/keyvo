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
 * @todo Implement hashing functionality.
 *
 * - gperf
 * - sparsehash
 * - libkeccak
 * - mhash
 * - xxhash
 *
 * - murmurhash3
 * - jenkinshash
 * 
 */

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
            syslog(LOG_ERR, "%s\n", "Memory-allocation failure.");
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @brief Once the server enters this function, it is ready
 * to delete the file lock which was heretofore preventing
 * other potential incarnations of the process to become
 * the server.
 *
 * All of the data structures being used in the
 * implementation right now are the obvious and highly-
 * inefficient kind for the purposes of prototyping. Once
 * we move to a data-model that relies on eventual-
 * consistency and optimistic replication. For the moment,
 * however, our low-tech mutex is the state of the part, as
 * far as the project is concerned, and the file lock must
 * be removed if we would like to be able to re-run the
 * server without manual intervention.
 * 
 */
void remove_lock_on_exit(void) {
    /**
     * @note Note, the callback is going to be registered
     * with the *normal* process termination callback only
     * for the moment, so the file lock will not be removed
     * on a quick exist, such as that invoked by the _Exit()
     * or _exit() system calls.
     *
     * Begin by clearing errno to make sure we don't pollute
     * the error code.
     * 
     */
    errno = 0;

    /**
     * @brief Carry out the deletion command on the
     * lockfile.
     * 
     */
    int result = unlink(LOCKFILE);

    /**
     * @brief Since we're exiting anyway, there's no need do
     * to any branched returns; we're simply going to output
     * a diagnostic message to standard out, but other than
     * that, there isn't much else for us to do here.
     *
     * @note This branch keeps getting triggered for some
     * reason even though the lockfile does exist and is
     * being successfully deleted. I wonder if it has
     * something to do with the fact that the function is
     * being called as an exit callback.
     *
     * @note The error message is not wrong; I forgot the
     * server has to call fork() in order to become a
     * daemon. The filelock deletion callback is therefore
     * being triggered twice but failing once, since
     * obviously the filelock no longer exists after being
     * deleted.
     * 
     */
    if (result == -1) {
        /**
         * @brief Echo diagnostic information out to the
         * system log, making sure to get the official 
         * error message for the value in errno via a call
         * to strerror.
         * 
         */
        syslog(LOG_ERR, "Could not delete the file lock: %s - %s", strerror(errno), "The filelock mutex was not deleted, and will **prevent the server from starting until it is manually removed**.");
    }
}

/**
 * @brief This function's entire purpose in life is to make
 * sure a trace call to the system log is the last thing
 * the world remembers of this process.
 * 
 */
void print_to_syslog_on_exit(void) {
    /**
     * @brief As this processes' last action on this Earth,
     * print to syslog, and become truly immortal.
     * 
     */
    syslog(LOG_DEBUG, "%s", "Server shutdown in progress...");
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
    
    /** Reset errno for diagnostics */
    errno = 0;

    /**
     * @brief Test whether the global file lock is in its
     * configured location. Additionally, the server checks
     * to make sure the file lock is both readable and
     * writable, thus preventing multiple processes from
     * running into one another.
     *
     * @bug The file permissions belowe are meant to allow
     * for the file in question to act as a mutex but
     * allowing a process to open it in read-write mode if
     * and only if it has not already been opened by
     * another process. Whether the file exists before it
     * is opened should be irrelevant.
     *
     * Update: I've added the O_CRET flag to the function
     * call, so we'll see if it fixes the problem.
     * 
     */
    int lockfile_descriptor = open(LOCKFILE, O_CREAT | O_RDWR | O_CREAT | O_EXCL, LOCKMODE);

    if (lockfile_descriptor == -1) {
        
        /**
         * @brief Someone else has access to it at the
         * moment, which means we are not the first process
         * to try to become the server.
         *
         * @details Rather than simply exiting with an error
         * right away, let the user know exactly what they
         * need to do to correctly increase the server's
         * capacity to handle traffic.
         * 
         */
        syslog(LOG_ERR, "%s: %s", "Cannot open lock file", LOCKFILE);

        /**
         * @brief Return to daemonize().
         * 
         */
        return true;
    }

    /**
     * @brief Close the file descriptor holding a reference
     * to the lockfile; we don't need it. The pre-existence
     * of a lock file is enough to trigger a failure when
     * inadvertently spawning another instance of the
     * server.
     * 
     */
    close(lockfile_descriptor);

    /**
     * @brief No problem found; continue establishing
     * daemon environment.
     * 
     */
    return false;
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
         * there's nothing for us to do. Let the user know
         * what happened and exit with an error status.
         * 
         */
        syslog(LOG_ERR, "%s\n", "It seems you were already running a primary server. Are looking for replication?");

        /**
         * @brief Exit with an error status so both the
         * kernel and the user know that something went
         * wrong.
         * 
         */
        exit(EXIT_FAILURE);
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
            syslog(LOG_ERR, "%s\n", "Error after calling fork()");
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
            /**
             * @brief Print a message to the system log for
             * debugging purposes.
             * 
             */
            syslog(LOG_DEBUG, "%s\n", "Keyvo parent threat terminating...");

            /**
             * @brief Go ahead and terminate the parent
             * process now that all of the complex setuid()
             * bookkeeping has been handled.
             * 
             * @return Exit 
             */
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
        syslog(LOG_ERR, "%s\n", "Failed to change directory");
        exit(EXIT_FAILURE);
    }

    /**
     * @brief Register the final syslog trace as a callback
     * pending the normal termination of the program.
     * 
     */
    if (atexit(print_to_syslog_on_exit) != 0) {
        /**
         * @brief The 'atexit' function returns zero on
         * success and an unspecified non-zero value on
         * failure, so if we made it to this branch,
         * something went wrong.
         *
         * @details Since all we're doing is registering a
         * debug callback, we're not going to treat this
         * error as fatal, but it will definitely need to
         * be examined further before we can assertain the
         * error to be genuinely spurious.
         *
         * Ironically, if we fail to register a callback
         * that prints to prints to syslog, we print to
         * syslog. If that call fails, it doesn't matter,
         * since, again, it's just a minor additional datum.
         * Besides, we can mathematically show that this
         * problem beta-reduces to the Byzantine Generals
         * problem, and are therefore for all intents and
         * purposes the same thing. Thus, we move on with
         * our lives.
         */
        syslog(LOG_WARNING, "%s", "Failed to register syslog exit tracer callback.");
    }

    /**
     * @brief Register the filelock-deletion subroutine as a
     * callback on the regular-priority exit handler.
     * 
     */
    if (atexit(remove_lock_on_exit) != 0) {
        /**
         * @brief A diagnostic is technically more valuable
         * in this case, but the repercussions here last
         * beyond the scope of the final execution, since
         * the server will be unable to start when it's
         * being blocked by the mutex. On the other hand, 
         * however, the repercussions at worst are
         * technically just the same information they would
         * have gotten from us here, minus a few details.
         *
         * We're going to speed things up by simply logging
         * any diagnostics to the journal, so that any
         * system administrators can solve their problem as
         * soon as they look in the first logical place.
         * 
         */
        syslog(LOG_ERR, "%s", "The filelock mutex deletion callback could not be registered.");
    }

    /**
     * @brief Get the resource limits for the current user
     * so we can evaluate the filehandle situation.
     * 
     */
    struct rlimit rl;

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        syslog(LOG_ERR, "%s", "Error in call to getrlimit()");
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
        syslog(LOG_ERR, "%s", "Fatal error after calling sigaction()");
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
        syslog(LOG_ERR, "%s %s %s", "Unexpected file descriptors", "Unexpected file descriptors", "Unexpected file descriptors");
        exit(EXIT_FAILURE);
    }

    /**
     * @brief Print a message to the system log once we've
     * completed the daemonization process.
     * 
     */
    syslog(LOG_DEBUG, "%s", "Daemonization complete; the server has been initialized.");
}

/**
 * @brief This variable is set by the --verbose or --quiet
 * command-line options.
 * 
 */
static int verbose = false;

/**
 * @brief This variable is set by the --filename ARG or
 * -F ARG command-line options.
 * 
 */
const char* configuration_filename = NULL;

/**
 * @brief The following table contains a description of the
 * long options supported by the server.
 * 
 */
static struct option long_options[] = {
    { "help",           no_argument,        0,                  'h' },
    { "verbose",        no_argument,        &verbose,            1  },
    { "quiet",          no_argument,        &verbose,            0  },
    { "configuration-filename",         required_argument,  0,  'f' },
    {   0,              0,              0, 0 }
};

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
    /**
     * @brief Begin parsing command-line options.
     *
     * @bug The character option string for the command-
     * line arguments was just thrown together without much
     * thought. As a result, command-line flags with no
     * values, such as '--verbose', '--help', or '--quiet',
     * are sometimes recognized and sometimes not. Dare I
     * say it would be nice of us to at least decrease the
     * variance inherent in the given error distribution, if
     * we were so inclined, to speak nothing of actually
     * *fixing* the problem, of course.
     * 
     */
    int option_index = 0;

    /**
     * @brief The return value from getopt_long is set
     * to -1 when there are no more command-line
     * arguments to parse, at which point we can exit
     * this loop;
     * 
     */
    int c = 0;

    /**
     * @brief Commence command-line argument parsing.
     * 
     */
    while ((c = getopt_long(argc, argv, "+vqhf:", long_options, &option_index)) != -1) {
        switch (c) {
            case 0: {
                /** @todo Fix this */
            } break;

            case 'f': {
                configuration_filename = optarg;
                
                /** @todo Remove after testing */
                printf("Filename: %s\n", optarg);
            } break;

            case 'h': {
                /** @todo Remove after testing */
                printf("Help Menu\n");
            } break;

            case 'v': {
                /** @todo Remove after testing */
                printf("verbose\n");
            } break;

            case '?': {
                /** @todo Remove after testing */
                //printf("?? %s\n", long_options[optind].flag);
                /**
                 * @brief There's no need to display an
                 * error message here because getopt_long
                 * will have already done that.
                 *
                 * We can simply exit with a non-zero status
                 * to indicate an error has ocurred.
                 * 
                 */
                return EXIT_FAILURE;
            } break;

            default: {
                /**
                 * @brief This branch should never be taken;
                 * if it is, it represents a catastrophic
                 * breakdown in logic, and concrete proof
                 * that I'm dumb.
                 * 
                 */
                fprintf(stderr, "%s\n", "[Fatal Error] Invalid code path in option parsing");

                /**
                 * @brief Exit with a non-zero exit status
                 * to make the shame both objectively real
                 * and quantitative.
                 * 
                 */
                return EXIT_FAILURE;
            } break;
        }
    }

    /** @todo Remove after testing */
    if (option_index < argc) {
        printf("non-option argv-elements: ");

        while (optind < argc) {
            printf("%s ", argv[optind++]);
        }

        printf("\n");
    }

    /** @todo Remove after testing */
    printf("Verbose: %d\n", verbose);

    // TODO: Read command-line line args
    // TODO: Read configuration file ----------
    // TODO: Daemonize
    // TODO: Listen for SIGHUP to reload configuration
    // TODO: Wait for incoming socket connections
    // TODO: Accept commands: ( DEFINE | UPDATE | DROP )

    /**
     * @brief Cross over to the spirit world.
     *
     * @details If there is any problem during the
     * daemonization procedure, the server simply exists
     * while outputting a trace to the system log. Any code
     * beyond this point can be safely assumed to execute
     * only after the server's environment has been properly
     * set up.
     *
     */
    daemonize();

    return EXIT_SUCCESS;
}
