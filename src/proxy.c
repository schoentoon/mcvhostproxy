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

#include "proxy.h"

#include "debug.h"

#include <stdlib.h>
#include <string.h>

#include <event2/buffer.h>
#include <ctype.h>

struct proxyy* new_proxy(struct listener* listener, struct bufferevent* bev) {
  struct proxyy* output = malloc(sizeof(struct proxyy));
  bzero(output, sizeof(struct proxyy));
  output->listener = listener;
  output->client = bev;
  return output;
};

void free_proxy(struct proxyy* proxy) {
  free(proxy);
};

void disconnect_after_write(struct bufferevent* bev, void* context) {
  struct evbuffer* output = bufferevent_get_output(bev);
  if (evbuffer_get_length(output) == 0)
    bufferevent_free(bev);
};

void preproxy_readcb(struct bufferevent* bev, void* context) {
  struct proxyy* proxy = context;
  struct listener* listener = proxy->listener;
  struct evbuffer* input = bufferevent_get_input(bev);
  size_t length = evbuffer_get_length(input);
  unsigned char buffer[length + 1];
  if (evbuffer_copyout(input, buffer, length) == length) {
    buffer[length] = '\0';
    if (buffer[0] == 0x02) {
      size_t i;
      for (i = 5; i < length; i++) {
        if (i % 2 == 1) {
          if (!isalnum(buffer[i]))
            break;
        } else if (buffer[i] != 0x00)
          goto disconnect;
      }
      char hostbuf[BUFSIZ];
      char *h = hostbuf;
      for (i += 2; i < length; i++) {
        if (i % 2 == 1) {
          if (isascii(buffer[i])) {
            *(h++) = buffer[i];
            if (buffer[i] == 0x00)
              break;
          }
        } else if (buffer[i] != 0x00)
          goto disconnect;
      }
      for (i = 0; listener->vhosts[i] != NULL; i++) {
        struct vhost* vhost = listener->vhosts[i];
        if (proxy->proxied_connection == NULL && strcmp(vhost->vhost, hostbuf) == 0) {
          struct event_base* base = bufferevent_get_base(bev);
          proxy->proxied_connection = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
          struct timeval timeout = { 10, 0 };
          bufferevent_set_timeouts(proxy->proxied_connection, &timeout, NULL);
          bufferevent_socket_connect(proxy->proxied_connection, (struct sockaddr*) vhost->address, sizeof(struct sockaddr_in));
          bufferevent_setcb(proxy->proxied_connection, proxied_conn_readcb, NULL, free_on_disconnect_eventcb, proxy);
          bufferevent_enable(proxy->proxied_connection, EV_READ);
          bufferevent_setcb(bev, proxy_readcb, NULL, free_on_disconnect_eventcb, proxy);
          struct evbuffer* proxied_output = bufferevent_get_output(proxy->proxied_connection);
          bufferevent_read_buffer(bev, proxied_output);
          break;
        }
      }
    }
  }
  return;
disconnect:
  DEBUG(255, "AUCH, someone did goto disconnect :(");
  free_on_disconnect_eventcb(bev, BEV_FINISHED, context);
};

void proxy_readcb(struct bufferevent* bev, void* context) {
  struct proxyy* proxy = context;
  struct evbuffer* proxied_output = bufferevent_get_output(proxy->proxied_connection);
  bufferevent_read_buffer(bev, proxied_output);
};

void proxied_conn_readcb(struct bufferevent* bev, void* context) {
  struct proxyy* proxy = context;
  struct evbuffer* client_output = bufferevent_get_output(proxy->client);
  bufferevent_read_buffer(bev, client_output);
};

void free_on_disconnect_eventcb(struct bufferevent* bev, short events, void* context) {
  if (!(events & BEV_EVENT_CONNECTED)) {
    if (context) {
      struct proxyy* proxy = context;
      struct bufferevent* to_disconnect_later = NULL;
      if (proxy->client == bev)
        to_disconnect_later = proxy->proxied_connection;
      else if (proxy->proxied_connection == bev)
        to_disconnect_later = proxy->client;
      bufferevent_free(bev);
      free_proxy(proxy);
      if (to_disconnect_later) {
        struct evbuffer* output = bufferevent_get_output(to_disconnect_later);
        if (evbuffer_get_length(output) == 0)
          bufferevent_free(to_disconnect_later);
        else {
          bufferevent_setcb(to_disconnect_later, NULL, disconnect_after_write, free_on_disconnect_eventcb, NULL);
          bufferevent_enable(to_disconnect_later, EV_WRITE);
        }
      }
    }
  }
};