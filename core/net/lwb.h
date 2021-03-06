/*
 * Copyright (c) 2016, Swiss Federal Institute of Technology (ETH Zurich).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author:  Reto Da Forno
 *          Federico Ferrari
 *          Marco Zimmerling
 */

/**
 * @addtogroup  Net
 * @{
 *
 * @defgroup    lwb The Low-Power Wireless Bus (LWB)
 * @{
 *
 * @file 
 * 
 * @brief    the low-power wireless bus (LWB)
 */
 
#ifndef __LWB_H__
#define __LWB_H__


#include "contiki.h"
#include "platform.h"   /* necessary for SMCLK_SPEED in clock.h */

#ifndef HOST_ID
#warning "HOST_ID not defined, set to 0"
#define HOST_ID                         0
#endif

#ifndef LWB_VERSION
#define LWB_VERSION     1       /* default version */
#endif /* LWB_VERSION */

/*---------------------------------------------------------------------------*/

#ifndef LWB_CONF_MAX_DATA_PKT_LEN
/* max. length of a data packet incl. the LWB header (node + stream ID), 
 * determines T_Slot (LWB_CONF_T_DATA) and influences the power dissipation, 
 * choose as small as possible; must be <= (LWB_CONF_MAX_PKT_LEN - 5) 
 * NOTE: LWB_CONF_MAX_DATA_PKT_LEN must not exceed LWB_CONF_MAX_PKT_LEN
 * and the max. data payload length is LWB_CONF_MAX_DATA_PKT_LEN - 3 */
#define LWB_CONF_MAX_DATA_PKT_LEN       LWB_CONF_MAX_PKT_LEN
#endif /* LWB_CONF_MAX_DATA_PKT_LEN */

#ifndef LWB_CONF_MAX_DATA_SLOTS
/* max. number of data slots per round, must not exceed MIN(63, 
 * (LWB_CONF_MAX_PKT_LEN - LWB_SCHED_PKT_HEADER_LEN) / 2), 
 * must be at least 2 */
#define LWB_CONF_MAX_DATA_SLOTS         20        
#endif /* LWB_CONF_MAX_DATA_SLOTS */

#ifndef LWB_CONF_HEADER_LEN
#define LWB_CONF_HEADER_LEN             3       /* default header size */
#endif /* LWB_CONF_HEADER_LEN */

/* set to 1 to skip the state QUASI_SYNCED and jump directly to SYNCED after 
 * BOOTSTRAP; reduces the delay to the first slot */
#ifndef LWB_CONF_SKIP_QUASI_SYNCED
#define LWB_CONF_SKIP_QUASI_SYNCED      0
#endif /* LWB_CONF_SKIP_QUASI_SYNCED */

/* scaling factor to enable periods and IPIs of less than 1 second 
 * (e.g. a scale 10 means the LWB runs 10x faster, i.e. a round period of 1s
 * will effectively only be 100ms long) */
#ifndef LWB_CONF_TIME_SCALE
#define LWB_CONF_TIME_SCALE             1
#endif /* LWB_CONF_TIME_SCALE */

/* if set to 1, source nodes will only relay packets, but never receive 
 * (= store in internal memory for further processing) or transmit 
 * (= generate) packets, and therefore require less memory */
#ifndef LWB_CONF_RELAY_ONLY
#define LWB_CONF_RELAY_ONLY             0
#endif /* LWB_CONF_RELAY_ONLY */

#if !LWB_CONF_TIME_SCALE
#error "invalid value for LWB_CONF_TIME_SCALE"
#endif

#ifndef LWB_CONF_USE_XMEM
/* by default, don't use the external memory to store the message queues and
 * statistics; due to memory (SRAM) constraints, using the (slow) external 
 * memory may be unavoidable */
#define LWB_CONF_USE_XMEM               0
#endif /* LWB_CONF_USE_XMEM */

#ifndef LWB_CONF_USE_LF_FOR_WAKEUP
/* use the low-frequency timer to schedule the wake-up from LPM for the LWB 
 * round? this enables the application to disable the HF clock during periods
 * of inactivity */
#define LWB_CONF_USE_LF_FOR_WAKEUP      0
#endif /* LWB_CONF_USE_LF_FOR_WAKEUP */

