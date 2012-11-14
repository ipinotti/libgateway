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
 * File name: dsp.c
 * Created on: Oct 15, 2012
 * Author: Thomas Del Grande <tgrande@pd3.com.br>
 *
 * Functions to configure the DSP core in the Comcerto device
 * using the VAPI library
 *
 * Note: Must be used by one process only, e.g. Asterisk
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

/* Base of tone list */
static struct tone_event_t base_tone_event = {
		.conn_id = -1,
		.tone = -1,
		.next = NULL,
};


/* Mutexes */
pthread_mutex_t tone_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t channel_mutex[32];

int __channel_lock(int conn_id)
{
	return pthread_mutex_lock(&channel_mutex[conn_id]);
}

int __channel_unlock(int conn_id)
{
	return pthread_mutex_unlock(&channel_mutex[conn_id]);
}

int libamg_dsp_channel_lock_init(void)
{
	int i;

	for (i = 0; i < 32; i++)
		pthread_mutex_init(&channel_mutex[i], NULL);

	return 0;
}

/**
 * Low-level function to send message to MSP to modify connection parameters
 *
 * @param conn_id 	: Connection to be modified
 * @param data 		: Message data
 * @return	0 if success, -1 if error
 */
static int __write_to_device(U16 conn_id, SMsg *data)
{
	VSTATUS status;

	U8 device_response[DEFAULT_FIFO_MAX_SIZE];
	U32 response_size = DEFAULT_FIFO_MAX_SIZE;

	/* Disable connection at first */
	status = VAPI_SetConnectionState(conn_id, eInactive, NULL);
	if (status != SUCCESS) {
		amg_err("(Channel %d) : Could not deactivate connection. Error %d\n",
				conn_id, status);
		return -1;
	}

	/* Send the new configuration */
	status = VAPI_SendConnectionMessage(conn_id, data, NULL,
			device_response, &response_size);
	if (status != SUCCESS) {
		amg_err("(Channel %d) : Could not send message to DSP. Error %d\n",
				conn_id, status);
		return -1;
	}

	/* Reactivate */
	status = VAPI_SetConnectionState(conn_id, eTdmActive, NULL);
	if (status != SUCCESS) {
		amg_err("(Channel %d) : Could not activate connection. Error %d\n",
				conn_id, status);
		return -1;
	}

	return 0;
}

/**
 * Low-level function to create message to be sent to MSP.
 * Uses _write_to_device() to actually send the message.
 *
 * @param conn_id  : Connection in question
 * @param opt_type : Type of message
 * @param opt	   : Payload
 * @return 0 if success, -1 if error
 */
static int _write_opts(int conn_id, unsigned short opt_type, void *opt, int opt_len)
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
	len += param_size = opt_len;

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

static int _write_vceopt(int conn_id, struct _VOIP_VCEOPT *opt)
{
	return _write_opts(conn_id, FC_VOIP_VCEOPT, (void *)opt, sizeof(struct _VOIP_VCEOPT));
}

static int _write_dtmfopt(int conn_id, struct _VOIP_DTMFOPT *opt)
{
	return _write_opts(conn_id, FC_VOIP_DTMFOPT, (void *)opt, sizeof(struct _VOIP_DTMFOPT));
}

static int _write_echocan_opt(int conn_id, struct _VOIP_ECHOCAN *opt)
{
	return _write_opts(conn_id, FC_VOIP_ECHOCAN, (void *)opt, sizeof(struct _VOIP_ECHOCAN));
}

static int _write_jb_opt(int conn_id, struct _VOIP_JBOPT *opt)
{
	return _write_opts(conn_id, FC_VOIP_JBOPT, (void *)opt, sizeof(struct _VOIP_JBOPT));
}

static int _write_dgain(int conn_id, struct _VOIP_DGAIN *opt)
{
	return _write_opts(conn_id, FC_VOIP_DGAIN, (void *)opt, sizeof(struct _VOIP_DGAIN));
}

static int _write_sigdet(int conn_id, struct _VOIP_SIGDET *opt)
{
	return _write_opts(conn_id, FC_VOIP_SIGDET, (void *)opt, sizeof(struct _VOIP_SIGDET));
}

static int _write_mfdpar(int conn_id, struct _VOIP_MFDPAR *opt)
{
	return _write_opts(conn_id, FC_VOIP_MFDPAR, (void *)opt, sizeof(struct _VOIP_MFDPAR));
}

