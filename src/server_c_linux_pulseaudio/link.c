/***
 *	This file is part of pa-network-volume-control.
 *
 *  Copyright 2010 - 2010 Tor Martin Slåen <tormsl@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***/

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#include "link.h"

#define BUFSIZE 1024

static int udp_socket;
static struct sockaddr_in addr;

void (*receive_callback)(char*, char*, int);

int init_link(uint16_t port) {
	int err;

	udp_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if( udp_socket < 0 ) {
		fprintf(stderr, "Failed to create local UDP socket: `%s'\n", strerror(errno));
		return -1;
	}

	bzero( &addr, sizeof(struct sockaddr_in) );
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	err = bind( udp_socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in) );
	if( err < 0 ) {
		fprintf(stderr, "Failed to bind local UDP socket to port %d\n", port );
		return -1;
	}

	return 0;
}

void link_subscribe_receive(void (*callback)(char*, char*, int)) {
	receive_callback = callback;
}

void link_read() {
	ssize_t n;
	struct	sockaddr_in remote;
	char*	remote_host;
	int		remote_port;
	int		len = sizeof(struct sockaddr_in);
	char	buf[BUFSIZE];

	bzero(&remote, sizeof(struct sockaddr_in));
	bzero(buf, BUFSIZE);
	n = recvfrom(udp_socket, buf, BUFSIZE, 0, (struct sockaddr*)&remote, (socklen_t*)&len);

	remote_host = inet_ntoa(remote.sin_addr);
	remote_port = htons(remote.sin_port);

	if( n < 0 ) {
		return;
	} else {
		receive_callback(remote_host, buf, n);
	}
}

int link_get_fd() {
	return udp_socket;
}



void cleanup_link() {
	close(udp_socket);
}