/* error checking: can only use LFXT for wakeup if it's available on the PCB */
#if LWB_CONF_USE_LF_FOR_WAKEUP && !CLOCK_CONF_XT1_ON
#error "Can't use LF for wakeup (LWB)"
#endif

#ifndef LWB_CONF_T_REF_OFS
/* constant time offset that is subtracted from t_ref in each round to align  
 * the glossy start pulses on host and source nodes */
#define LWB_CONF_T_REF_OFS              (RTIMER_SECOND_HF / 800)
#endif /* LWB_CONF_T_REF_OFS */

#ifndef LWB_CONF_T_SCHED
/* length of a schedule slot: should be approx. 25 ms */
#define LWB_CONF_T_SCHED                LWB_T_SLOT_MIN(LWB_CONF_MAX_PKT_LEN)    
#endif /* LWB_CONF_T_SCHED */
  
#ifndef LWB_CONF_T_DATA
/* length of a data slot */
#define LWB_CONF_T_DATA               LWB_T_SLOT_MIN(LWB_CONF_MAX_DATA_PKT_LEN)
#endif /* LWB_CONF_T_DATA */

#ifndef LWB_CONF_T_CONT
/* length of a contention slot */
#define LWB_CONF_T_CONT                 (RTIMER_SECOND_HF / 200)
#endif /* LWB_CONF_T_CONT */

#ifndef LWB_CONF_T_GAP
/* gap between two consecutive slots: 4 ms, must be high enough to finish all 
 * local computations between the slots */
#define LWB_CONF_T_GAP                  (RTIMER_SECOND_HF / 250)
#endif /* LWB_CONF_T_GAP */

#ifndef LWB_CONF_T_GUARD
/* initial guard time, (RTIMER_SECOND_HF / 2000) = 0.5 ms, guard times are 
 * needed to account for clock drifts */
#define LWB_CONF_T_GUARD                (RTIMER_SECOND_HF / 2000)      
#endif /* LWB_CONF_T_GUARD */

#ifndef LWB_CONF_T_GUARD_1
/* 1 ms (must not be higher than 1 ms, otherwise T_GAP is too short!) */
#define LWB_CONF_T_GUARD_1              (RTIMER_SECOND_HF / 1000)
#endif /* LWB_CONF_T_GUARD_1 */

#ifndef LWB_CONF_T_GUARD_2
#define LWB_CONF_T_GUARD_2              (RTIMER_SECOND_HF / 200)     /* 5 ms */
#endif /* LWB_CONF_T_GUARD_2 */

#ifndef LWB_CONF_T_GUARD_3
#define LWB_CONF_T_GUARD_3              (RTIMER_SECOND_HF / 100)    /* 10 ms */
#endif /* LWB_CONF_T_GUARD_3 */

#ifndef LWB_CONF_IN_BUFFER_SIZE         
/* size (#elements) of the internal data buffer/queue for incoming messages,
 * should be at least LWB_CONF_MAX_DATA_SLOTS */
#define LWB_CONF_IN_BUFFER_SIZE         LWB_CONF_MAX_DATA_SLOTS
#endif /* LWB_CONF_IN_BUFFER_SIZE */

#ifndef LWB_CONF_OUT_BUFFER_SIZE         
/* size (#elements) of the internal data buffer/queue for outgoing messages */
#define LWB_CONF_OUT_BUFFER_SIZE        3
#endif /* LWB_CONF_IN_BUFFER_SIZE */

/* ensure that the buffer can at least hold one data packet */
#if !LWB_CONF_IN_BUFFER_SIZE || !LWB_CONF_OUT_BUFFER_SIZE
#error "invalid LWB buffer size configuration!"
#endif 

#ifndef LWB_CONF_STATS_NVMEM
/* keep statistics in non-volatile memory? */
#define LWB_CONF_STATS_NVMEM            0         
#endif /* LWB_CONF_STATS_NVMEM */

#ifndef LWB_CONF_MAX_PKT_LEN
/* the max. length of a packet (limits the message size as well as the max. 
 * size of a LWB packet and the schedule); do not change this value before
 * you have adjusted the radio module configuration! */
