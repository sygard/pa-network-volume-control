#ifndef LINK_H
#define LINK_H

#include <stdint.h>

int init_link(uint16_t port);
void cleanup_link();

void link_read();
void link_subscribe_receive(void (*callback)(char*, char*, int));

int link_get_fd();

#endif
