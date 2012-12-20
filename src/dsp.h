/*
 * Copyright (c) 2012 PD3 Tecnologia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * File name: dsp.h
 * Created on: Oct 15, 2012
 * Author: Thomas Del Grande <tgrande@pd3.com.br>
 *
 * Definitions for dsp.c
 */

#ifndef DSP_H_
#define DSP_H_

#include <vapi/vapi.h>
#include <vapi/ut.h>
#include <vapi/gtl.h>
#include <vapi/comcerto-stat-combined-voice-types.h>

#include <syslog.h>

#define amg_err(x,...) syslog(LOG_ERR, x, ##__VA_ARGS__)
#define amg_log(x,...) syslog(LOG_INFO, x, ##__VA_ARGS__)
#define amg_dbg(x,...) syslog(LOG_DEBUG, x, ##__VA_ARGS__)

/**
 * Set RTP packet interval for connection
 *
 * @param conn_id: Connection in question
 * @param parms: Pointer to channel parameters
 * @param interval: interval for packets in miliseconds
 * @return 0 if success, negative if error
 */
int libamg_dsp_set_rtp_interval(int conn_id, SVoIPChnlParams *parms, int interval);

/**
 * Enable/Disable Voice Active Detection on connection
 *
 * @param conn_id: Connection in question
 * @param parms: Pointer to channel parameters
 * @param enable: 1 to enable, 0 to disable
 * @param tune: Value from 0 (more bandwidth saving ) to 4 (more quality)
 * @return 0 if success, negative if error
 */
int libamg_dsp_set_vad(int conn_id, SVoIPChnlParams *parms, int enable, int tune);

/**
 * Set rate for codec G.723 - 6.3kbps or 5.3kbps
 *
 * @param conn_id: Connection in question
 * @param parms: Pointer to channel parameters
 * @param high: 1 for 6.3, 0 for 5.3
 * @return 0 if success, negative if error
 */
int libamg_dsp_set_g723_rate(int conn_id, SVoIPChnlParams *parms, int high);

/**
 * Set voice gain
 *
 * @param conn_id: Connection in question
 * @param parms: Pointer to channel parameters
 * @param txgain: TDM -> IP gain
 * @param rxgain: IP -> TDM gain
 * @return 0 if success, negative if error
 */
int libamg_dsp_set_gain(int conn_id, SVoIPChnlParams *parms, int txgain, int rxgain);



/**
 * Enable/Disable Confort Noise Generation
 *
 * @param conn_id: Connection in question
 * @param parms: Pointer to channel parameters
 * @param enable: 1 to enable, 0 to disable
 * @return 0 if success, negative if error
 */
int libamg_dsp_set_cng(int conn_id, SVoIPChnlParams *parms, int enable);

/**
 * Enable/Disable the DSP echo canceling feature
 *
 * @param conn_id: Connection in question
 * @param parms: Pointer to channel parameters
 * @param enable: 1 enable, 0 disable
 * @param tail_size: 8 - 128, in steps of 8
 * @return 0 if success, negative if error
 */
int libamg_dsp_set_echocan(int conn_id, SVoIPChnlParams *parms, int enable, int tail_size);

/**
 * Set DTMF mode: Inband, RFC2833 or SIP Info
 *
 * @param conn_id: Connection in question
 * @param parms: Pointer to channel parameters
 * @param dtmf_mode: DTMF mode (ast enum)
 * @return 0 if success, negative if error
 */

enum _dtmf_mode {
	DTMF_MODE_NONE = 0,
	DTMF_MODE_RFC2833,
	DTMF_MODE_INBAND,
};
int libamg_dsp_set_dtmf_mode(int conn_id, SVoIPChnlParams *parms, enum _dtmf_mode dtmf_mode);


struct jb_params {
	int jbmin;
	int jbinit;
	int jbmax;
	int deletion_threshold;
};

/**
 * Set Jitter Buffer settings
 *
 * @param conn_id: Connection in question
 * @param parms: Pointer to channel parameters
 * @param jb: Pointer to jb parameters
 * @return 0 if success, negative if error
 */
int libamg_dsp_set_jitter_buffer(int conn_id, SVoIPChnlParams *parms, struct jb_params *jb);


