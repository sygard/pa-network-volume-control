#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "link.h"

struct phys_conn {
	int					device;
	char*				remote_hostname;
	uint16_t			remote_port;
	struct sockaddr_in	addr;
};

static struct phys_conn *conn;

int init_link(char *hostname, unsigned short port) {
	fprintf(stderr, "setup_link(%s, %d)\n", hostname, port);
	
	conn = calloc(1, sizeof(struct phys_conn));
	conn->remote_hostname = strdup(hostname);
	conn->remote_port = port;

	conn->addr.sin_family = AF_INET;
	conn->addr.sin_port = htons(port);

	struct hostent *he;
	if( !(he = gethostbyname(hostname)) ) {
		herror("gethostbyname");
		return -1;
	}

	memcpy(&conn->addr.sin_addr, he->h_addr, he->h_length);

	conn->device = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

int link_send(messagetype_t type) {

	char message;
	switch(type) {
		case VOL_UP:
			message = '+';
			break;
		case VOL_DN:
			message = '-';
			break;
		case MUTE:
			message = 'M';
			break;
		case NOMUTE: 
			message = 'm';
			break;
		default:
			return -1;
	}
	
	ssize_t n = sendto(conn->device, &message, 1, 0, (struct sockaddr*) &conn->addr, sizeof(struct sockaddr_in));
	if( n < 0 ) {
		perror("sendto");
		return -1;
	}

}


int cleanup_link() {
	fprintf(stderr, "cleanup_link()\n");

	close( conn->device );
	free( conn->remote_hostname );
	free( conn );
}

