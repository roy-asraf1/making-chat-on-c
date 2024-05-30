#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#define PORT "9035"   // Port we're listening on

// Definition of event handler function type
typedef void* (*EventHandler)(void*);

// Definition of Event struct
typedef struct {
    int id;  // Event identifier
    EventHandler handler;  // Event handler function
} Event;

// Definition of Reactor struct
typedef struct {
    Event* events;  // Array of events
    size_t event_count;  // Number of events
    int running;  // Flag to indicate if the reactor is running
    struct pollfd* pfds;  // Array of poll file descriptors
    int fd_count;  // Number of file descriptors
    int fd_size;  // Size of the file descriptor array
	pthread_t thread;
} Reactor;


// Function to create a Reactor
Reactor* createReactor() {
    Reactor* reactor = (Reactor*)malloc(sizeof(Reactor));
    if (reactor == NULL) {
        return NULL;  // Failed to allocate memory
    }

    reactor->events = NULL;
    reactor->event_count = 0;
    reactor->running = 0;
    reactor->pfds = NULL;
    reactor->fd_count = 0;
    reactor->fd_size = 5;

    return reactor;
}

// Function to add an event to the Reactor
void addEvent(Reactor* reactor, int id, EventHandler handler) {
    // Extend the events array
    reactor->events = (Event*)realloc(reactor->events, sizeof(Event) * (reactor->event_count + 1));
    if (reactor->events == NULL) {
        return;  // Failed to allocate memory
    }

    // Add the new event
    reactor->events[reactor->event_count].id = id;
    reactor->events[reactor->event_count].handler = handler;
    reactor->event_count++;
}


// Stop the reactor
void stopReactor(Reactor* reactor) {
    reactor->running = 0;
}
// Wait for the reactor to stop
void waitForReactor(Reactor* reactor,void * this) {
      // pthread_cancel(3);
	pthread_create (&reactor->thread,NULL,this,&reactor);
	   pthread_join(reactor->thread,NULL);
	   pthread_detach(reactor->thread);
}


// Function to handle an event
void handleEvent(Reactor* reactor, int id, void* context) {
    for (size_t i = 0; i < reactor->event_count; i++) {
        if (reactor->events[i].id == id) {
            reactor->events[i].handler(context);
            return;
        }
    }

	 
}

// Free the Reactor
void destroyReactor(Reactor* reactor) {
    if (reactor == NULL) {
        return;
    }

    if (reactor->events) {
        free(reactor->events);
    }
    if (reactor->pfds) {
        free(reactor->pfds);
    }
	
    free(reactor);
}

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        return -1;
    }
    return 0;
}

// Get the listener socket
int get_listener_socket(void) {
    int listener;     // Listening socket descriptor
    int yes = 1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    if (p == NULL) {
        freeaddrinfo(ai); // Cleanup memory
        return -1;
    }

    freeaddrinfo(ai); // Cleanup memory

    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}


// Add a new file descriptor to the set
void addFd(Reactor* reactor, int newfd, EventHandler handler) {
    // If we don't have room, add more space in the pfds array
    if (reactor->fd_count == reactor->fd_size) {
        reactor->fd_size *= 2; // Double it

        reactor->pfds = realloc(reactor->pfds, sizeof(struct pollfd) * reactor->fd_size);
        if (reactor->pfds == NULL) {
            fprintf(stderr, "Failed to allocate memory for pfds\n");
            exit(1);
        }
    }
    reactor->pfds[reactor->fd_count].fd = newfd;
    reactor->pfds[reactor->fd_count].events = POLLIN; // Check ready-to-read

    reactor->fd_count++;

    addEvent(reactor, newfd, handler);
}

// Remove an index from the set
void del_from_pfds(Reactor* reactor, int i)
{
    // Copy the one from the end over this one
    reactor->pfds[i] = reactor->pfds[reactor->fd_count - 1];

    reactor->fd_count--;
	// printf("reactor->fd_count = %d \n",reactor->fd_count);
		if(reactor->fd_count == 1){
			stopReactor(reactor);
			// waitForReactor(reactor);
					
					
		}
}

