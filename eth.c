/*
 * 	eth.c
 * 
 * 2006 Copyright (c) Evgeniy Polyakov <johnpol@2ka.mipt.ru>
 * All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/poll.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#include "sys.h"

int eth_build_header(struct nc_buff *ncb)
{
	struct ether_header *eth;

	eth = ncb_push(ncb, sizeof(struct ether_header));
	if (!eth)
		return -ENOMEM;

	memcpy(eth->ether_dhost, ncb->dst->edst, ETH_ALEN);
	memcpy(eth->ether_shost, ncb->dst->esrc, ETH_ALEN);
	eth->ether_type = htons(ETH_P_IP);
	return 0;
}

int packet_eth_process(void *data, unsigned int size)
{
	struct nc_buff *ncb;
	struct ether_header *eth;
	int err = -ENOMEM;

	ncb = ncb_alloc(size);
	if (!ncb)
		return -ENOMEM;

	memcpy(ncb->head, data, size);

	eth = ncb_pull(ncb, sizeof(struct ether_header));
	if (!eth)
		goto err_out_free;

	if (ntohs(eth->ether_type) != ETH_P_IP) {
		err = -1;
		goto err_out_free;
	}

	err = packet_ip_process(ncb);
	if (err)
		goto err_out_free;

	return 0;

err_out_free:
	ncb_put(ncb);
	return err;
}
