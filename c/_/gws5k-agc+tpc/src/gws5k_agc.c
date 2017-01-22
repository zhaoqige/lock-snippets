/*
 * gws5k_agc.c
 *
 *  Created on: Jul 27, 2016
 *  Updated on: Aug 6, 2016
 *  Updated on: Nov 13, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "six.h"

#include "grid.h"

#include "app.h"
#include "task.h"
#include "gws5k_ctrl.h"


#include "gws5k_agc.h"


static int gws5k_agc_standby(const void *task_env);
static int gws5k_agc_exec(const void *task_env, const int flag_range_control);

static int gws5k_agc_comm(const void *task_env);
static int gws5k_agc_comm_parse(const char *filter, const char *data);

static int gws5k_agc_cache_find(const char *key);
static int gws5k_agc_cache_audit(const int txpwr, const int rxgain);
static int gws5k_agc_cache_peerX(struct tpc_peer_cache *peerX);

static int gws5k_agc_adjust(const void *task_env);
static int gws5k_agc_report(const void *task_env);


// local cache of each peer
// 1. reset when offline;
// 2. reset in first exec.
static struct tpc_local_cache _local_cache;


/*
 * Tx Power Control (with AGC/RssiCal)
 * CASE I:      offline
 * CASE II:     online, 1 vs 1/N
 * 
 * Steps:
 * 1. split STANDBY & ONLINE;
 * 2.1 if STANDBY, set STANDBY;
 * 2.2 if ONLINE, exec:
 * 2.2.1 recv REPORT from peer;
 * 2.2.2 parse REPORT into local_cache;
 * 2.2.3 adjust txpwr/txmcs based on each peer's snr/rxgain;
 * 2.2.4 send REPORT to each peer.
 *
 * by Qige Zhao <qige@6harmonics.com> @ 2016.11.11
 *
 * 
 * Adjust Algorithm II (AGC+TPC):
 * conditions:
 * 1. very near (snr >= 45 && rxgain < -28):
 * 		free tx mcs;
 * 		turn remote txpower, step -6;
 *
 * 2. too far (snr <= 20 && rxgain >= -8):
 * 		set tx mcs = 1;
 *              turn remote txpower, step 9;
 */
int gws5k_agc_run(const void *task_env)
{
    int ret = AGC_OK;
    struct TASK_ENV *agc_env = (struct TASK_ENV *) task_env;

    int target_peer_qty;

    //gws5k_tpc_conf_check(task_env);

    DBG_TPC("%ld: bb.seq = %ld, hw.seq = %ld\n",
            _local_cache.seq,
            agc_env->data.bb.seq,
            agc_env->data.hw.seq);

    // min inactive < 4000 ms, avg snr > 0
    target_peer_qty = 1;
    switch(agc_env->data.bb.nl_mode) {
    case GWS_MODE_MESH:
    	target_peer_qty = 2;
    	break;
    case GWS_MODE_ARN_CAR:
    	target_peer_qty = GWS_CONF_BB_STA_MAX;
    	break;
    case GWS_MODE_ARN_EAR:
    default:
    	target_peer_qty = 1;
    	break;
    }

    if (agc_env->conf.algorithm > 0)
    	target_peer_qty = agc_env->conf.algorithm;

    // if peer qty not enough, go max txpwr;
    // if peer qty is enough, do adjust.
    if (agc_env->data.bb.peers_qty >= target_peer_qty) {
    	if (agc_env->data.bb.peers_qty > 0 &&
    			agc_env->data.bb.avg.signal > agc_env->data.bb.avg.noise)
		{
    		// AGC_ERR_CORE_EXEC;
    		ret = gws5k_agc_exec(task_env, 0);
		} else {
			//ret = AGC_ERR_CORE_STANDBY;
			ret = gws5k_agc_standby(task_env);
		}
    } else {
		//ret = AGC_ERR_CORE_RANGE_CONTROL;
		ret = gws5k_agc_exec(task_env, 1);
    }
    
    _local_cache.seq ++;
    
    return ret;
}