// Start the reactor
void startReactor(Reactor* reactor) {
    if (reactor->running) {
        return;  // Reactor already running
    }

    reactor->running = 1;
    reactor->fd_count = 1;

   
   // Main loop
 
    set_nonblocking(reactor->pfds[0].fd);
while (reactor->running) {
    int poll_count = poll(reactor->pfds, reactor->fd_count, 25000);

    if (poll_count == -1) {
        perror("poll");
        exit(1);
    }

    // Create an array to store the indices of descriptors to be removed
    int descriptorsToRemove[reactor->fd_count];
    int descriptorsToRemoveCount = 0;

    // Run through the existing connections looking for events
    for (int i = 0; i < reactor->fd_count; i++) {
        // Check if there are any events for this file descriptor
        if (reactor->pfds[i].revents != 0) {
            // Check if someone's ready to read
            if (reactor->pfds[i].revents & POLLIN) {
                if (reactor->pfds[i].fd == reactor->pfds[0].fd) {
                    // If the listener is ready to read, handle a new connection
                    socklen_t addrlen;
                    int newfd;
                    struct sockaddr_storage remoteaddr; // Client address
                    char remoteIP[INET6_ADDRSTRLEN];

                    addrlen = sizeof remoteaddr;
                    newfd = accept(reactor->pfds[0].fd,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        printf("pollserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);

                        // Call the event handler for new connection
                        handleEvent(reactor, newfd, NULL);

                        // Add the new file descriptor to the reactor
                        addFd(reactor, newfd, NULL);
                    }
                } else {
                    // If not the listener, handle data from a client
                    int nbytes;
                    char buf[256],msg[256]="Client socket ",num_c[5];
                    nbytes = recv(reactor->pfds[i].fd, buf, sizeof(buf), 0);

                    if (nbytes <= 0) {
                        // Got error or connection closed by client
                        if (nbytes == 0) {
                            // Connection closed
                            printf("pollserver: socket %d hung up\n", reactor->pfds[i].fd);
                        } else {
                            perror("recv");
                        }

                        descriptorsToRemove[descriptorsToRemoveCount++] = i;

                        continue; // Skip further processing for this file descriptor
                    }
					
					buf[nbytes] = '\0';
					printf("Client socket %d: %s\n",reactor->pfds[i].fd, buf);
					sprintf(num_c, "%d", reactor->pfds[i].fd); // integer to string
					strcat(msg,num_c);
					strcat(msg,": ");
					strcat(msg,buf);
                    // We got some good data from a client
                    for (int j = 0; j < reactor->fd_count; j++) {
                        // Send to everyone!
                        int dest_fd = reactor->pfds[j].fd;

                        // Except the listener and ourselves
                        if (dest_fd != reactor->pfds[0].fd && dest_fd != reactor->pfds[i].fd) {
                            if (send(dest_fd, msg, sizeof(msg), 0) == -1) {
								if (errno != EWOULDBLOCK && errno != EAGAIN) {
											perror("send");
											exit(1);
									}
                            }
                        }
                    }
					memset(buf, 0, sizeof(buf));
					memset(msg, 0, sizeof(msg));
                } 

			}
			// if (reactor->pfds[0].revents & POLLIN)
			// {
            //     char buffer[256];
			// 	memset(buffer, 0, sizeof(buffer));
			// 	fgets(buffer, sizeof(buffer), stdin);
			// 	buffer[strlen(buffer) ] = '\n';
			// 	buffer[strlen(buffer) - 1] = '\0';
			// 	for (int j = 0; j < reactor->fd_count; j++) {
			// 		// Send to everyone!
			// 		int dest_fd = reactor->pfds[j].fd;

			// 		// Except ourselves
			// 		if (dest_fd != reactor->pfds[0].fd) {
			// 			if (send(dest_fd, buffer,sizeof(buffer) , 0) == -1) {
							
			// 				if (errno != EWOULDBLOCK && errno != EAGAIN) {
			// 							perror("send");
			// 							exit(1);
			// 					}
			// 			}
			// 		}
			// 	}
			// }
			
            // Check if there was an error or hang up
            if (reactor->pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                printf("pollserver: socket %d error or hang up\n", reactor->pfds[i].fd);

                close(reactor->pfds[i].fd); // Bye!

                handleEvent(reactor, reactor->pfds[i].fd, NULL);
                descriptorsToRemove[descriptorsToRemoveCount++] = i;
			
            }
        }
    } // END looping through file descriptors

    // Remove the descriptors from reactor->pfds
    for (int i = 0; i < descriptorsToRemoveCount; i++) {
        del_from_pfds(reactor, descriptorsToRemove[i]);
    }
} // END for(;;)--and you thought it would never end!
}



int main()
{
    int listener;     // Listening socket descriptor
    struct pollfd *pfds = NULL;

    // Set up and get a listening socket
    listener = get_listener_socket();

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    pfds = calloc(5, sizeof(struct pollfd));
    if (pfds == NULL) {
        fprintf(stderr, "Failed to allocate memory for pfds\n");
        exit(1);
    }

    // Add the listener to the set
    pfds[0].fd = listener;
    pfds[0].events = POLLIN; // Report ready to read on incoming connection

    // Create the reactor
    Reactor* reactor = createReactor();

    if (reactor == NULL) {
        fprintf(stderr, "error creating reactor\n");
        exit(1);
    }

    // Assign the address of the pfds array to reactor->pfds
    reactor->pfds = pfds;

    // Start the reactor
    startReactor(reactor);
    // Stop the reactor
     //stopReactor(reactor);

     // Wait for the reactor to stop
      //waitForReactor(reactor,reactor->events);
    // Cleanup
    destroyReactor(reactor);
	close(listener);
    // free(pfds);

    return 0;
}
