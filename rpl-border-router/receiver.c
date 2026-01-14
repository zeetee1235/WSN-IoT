/* receiver.c - Contiki-NG Cooja mote UDP receiver/logger
 *
 * Listens on UDP port 8765 and prints CSV logs:
 *   RX,<src_ip>,<src_port>,<seq>,<t_send>,<t_recv>,<delay_ticks>,<len>
 *
 * Works with sensor.c payload format: "seq=<n> t=<clock>"
 */

#include "contiki.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uiplib.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/routing.h"
#include "sys/log.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define LOG_MODULE "RECV"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_PORT 8765

/* Small per-sender state (best-effort) */
#define MAX_SENDERS 32

typedef struct {
  uip_ipaddr_t ip;
  uint32_t last_seq;
  uint8_t used;
} sender_state_t;

static sender_state_t senders[MAX_SENDERS];

static struct simple_udp_connection udp_conn;

static int
parse_payload(const uint8_t *data, uint16_t len, uint32_t *seq_out, uint32_t *t_out)
{
  /* Payload expected: "seq=<num> t=<num>" */
  char buf[96];
  if(len >= sizeof(buf)) {
    len = sizeof(buf) - 1;
  }
  memcpy(buf, data, len);
  buf[len] = '\0';

  unsigned long seq = 0, t = 0;
  int matched = sscanf(buf, "seq=%lu t=%lu", &seq, &t);
  if(matched == 2) {
    *seq_out = (uint32_t)seq;
    *t_out   = (uint32_t)t;
    return 1;
  }
  return 0;
}

static sender_state_t *
get_sender_state(const uip_ipaddr_t *ip)
{
  /* Try find existing */
  for(int i = 0; i < MAX_SENDERS; i++) {
    if(senders[i].used && uip_ipaddr_cmp(&senders[i].ip, ip)) {
      return &senders[i];
    }
  }
  /* Allocate new */
  for(int i = 0; i < MAX_SENDERS; i++) {
    if(!senders[i].used) {
      senders[i].used = 1;
      uip_ipaddr_copy(&senders[i].ip, ip);
      senders[i].last_seq = 0;
      return &senders[i];
    }
  }
  return NULL;
}

static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
  (void)c; (void)receiver_addr; (void)receiver_port;

  uint32_t seq = 0, t_send = 0;
  uint32_t t_recv = (uint32_t)clock_time();

  int ok = parse_payload(data, datalen, &seq, &t_send);
  uint32_t delay = ok ? (t_recv - t_send) : 0;

  /* Track loss estimate per sender (best-effort) */
  uint32_t gap = 0;
  sender_state_t *st = get_sender_state(sender_addr);
  if(st != NULL && ok) {
    if(seq > st->last_seq + 1) {
      gap = seq - (st->last_seq + 1);
    }
    st->last_seq = seq;
  }

  /* Print CSV header once (optional) */
  /* You can comment this out after first run */
  static uint8_t printed_header = 0;
  if(!printed_header) {
    printed_header = 1;
    printf("CSV,tag,src_ip,src_port,seq,t_send,t_recv,delay_ticks,len,gap\n");
  }

  /* CSV row */
  printf("CSV,RX,");
  /* src_ip */
  uiplib_ipaddr_print(sender_addr);
  printf(",%u,", sender_port);

  if(ok) {
    printf("%lu,%lu,%lu,%lu,%u,%lu\n",
           (unsigned long)seq,
           (unsigned long)t_send,
           (unsigned long)t_recv,
           (unsigned long)delay,
           (unsigned)datalen,
           (unsigned long)gap);
  } else {
    /* If payload doesn't match expected format */
    printf("NA,NA,%lu,0,%u,0\n",
           (unsigned long)t_recv,
           (unsigned)datalen);
  }
}

PROCESS(receiver_process, "UDP receiver/logger");
AUTOSTART_PROCESSES(&receiver_process);

PROCESS_THREAD(receiver_process, ev, data)
{
  (void)ev; (void)data;

  PROCESS_BEGIN();

  LOG_INFO("boot\n");

  /* Register UDP listener */
  simple_udp_register(&udp_conn, UDP_PORT,
                      NULL, UDP_PORT,
                      udp_rx_callback);

  /* Wait until we are part of a routing topology (optional but nice) */
  LOG_INFO("waiting for routing...\n");
  while(!NETSTACK_ROUTING.node_is_reachable()) {
    static struct etimer et;
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  LOG_INFO("routing reachable\n");

  /* Print my first preferred global IP if any */
  for(int i = 0; i < UIP_DS6_ADDR_NB; i++) {
    if(uip_ds6_if.addr_list[i].isused &&
       uip_ds6_if.addr_list[i].state == ADDR_PREFERRED &&
       !uip_is_addr_linklocal(&uip_ds6_if.addr_list[i].ipaddr)) {
      LOG_INFO("my ip = ");
      LOG_INFO_6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      LOG_INFO_("\n");
      break;
    }
  }

  while(1) {
    PROCESS_YIELD();
  }

  PROCESS_END();
}