static int gws5k_agc_standby(const void *task_env)
{
    DBG_TPC("enter AGC_STANDBY\n");

    // clear peer cache
    memset(&_local_cache, 0, sizeof(struct tpc_local_cache));

    // reset local txpwr/txmcs/rxgain
    if (gws5k_ctrl_range_max(task_env))
        return AGC_ERR_CORE_STANDBY;
    
    return AGC_OK;
}

static int gws5k_agc_exec(const void *task_env, const int flag_range_control)
{
    DBG_TPC("enter AGC_EXEC\n");

    int ret = AGC_OK;
    struct TASK_ENV *agc_env = (struct TASK_ENV *) task_env;
    
    int seq;

    if (_local_cache.seq < 3) {
        seq = _local_cache.seq;
        memset(&_local_cache, 0, sizeof(struct tpc_local_cache));
        _local_cache.seq = seq;
    }

    if (gws5k_agc_comm(task_env)) {
        printf(" (warning) NO data received.\n");
        ret = AGC_ERR_BASE_COMM;
    }

    // adjust & send every X second(s)
    if (_local_cache.seq % agc_env->conf.intl == 0) {
        if (flag_range_control > 0) {
            // reset local txpwr/txmcs/rxgain
			gws5k_ctrl_range_max(task_env);
			printf(" (warning) range control OVERRIDE!\n");
			ret = AGC_ERR_CORE_RANGE_CONTROL;
        } else {
        	if (gws5k_agc_adjust(task_env)) {
				printf(" (warning) local cache all TIMEOUT!\n");
				ret = AGC_ERR_BASE_ADJUST;
			}
        }

    	if (gws5k_agc_report(task_env)) {
    		printf(" (warning) NO data sent.\n");
    		ret = AGC_ERR_BASE_REPORT;
    	}
    }
    
    // FIXME: there maybe not enough SNR to send/recv REPORT
    // turn txpower up
    //if (ret == AGC_ERR_BASE_REPORT) {
        //printf("AGC 4> UP txpower < comm/adjust/report FAILED!\n");
        //gws5k_set_txpower(agc_env->data.hw.txpower,
                //GWS5K_AGC_CONF_ADJ_STEP, agc_env->conf.txpwr_max);
    //}

    return ret;
}



