/*  mcvhostproxy
 *  Copyright (C) 2013  Toon Schoenmakers
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
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#include <arpa/inet.h>

#include <event2/event.h>

#define FORWARD_PING ((struct ping_mode*) NULL)

#define STDOUT (char*) stdout
#define STDERR (char*) stderr

static char* SYSLOG = "syslog";

struct ping_mode {
  char* motd;
  char* version;
  unsigned short numplayers;
  unsigned short maxplayers;
};

struct vhost {
  char* vhost;
  struct sockaddr_in* address;
};

struct listener {
  struct sockaddr_in* address;
  struct evconnlistener* listener;
  struct vhost** vhosts;
  struct ping_mode* ping_mode;
  char* logfile;
};

struct listener* new_listener(char* address);

struct vhost* new_vhost(char* vhost);

int fill_in_vhost_address(struct vhost* vhost, char* address);

int parse_config(char* filename);

int dispatch_config(struct event_base* base);

#endif //_CONFIG_H