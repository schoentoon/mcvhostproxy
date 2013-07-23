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

#include "listener.h"

#include "proxy.h"

#include <errno.h>
#include <string.h>

#include <event2/listener.h>
#include <event2/bufferevent.h>

static void listener_callback(struct evconnlistener* evconn, evutil_socket_t fd
                             ,struct sockaddr* sa, int socklen, void* arg);

int init_listener(struct event_base* base, struct listener* listener) {
  listener->listener = evconnlistener_new_bind(base, listener_callback, listener
                                              ,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1
                                              ,(struct sockaddr*) listener->address, sizeof(struct sockaddr_in));
  if (!listener->listener) {
    char ipbuf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &listener->address->sin_addr, ipbuf, sizeof(ipbuf));
    fprintf(stderr, "Error '%s' on %s:%d.\n", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()), ipbuf, ntohs(listener->address->sin_port));
    return 1;
  }
  return 0;
};

static void listener_callback(struct evconnlistener* evconn, evutil_socket_t fd
                             ,struct sockaddr* sa, int socklen, void* arg) {
  struct event_base* base = evconnlistener_get_base(evconn);
  struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  if (!bev)
    event_base_loopbreak(base);
  else {
    struct listener* listener = arg;
    struct proxyy* proxyy = new_proxy(listener, bev);
    bufferevent_setcb(bev, preproxy_readcb, NULL, free_on_disconnect_eventcb, proxyy);
    bufferevent_enable(bev, EV_READ);
  }
};