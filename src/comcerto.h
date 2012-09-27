/*
 * comcerto.h
 *
 *  Created on: Sep 5, 2012
 *      Author: Igor Kramer Pinotti
 */

#ifndef COMCERTO_H_
#define COMCERTO_H_

/*
 * About
 */
/*
	Arquivo 'comcerto_tdm.conf' para configurar
	alguns parâmetros (na interface TDM) que existem apenas no DSP e não no Asterisk (chan_dahdi.conf/system.conf).

	Esse arquivo eh escrito pelo daemon de configuração e lido pelos módulo Comcerto TDM do Asterisk.

	Parâmetros:

	[general] - HARDCODED_NEEDED_BY_ASTERISK

	Voice Active Detection (VAD)
	Ativado			=>	vad_enable=yes
	Desativado		=>	vad_enable=no

	VAD Tune
	Economia de banda máxima		=> vad_level= 0 (max)
	5dB SNR							=> vad_level= 5
	10dB SNR						=> vad_level= 10
	20dB SNR						=> vad_level= 20
	30dB SNR						=> vad_level= 30

	Confort Noise Generation
	Ativado			=>	cng=yes
	Desativado		=>	cng=no

	Echo Cancelling Tail Size
	de 8 a 128ms, em passos de 8ms		=>	ectail=128

	E1 Enable
	Ativado			=>	e1_enable=yes
	Desativado		=>	e1_enable=no

	E1 Loopback
	Ativado			=>	e1_loopback=yes
	Desativado		=>	e1_loopback=no

	Jitter Buffer Configs (Comcerto e Sip)
	Ativado			=>	jb_enable=yes (ALWAYS ON FOR COMCERTO)
	Tamanho maximo do Jitter Buffer em "ms"		=>	jb_maxsize=num
	Implementacao do Jitter buffer [fixed = 0 / adaptative = 1]		=>	jb_impl=fixed

	Jitter Buffer (Comcerto)
	Min Delay (0-200ms)				=>	jb_mindelay=num
	Typical Delay (0-200ms)			=>	jb_typdelay=num
	Max Delay (0-200ms)				=>	jb_maxdelay=num
	Deletion Threshold (0-500ms)	=>	jb_delet_thrld=num

*/

/*
 * General Defines
 */
#define FILE_COMCERTO_CONF		"/etc/asterisk/comcerto_tdm.conf"

/*
 * General Structures
 */

/* Jitter Buffer Config used in SIP_conf and Comcerto_conf*/
typedef struct libamg_jb_config {
	int jb_enable;
	int jb_maxsize;
	int jb_impl;
	int jb_mindelay;
	int jb_typdelay;
	int jb_maxdelay;
	int jb_delet_thrld;
} libamg_jb_config;

struct libamg_comcerto_config {
	int vad_enable;
	int vad_level;
	int cng_enable;
	int echocan_enable;
	int ectail;
	int txgain;
	int rxgain;
	int e1_enable;
	int e1_loopback_enable;
	libamg_jb_config jb_conf;
};

/*
 * Functions Declaration
 */

/**
 * libamg_comcerto_rtp_reload
 *
 * Reload the RTP module using the new configuration
 * from the comcerto configuration file
 *
 * @return 0 if success, negative if error
 */
int libamg_comcerto_rtp_reload(void);

/**
 * libamg_comcerto_parse_config
 *
 * Get configuration from Comcerto config file.
 *
 * @return struct libamg_comcerto_config *, NULL if error
 */
struct libamg_comcerto_config *libamg_comcerto_parse_config(void);

/**
 * libamg_comcerto_save_config
 *
 * Save configuration inside Comcerto's config file.
 *
 * @param conf
 * @return 0 if ok, -1 if error
 */
int libamg_comcerto_save_config(struct libamg_comcerto_config *conf);



#endif /* COMCERTO_H_ */