#define LWB_CONF_MAX_PKT_LEN            127
#endif /* LWB_CONF_MAX_PKT_LEN */

#ifndef LWB_CONF_TX_CNT_SCHED
/* max. number of TX phases for a schedule packet (how many times each node 
 * transmits a packet) */
#define LWB_CONF_TX_CNT_SCHED           3
#endif /* LWB_CONF_TX_CNT_SCHED */

#ifndef LWB_CONF_TX_CNT_DATA
/* max. number of TX phases for a data packet */
#define LWB_CONF_TX_CNT_DATA            3
#endif

#ifndef LWB_CONF_MAX_HOPS
/* max. number of hops in the network to reach all nodes (only used to 
 * calculate T_SLOT_MIN) */
#define LWB_CONF_MAX_HOPS               5
#endif /* LWB_CONF_MAX_HOPS */

#ifndef LWB_CONF_T_SILENT               /* set to 0 to disable this feature */
/* if no communication happens within this time (i.e. no schedule received),
 * the source node goes into deepsleep mode for LWB_CONF_T_DEEPSLEEP LF ticks*/
#define LWB_CONF_T_SILENT               (180 * RTIMER_SECOND_HF)
#endif /* LWB_CONF_T_SILENT */

#ifndef LWB_CONF_T_DEEPSLEEP
#define LWB_CONF_T_DEEPSLEEP            (RTIMER_SECOND_LF * 3600)      /* 1h */
#endif /* LWB_CONF_T_DEEPSLEEP */

#ifndef LWB_CONF_T_SCHED2_START
/* start point (offset) of the second schedule at the end of a round
 * must be after LWB_T_ROUND_MAX; this ensures that the host node has at least
 * LWB_CONF_T_SCHED timer clock ticks to compute the new schedule before
 * transmitting it at the end of the round */
#define LWB_CONF_T_SCHED2_START         LWB_T_ROUND_MAX
#endif /* LWB_CONF_T_SCHED2_START */

#ifndef LWB_CONF_T_PREPROCESS
/* in milliseconds, set this to 0 to disable preprocessing before a LWB round*/
#define LWB_CONF_T_PREPROCESS           0
#endif /* LWB_CONF_T_PREPROCESS */

#ifndef LWB_CONF_MAX_CLOCK_DEV
/* the max. clock deviation (according to the specs of the oscillator), in 
 * clock ticks per second */
 #if LWB_CONF_USE_LF_FOR_WAKEUP
  #define LWB_CONF_MAX_CLOCK_DEV        2560      /* LF clock ticks * 256 */     
 #else /* LWB_CONF_USE_LF_FOR_WAKEUP */
  #define LWB_CONF_MAX_CLOCK_DEV        500     /* HF clock ticks */
 #endif /* LWB_CONF_USE_LF_FOR_WAKEUP */
#endif /* LWB_CONF_MAX_CLOCK_DEV */

#ifndef LWB_CONF_RTIMER_ID
/* ID of the rtimer used for the LWB, must be of type rtimer_t */
#define LWB_CONF_RTIMER_ID              RTIMER_HF_1     
#endif /* LWB_CONF_RTIMER_ID */

#ifndef LWB_CONF_LF_RTIMER_ID
/* ID of the rtimer used to trigger the wake-up */
#define LWB_CONF_LF_RTIMER_ID           RTIMER_LF_1    
#endif /* LWB_CONF_LF_RTIMER_ID */

#ifndef LWB_CONF_MAX_N_STREAMS_PER_NODE
/* this value may not be higher than 32! */
#define LWB_CONF_MAX_N_STREAMS_PER_NODE 32
#endif /* LWB_CONF_MAX_STREAM_CNT_PER_NODE */

#ifndef LWB_CONF_MAX_N_STREAMS
/* max. number of streams (bounds the required memory on the host) */
#define LWB_CONF_MAX_N_STREAMS          32 
#endif /* N_STREAMS_MAX */

/* max. number of rounds a node backs off after sending a stream request 
 * before it tries again */
#ifndef LWB_CONF_MAX_CONT_BACKOFF
#define LWB_CONF_MAX_CONT_BACKOFF       8
#endif /* LWB_CONF_MAX_CONT_BACKOFF */

