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

#ifndef PROJECT_INCLUDES_KEYVO_H
#define PROJECT_INCLUDES_KEYVO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#if !defined(unix) || !defined(linux)
    #include <sys/stat.h>
    #include <sys/resource.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <syslog.h>
    #include <unistd.h>
    #include <errno.h>

    #include <getopt.h>
#else
    #error "The current platform is not supported."
#endif /** @todo Move to a configuration file */

#ifndef LOCKFILE
#define LOCKFILE "keyvo.lock"
#endif /** Keyvo lockfile */

#ifndef LOCKMODE
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#endif /** @todo Move to a configuration file */

/**
 * @brief The errno variable is simply declared to have
 * external linkage here, so that no one has any linking
 * problems.
 * 
 */
extern int errno;

#endif /** PROJECT_INCLUDES_KEYVO_H */
