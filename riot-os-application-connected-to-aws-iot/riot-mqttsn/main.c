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
static char topics[NUMOFSUBS][TOPIC_MAXLEN];

static void *emcute_thread(void *arg)
{
    (void)arg;
    emcute_run(CONFIG_EMCUTE_DEFAULT_PORT, EMCUTE_ID);
    return NULL; /* should never be reached */
}

static void on_pub(const emcute_topic_t *topic, void *data, size_t len)
{
    char *in = (char *)data;

    printf("### got publication for topic '%s' [%i] ###\n",
           topic->name, (int)topic->id);
    for (size_t i = 0; i < len; i++)
    {
        printf("%c", in[i]);
    }
    puts("");
}

static unsigned get_qos(const char *str)
{
    int qos = atoi(str);
    switch (qos)
    {
    case 1:
        return EMCUTE_QOS_1;
    case 2:
        return EMCUTE_QOS_2;
    default:
        return EMCUTE_QOS_0;
    }
}

static int cmd_con(int argc, char **argv)
{
    printf("argc %d\n", argc);
    sock_udp_ep_t gw = {.family = AF_INET6, .port = CONFIG_EMCUTE_DEFAULT_PORT};
    char *topic = NULL;
    char *message = NULL;
    size_t len = 0;

    // if (argc < 2)
    //{
    //    printf("usage: %s <ipv6 addr> [port] [<will topic> <will message>]\n",
    //          argv[0]);
    //  return 1;
    //}

    /* parse address */
    if (ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, argv[1]) == NULL)
    {
        printf("error parsing IPv6 address\n");
        return 1;
    }

    // if (argc >= 3)
    //{
    //    gw.port = atoi(argv[2]);
    //}
    // if (argc >= 5)
    //{
    // topic = argv[3];
    // message = argv[4];
    //  puts("message");
    //    puts(argv[4]);
    //      len = strlen(message);
    //  }

    if (emcute_con(&gw, true, topic, message, len, 0) != EMCUTE_OK)
    {
        printf("error: unable to connect to [%s]:%i\n", argv[1], (int)gw.port);
        return 1;
    }
    printf("Successfully connected to gateway at [%s]:%i\n",
           argv[1], (int)gw.port);

    return 0;
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

static int cmd_discon(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int res = emcute_discon();
    if (res == EMCUTE_NOGW)
    {
        puts("error: not connected to any broker");
        return 1;
    }
    else if (res != EMCUTE_OK)
    {
        puts("error: unable to disconnect");
        return 1;
    }
    puts("Disconnect successful");
    return 0;
}

static int cmd_pub(int argc, char **argv)
{
    emcute_topic_t t;
    unsigned flags = EMCUTE_QOS_0;

    if (argc < 3)
    {
        printf("usage: %s <topic name> <data> [QoS level]\n", argv[0]);
        return 1;
    }

    /* parse QoS level */
    if (argc >= 4)
    {
        flags |= get_qos(argv[3]);
    }

    printf("pub with topic: %s and name %s and flags 0x%02x\n", argv[1], argv[2], (int)flags);

    /* step 1: get topic id */
    t.name = argv[1];
    if (emcute_reg(&t) != EMCUTE_OK)
    {
        puts("error: unable to obtain topic ID");
        return 1;
    }

    /* step 2: publish data */
    if (emcute_pub(&t, argv[2], strlen(argv[2]), flags) != EMCUTE_OK)
    {
        printf("error: unable to publish data to topic '%s [%i]'\n",
               t.name, (int)t.id);
        return 1;
    }

    printf("Published %i bytes to topic '%s [%i]'\n",
           (int)strlen(argv[2]), t.name, t.id);

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


/*static int pub_message(void)
{
    emcute_topic_t t;
    unsigned flags = EMCUTE_QOS_0;

    printf("pub with topic: %s and name %s and flags 0x%02x\n", "sensor/values", "test", (int)flags);

    //step 1: get topic id
    t.name = "sensor/values";
    if (emcute_reg(&t) != EMCUTE_OK)
    {
        puts("error: unable to obtain topic ID");
        return 1;
    }

    // step 2: publish data
    if (emcute_pub(&t, "test", strlen("test"), flags) != EMCUTE_OK)
    {
        printf("error: unable to publish data to topic '%s [%i]'\n",
               t.name, (int)t.id);
        return 1;
    }

    printf("Published %i bytes to topic '%s [%i]'\n",
           (int)strlen("test"), t.name, t.id);

    return 0;
}
*/
static int cmd_sub(int argc, char **argv)
{
    unsigned flags = EMCUTE_QOS_0;

    if (argc < 2)
    {
        printf("usage: %s <topic name> [QoS level]\n", argv[0]);
        return 1;
    }

    if (strlen(argv[1]) > TOPIC_MAXLEN)
    {
        puts("error: topic name exceeds maximum possible size");
        return 1;
    }
    if (argc >= 3)
    {
        flags |= get_qos(argv[2]);
    }

    /* find empty subscription slot */
    unsigned i = 0;
    for (; (i < NUMOFSUBS) && (subscriptions[i].topic.id != 0); i++)
    {
    }
    if (i == NUMOFSUBS)
    {
        puts("error: no memory to store new subscriptions");
        return 1;
    }

    subscriptions[i].cb = on_pub;
    strcpy(topics[i], argv[1]);
    subscriptions[i].topic.name = topics[i];
    if (emcute_sub(&subscriptions[i], flags) != EMCUTE_OK)
    {
        printf("error: unable to subscribe to %s\n", argv[1]);
        return 1;
    }

    printf("Now subscribed to %s\n", argv[1]);
    return 0;
}

static int cmd_unsub(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage %s <topic name>\n", argv[0]);
        return 1;
    }

    /* find subscriptions entry */
    for (unsigned i = 0; i < NUMOFSUBS; i++)
    {
        if (subscriptions[i].topic.name &&
            (strcmp(subscriptions[i].topic.name, argv[1]) == 0))
        {
            if (emcute_unsub(&subscriptions[i]) == EMCUTE_OK)
            {
                memset(&subscriptions[i], 0, sizeof(emcute_sub_t));
                printf("Unsubscribed from '%s'\n", argv[1]);
            }
            else
            {
                printf("Unsubscription form '%s' failed\n", argv[1]);
            }
            return 0;
        }
    }

    printf("error: no subscription for topic '%s' found\n", argv[1]);
    return 1;
}

static int cmd_will(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("usage %s <will topic name> <will message content>\n", argv[0]);
        return 1;
    }

    if (emcute_willupd_topic(argv[1], 0) != EMCUTE_OK)
    {
        puts("error: unable to update the last will topic");
        return 1;
    }
    if (emcute_willupd_msg(argv[2], strlen(argv[2])) != EMCUTE_OK)
    {
        puts("error: unable to update the last will message");
        return 1;
    }

    puts("Successfully updated last will topic and message");
    return 0;
}

static const shell_command_t shell_commands[] = {
    {"con", "connect to MQTT broker", cmd_con},
    {"discon", "disconnect from the current broker", cmd_discon},
    {"pub", "publish something", cmd_pub},
    {"sub", "subscribe topic", cmd_sub},
    {"unsub", "unsubscribe from topic", cmd_unsub},
    {"will", "register a last will", cmd_will},
    {NULL, NULL, NULL}};

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
    puts("MQTT-SN example application\n");
    puts("Type 'help' to get started. Have a look at the README.md for more"
         "information.");

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

    /* start shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
