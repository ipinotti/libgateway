/*
 * dsp.h
 *
 *  Created on: Oct 15, 2012
 *      Author: tgrande
 */

#ifndef DSP_H_
#define DSP_H_

#include <vapi/vapi.h>
#include <vapi/ut.h>
#include <vapi/gtl.h>

#include <asterisk.h>
#include <asterisk/module.h>
#include <asterisk/logger.h>
#include <asterisk/rtp_engine.h>

#include <syslog.h>

#define amg_err(x,...) syslog(LOG_ERR, x, ##__VA_ARGS__)
#define amg_log(x,...) syslog(LOG_INFO, x, ##__VA_ARGS__)
#define amg_dbg(x,...) syslog(LOG_DEBUG, x, ##__VA_ARGS__)

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
int libamg_dsp_set_dtmf_mode(int conn_id, SVoIPChnlParams *parms, enum ast_rtp_dtmf_mode dtmf_mode);


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


enum inband_signaling_t {
	INBAND_SIG_OFF = 		0x00,
	INBAND_SIG_MF_R2_BCK = 	0x01,
	INBAND_SIG_MF_R2_FWD = 	0x02,
};

/**
 * Set inband signaling mode
 *
 * This mode must be set if MF-R2 tones should be detected by the DSP.
 * Turn it off as soon as RTP traffic should be enabled.
 *
 * @param conn_id: Connection in question
 * @param parms: Pointer to channel parameters
 * @param mode: Signaling mode to be set
 * @return 0 if success, negative if error
 *
 */
int libamg_dsp_set_inband_signaling(int conn_id, SVoIPChnlParams *parms,
		enum inband_signaling_t mode);

/**
 * Set MF R2 timing definitions
 */
int libamg_dsp_set_mf_r2_timings(int conn_id);


#endif /* DSP_H_ */
