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

	Voice Active Detection (VAD)
	Ativado => vad_enable=yes
	Desativado => vad_enable=no

	VAD Tune
	Economia de banda máxima  => vad_level= max
	5dB SNR                             => vad_level= 5
	10dB SNR                            => vad_level= 10
	20dB SNR                            => vad_level= 20
	30dB SNR                            => vad_level= 30

	Confort Noise Generation
	Ativado      => cng=yes
	Desativado   => cng=no

	Echo Cancelling Tail Size
	de 8 a 128ms, em passos de 8ms         => ectail=128

	E1 Loopback
	Ativado       => loopback=yes
	Desativado  => loopback=no
*/

/*
 * General Defines
 */
#define FILE_COMCERTO_CONF		"/etc/comcerto_tdm.conf"

/*
 * General Structures
 */
struct libamg_comcerto_config {
	int vad_enable;
	int vad_level;
	int cng_enable;
	int ectail;
	int loopback_enable;
};

/*
 * Functions Declaration
 */

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