#ifndef LWB_CONF_DATA_ACK
#define LWB_CONF_DATA_ACK               0
#endif /* LWB_CONF_DATA_ACK */

/*---------------------------------------------------------------------------*/

#ifndef RF_CONF_TX_POWER                /* set the radio antenna gain */
#define RF_CONF_TX_POWER                RF1A_TX_POWER_0_dBm   
#endif /* RF_CONF_TX_POWER */

#ifndef RF_CONF_TX_CH 
#define RF_CONF_TX_CH                   5
#endif /* RF_CONF_TX_CH */

#ifndef RF_CONF_TX_BITRATE
/* radio transmission bitrate, do not change */
#define RF_CONF_TX_BITRATE              250000
#else /* RF_CONF_TX_BITRATE */
#error "RF_CONF_TX_BITRATE already defined!"
#endif /* RF_CONF_TX_BITRATE */

// -> defined in rf1a-core.h
#ifndef RF_CONF_MAX_PKT_LEN
#define RF_CONF_MAX_PKT_LEN             LWB_CONF_MAX_PKT_LEN
#endif /* RF_CONF_MAX_PKT_LEN */

/*---------------------------------------------------------------------------*/

/* important values, do not modify */

#define LWB_T_ROUND_MAX             ((LWB_CONF_MAX_DATA_SLOTS + 1 + \
                                      LWB_CONF_DATA_ACK) * \
                                     (LWB_CONF_T_DATA + LWB_CONF_T_GAP) + \
                                     (LWB_CONF_T_SCHED + LWB_CONF_T_GAP) + \
                                     (LWB_CONF_T_CONT + LWB_CONF_T_GAP))

/* min. duration of 1 packet transmission with glossy (approx. values, taken 
 * from TelosB platform measurements) -> for 127b packets ~4.5ms, for 50b 
 * packets just over 2ms */
#define LWB_T_HOP(len)              ((RTIMER_SECOND_HF * \
                                     (3 + 24 + 192 + 192 + ((1000000 * \
                                     (len) * 8) / RF_CONF_TX_BITRATE))) \
                                     / 1000000)  
/* minimum duration of a data slot according to "Energy-efficient Real-time 
 * Communication in Multi-hop Low-power Wireless Networks" (Zimmerling et al.),
 * Appendix I. For 127b packets ~22.5ms, for 50b packets just over 10ms */
#define LWB_T_SLOT_MIN(len)         ((LWB_CONF_MAX_HOPS + \
                                     (2 * LWB_CONF_TX_CNT_DATA) - 2) * \
                                     LWB_T_HOP(len))
                                                                         
#define LWB_RECIPIENT_HOST          0x0000  /* to the host */
#define LWB_RECIPIENT_SINKS         0xfffe  /* to all sinks */
#define LWB_RECIPIENT_BROADCAST     0xffff  /* to all nodes / sinks */

#define LWB_RECIPIENT_GROUP_MASK    0xf000  /* group ID mask */
#define LWB_RECIPIENT_NODE_MASK     0x0fff  /* node ID mask */

/*---------------------------------------------------------------------------*/

#define MAX(x, y)                   ((x) > (y) ? (x) : (y))


/**
 * @brief keep some statistics
 */
typedef struct {
    uint8_t  relay_cnt;
    uint8_t  period_last;
    uint8_t  unsynced_cnt;
    uint8_t  bootstrap_cnt;
    uint16_t reset_cnt;
    uint16_t pck_cnt;     /* total number of received packets */
    uint16_t t_sched_max; /* max. time needed to calculate the new schedule */
    uint16_t t_proc_max;  /* max. time needed to process the rcvd data pkts */
    uint16_t crc;         /* crc of this struct (with crc set to 0!) */
    uint32_t t_slot_last; /* last slot assignment (in seconds) */
    uint32_t data_tot;
} lwb_statistics_t;

/**
 * @brief simplified 'connection state' of a source node
 * When a source node first boots, it is in LWB_STATE_INIT state. The node 
 * then tries to connect to a host. Upon successful reception of a schedule 
 * packet, its state chances to LWB_STATE_CONNECTED. When no schedule packet
 * is received for a certain amount of time, the node's state is degraded to 
 * LWB_STATE_CONN_LOST.
 */
