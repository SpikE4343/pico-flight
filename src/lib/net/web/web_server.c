#include <stdio.h>
#include <stddef.h>
#include "usb_network.h"
#include "lwip/tcp.h"

#include "data_vars.h"
#include "web_server.h"
#include <string.h>
#include "http.h"



// #include "mongoose.h"

 // ---------------------------------------------------------------
struct tcp_pcb* client;


 // ---------------------------------------------------------------
static void srv_close(struct tcp_pcb *pcb)
{
    // Cancel send timer.
    // cancel_repeating_timer(&timer);

    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_close(pcb);
}

 // ---------------------------------------------------------------
static void srv_err(void *arg, err_t err) 
{
    // Probably an indication that the client connection went kaput! Stopping stream...
    srv_close(client);
}

 // ---------------------------------------------------------------
static err_t srv_receive(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) 
{
    if (err != ERR_OK && p != NULL) {
        goto exception;
    }

    tcp_recved(pcb, p->tot_len);
    


    // // The connection is closed if the client sends "X".
    // if (((char*)p->payload)[0] == 'X') {
    //     srv_close(pcb);
    // }

    uint8_t* data = ((uint8_t*)p->payload);
    uint32_t len = p->tot_len;

    char* line  = data;
   
    HttpRequest_t req;
    memset(&req, 0, sizeof(req));
    if( !http_read_request(&req, data, len))
      goto exception;


    char rep[] = "HTTP/1.0 200 OK\r\n\
Server: tiny.usb v0.1\r\n\
Content-Type: text/html\r\n\
\r\n\
<head></head><body></body>";    

    tcp_write(pcb, rep, sizeof(rep)-1, 0);
    tcp_output(pcb);


    srv_close(pcb);
    

exception:
    pbuf_free(p);
    return err;
}

// ---------------------------------------------------------------
static err_t srv_accept(void * arg, struct tcp_pcb * pcb, err_t err) 
{
    if (err != ERR_OK) {
        return err;
    }
        
    tcp_setprio(pcb, TCP_PRIO_MAX);
    tcp_recv(pcb, srv_receive);
    tcp_err(pcb, srv_err);
    tcp_poll(pcb, NULL, 4);

    client = pcb;


    return err;
}


 // ---------------------------------------------------------------
void webServerInit()
{
   // Init network RNDIS stack.
    network_init();

    // Start TCP server.
    struct tcp_pcb* pcb = tcp_new();
    pcb->so_options |= SOF_KEEPALIVE;
    pcb->keep_intvl = 75000000;
    tcp_bind(pcb, IP_ADDR_ANY, 80);

    // Start listening for connections.
    struct tcp_pcb* listen = tcp_listen(pcb);
    tcp_accept(listen, srv_accept);
}

// ---------------------------------------------------------------
void webServerUpdate()
{
   network_step();
}

// ---------------------------------------------------------------
void webServerShutdown()
{

}

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_signal_lost, false, 
//   "rc.signalLost",
//   "True when input controls loses connection",
//   b8, Tdm_RW);

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_failsafe, false, 
//   "rc.failsafe",
//   "True when rc control system indicates failsafe",
//   b8, Tdm_RW);

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_frames_recv, 0, 
//   "rc.frames.recv",
//   "Count of rc data frames received",
//   u32, Tdm_read | Tdm_realtime);

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_packet_loss, 0, 
//   "rc.packet_loss",
//   "Packets loss indicator from rx device",
//   u32, Tdm_read);

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_rssi, 0, 
//   "rc.rssi",
//   "Receive signal strengh indicator from rx device",
//   i8, Tdm_read | Tdm_realtime);

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_uart_id, 1, 
//   "rc.uart.id",
//   "Id of uart to use",
//   u8, Tdm_RW | Tdm_config);

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_uart_pins_tx, 21, 
//   "rc.uart.pins.tx",
//   "Id of gpio pin to use for transmit",
//   u8, Tdm_RW | Tdm_config);

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_uart_pins_rx, 20, 
//   "rc.uart.pins.rx",
//   "Id of gpio pin to use for receive",
//   u8, Tdm_RW | Tdm_config);

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_uart_baud, 115200,
//   "rc.uart.baud",
//   "Baud rate to use for rc communication",
//   u32, Tdm_RW | Tdm_config);

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_recv_state, 0,
//   "rc.recv.state",
//   "Baud rate to use for rc communication",
//   u8, Tdm_RW);

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_uart_rx_bytes, 0,
//   "rc.uart.rx",
//   "Bytes read from rc uart",
//   u32, Tdm_RW | Tdm_realtime);

// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_uart_tx_bytes, 0,
//   "rc.uart.tx",
//   "Bytes sent to rc uart",
//   u32, Tdm_RW | Tdm_realtime);


// // ---------------------------------------------------------------
// DEF_DATA_VAR(tdv_rc_last_recv_us, 0,
//   "rc.recv.last.us",
//   "Last time data recv from rx",
//   u32, Tdm_read);

// // ---------------------------------------------------------------
// #define rc_control_name "rc.input"
// #define rc_control_desc "RC control normalized inputs"

// BEGIN_DEF_DV_ARRAY( tdv_rc_input )
//   DEF_DV_ARRAY_ITEM(0, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   DEF_DV_ARRAY_ITEM(1, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   DEF_DV_ARRAY_ITEM(2, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   DEF_DV_ARRAY_ITEM(3, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   DEF_DV_ARRAY_ITEM(4, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   DEF_DV_ARRAY_ITEM(5, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   DEF_DV_ARRAY_ITEM(6, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   DEF_DV_ARRAY_ITEM(7, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),

//   // DEF_DV_ARRAY_ITEM(8, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(9, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(10, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(11, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(12, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(13, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(14, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(15, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),

//   // DEF_DV_ARRAY_ITEM(16, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(17, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(18, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(19, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(20, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(21, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(22, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(23, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),

//   // DEF_DV_ARRAY_ITEM(24, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(25, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(26, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(27, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(28, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(29, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(30, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
//   // DEF_DV_ARRAY_ITEM(31, 0.0f, rc_control_name, rc_control_desc, f32, Tdm_RW | Tdm_realtime),
// END_DEF_DV_ARRAY();

// // ---------------------------------------------------------------
// // map raw inputs --> control inputs
// #define rc_mapping_name "rc.mapping"
// #define rc_mapping_desc "RC input mapping to controls"

// // AETR
// BEGIN_DEF_DV_ARRAY( tdv_rc_mapping )
//   DEF_DV_ARRAY_ITEM(0, 0, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
//   DEF_DV_ARRAY_ITEM(1, 1, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
//   DEF_DV_ARRAY_ITEM(3, 2, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
//   DEF_DV_ARRAY_ITEM(2, 3, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
//   DEF_DV_ARRAY_ITEM(4, 4, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
//   DEF_DV_ARRAY_ITEM(5, 5, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
//   DEF_DV_ARRAY_ITEM(6, 6, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
//   DEF_DV_ARRAY_ITEM(7, 7, rc_mapping_name, rc_mapping_desc, u8, Tdm_RW),
// END_DEF_DV_ARRAY();


