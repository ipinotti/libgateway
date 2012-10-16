/*
 * dsp.c
 *
 *  Created on: Oct 15, 2012
 *      Author: tgrande
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <fcntl.h>

#include "dsp.h"

static int __write_to_device(U16 conn_id, SMsg *data)
{
	VSTATUS status;

	U8 device_response[DEFAULT_FIFO_MAX_SIZE];
	U32 response_size = DEFAULT_FIFO_MAX_SIZE;

	/* Disable connection at first */
	status = VAPI_SetConnectionState(conn_id, eInactive, NULL);
	if (status != SUCCESS) {
		amg_err("%d : Could not deactivate connection %d\n", status, conn_id);
		return -1;
	}

	/* Send the new configuration */
	status = VAPI_SendConnectionMessage(conn_id, data, NULL,
			device_response, &response_size);
	if (status != SUCCESS) {
		amg_err("%d : Could not send message to connection %d\n", status, conn_id);
		return -1;
	}

	/* Reactivate */
	status = VAPI_SetConnectionState(conn_id, eTdmActive, NULL);
	if (status != SUCCESS) {
		amg_err("%d : Could not activate connection %d\n", status, conn_id);
		return -1;
	}

	return 0;
}

static int _write_opts(int conn_id, unsigned short opt_type, void *opt)
{
	void *config;
	int len, status, param_size;

	/*
	 * Minimum packet length:
	 * The command headers (8 bytes each).
	 * The parameters of each commands.
	 * The potential inter commands padding.
	 * The terminator 4 bytes.
	 */
	len = sizeof(struct comcerto_api_hdr);
	len += PADDING_SIZE * 2; /* Someone figure this out, plz ? */

	switch (opt_type) {
	case FC_VOIP_VOPENA:
		len += param_size = sizeof(struct _VOIP_VOPENA);
		break;
	case FC_VOIP_VCEOPT:
		len += param_size = sizeof(struct _VOIP_VCEOPT);
		break;
	case FC_VOIP_DTMFOPT:
		len += param_size = sizeof(struct _VOIP_DTMFOPT);
		break;
	case FC_VOIP_ECHOCAN:
		len += param_size = sizeof(struct _VOIP_ECHOCAN);
		break;
	case FC_VOIP_JBOPT:
		len += param_size = sizeof(struct _VOIP_JBOPT);
		break;
	case FC_VOIP_DGAIN:
		len += param_size = sizeof(struct _VOIP_DGAIN);
		break;
	case FC_VOIP_SIGDET:
		len += param_size = sizeof(struct _VOIP_SIGDET);
		break;
	case FC_VOIP_MFDPAR:
		len += param_size = sizeof(struct _VOIP_MFDPAR);
		break;
	default:
		return -1;
	}

	config = VAPI_AllocateMessage(len);
	if (config == NULL)
		return -1;

	status = VAPI_SetMessageFromBuffer(config, /*message to be filled*/
			CMD_CLASS_CONF_CHANNEL,
			CMD_TYPE_CONF_CHANGE,
			opt_type,
	                param_size / 2, /* number of parameters = fifo size of this command divided by 2*/
	                (U16 *) opt); /* Voice option to be added to the config_30ms message*/

	if (status == SUCCESS)
		status = __write_to_device(conn_id, (SMsg *)config);

	VAPI_FreeMessage(config);

	return status;
}

/*********************************************************************************/

static int _write_vopena(int conn_id, struct _VOIP_VOPENA *opt)
{
	return _write_opts(conn_id, FC_VOIP_VOPENA, (void *)opt);
}

static int _write_vceopt(int conn_id, struct _VOIP_VCEOPT *opt)
{
	return _write_opts(conn_id, FC_VOIP_VCEOPT, (void *)opt);
}

static int _write_dtmfopt(int conn_id, struct _VOIP_DTMFOPT *opt)
{
	return _write_opts(conn_id, FC_VOIP_DTMFOPT, (void *)opt);
}

static int _write_echocan_opt(int conn_id, struct _VOIP_ECHOCAN *opt)
{
	return _write_opts(conn_id, FC_VOIP_ECHOCAN, (void *)opt);
}

static int _write_jb_opt(int conn_id, struct _VOIP_JBOPT *opt)
{
	return _write_opts(conn_id, FC_VOIP_JBOPT, (void *)opt);
}

static int _write_dgain(int conn_id, struct _VOIP_DGAIN *opt)
{
	return _write_opts(conn_id, FC_VOIP_DGAIN, (void *)opt);
}

static int _write_sigdet(int conn_id, struct _VOIP_SIGDET *opt)
{
	return _write_opts(conn_id, FC_VOIP_SIGDET, (void *)opt);
}

static int _write_mfdpar(int conn_id, struct _VOIP_MFDPAR *opt)
{
	return _write_opts(conn_id, FC_VOIP_MFDPAR, (void *)opt);
}



/****************************************************/
/****************************************************/
/****************************************************/
/************* EXPORTED FUNCTIONS *******************/
/****************************************************/
/****************************************************/
/****************************************************/


