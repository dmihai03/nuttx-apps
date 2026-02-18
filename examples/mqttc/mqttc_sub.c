/****************************************************************************
* apps/examples/mqttc/mqttc_sub.c
*
* SPDX-License-Identifier: Apache-2.0
*
* Licensed to the Apache Software Foundation (ASF) under one or more
* contributor license agreements.  See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.  The
* ASF licenses this file to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance with the
* License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
* License for the specific language governing permissions and limitations
* under the License.
*
****************************************************************************/

/****************************************************************************
* Included Files
****************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <mqtt.h>

#define EXTRA_OPT ""

/****************************************************************************
* Private Types
****************************************************************************/

struct mqttc_cfg_s
{
    FAR const char *host;
    FAR const char *port;
    FAR const char *topic;
    FAR const char *msg;
    FAR const char *id;
    FAR const char *user;
    FAR const char *pass;
    uint32_t tmo;
    uint8_t flags;
    uint8_t qos;
};

struct mqtt_conn_context_s
{
    struct mqtt_client client;
    uint8_t sendbuf[CONFIG_EXAMPLES_MQTTC_TXSIZE];
    uint8_t recvbuf[CONFIG_EXAMPLES_MQTTC_RXSIZE];
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static FAR void *client_refresher(FAR void *data);
static void parsearg(int argc, FAR char *argv[], FAR struct mqttc_cfg_s *cfg,
                     FAR int *n);
static int init_conn(FAR const struct mqttc_cfg_s *cfg,
                     FAR struct mqtt_conn_context_s *ctx,
                     FAR mqtt_pal_socket_handle *handle);
static void close_conn(FAR struct mqtt_conn_context_s *ctx);

/****************************************************************************
 * Private Functions
 ***************************************************************************/

/****************************************************************************
 * Name: client_refresher
 *
 * Description:
 *   The client's refresher. This function triggers back-end routines to
 *   handle ingress/egress traffic to the broker.
 *
 ****************************************************************************/

static FAR void *client_refresher(FAR void *data)
{
  while (1)
    {
      mqtt_sync((FAR struct mqtt_client *)data);
      usleep(100000U);
    }

  return NULL;
}

/****************************************************************************
 * Name: parsearg
 *
 * Description:
 *   Parse command line arguments.
 *
 ****************************************************************************/

static void parsearg(int argc, FAR char *argv[],
                     FAR struct mqttc_cfg_s *cfg, FAR int *n)
{
  int opt;

  while ((opt = getopt(argc, argv, "h:p:m:t:n:q:" EXTRA_OPT)) != ERROR)
    {
      switch (opt)
        {
          case 'h':
            cfg->host = optarg;
            break;

          case 'p':
            cfg->port = optarg;
            break;

          case 'm':
            cfg->msg = optarg;
            break;

          case 't':
            cfg->topic = optarg;
            break;

          case 'n':
            *n = strtol(optarg, NULL, 10);
            break;

          case 'q':
            switch (strtol(optarg, NULL, 10))
              {
                case '0':
                  cfg->qos = MQTT_PUBLISH_QOS_0;
                  break;
                case '1':
                  cfg->qos = MQTT_PUBLISH_QOS_1;
                  break;
                case '2':
                  cfg->qos = MQTT_PUBLISH_QOS_2;
                  break;
                }
            break;

          default:
            fprintf(stderr, "ERROR: Unrecognized option\n");
            break;
        }
    }
}

/****************************************************************************
 * Name: init_conn
 *
 * Description:
 *   Resolve server's name and try to establish a connection.
 *
 ****************************************************************************/

static int init_conn(FAR const struct mqttc_cfg_s *cfg,
                     FAR struct mqtt_conn_context_s *conn,
                     FAR mqtt_pal_socket_handle *socketfd)
{
  struct addrinfo hints;
  FAR struct addrinfo *servinfo;
  FAR struct addrinfo *itr;
  int fd;
  int ret;

  printf("Connecting to %s:%s...\n", cfg->host, cfg->port);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family  = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  ret = getaddrinfo(cfg->host, cfg->port, &hints, &servinfo);
  if (ret != 0)
    {
      printf("ERROR! getaddrinfo() failed: %s\n", gai_strerror(ret));
      return -1;
    }

  itr = servinfo;
  do
    {
      fd = socket(itr->ai_family, itr->ai_socktype, itr->ai_protocol);
      if (fd < 0)
        {
          continue;
        }

      ret = connect(fd, itr->ai_addr, itr->ai_addrlen);
      if (ret == 0)
        {
          break;
        }

      close(fd);
      fd = -1;
    }
  while ((itr = itr->ai_next) != NULL);

  freeaddrinfo(servinfo);

  if (fd < 0)
    {
      printf("ERROR! Couldn't create socket\n");
      return -1;
    }

  ret = fcntl(fd, F_GETFL, 0);
  if (ret < 0)
    {
      close(fd);
      printf("ERROR! fcntl() F_GETFL failed, errno: %d\n", errno);
      return -1;
    }

  ret = fcntl(fd, F_SETFL, ret | O_NONBLOCK);
  if (ret < 0)
    {
      close(fd);
      printf("ERROR! fcntl() F_SETFL failed, errno: %d\n", errno);
      return -1;
    }

  *socketfd = fd;

  return 0;
}

/****************************************************************************
 * Name: close_conn
 *
 * Description:
 *   Shut down connection to server established by init_conn.
 *
 ****************************************************************************/

static void close_conn(FAR struct mqtt_conn_context_s *conn)
{
  close(conn->client.socketfd);
}


/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
    return 0;
}
