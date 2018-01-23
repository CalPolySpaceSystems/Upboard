#include <uart.h>
#include <usart.h>
#include <hal/hal_uart.h>
#include <stdlib.h>
#include <assert.h>

/*
 The board has 1 UART and 3 USARTS
*/
#define UART_COUNT 4
#define USART_CLOCK_RATE 80000
#define TX_BUFFER_SIZE (8)

struct hal_uart {
    void * uart;
    uint8_t u_open;
    uint8_t tx_on;
    int16_t rxdata;
    uint8_t txdata[TX_BUFFER_SIZE];
    hal_uart_rx_char u_rx_func;
    hal_uart_tx_char u_tx_func;
    hal_uart_tx_done u_tx_done;
    void *u_func_arg;
    sam_usart_opt_t options;
};
typedef struct hal_uart hal_uart_t;
static hal_uart_t uarts[UART_COUNT] = {'\0'};

/* Internal helper function */
void *translate_port_to_uart(int port){
    switch (port){
        case 0:
            return USART0;
        break;
        case 1:
            return USART1;
        break;
        case 2:
            return USART2;
        break;
        case 3:
            return UART;
        break;
    }
    assert(false);
    return NULL;
}

int is_usart(void *uart){
    if (uart == UART){
        return 0;
    }
    return 1;
}

/**
 * Initialize the HAL uart.
 *
 * @param uart  The uart number to configure
 * @param cfg   Hardware specific uart configuration.  This is passed from BSP
 *              directly to the MCU specific driver.
 */
int hal_uart_init(int uart, void *cfg){
    /* Default config is console */
    if (uart >= UART_COUNT){
        return -1;
    }
    return 0;
}

int hal_uart_init_cbs(int uart, hal_uart_tx_char tx_func,
  hal_uart_tx_done tx_done, hal_uart_rx_char rx_func, void *arg);


int hal_usart_config(hal_uart_t *uart, int32_t speed, uint8_t databits, uint8_t stopbits,
                        enum hal_uart_parity parity, enum hal_uart_flow_ctl flow_ctl){
    uart->options.baudrate = speed;
    
    /* Set char length */
    switch (databits){
        case 5:
            uart->options.char_length = US_MR_CHRL_5_BIT;
        break;
        case 6:
            uart->options.char_length = US_MR_CHRL_6_BIT;
        break;
        case 7:
            uart->options.char_length = US_MR_CHRL_7_BIT;
        break;
        case 8:
            uart->options.char_length = US_MR_CHRL_8_BIT;
        break;
        case 9:
            uart->options.char_length = US_MR_MODE9;
        break;
        default:
            /* Unsupported mode */
            return -1;
    }

    /* Set parity mode */
    switch (parity){
        case HAL_UART_PARITY_NONE:
            uart->options.parity_type = US_MR_PAR_NO;
        break;
        case HAL_UART_PARITY_ODD:
            uart->options.parity_type = US_MR_PAR_ODD;
        break;
        case HAL_UART_PARITY_EVEN:
            uart->options.parity_type = US_MR_PAR_EVEN;
        break;
        default:
            /* Unsupported mode (there are a few that usart supports that this interface does not) */
            return -1;
    }

    /* Set stop bits */
    switch(stopbits){
        case 1:
            uart->options.stop_bits = US_MR_NBSTOP_1_BIT;
        break;
        case 2:
            uart->options.stop_bits = US_MR_NBSTOP_2_BIT;
        break;
        default:
            return -1;
    }   

    /* Ignore flow control, but throw error on illegal argument */
    switch (flow_ctl){
        case HAL_UART_FLOW_CTL_NONE:
        case HAL_UART_FLOW_CTL_RTS_CTS:
            break;
        default:
            return -1;
    }

    /* clock rate should be something like - sysclk_get_peripheral_hz() */
    usart_init_rs232(uart->uart, &(uart->options), USART_CLOCK_RATE);

    return 0;
}

/**
 * hal uart config
 *
 * Applies given configuration to UART.
 */
int hal_uart_config(int uart, int32_t speed, uint8_t databits, uint8_t stopbits,
  enum hal_uart_parity parity, enum hal_uart_flow_ctl flow_ctl){
      void *uart_ptr = uarts[uart].uart;
      if (is_usart(uart_ptr)){
          /* USART configuration */
          /* Assume USART clock and Board are initialized */
        hal_usart_config(&uarts[uart], speed, databits, stopbits, parity, flow_ctl);
      }else{
          /* UART configuration */
      }
      return 0;
}

/*
 * Close UART port. Can call hal_uart_config() with different settings after
 * calling this.
 */
int hal_uart_close(int port);

/**
 * hal uart start tx
 *
 * More data queued for transmission. UART driver will start asking for that
 * data.
 */
void hal_uart_start_tx(int uart);

/**
 * hal uart start rx
 *
 * Upper layers have consumed some data, and are now ready to receive more.
 * This is meaningful after uart_rx_char callback has returned -1 telling
 * that no more data can be accepted.
 */
void hal_uart_start_rx(int uart);

/**
 * hal uart blocking tx
 *
 * This is type of write where UART has to block until character has been sent.
 * Used when printing diag output from system crash.
 * Must be called with interrupts disabled.
 */
void hal_uart_blocking_tx(int uart, uint8_t byte);