// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>
#include <lcom/lab4.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "mouse.h"
#include "timer.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

static int cleanup_mouse_test(bool mouse_enabled, bool timer_subscribed, bool mouse_subscribed) {
  int ret = 0;

  if (mouse_enabled) {
    if (mouse_disable_data_reporting() != 0) {
      printf("Failed to disable data reporting on cleanup\n");
      ret = 1;
    }
  }

  if (timer_subscribed) {
    if (timer_unsubscribe_int() != 0) {
      printf("Failed to unsubscribe timer interrupts\n");
      ret = 1;
    }
  }

  if (mouse_subscribed) {
    if (mouse_unsubscribe_int() != 0) {
      printf("Failed to unsubscribe mouse interrupts\n");
      ret = 1;
    }
  }

  return ret;
}

int (mouse_test_packet)(uint32_t cnt) {
    int ipc_status;
    message msg;
    uint8_t mouse_bit_no;
    uint32_t packets_read = 0;
    struct packet pp;
    
    bool mouse_subscribed = false;
    bool mouse_enabled = false;

    mouse_reset_packet_state();

    if (mouse_subscribe_int(&mouse_bit_no) != 0) {
        printf("Failed to subscribe mouse interrupts\n");
        return 1;
    }
    mouse_subscribed = true;

    if (mouse_disable_data_reporting() != 0) {
        printf("Failed to disable data reporting before start\n");
        return cleanup_mouse_test(false, false, mouse_subscribed);
    }

    if (mouse_enable_data_reporting_cmd() != 0) {
        printf("Failed to enable data reporting\n");
        return cleanup_mouse_test(false, false, mouse_subscribed);
    }
    mouse_enabled = true;

    uint32_t mouse_irq_set = BIT(mouse_bit_no);

    while (packets_read < cnt) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0){
            continue;
        }

        if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) {
            if (msg.m_notify.interrupts & mouse_irq_set) {
                mouse_ih();

                if (!mouse_get_error()) {
                  mouse_sync_packet(mouse_get_byte());

                  if (mouse_packet_ready()) {
                      if (mouse_get_packet(&pp) == 0) {
                      mouse_print_packet(&pp);
                      packets_read++;
                      }
                  }
                }
            }
        }
    }

    return cleanup_mouse_test(mouse_enabled, false, mouse_subscribed);
}

int (mouse_test_async)(uint8_t idle_time) {
    int ipc_status;
    message msg;
    uint8_t mouse_bit_no, timer_bit_no;
    struct packet pp;

    bool mouse_subscribed = false;
    bool timer_subscribed = false;
    bool mouse_enabled = false;

    mouse_reset_packet_state();

    uint32_t sys_freq = sys_hz();
    uint32_t timer_ticks_count = 0;
    uint32_t max_ticks = idle_time * sys_freq;

    if (mouse_subscribe_int(&mouse_bit_no) != 0) {
        printf("Failed to subscribe mouse interrupts\n");
        return 1;
    }
    mouse_subscribed = true;

    if (timer_subscribe_int(&timer_bit_no) != 0) {
        printf("Failed to subscribe timer interrupts\n");
        return cleanup_mouse_test(false, false, mouse_subscribed);
    }
    timer_subscribed = true;

    if (mouse_disable_data_reporting() != 0) {
        printf("Failed to disable data reporting before start\n");
        return cleanup_mouse_test(false, timer_subscribed, mouse_subscribed);
    }

    if (mouse_enable_data_reporting_cmd() != 0) {
        printf("Failed to enable data reporting\n");
        return cleanup_mouse_test(false, timer_subscribed, mouse_subscribed);
    }
    mouse_enabled = true;

    uint32_t mouse_irq_set = BIT(mouse_bit_no);
    uint32_t timer_irq_set = BIT(timer_bit_no);

    while (timer_ticks_count < max_ticks) {
        bool packet_received = false;

        if (driver_receive(ANY, &msg, &ipc_status) != 0){
          continue;
        }

        if (is_ipc_notify(ipc_status) && _ENDPOINT_P(msg.m_source) == HARDWARE) {

          if (msg.m_notify.interrupts & mouse_irq_set) {
              mouse_ih();

              if (!mouse_get_error()) {
                mouse_sync_packet(mouse_get_byte());

                if (mouse_packet_ready()) {
                    if (mouse_get_packet(&pp) == 0) {
                      mouse_print_packet(&pp);
                      packet_received = true;
                    }
                }
              }
          }

          if (msg.m_notify.interrupts & timer_irq_set) {
              timer_int_handler();
              timer_ticks_count++;
          }

          if (packet_received) {
              timer_ticks_count = 0;
          }
        }
    }

    return cleanup_mouse_test(mouse_enabled, timer_subscribed, mouse_subscribed);
}
