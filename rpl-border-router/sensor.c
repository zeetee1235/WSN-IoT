/*
 * sensor.c
 * - RPL node that periodically sends UDP packets to root (aaaa::1)
 * - Payload: "seq=<n> t=<clock>"
 */

#include "contiki.h"
#include "sys/log.h"

#include "net/ipv6/uip.h"
#include "net/ipv6/uiplib.h"
#include "net/ipv6/uip-ds6.h"
#include "net/routing/routing.h"
#include "net/ipv6/simple-udp.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define LOG_MODULE "SENSOR"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_REMOTE_PORT 8765
#define UDP_LOCAL_PORT  8765
#define SEND_INTERVAL (5 * CLOCK_SECOND)

static const char *ROOT_IP_STR = "aaaa::1";

static struct simple_udp_connection udp_conn;
static uint32_t seq = 0;

PROCESS(sensor_process, "UDP sensor sender");
AUTOSTART_PROCESSES(&sensor_process);

PROCESS_THREAD(sensor_process, ev, data)
{
  static struct etimer periodic;
  static uip_ipaddr_t root_ip;

  PROCESS_BEGIN();

  LOG_INFO("boot\n");

  simple_udp_register(&udp_conn, UDP_LOCAL_PORT,
                      NULL, UDP_REMOTE_PORT,
                      NULL);

  if(!uiplib_ipaddrconv(ROOT_IP_STR, &root_ip)) {
    LOG_ERR("bad root ip string: %s\n", ROOT_IP_STR);
    PROCESS_EXIT();
  }

  /* Wait until routing says we can reach the root/DAG */
  LOG_INFO("waiting for RPL route...\n");
  while(!NETSTACK_ROUTING.node_is_reachable()) {
    static struct etimer et;
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  LOG_INFO("RPL reachable\n");

  etimer_set(&periodic, SEND_INTERVAL);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic));

    char msg[64];
    uint32_t now = (uint32_t)clock_time();

    int n = snprintf(msg, sizeof(msg), "seq=%lu t=%lu",
                     (unsigned long)seq,
                     (unsigned long)now);

    if(n > 0) {
      simple_udp_sendto(&udp_conn, msg, (size_t)n, &root_ip);
      LOG_INFO("tx to ");
      LOG_INFO_6ADDR(&root_ip);
      LOG_INFO_(":%u  %s\n", UDP_REMOTE_PORT, msg);
      seq++;
    }

    etimer_reset(&periodic);
  }

  PROCESS_END();
}

