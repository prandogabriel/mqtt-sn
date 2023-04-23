/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application for demonstrating RIOT's MQTT-SN library
 *              emCute
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "shell.h"
#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"
#include "thread.h"
#include "net/netif.h"
#include "ztimer.h"


#ifndef EMCUTE_ID
#define EMCUTE_ID ("gertrud")
#endif
#define EMCUTE_PRIO (THREAD_PRIORITY_MAIN - 1)

#define NUMOFSUBS (16U)
#define TOPIC_MAXLEN (64U)

// struct that contains sensors
typedef struct sensors
{
    int temperature;
    int humidity;
    int windDirection;
    int windIntensity;
    int rainHeight;
} t_sensors;

static char stack[THREAD_STACKSIZE_DEFAULT];
static msg_t queue[8];

static emcute_sub_t subscriptions[NUMOFSUBS];

static void *emcute_thread(void *arg)
{
    (void)arg;
    emcute_run(CONFIG_EMCUTE_DEFAULT_PORT, EMCUTE_ID);
    return NULL; /* should never be reached */
}


static int connect_to_gateway(void)
{
    sock_udp_ep_t gw = {.family = AF_INET6, .port = CONFIG_EMCUTE_DEFAULT_PORT};
    char *topic = NULL;
    char *message = NULL;
    size_t len = 0;
    char *addr_ipv6 = "fec0:affe::1";

    if (ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, addr_ipv6) == NULL)
    {
        printf("error parsing IPv6 address\n");
        return 1;
    }

    printf("antes \n");
    if (emcute_con(&gw, true, topic, message, len, 0) != EMCUTE_OK)
    {
        printf("error: unable to connect to [%s]:%i\n", addr_ipv6, (int)gw.port);
        return 1;
    }
    printf("Successfully connected to gateway at [%s]:%i\n",
           addr_ipv6, (int)gw.port);

    return 0;
}

static int pub_message(char* topic, char* data, int qos){
  emcute_topic_t t;
  unsigned flags = EMCUTE_QOS_0;

  switch (qos) {
      case 1:
        flags |= EMCUTE_QOS_1;
        break;
      case 2:
        flags |= EMCUTE_QOS_2;
        break;
      default:
        flags |= EMCUTE_QOS_0;
        break;
  }

 // step 1: get topic id
  t.name = topic;
  if (emcute_reg(&t) != EMCUTE_OK) {
      puts("error: unable to obtain topic ID");
      return 1;
  }

 // step 2: publish data
  if (emcute_pub(&t, data, strlen(data), flags) != EMCUTE_OK) {
      printf("error: unable to publish data to topic '%s [%i]'\n",
              t.name, (int)t.id);
      return 1;
  }

  printf("published %s on topic %s\n", data, topic);

  return 0;
}

static void setup_ipv6_addr_add(void)
{

    /* Defina o endereço IPv6 que você deseja adicionar */
    ipv6_addr_t my_ipv6_addr;
    ipv6_addr_from_str(&my_ipv6_addr, "fec0:affe::99");

    /* Obtenha a interface de rede */
    gnrc_netif_t *netif = gnrc_netif_get_by_pid(5);

    /* Adicione o endereço IPv6 à interface de rede */
    gnrc_netif_ipv6_addr_add(netif, &my_ipv6_addr, 64, 0);

    puts("success: added fec0:affe::99/64 to interface 5\n");
}

int rand_val(int min, int max)
{
    srand(time(NULL));
    return (rand() % (int)((max - min + 1) * 100)) / 100 + min;
}

static void gen_sensors_values(t_sensors *sensors)
{
    sensors->temperature = rand_val(-50, 50);
    sensors->humidity = rand_val(0, 100);
    sensors->windDirection = rand_val(0, 360);
    sensors->windIntensity = rand_val(0, 100);
    sensors->rainHeight = rand_val(0, 50);
}

static int start(void)
{
    // sensors struct
    t_sensors sensors;
    // name of the topic
    char *topic = "sensor/values";

    // json that it will published
    char json[128];

    while (1)
    {
        // updates sensor values
        gen_sensors_values(&sensors);

        // fills the json document
        sprintf(json, "{\"id\": \"1\",  \"temperature\": "
                      "\"%d\", \"humidity\": \"%d\", \"windDirection\": \"%d\", "
                      "\"windIntensity\": \"%d\", \"rainHeight\": \"%d\"}",
                 sensors.temperature, sensors.humidity,
                sensors.windDirection, sensors.windIntensity, sensors.rainHeight);

        // publish to the topic
        pub_message(topic, json, 0);

        // it sleeps for five seconds
        ztimer_sleep(ZTIMER_MSEC, 5000);
    }

    return 0;
}

int main(void)
{
    puts("MQTT-SN application\n");

    /* the main thread needs a msg queue to be able to run `ping`*/
    msg_init_queue(queue, ARRAY_SIZE(queue));

    /* initialize our subscription buffers */
    memset(subscriptions, 0, (NUMOFSUBS * sizeof(emcute_sub_t)));

    /* start the emcute thread */
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0,
                  emcute_thread, NULL, "emcute");

    // configura interface de rede
    setup_ipv6_addr_add();

    // connect to gateway
    connect_to_gateway();
 
    start();

    /* should be never reached */
    return 0;
}
