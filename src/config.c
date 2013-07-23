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

#include "config.h"
#include "listener.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

static struct config {
  struct listener** listeners;
} global_config;

struct config* config = &global_config;

struct listener* new_listener(char* address) {
  struct listener* output = malloc(sizeof(struct listener));
  if (!output)
    return NULL;
  bzero(output, sizeof(struct listener));
  output->address = malloc(sizeof(struct sockaddr_in));
  if (!output->address) {
    free(output);
    return NULL;
  }
  memset(output->address, 0, sizeof(struct sockaddr_in));
  char ip[BUFSIZ];
  in_port_t port;
  if (sscanf(address, "%[0-9.]:%hd", ip, &port) == 2) {
    output->address->sin_family = AF_INET;
    output->address->sin_port = htons(port);
    inet_pton(AF_INET, ip, &output->address->sin_addr);
  } else if (sscanf(address, "%hd", &port) == 1) {
    output->address->sin_family = AF_INET;
    output->address->sin_port = htons(port);
  } else {
    free(output->address);
    free(output);
    return NULL;
  }
  return output;
};

struct vhost* new_vhost(char* vhost) {
  struct vhost* output = malloc(sizeof(struct vhost));
  bzero(output, sizeof(struct vhost));
  output->vhost = strdup(vhost);
  output->address = malloc(sizeof(struct sockaddr_in));
  return output;
};

int fill_in_vhost_address(struct vhost* vhost, char* address) {
  char ip[BUFSIZ];
  in_port_t port;
  if (sscanf(address, "%[0-9.]:%hd", ip, &port) == 2) {
    vhost->address->sin_family = AF_INET;
    vhost->address->sin_port = htons(port);
    inet_pton(AF_INET, ip, &vhost->address->sin_addr);
  } else if (sscanf(address, "%hd", &port) == 1) {
    vhost->address->sin_family = AF_INET;
    vhost->address->sin_port = htons(port);
  } else
    return 0;
  return 1;
};

int parse_config(char* filename) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Error '%s' while opening '%s'.\n", strerror(errno), filename);
    return 0;
  }
  bzero(config, sizeof(struct config));
  char linebuffer[BUFSIZ];
  unsigned int line_count = 0;
  while (fgets(linebuffer, sizeof(linebuffer), f)) {
    line_count++;
    if (linebuffer[0] == '#' || linebuffer[1] == 1)
      continue;
    char key[BUFSIZ];
    char value[BUFSIZ];
    struct listener* listener = NULL;
    struct vhost* vhost = NULL;
    if (sscanf(linebuffer, "%[a-z_] = %[^\t\n]", key, value) == 2) {
      if (strcmp(key, "listener") == 0) {
        if (config->listeners == NULL) {
          listener = new_listener(value);
          if (listener) {
            config->listeners = malloc(sizeof(struct listener) * 2);
            bzero(config->listeners, sizeof(struct listener) * 2);
            config->listeners[0] = listener;
          } else {
            fprintf(stderr, "%s is not valid.", value);
            return 0;
          }
        } else {
          listener = new_listener(value);
          if (listener) {
            size_t i = 0;
            while (config->listeners[++i]);
            config->listeners = realloc(config->listeners, sizeof(struct listener) * (i + 2));
            config->listeners[i] = listener;
            config->listeners[++i] = NULL;
          } else {
            fprintf(stderr, "%s is not valid.", value);
            return 0;
          }
        }
      } else if (listener && strcmp(key, "vhost") == 0) {
        vhost = new_vhost(value);
        if (listener->vhosts == NULL) {
          listener->vhosts = malloc(sizeof(struct vhost) * 2);
          bzero(listener->vhosts, sizeof(struct vhost) * 2);
          listener->vhosts[0] = vhost;
        } else {
          size_t i = 0;
          while (listener->vhosts[++i]);
          listener->vhosts = realloc(listener->vhosts, sizeof(struct vhost) * (i + 2));
          listener->vhosts[i] = vhost;
          listener->vhosts[++i] = NULL;
        }
      } else if (vhost && strcmp(key, "internaladdress") == 0) {
        if (fill_in_vhost_address(vhost, value) == 0) {
          fprintf(stderr, "%s is not valid.", value);
          return 0;
        }
      }
    }
  }
  return line_count;
};

int dispatch_config(struct event_base* base) {
  size_t i;
  for (i = 0; config->listeners[i]; i++) {
    int ret = init_listener(base, config->listeners[i]);
    if (ret != 0)
      return ret;
  }
  return 0;
};