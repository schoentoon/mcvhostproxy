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

#ifndef _PROXYY_H
#define _PROXYY_H

#include "listener.h"

#include <event2/bufferevent.h>

struct proxyy {
  struct listener* listener;
  struct bufferevent* client;
  struct bufferevent* proxied_connection;
  char client_ip[INET_ADDRSTRLEN];
};

struct proxyy* new_proxy(struct listener* listener, struct bufferevent* bev);

void preproxy_readcb(struct bufferevent* bev, void* context);

void proxy_readcb(struct bufferevent* bev, void* context);

void proxied_conn_readcb(struct bufferevent* bev, void* context);

void free_on_disconnect_eventcb(struct bufferevent* bev, short events, void* context);

#endif //_PROXYY_H