int libamg_dsp_set_vad(int conn_id, SVoIPChnlParams *parms, int enable, int tune)
{
	struct _VOIP_VCEOPT *opt = &parms->stVoiceOpt;

	if (tune < 0 || tune > 4)
		return -1;

	if (enable)
		opt->param_4.bits.vadtype = VOIP_VCEOPT_VADTYPE_ENABLE;
	else
		opt->param_4.bits.vadtype = VOIP_VCEOPT_VADTYPE_DISABLE;

	opt->vad_tune = tune; /* 0 - 4 */

	return _write_vceopt(conn_id, opt);
}

int libamg_dsp_set_g723_rate(int conn_id, SVoIPChnlParams *parms, int high)
{
	struct _VOIP_VCEOPT *opt = &parms->stVoiceOpt;

	if (high)
		opt->param_4.bits.g_723_rate = VOIP_VCEOPT_G_723_RATE_HIGH; /* 6.3 */
	else
		opt->param_4.bits.g_723_rate = VOIP_VCEOPT_G_723_RATE_LOW; /* 5.3 */

	return _write_vceopt(conn_id, opt);
}

int libamg_dsp_set_gain(int conn_id, SVoIPChnlParams *parms, int txgain, int rxgain)
{
	struct _VOIP_DGAIN *opt = &parms->stDgain;

	opt->pcm_to_packet_gain = txgain;
	opt->packet_to_pcm_gain = rxgain;

	return _write_dgain(conn_id, opt);
}

int libamg_dsp_set_cng(int conn_id, SVoIPChnlParams *parms, int enable)
{
	struct _VOIP_VCEOPT *opt = &parms->stVoiceOpt;

	if (enable)
		opt->param_4.bits.cng = VOIP_VCEOPT_CNG_ENABLE;
	else
		opt->param_4.bits.cng = VOIP_VCEOPT_CNG_DISABLE;

	return _write_vceopt(conn_id, opt);
}


int libamg_dsp_set_echocan(int conn_id, SVoIPChnlParams *parms, int enable, int tail_size)
{
	struct _VOIP_ECHOCAN *opt = &parms->stEchoCan;

	amg_dbg("%sabling echo cancelling in connection %d\n",
	        enable ? "En" : "Dis", conn_id);

	if (enable)
		opt->param_4.bits.ecenb = VOIP_ECHOCAN_ECENB_ENABLE;
	else
		opt->param_4.bits.ecenb = VOIP_ECHOCAN_ECENB_DISABLE;

	return _write_echocan_opt(conn_id, opt);
}

int libamg_dsp_set_dtmf_mode(int conn_id, SVoIPChnlParams *parms, enum ast_rtp_dtmf_mode dtmf_mode)
{
	struct _VOIP_DTMFOPT *opt = &parms->stDtmfOpt;

	opt->param_4.bits.dtmf_voice = VOIP_DTMFOPT_DTMF_VOICE_DISABLE;
	opt->param_4.bits.dtmf_rtp = VOIP_DTMFOPT_DTMF_RTP_DISABLE;

	switch (dtmf_mode) {
	case AST_RTP_DTMF_MODE_NONE:
		break;
	case AST_RTP_DTMF_MODE_INBAND:
		opt->param_4.bits.dtmf_voice = VOIP_DTMFOPT_DTMF_VOICE_ENABLE;
		break;
	case AST_RTP_DTMF_MODE_RFC2833:
	default:
		opt->param_4.bits.dtmf_rtp = VOIP_DTMFOPT_DTMF_RTP_ENABLE;
		break;
	}

	return _write_dtmfopt(conn_id, opt);
}

int libamg_dsp_set_jitter_buffer(int conn_id, SVoIPChnlParams *parms, struct jb_params *jb)
{
	struct _VOIP_JBOPT *opt = &parms->stJbopt;

	opt->delay_init = jb->jbinit;
	opt->delay_max = jb->jbmax;
	opt->delay_min = jb->jbmin;
	opt->deletion_threshold = jb->deletion_threshold;

	return _write_jb_opt(conn_id, opt);
}

int libamg_dsp_set_inband_signaling(int conn_id, SVoIPChnlParams *parms,
		enum inband_signaling_t mode)
{
	struct _VOIP_SIGDET *sigdet_opt = &parms->stSigdet;
	EConnOpMode conn_state;

	sigdet_opt->param_4.bits.mode = mode;

	/*
	 * Must send SIGDET and VOPENA to enable/disable
	 * inband signaling detection
	 */
	if (_write_sigdet(conn_id, sigdet_opt) < 0) {
		amg_err("Error sending SIGDET for connection %d\n", conn_id);
		return -1;
	}

	libamg_dsp_set_mf_r2_timings(conn_id);

	if (mode == INBAND_SIG_OFF)
		conn_state = eInbandToneDetActive;
	else
		conn_state = eTdmActive;

	if (VAPI_SetConnectionState(conn_id, eInbandToneDetActive, NULL) < 0) {
		amg_err("Conn %d: Error setting connection state to eInbandToneDetActive\n", conn_id);
		return -1;
	}

	return 0;
}

int libamg_dsp_set_mf_r2_timings(int conn_id)
{
	struct _VOIP_MFDPAR opt;

	opt.mf_selector = 1; /* R2 */

	/* FIXME Review these values */
	opt.minimum_off_time = 100;
	opt.minimum_on_time = 100;
	opt.maximum_dropout_time = 30;

	return _write_mfdpar(conn_id, &opt);
}
