#ifndef LINK_H
#define LINK_H

enum messagetype {
	VOL_UP,
	VOL_DN,
	MUTE,
	NOMUTE,
};

typedef enum messagetype messagetype_t;

int init_link(char *hostname, uint16_t port);
int cleanup_link();
int link_send(messagetype_t type);


#endif
