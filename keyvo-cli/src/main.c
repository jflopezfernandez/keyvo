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

int main(void)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* bind_address = NULL;

    int error = 0;

    if ((error = getaddrinfo(0, "8080", &hints, &bind_address)) != 0) {
        fprintf(stderr, "%s\n", gai_strerror(error));
        return EXIT_FAILURE;
    }

    int listener_socket = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

    if (listener_socket == -1) {
        fprintf(stderr, "%s\n", "Error in call to socket().");
        return EXIT_FAILURE;
    }

    if (bind(listener_socket, bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "%s\n", "Error in call to bind().");
        return EXIT_FAILURE;
    }

    freeaddrinfo(bind_address);

    fd_set master;
    FD_ZERO(&master);
    FD_SET(listener_socket, &master);
    int max_socket = listener_socket;

    printf("%s\n", "Server ready...");

    while (1) {
        fd_set reads = master;

        if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
            fprintf(stderr, "%s\n", "select() failed");
            return EXIT_FAILURE;
        }

        if (FD_ISSET(listener_socket, &reads)) {
            struct sockaddr_storage client_address;
            socklen_t client_len = sizeof (client_address);

            char read[1024] = { 0 };

            int bytes_received = recvfrom(listener_socket, read, 1024, 0, (struct sockaddr *) &client_address, &client_len);

            if (bytes_received < 1) {
                fprintf(stderr, "%s\n", "The connection was closed.");
                return EXIT_FAILURE;
            }

            for (int i = 0; i < bytes_received; ++i) {
                read[i] = toupper(read[i]);

                if (read[i] == EOF) {
                    fprintf(stderr, "%s\n", "The connection has been closed.");
                    break;
                }
            }

            ssize_t result = sendto(listener_socket, read, bytes_received, 0, (struct sockaddr *) &client_address, client_len);

            if (result == -1) {
                fprintf(stderr, "%s\n", "Error in call to sendto()");
                break;
            }
        }
    }

    printf("%s\n", "Shutting down...");

    close(listener_socket);

    return EXIT_SUCCESS;
}