// recv REPORT from socket
// call PARSE, save to _local_cache
static int gws5k_agc_comm(const void *task_env)
{
    int ret = AGC_OK;
    struct TASK_ENV *agc_env = (struct TASK_ENV *) task_env;

    int recv_bytes, recv_bytes_valid;
    char buffer[128];

    int addr_length;
    struct sockaddr_in addr;

    recv_bytes_valid = 0;
    do {
        recv_bytes = 0;
        memset(buffer, 0, sizeof(buffer));

        addr_length = sizeof(addr);
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(agc_env->comm.remote_port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        // non-blocking
        recv_bytes = recvfrom(agc_env->comm.sockfd, buffer, sizeof(buffer)-1, MSG_DONTWAIT,
            (struct sockaddr *) &addr, (size_t *) &addr_length);
        printf("AGC 4> recv %d bytes\n", recv_bytes);
        if (recv_bytes <= 0) {
            break;
        }

        // save peer to local_cache
        //sprintf(buffer, "AC:EE:3B:D0:02:17 AC:EE:3B:D0:00:26 55 33 -15");
        if (gws5k_agc_comm_parse(agc_env->wls_mac, buffer)) 
            continue;

        recv_bytes_valid += recv_bytes;
    } while(recv_bytes > 0);


    // check valid bytes
    if (recv_bytes_valid < 1) {
        ret = AGC_ERR_BASE_COMM;
    }
    
    return ret;
}

static int gws5k_agc_comm_parse(const char *filter, const char *data)
{
    int idx;
    int remote_snr, remote_rxgain, remote_txpwr;
    char dmac[18], smac[18];

    memset(dmac, 0, sizeof(dmac));
    memset(smac, 0, sizeof(smac));
    remote_snr = remote_rxgain = remote_txpwr = 0;
    if (sscanf(data, "%s %s %d %d %d", dmac, smac, &remote_snr, &remote_txpwr, &remote_rxgain) == -1) {
        return AGC_ERR_BASE_PARSE_FORMAT;
    }

    // check destination mac
    if (strstr(dmac, filter) > 0) {
        // TODO: save data to _local_cache
        // find mac in _local_cache
        // if FOUND, update; if NOT, save as new; if FULL, return ERR
        idx = gws5k_agc_cache_find(smac);
        printf("AGC 5> (remote %d to local) report: %s\n", idx, data);

        if (idx >= 0 && idx < GWS_CONF_BB_STA_MAX) {
            snprintf(_local_cache.peers[idx].mac, 18, "%s", smac);
            _local_cache.peers[idx].snr = remote_snr;
            _local_cache.peers[idx].txpwr = remote_txpwr;
            _local_cache.peers[idx].rxgain = remote_rxgain;
            _local_cache.peers[idx].seq = _local_cache.seq;            
        } else {
            return AGC_ERR_BASE_CACHE_FULL;
        }
    } else {
        return AGC_ERR_BASE_PARSE_TARGET;
    }

    return AGC_OK;
}

static int gws5k_agc_cache_find(const char *key)
{
    int i, idx_blank, idx;
    char peer_mac[18];
    
    // return blank index, or found index
    // 1. try new peer;
    // 2. try peer exists;
    // 3. try full peers.
    idx_blank = -1;
    memset(peer_mac, 0, sizeof(peer_mac));
    for(i = 0; i < GWS_CONF_BB_STA_MAX; i ++) {
        snprintf(peer_mac, sizeof(peer_mac), "%s", _local_cache.peers[i].mac);
        if (idx_blank < 0 && strlen(peer_mac) < 1) {
            idx_blank = i; // first blank record found
        }
        
        if (strstr(peer_mac, key) > 0) {
            break; // peer exists found
        }
    }

    idx = -1;
    if (i < GWS_CONF_BB_STA_MAX) { // peer pos
        idx = i;
    } else if (idx_blank >= 0) { // new peer pos
        idx = idx_blank;
    }
    
    return idx;
}

// wipe out inactive peer(s);
static int gws5k_agc_cache_audit(const int txpwr, const int rxgain)
{
    int i;
    
    for(i = 0; i < GWS_CONF_BB_STA_MAX; i ++) {
        if (_local_cache.seq - _local_cache.peers[i].seq >= GWS5K_AGC_CONF_TIMEOUT_BAR) {
            memset(&_local_cache.peers[i], 0, sizeof(struct tpc_peer_cache));
            continue;
        }

        if (_local_cache.peers[i].snr > 0)
        DBG_TPC("cache_audit -> peer%d: snr=%d, txpwr=%d, rxgain=%d\n",
                i, _local_cache.peers[i].snr, _local_cache.peers[i].txpwr,
                _local_cache.peers[i].rxgain);
    }
    
    return AGC_OK;
}

// find worst peer farthest
static int gws5k_agc_cache_peerX(struct tpc_peer_cache *peerX)
{
    int i, idx, snr_min;
    
    idx = -1;
    snr_min = -1;
    for(i = 0; i < GWS_CONF_BB_STA_MAX; i ++) {
        //DBG_TPC("cache_peerX (idx=%d) -> peer%d: snr=%d, txpwr=%d, rxgain=%d\n",
        //        idx, i, _local_cache.peers[i].snr, _local_cache.peers[i].txpwr,
        //        _local_cache.peers[i].rxgain);
        // verify ts, find min snr & cache peer mac
        if ((_local_cache.peers[i].snr > 0 && _local_cache.peers[i].snr < snr_min) ||
        		snr_min == -1)
        {
            snr_min = _local_cache.peers[i].snr;
            idx = i;
        }
    }
    
    if (idx < 0) {
        return AGC_ERR_BASE_CACHE_EMPTY;
    }
    
    memcpy(peerX, &_local_cache.peers[idx], sizeof(struct tpc_peer_cache));
    return AGC_OK;
}


// 1. audit _local_cache;
// 2. find min SNR, MAC;
// 3. adjust txpwr/txmcs/rxgain;
// 4. set _local_cache.
static int gws5k_agc_adjust(const void *task_env)
{
    int ret = AGC_OK;
    
    struct TASK_ENV *agc_env = (struct TASK_ENV *) task_env;

    //int i;
    int txpwr_step, txmcs_val;
    
    struct tpc_peer_cache weak_peer;
    char smac[18];

    // check peers' status
    gws5k_agc_cache_audit(agc_env->data.hw.txpower, agc_env->data.hw.rxgain);
    //if (gws5k_agc_cache_audit()) {
    //    ret = AGC_ERR_BASE_AUDIT;
    //    return ret;
    //}

    // 1. check adjust ts/interval;
    // 2. find minimum snr, inactive;
    // 3. according to policy, do adjust: set txpwr/txmcs
    txpwr_step = txmcs_val = 0;
    memset(&weak_peer, 0, sizeof(weak_peer));
    memset(smac, 0, sizeof(smac));
    snprintf(smac, sizeof(smac), "%s", agc_env->wls_mac);

    if (gws5k_agc_cache_peerX(&weak_peer)) 
        return AGC_ERR_BASE_CACHE_EMPTY;
    
    if (strlen(weak_peer.mac) < 1)
        return AGC_ERR_BASE_CACHE_EMPTY;

    DBG_TPC("!!! weak peer: snr=%d, txpwr=%d, rxgain=%d\n",
            weak_peer.snr, weak_peer.txpwr, weak_peer.rxgain);
    
    //+ offline or min SNR too low
    if (weak_peer.snr < 1 || weak_peer.snr >= 25) {
            txmcs_val = GWS5K_CONF_TXMCS_WHEN_READY;
    } else {
            txmcs_val = GWS5K_CONF_TXMCS_WHEN_FAR;
    }
    gws5k_set_mcs(txmcs_val);
    
    
    
    // when snr lower than 50, adjust local txpwr
    // according to remote rxgain
    // rxgain > -10, +9 dBm each time
    // rxgain <= -28 (-32+5), - 6 dBm each time.
    if (weak_peer.rxgain < -22 || weak_peer.snr > 45) {
        printf("AGC 5> (%d) Too STRONG!\n", weak_peer.rxgain);
        txpwr_step = 0 - GWS5K_AGC_CONF_ADJ_STEP * 3;
    } else if (weak_peer.rxgain >= -8) {
        printf("AGC 5> (%d) Too LOW!\n", weak_peer.rxgain);
        txpwr_step = GWS5K_AGC_CONF_ADJ_STEP * 3;
    } else {
        printf("AGC 5> (%d) Good! Local TxPwr = %d\n",
                weak_peer.rxgain, agc_env->data.hw.txpower);
        txpwr_step = 0;
    }

    //+ make sure turn up txpwr when remote snr is too low
    // failsafe for BAD AGC;
    if (weak_peer.snr < 20) {
        DBG_TPC("remote snr is too low, override (%d/%d)\n", 
                weak_peer.snr, txpwr_step);
        txpwr_step = GWS5K_AGC_CONF_ADJ_STEP * 3;
    } else if (weak_peer.snr > 45) {
        DBG_TPC("remote snr is too high, override (%d/%d)\n", 
                weak_peer.snr, txpwr_step);
        txpwr_step = 0 - GWS5K_AGC_CONF_ADJ_STEP * 3;
    }
    if (txpwr_step != 0) {
            gws5k_set_txpower(agc_env->data.hw.txpower, 
                    txpwr_step, agc_env->conf.txpwr_max);
    }

    printf("### Local: snr=%d/%d/%d, txpwr=%d (step %d) ###\n",
            agc_env->data.bb_shm->avg.signal, agc_env->data.bb_shm->avg.noise,
            agc_env->data.bb_shm->avg.signal - agc_env->data.bb_shm->avg.noise,
            agc_env->data.hw.txpower, txpwr_step);
    printf("### Remote: tx power=%d, rx snr=%d, rxgain=%d ####\n",
            weak_peer.txpwr,
            weak_peer.snr,
            weak_peer.rxgain);

    return ret;
}

static int gws5k_agc_report(const void *task_env)
{
    int i, snr[GWS_CONF_BB_STA_MAX], snr_min, snr_max;
    char data[64], remote[16];

    struct TASK_ENV *tpc_env = (struct TASK_ENV *) task_env;

    int sent_bytes;
    struct sockaddr_in addr;

    memset(&snr, 0, sizeof(snr));
    snr_min = 0;
    snr_max = 0;
    for(i = 0; i < GWS_CONF_BB_STA_MAX; i ++) {
        if (strlen(tpc_env->data.bb.peers[i].mac) > 0) {
            snr[i] = tpc_env->data.bb.peers[i].signal - tpc_env->data.bb.peers[i].noise;
            DBG_TPC(" peer%d: %s, %d/%d/%d, %d ms\n",
                    i, tpc_env->data.bb.peers[i].mac,
                    tpc_env->data.bb.peers[i].signal, tpc_env->data.bb.peers[i].noise,
                    snr[i],
                    tpc_env->data.bb.peers[i].inactive);

            //+ find ip address: "rarp <wmac>", need "+wrarpd +rarp"
            memset(data, 0, sizeof(data));
            memset(remote, 0, sizeof(remote));
            snprintf(data, sizeof(data)-1, GWS5K_RARP_FORMAT, tpc_env->data.bb.peers[i].mac);
            snprintf(remote, sizeof(remote)-1, "%s", cli_read_line(data));

            DBG_TPC(" peer%d: %s\n", i, remote);

            //+ send report
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(tpc_env->comm.remote_port);
            addr.sin_addr.s_addr = inet_addr(remote);
            if (addr.sin_addr.s_addr == INADDR_NONE) {
                return AGC_ERR_BASE_REPORT_SENDTO_TARGET;
            }

            memset(data, 0, sizeof(data));
            snprintf(data, sizeof(data)-1, "%s %s %d %d %d",
                    tpc_env->data.bb.peers[i].mac,
                    tpc_env->wls_mac,
                    snr[i],
                    tpc_env->data.hw.txpower,
                    tpc_env->data.hw.rxgain);

            printf("AGC 4> (local to remote) report: %s\n", data);
            sent_bytes = sendto(tpc_env->comm.sockfd, data, strlen(data),
                        0, (struct sockaddr *) &addr, sizeof(addr));
            if (sent_bytes <= 0) {
                printf(" (warning) send report to remote failed (%d/%s:%d)\n",
                        tpc_env->comm.sockfd, remote, tpc_env->comm.remote_port);
                return AGC_ERR_BASE_REPORT_SENDTO;
            }
            printf("AGC 3> sent %d bytes to remote %s:%d\n",
                    sent_bytes, remote, tpc_env->comm.remote_port);

            if (snr_min == 0 || snr[i] < snr_min) {
                snr_min = snr[i];
            }
            if (snr[i] > snr_max) {
                snr_max = snr[i];
            }
        }
        //break; // uncomment v10.0-39, 111116
    }

    return AGC_OK;
}



void gws5k_agc_idle(const int intl_sec)
{
	sleep(TASK_CONF_INTL_MIN);

	/*if (intl_sec < TASK_CONF_INTL_MIN) {
        sleep(TASK_CONF_INTL_MIN);
    } else {
        sleep(intl_sec);
    }*/

    //usleep(intl_ms * 1000);
}