typedef enum {
    LWB_STATE_INIT = 0, /* bootstrap */
    LWB_STATE_CONNECTED, 
    LWB_STATE_CONN_LOST,
} lwb_conn_state_t;


#include "scheduler.h"
#include "stream.h"


/**
 * @brief start the Low-Power Wireless Bus
 * @param pre_lwb_proc a pointer to a function that needs to be executed
 * before an LWB round. Set LWB_T_PREPROCESS to the worst-case 
 * execution time of this function.
 * @param pre_lwb_proc a pointer to the application task process control 
 * block (struct process)
 */
void lwb_start(void (*pre_lwb_func)(void), void *post_lwb_proc);


/**
 * @brief pause the LWB by stopping the rtimer
 */
void lwb_pause(void);

/**
 * @brief resume the LWB by scheduling the rtimer
 */
void lwb_resume(void);


/**
 * @brief query the connection status of the LWB
 * @return the LWB state, lwb_conn_state_t 
 */
lwb_conn_state_t lwb_get_state(void);

/**
 * @brief schedule a packet for transmission over the LWB
 * @param data a pointer to the data packet to send
 * @param len the length of the data packet (must be less or equal 
 * LWB_CONF_MAX_PKT_LEN)
 * @return 1 if successful, 0 otherwise (queue full)
 */
#if LWB_VERSION == 2
uint8_t lwb_put_data(const uint8_t * const data, 
                     uint8_t len);
#else
uint8_t lwb_put_data(uint16_t recipient, 
                     uint8_t stream_id, 
                     const uint8_t * const data, 
                     uint8_t len);
#endif

/** 
 * @brief get a data packet that have been received during the previous LWB
 * rounds
 * @param out_data A valid memory block that can hold one data packet. The 
 * buffer must be at least LWB_CONF_MAX_PKT_LEN bytes long.
 * @param out_node_id the ID of the node that sent the message (optional 
 * parameter, pass 0 if not interested in this data)
 * @param out_stream_id the stream ID (optional parameter, pass 0 if not 
 * interested in this data)
 * @return the length of the data packet in bytes or 0 if the queue is empty
 * @note once a data packet was requested, it will be removed from the internal
 * buffer
 */
#if LWB_VERSION == 2
uint8_t lwb_get_data(uint8_t* out_data);
#else
uint8_t lwb_get_data(uint8_t* out_data, 
                     uint16_t * const out_node_id, 
                     uint8_t * const out_stream_id);
#endif

/**
 * @brief check the status of the receive buffer (incoming messages)
 * @return 1 if there is at least 1 message in the queue, 0 otherwise
 */
uint8_t lwb_get_rcv_buffer_state(void);

/**
 * @brief check the status of the send buffer (outgoing messages)
 * @return the number of remaining packets in the queue
 */
uint8_t lwb_get_send_buffer_state(void);

/**
 * @brief schedules a stream request to be sent during the contention slot
 * @param stream_request a pointer to the structure containing the stream 
 * information
 * @param urgent if you set this parameter to 1, the LWB tries to piggyback 
 * the stream request onto a data packet
 * @return 0 if an error occurred (e.g. queue full), 1 otherwise
 * @note: the stream info data to which stream_request points will be copied
 * into an internal buffer and may be deleted after calling 
 * lwb_request_stream().
 */
uint8_t lwb_request_stream(lwb_stream_req_t* stream_request, uint8_t urgent);

/**
 * @brief get the LWB statistics
 */
const lwb_statistics_t * const lwb_get_stats(void);

/**
 * @brief reset the statistics
 */
void lwb_stats_reset(void);

/**
 * @brief get the time of the LWB, i.e. the network-wide scheduler time 
 * @param reception_time timestamp of the reception of the last schedule,
 * optional parameter (pass 0 if not needed)
 * @return the relative time in seconds since the host started
 */
uint32_t lwb_get_time(rtimer_clock_t* reception_time);


#endif /* __LWB_H__ */

/**
 * @}
 * @}
 */