static int _write_ptset(int conn_id, struct _VOIP_PTSET *opt)
{
	return _write_opts(conn_id, FC_VOIP_PTSET, (void *)opt, sizeof(struct _VOIP_PTSET));
}

static int _write_autoswitch(int conn_id, struct _SET_PASSTHRU_AUTOSWITCH *opt)
{
	return _write_opts(conn_id, FC_SET_PASSTHRU_AUTOSWITCH, (void *)opt, sizeof(struct _SET_PASSTHRU_AUTOSWITCH));
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

	amg_dbg("(Channel %d) %sabling echo cancelling\n", conn_id,
	        enable ? "En" : "Dis");

	if (enable)
		opt->param_4.bits.ecenb = VOIP_ECHOCAN_ECENB_ENABLE;
	else
		opt->param_4.bits.ecenb = VOIP_ECHOCAN_ECENB_DISABLE;

	return _write_echocan_opt(conn_id, opt);
}

int libamg_dsp_set_dtmf_mode(int conn_id, SVoIPChnlParams *parms, enum _dtmf_mode dtmf_mode)
{
	struct _VOIP_DTMFOPT *opt = &parms->stDtmfOpt;

	opt->param_4.bits.dtmf_voice = VOIP_DTMFOPT_DTMF_VOICE_DISABLE;
	opt->param_4.bits.dtmf_rtp = VOIP_DTMFOPT_DTMF_RTP_DISABLE;

	switch (dtmf_mode) {
	case DTMF_MODE_NONE:
		break;
	case DTMF_MODE_INBAND:
		opt->param_4.bits.dtmf_voice = VOIP_DTMFOPT_DTMF_VOICE_ENABLE;
		break;
	case DTMF_MODE_RFC2833:
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

int libamg_dsp_set_codec_payload_type(int conn_id, ECodecIndex codec, int pt)
{
	struct _VOIP_PTSET opt;

	amg_dbg("(Channel %d) Setting payload type to %d for codec %d\n", conn_id, pt, codec);

	opt.param_4.bits.pt_index = codec;
	opt.param_4.bits.pt_value = pt;

	return _write_ptset(conn_id, &opt);
}

/***********************************************************/
/******************* Tone queue ****************************/
/***********************************************************/

int libamg_dsp_queue_tone_event(struct tone_event_t *tone)
{
	struct tone_event_t *t, *tmp;

	t = malloc(sizeof(struct tone_event_t));
	if (t == NULL) {
		amg_err("No memory to alloc tone event\n");
		return -1;
	}

	t->conn_id = tone->conn_id;
	t->tone = tone->tone;
	t->next = NULL;

	pthread_mutex_lock(&tone_mutex);

	/* Go to end of list */
	for (tmp = &base_tone_event; tmp->next != NULL; tmp = tmp->next);

#ifdef QUEUE_DEBUG
	amg_log("QUEUE: base = %p\n", &base_tone_event);
	amg_log("QUEUE: tmp = %p\n", tmp);
	amg_log("QUEUE: t = %p\n", t);
#endif

	/* Insert tone */
	tmp->next = t;

	pthread_mutex_unlock(&tone_mutex);

	return 0;
}

int libamg_dsp_dequeue_tone_event(int conn_id)
{
	struct tone_event_t *t, *p;
	int tone = -1;

	pthread_mutex_lock(&tone_mutex);

	/* Travel through list */
	for (t = p = &base_tone_event; t != NULL; p = t, t = t->next) {

		/* Get the correct connection ID */
		if (conn_id != t->conn_id)
			continue;

		tone = t->tone; /* Store tone */
		p->next = t->next; /* Update queue */

		free(t); /* Free unqueued tone */

		break; /* Got what we came here for */
	}

	pthread_mutex_unlock(&tone_mutex);

#ifdef QUEUE_DEBUG
	if (tone != -1)
		amg_log("Dequeing tone %d for connection %d\n", tone, conn_id);
#endif

	return tone;
}

/*********************************************************/
/********************** MF R2 ****************************/
/*********************************************************/

int libamg_dsp_set_mf_r2_timings(int conn_id)
{
#if 1
	struct _VOIP_MFDPAR opt;

	opt.mf_selector = 1; /* R2 */

	/*
	 * FIXME Review these values
	 * Lab tests showed these values to be robust,
	 * but slow. Maybe we can lower this?
	 */

	opt.minimum_off_time = 300;
	opt.minimum_on_time = 300;
	opt.maximum_dropout_time = 200;

	if (_write_mfdpar(conn_id, &opt) < 0)
		return -1;

	return 0;
#else
	struct _VOIP_MFTUNE tune;

	tune.frequency_deviation = 12;
	tune.negative_twist = 100;
	tune.positive_twist = 100;
	tune.minimum_level_threshold = 350;
	tune.snr_threshold = 0;
	tune.param_9.bits.detection_delay = 3; /* Minimize false detections */
	tune.mf_selector = 1; /* R2 */

	return _write_mftune(conn_id, &tune);
#endif
}

int libamg_dsp_get_inband_signaling(int conn_id)
{
	SVoIPChnlParams *parms;

	if (VAPI_GetChnl_Info(conn_id, &parms) != SUCCESS)
		return -1;

	return parms->stSigdet.param_4.bits.mode;
}

int libamg_dsp_set_inband_signaling(int conn_id, enum inband_signaling_t mode)
{
	SVoIPChnlParams *parms;
	struct _VOIP_SIGDET *sigdet_opt;
	EConnOpMode conn_state;

	amg_dbg("(Channel %d): %sabling MFC R2 tone detection\n",
			conn_id, mode == INBAND_SIG_OFF ? "Dis" : "En");

	if (VAPI_GetChnl_Info(conn_id, &parms) != SUCCESS)
		return -1;

	sigdet_opt = &parms->stSigdet;
	sigdet_opt->param_4.bits.mode = mode;

	libamg_dsp_set_mf_r2_timings(conn_id);

	/*
	 * Must send SIGDET and VOPENA to enable/disable
	 * inband signaling detection
	 */
	if (_write_sigdet(conn_id, sigdet_opt) < 0) {
		amg_err("Error sending SIGDET for connection %d\n", conn_id);
		return -1;
	}

	if (mode == INBAND_SIG_OFF)
		conn_state = eTdmActive;
	else {
		conn_state = eInbandToneDetActive;
	}

	amg_dbg("Setting connection state to %s\n",
			conn_state == eInbandToneDetActive ? "Inband detection" : "TDM active");
	if (VAPI_SetConnectionState(conn_id, conn_state, NULL) < 0) {
		amg_err("(Channel %d): Error setting connection state to %s mode\n",
				conn_id, conn_state == eInbandToneDetActive ?
						"Inband detection" : "TDM active");
		return -1;
	}

	return 0;
}

static const char r2_mf_tone_codes[] = "1234567890BCDEF"; /* Borrowed from OpenR2 */
char libamg_dsp_mfcr2_tone_int_to_char(int tone)
{
	if (tone == 0xff)
		return 0;

	return r2_mf_tone_codes[tone-1];
}


struct tone_map_t {
	char tone;
	int id;
};

int libamg_dsp_mfcr2_start_tone(int conn_id, int fwd, char tone)
{
	int i, ret;
	int tone_id = -1;

	struct tone_map_t fmap[] = {
			{ '1', eMFCR2_FTONE_1 }, { '2', eMFCR2_FTONE_2 },
			{ '3', eMFCR2_FTONE_3 }, { '4', eMFCR2_FTONE_4 },
			{ '5', eMFCR2_FTONE_5 }, { '6', eMFCR2_FTONE_6 },
			{ '7', eMFCR2_FTONE_7 }, { '8', eMFCR2_FTONE_8 },
			{ '9', eMFCR2_FTONE_9 }, { '0', eMFCR2_FTONE_0 },
			{ 'B', eMFCR2_FTONE_B }, { 'C', eMFCR2_FTONE_C },
			{ 'D', eMFCR2_FTONE_D }, { 'E', eMFCR2_FTONE_E },
			{ 'F', eMFCR2_FTONE_F }, { '\0', 0 },
	};

	struct tone_map_t bmap[] = {
			{ '1', eMFCR2_BTONE_1 }, { '2',	eMFCR2_BTONE_2 },
			{ '3', eMFCR2_BTONE_3 }, { '4', eMFCR2_BTONE_4 },
			{ '5', eMFCR2_BTONE_5 }, { '6', eMFCR2_BTONE_6 },
			{ '7', eMFCR2_BTONE_7 }, { '8', eMFCR2_BTONE_8 },
			{ '9', eMFCR2_BTONE_9 }, { '0', eMFCR2_BTONE_0 },
			{ 'B', eMFCR2_BTONE_B }, { 'C', eMFCR2_BTONE_C },
			{ 'D', eMFCR2_BTONE_D }, { 'E', eMFCR2_BTONE_E },
			{ 'F', eMFCR2_BTONE_F }, { '\0', 0 },
	};

	struct tone_map_t *map = fwd ? fmap : bmap;

	for (i = 0; map[i].tone != '\0'; i++) {
		if (tone == map[i].tone) {
				tone_id =  map[i].id;
		}
	}

	if (tone_id < 0) {
		amg_err("(Channel %d) MFC/R2: Could not generate tone %c\n", conn_id, tone);
		return -1;
	}

	amg_dbg("(Channel %d) : Playing MFC/R2 %s tone %c\n",
			conn_id, fwd ? "Forward" : "Backward", tone);

	__channel_lock(conn_id);
	ret = VAPI_PlayTone(conn_id, tone_id, eDirToTDM, NULL, 0, NULL);
	__channel_unlock(conn_id);

	return ret;
}

int libamg_dsp_mfcr2_stop_tone(int conn_id)
{
	int ret;

	amg_dbg("(Channel %d) : Stopping MFC/R2 tone\n", conn_id);

	__channel_lock(conn_id);
	ret = VAPI_StopTone(conn_id, 0, eDirToTDM, NULL);
	__channel_unlock(conn_id);

	return ret;
}

/*********************************************************/
/********************** DTMF *****************************/
/*********************************************************/

int libamg_dsp_dtmf_start_tone(int conn_id, char tone)
{
	int i, ret;
	int tone_id = -1;

	struct tone_map_t map[] = {
			{ '1', eDTMFTONE_1 },
			{ '2', eDTMFTONE_2 },
			{ '3', eDTMFTONE_3 },
			{ '4', eDTMFTONE_4 },
			{ '5', eDTMFTONE_5 },
			{ '6', eDTMFTONE_6 },
			{ '7', eDTMFTONE_7 },
			{ '8', eDTMFTONE_8 },
			{ '9', eDTMFTONE_9 },
			{ '0', eDTMFTONE_0 },
			{ '*', eDTMFTONE_STAR },
			{ '#', eDTMFTONE_HASH },
			{ 'A', eDTMFTONE_A },
			{ 'B', eDTMFTONE_B },
			{ 'C', eDTMFTONE_C },
			{ 'D', eDTMFTONE_D },
			{ '\0', 0 },
	};

	amg_dbg("(Channel %d) : Playing DTMF tone %c\n", conn_id, tone);

	for (i = 0; map[i].tone != '\0'; i++) {
		if (tone == map[i].tone) {
				tone_id =  map[i].id;
		}
	}

	if (tone_id < 0) {
		amg_err("(Channel %d) DTMF: Could not generate tone %c\n", conn_id, tone);
		return -1;
	}

	__channel_lock(conn_id);
	ret = VAPI_PlayTone(conn_id, tone_id, eDirToTDM, NULL, 0, NULL);
	__channel_unlock(conn_id);

	return ret;
}

int libamg_dsp_dtmf_stop_tone(int conn_id)
{
	int ret;

	amg_dbg("(Channel %d) : Stopping DTMF tone\n", conn_id);

	__channel_lock(conn_id);
	ret = VAPI_StopTone(conn_id, 0, eDirToTDM, NULL);
	__channel_unlock(conn_id);

	return ret;
}

int libamg_dsp_fax_autoswitch_set(int conn_id)
{
	struct _SET_PASSTHRU_AUTOSWITCH opt;

	amg_dbg("(Channel %d) Enabling FAX autoswitch\n", conn_id);

	memset(&opt, 0, sizeof(opt));

	opt.param_4.bits.pt_autoswitch = 5; /* Fax Pass-through '101' */
	opt.param_4.bits.sw_ind = 1; /* Enable host indication */
	opt.param_4.bits.ptchng = 1; /* Autoswitch when VBD in RTP */
	opt.param_4.bits.rfc2833 = 1; /* Autoswitch when RFC2833 in RTP: Should we test dtmfmode? */

	opt.param_6.bits.delay = 60; /* default jitter buffer delay */

	return _write_autoswitch(conn_id, &opt);
}
