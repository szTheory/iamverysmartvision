/* SPDX-License-Identifier: MIT */
/* Flat test bus; behaviour is described in ivs_testbus.h. */
#include "ivs_testbus.h"

#include <string.h>

void ivs_testbus_init(ivs_testbus *b)
{
  memset(b, 0, sizeof *b);
}

void ivs_testbus_poke(ivs_testbus *b, uint16_t addr, uint16_t word)
{
  b->mem[addr] = word;
  b->valid[addr] = 1u;
}

int ivs_testbus_add_wait(ivs_testbus *b, uint16_t addr, uint16_t wait_cpu_cycles)
{
  if (b->n_waits >= IVS_TESTBUS_MAX_WAITS) {
    return -1;
  }
  b->wait_addr[b->n_waits] = addr;
  b->wait_cycles[b->n_waits] = wait_cpu_cycles;
  b->n_waits++;
  return 0;
}

void ivs_testbus_set_iab(ivs_testbus *b, uint16_t value)
{
  b->has_iab = 1u;
  b->iab = value;
}

void ivs_testbus_set_ebci(ivs_testbus *b, uint8_t e)
{
  b->ebci_true[e & 0x0Fu] = 1u;
}

static uint16_t ivs_testbus_read(ivs_testbus *b, uint16_t addr)
{
  if (!b->valid[addr]) {
    if (b->uninit_reads == 0u) {
      b->first_uninit_addr = addr;
    }
    b->uninit_reads++;
  }
  return b->mem[addr];
}

ivs_bus_reply ivs_testbus_bus(void *ctx, const ivs_cp1610_access *a)
{
  ivs_testbus *b = (ivs_testbus *)ctx;
  ivs_bus_reply reply;
  size_t i;
  reply.data = 0u;
  reply.wait_cpu_cycles = 0u;
  if (b->n_calls < IVS_TESTBUS_CALL_CAP) {
    b->calls[b->n_calls++] = *a;
  } else {
    b->n_dropped++;
  }
  for (i = 0; i < b->n_waits; i++) {
    if (b->wait_addr[i] == a->addr) {
      reply.wait_cpu_cycles = b->wait_cycles[i];
      break;
    }
  }
  switch (a->kind) {
  case IVS_ACC_FETCH:
  case IVS_ACC_READ:
  case IVS_ACC_ADAR:
    reply.data = ivs_testbus_read(b, a->addr);
    break;
  case IVS_ACC_WRITE:
  case IVS_ACC_DW:
    ivs_testbus_poke(b, a->addr, a->data);
    break;
  case IVS_ACC_IAB:
    if (b->has_iab) {
      reply.data = b->iab;
    } else {
      b->unexpected_iab++;
    }
    break;
  default:
    break;
  }
  return reply;
}

uint8_t ivs_testbus_ebci(void *ctx, uint8_t e)
{
  const ivs_testbus *b = (const ivs_testbus *)ctx;
  return b->ebci_true[e & 0x0Fu];
}

void ivs_testbus_port(ivs_testbus *b, ivs_cp1610_port *p, int with_ebci)
{
  p->bus = ivs_testbus_bus;
  p->ebci = with_ebci ? ivs_testbus_ebci : NULL;
  p->ctx = b;
}