/**
 * Set Payload type for codec
 *
 * @param conn_id: Connection in question
 * @param codec: Codec index in question
 * @param pt: Payload type to be used
 * @return 0 if success, negative if error
 */
int libamg_dsp_set_codec_payload_type(int conn_id, ECodecIndex codec, int pt);


enum inband_signaling_t {
	INBAND_SIG_OFF = 		0x00,
	INBAND_SIG_MF_R2_BCK = 	0x01,
	INBAND_SIG_MF_R2_FWD = 	0x02,
};

/**
 * Get current inband signaling mode
 *
 * Returns the inband signaling mode enabled: R2 backward, R2 forward or disabled (DTMF)
 *
 * @param conn_id: Connection in question
 * @return signaling mode or negative if error
 */
int libamg_dsp_get_inband_signaling(int conn_id);

/**
 * Set inband signaling mode
 *
 * This mode must be set if MF-R2 tones should be detected by the DSP.
 * Turn it off as soon as RTP traffic should be enabled.
 *
 * @param conn_id: Connection in question
 * @param mode: Signaling mode to be set
 * @return 0 if success, negative if error
 */
int libamg_dsp_set_inband_signaling(int conn_id, enum inband_signaling_t mode);

/**
 * Set MF R2 timing definitions
 */
int libamg_dsp_set_mf_r2_timings(int conn_id);

/**
 * Tone Event structure to be used for queuing/dequeing tones
 */
struct tone_event_t {
	int conn_id;
	int tone;
	struct tone_event_t *next;
};

/* Used in Asterisk to identify fax events frames */
#define COMCERTO_EVENT_FAX_CED		256
#define COMCERTO_EVENT_FAX_CNG		257
#define COMCERTO_EVENT_FAX_ANS		258

/**
 * Dequeue any existing tone event for a certain connection
 *
 * @param The Connection ID
 * @return The tone if any, negative if none
 *
 */
int libamg_dsp_dequeue_tone_event(int conn_id);

/**
 * Queue tone event for a certain connection
 *
 * @param Pointer to SToneDetectEventParams data
 * @return 0 if success, -1 if error
 *
 */
int libamg_dsp_queue_tone_event(struct tone_event_t *tone);

/**
 *	Translate tones from binary (Comcerto) to ASCII (OpenR2)
 *
 *  @param Tone value in binary
 *  @return Tone value in ASCII
 */
char libamg_dsp_mfcr2_tone_int_to_char(int tone);

/**
 * Start playing a MFC/R2 tone in channel
 *
 * @param conn_id : Which channel to act upon
 * @param fwd : 1 if forward tone , 0 if backward tone
 * @param tone : Tone value in ASCII
 * @return 0 if success, -1 if error
 */
int libamg_dsp_mfcr2_start_tone(int conn_id, int fwd, char tone);

/**
 * Stop playing MFC/R2 tone in channel
 *
 * @param conn_id: Which channel to act upon
 * @return 0 if success, -1 if error
 */
int libamg_dsp_mfcr2_stop_tone(int conn_id);

/**
 * Initialize locks (mutex) for DSP channels access
 *
 * @return Always 0
 */
int libamg_dsp_channel_lock_init(void);

/**
 * Start playing DTMF tone in channel
 *
 * @param conn_id: Which channel to act upon
 * @param tone
 * @return 0 if success, negative if error
 */
int libamg_dsp_dtmf_start_tone(int conn_id, char tone);

/**
 * Stop playing DTMF tone in channel
 *
 * @param conn_id: Which channel to act upon
 * @return 0 if success, negative if error
 */
int libamg_dsp_dtmf_stop_tone(int conn_id);

/**
 * Enable autoswitch on channel
 *
 * @param conn_id: Which channel to act upon
 * @return 0 if success, negative if error
 */
int libamg_dsp_fax_autoswitch_set(int conn_id);

/**
 * Reset statistics for a certain connection
 *
 * @param conn_id: Connection in question
 * @return 0 if success, -1 if error
 */
int libamg_dsp_rtp_reset_channel_stats(int conn_id);

/**
 * Fetch number of received RTP packets in connection
 *
 * @param conn_id: Connection in question
 * @return Number of Rx packets or -1 if error
 */
int libamg_dsp_rtp_get_rxpkts(int conn_id);

#endif /* DSP_H_ */
