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
