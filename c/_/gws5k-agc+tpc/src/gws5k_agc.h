/*
 * gws5k_agc.h
 *
 *  Created on: Jul 27, 2016
 *  Updated on: 2016.11.11/v10.0.111116
 *      Author: Qige Zhao
 */

#ifndef GWS5K_AGC_H_
#define GWS5K_AGC_H_

enum GWS5K_AGC_ERR {
    AGC_OK = 0,
    AGC_ERR_BASE_REPORT = -100,
    AGC_ERR_BASE_REPORT_SENDTO,
    AGC_ERR_BASE_REPORT_SENDTO_TARGET,
    AGC_ERR_BASE_COMM,
    AGC_ERR_BASE_PARSE,
    AGC_ERR_BASE_PARSE_FORMAT,
    AGC_ERR_BASE_PARSE_TARGET,
    AGC_ERR_BASE_CACHE_EMPTY,
    AGC_ERR_BASE_CACHE_FULL,
    AGC_ERR_BASE_AUDIT,
    AGC_ERR_BASE_ADJUST,
    AGC_ERR_CORE_STANDBY = -200,
    AGC_ERR_CORE_RUN,
    AGC_ERR_CORE_RANGE_CONTROL,
};


#define GWS5K_RARP_FORMAT			"rarp %s\n"

#define GWS5K_CONF_TXMCS_WHEN_READY	99
#define GWS5K_CONF_TXMCS_WHEN_FAR	1
#define GWS5K_AGC_CONF_ADJ_STEP		3
#define GWS5K_AGC_CONF_TIMEOUT_BAR      5


struct tpc_peer_cache {
    char mac[18];
    int  snr;
    int  rxgain;
    int  txpwr;
    int  seq;
};

struct tpc_local_cache {
    unsigned long seq;
    struct {
        int txpwr;
        int txpwr_step;
        int txmcs;
        int rxgain;
    } last;
    struct tpc_peer_cache peers[GWS_CONF_BB_STA_MAX];
};


int  gws5k_agc_run(const void *task_env);
void gws5k_agc_idle(const int intl_sec);

#endif /* GWS5K_AGC_H_ */
