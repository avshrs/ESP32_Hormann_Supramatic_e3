#pragma once
#include "serial_w.h"
#include <unistd.h>
#include <string>
#include <chrono>


class Mqtt_Client;
#define BROADCAST_ADDR            0x00
#define MASTER_ADDR               0x80
#define UAP1_ADDR                 0x28

#define UAP1_TYPE                 0x14

#define CMD_SLAVE_SCAN            0x01
#define CMD_SLAVE_STATUS_REQUEST  0x20
#define CMD_SLAVE_STATUS_RESPONSE 0x29
// Supramatic e3
#define RESPONSE_DEFAULT          0x00
#define RESPONSE_STOP             0xff
#define RESPONSE_OPEN             0x01
#define RESPONSE_CLOSE            0x02
#define RESPONSE_VENTING          0x10
#define RESPONSE_TOGGLE_LIGHT     0x08

#define CRC8_INITIAL_VALUE        0xF3

// Status mask for LineaMatic P:
// +------- (0x80) Unknown
//  +------ (0x40) Motor running: 1 == running. 0 == stopped.
//   +----- (0x20) Motor direction: 1 == closing. 0 == opening.
//    +---- (0x10) Unknown
//     +--- (0x08) Unknown
//      +-- (0x04) Unknown
//       +- (0x02) Fully closed 
//        + (0x01) Fully open


// Command mask for LineaMatic P:
// +------- (0x80) Unknown
//  +------ (0x40) Unknown
//   +----- (0x20) Unknown
//    +---- (0x10) Moves to 'H' (whatever that means)
//     +--- (0x08) Unknown
//      +-- (0x04) Impulse toggle
//       +- (0x02) Impulse close
//        + (0x01) Impulse open
//           0x00  default
// For some reason the second byte needs to be 0x10 (signals no error?)


class Hoermann{
    private:
        SerialW ser;

    private:
        unsigned long previousMillis = 0;  
        String * state;
        uint8_t tx_length = 0;
        uint8_t slave_respone_data = RESPONSE_DEFAULT;
        uint8_t master_address = 0x80;
        uint16_t broadcast_status = 0;
        uint8_t lz = 0;
        uint8_t broadcast_lengh = 0x02; 
        uint8_t reguest_lengh = 0x01; 
        int max_frame_delay = 6;
        const char* serial_name;
        int boudrate;

    public:
    
        void init(String * set_state, int tx_pin);
        void run_loop(void);
        void door_open();
        void door_close();
        void door_venting();
        void door_toggle_light();
        void door_stop();
        // void door_lock();
        String get_state();
        void set_state(String action);


    private:
        void pub_thread();

        
        void print_buffer(uint8_t* buf, int len);
        
        void update_broadcast_status(RX_Buffer &buf);
        
        uint8_t get_length(RX_Buffer &buf);
        uint8_t get_counter(RX_Buffer &buf);
        uint8_t get_master_address();
        uint8_t calc_crc8(uint8_t* p_data, uint8_t length);
        
        bool is_frame_corect(RX_Buffer &buf);
        bool is_broadcast(RX_Buffer &buf);
        bool is_slave_query(RX_Buffer &buf);
        bool is_slave_scan(RX_Buffer &buf);
        bool is_slave_status_req(RX_Buffer &buf);
        bool is_broadcast_lengh_correct(RX_Buffer &buf);
        bool is_req_lengh_correct(RX_Buffer &buf);
        void make_scan_responce_msg(RX_Buffer &buf, TX_Buffer &tx_buf);
        void make_status_req_msg(RX_Buffer &buf, TX_Buffer &tx_buf);
};

