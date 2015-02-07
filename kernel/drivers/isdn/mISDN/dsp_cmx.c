



/* delay.h is required for hw_lock.h */

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mISDNif.h>
#include <linux/mISDNdsp.h>
#include "core.h"
#include "dsp.h"

/* #define CMX_CONF_DEBUG */

/*#define CMX_DEBUG * massive read/write pointer output */
/*#define CMX_DELAY_DEBUG * gives rx-buffer delay overview */
/*#define CMX_TX_DEBUG * massive read/write on tx-buffer with content */

static inline int
count_list_member(struct list_head *head)
{
	int			cnt = 0;
	struct list_head	*m;

	list_for_each(m, head)
		cnt++;
	return cnt;
}

void
dsp_cmx_debug(struct dsp *dsp)
{
	struct dsp_conf	*conf;
	struct dsp_conf_member	*member;
	struct dsp		*odsp;

	printk(KERN_DEBUG "-----Current DSP\n");
	list_for_each_entry(odsp, &dsp_ilist, list) {
		printk(KERN_DEBUG "* %s hardecho=%d softecho=%d txmix=%d",
		    odsp->name, odsp->echo.hardware, odsp->echo.software,
		    odsp->tx_mix);
		if (odsp->conf)
			printk(" (Conf %d)", odsp->conf->id);
		if (dsp == odsp)
			printk(" *this*");
		printk("\n");
	}
	printk(KERN_DEBUG "-----Current Conf:\n");
	list_for_each_entry(conf, &conf_ilist, list) {
		printk(KERN_DEBUG "* Conf %d (%p)\n", conf->id, conf);
		list_for_each_entry(member, &conf->mlist, list) {
			printk(KERN_DEBUG
			    "  - member = %s (slot_tx %d, bank_tx %d, "
			    "slot_rx %d, bank_rx %d hfc_conf %d "
			    "tx_data %d rx_is_off %d)%s\n",
			    member->dsp->name, member->dsp->pcm_slot_tx,
			    member->dsp->pcm_bank_tx, member->dsp->pcm_slot_rx,
			    member->dsp->pcm_bank_rx, member->dsp->hfc_conf,
			    member->dsp->tx_data, member->dsp->rx_is_off,
			    (member->dsp == dsp) ? " *this*" : "");
		}
	}
	printk(KERN_DEBUG "-----end\n");
}

static struct dsp_conf *
dsp_cmx_search_conf(u32 id)
{
	struct dsp_conf *conf;

	if (!id) {
		printk(KERN_WARNING "%s: conference ID is 0.\n", __func__);
		return NULL;
	}

	/* search conference */
	list_for_each_entry(conf, &conf_ilist, list)
		if (conf->id == id)
			return conf;

	return NULL;
}


static int
dsp_cmx_add_conf_member(struct dsp *dsp, struct dsp_conf *conf)
{
	struct dsp_conf_member *member;

	if (!conf || !dsp) {
		printk(KERN_WARNING "%s: conf or dsp is 0.\n", __func__);
		return -EINVAL;
	}
	if (dsp->member) {
		printk(KERN_WARNING "%s: dsp is already member in a conf.\n",
			__func__);
		return -EINVAL;
	}

	if (dsp->conf) {
		printk(KERN_WARNING "%s: dsp is already in a conf.\n",
			__func__);
		return -EINVAL;
	}

	member = kzalloc(sizeof(struct dsp_conf_member), GFP_ATOMIC);
	if (!member) {
		printk(KERN_ERR "kzalloc struct dsp_conf_member failed\n");
		return -ENOMEM;
	}
	member->dsp = dsp;
	/* clear rx buffer */
	memset(dsp->rx_buff, dsp_silence, sizeof(dsp->rx_buff));
	dsp->rx_init = 1; /* rx_W and rx_R will be adjusted on first frame */
	dsp->rx_W = 0;
	dsp->rx_R = 0;

	list_add_tail(&member->list, &conf->mlist);

	dsp->conf = conf;
	dsp->member = member;

	return 0;
}


int
dsp_cmx_del_conf_member(struct dsp *dsp)
{
	struct dsp_conf_member *member;

	if (!dsp) {
		printk(KERN_WARNING "%s: dsp is 0.\n",
			__func__);
		return -EINVAL;
	}

	if (!dsp->conf) {
		printk(KERN_WARNING "%s: dsp is not in a conf.\n",
			__func__);
		return -EINVAL;
	}

	if (list_empty(&dsp->conf->mlist)) {
		printk(KERN_WARNING "%s: dsp has linked an empty conf.\n",
			__func__);
		return -EINVAL;
	}

	/* find us in conf */
	list_for_each_entry(member, &dsp->conf->mlist, list) {
		if (member->dsp == dsp) {
			list_del(&member->list);
			dsp->conf = NULL;
			dsp->member = NULL;
			kfree(member);
			return 0;
		}
	}
	printk(KERN_WARNING
	    "%s: dsp is not present in its own conf_meber list.\n",
	    __func__);

	return -EINVAL;
}


static struct dsp_conf
*dsp_cmx_new_conf(u32 id)
{
	struct dsp_conf *conf;

	if (!id) {
		printk(KERN_WARNING "%s: id is 0.\n",
		    __func__);
		return NULL;
	}

	conf = kzalloc(sizeof(struct dsp_conf), GFP_ATOMIC);
	if (!conf) {
		printk(KERN_ERR "kzalloc struct dsp_conf failed\n");
		return NULL;
	}
	INIT_LIST_HEAD(&conf->mlist);
	conf->id = id;

	list_add_tail(&conf->list, &conf_ilist);

	return conf;
}


int
dsp_cmx_del_conf(struct dsp_conf *conf)
{
	if (!conf) {
		printk(KERN_WARNING "%s: conf is null.\n",
		    __func__);
		return -EINVAL;
	}

	if (!list_empty(&conf->mlist)) {
		printk(KERN_WARNING "%s: conf not empty.\n",
		    __func__);
		return -EINVAL;
	}
	list_del(&conf->list);
	kfree(conf);

	return 0;
}


static void
dsp_cmx_hw_message(struct dsp *dsp, u32 message, u32 param1, u32 param2,
    u32 param3, u32 param4)
{
	struct mISDN_ctrl_req cq;

	memset(&cq, 0, sizeof(cq));
	cq.op = message;
	cq.p1 = param1 | (param2 << 8);
	cq.p2 = param3 | (param4 << 8);
	if (dsp->ch.peer)
		dsp->ch.peer->ctrl(dsp->ch.peer, CONTROL_CHANNEL, &cq);
}


void
dsp_cmx_hardware(struct dsp_conf *conf, struct dsp *dsp)
{
	struct dsp_conf_member	*member, *nextm;
	struct dsp		*finddsp;
	int		memb = 0, i, ii, i1, i2;
	int		freeunits[8];
	u_char		freeslots[256];
	int		same_hfc = -1, same_pcm = -1, current_conf = -1,
	    all_conf = 1, tx_data = 0;

	/* dsp gets updated (no conf) */
	if (!conf) {
		if (!dsp)
			return;
		if (dsp_debug & DEBUG_DSP_CMX)
			printk(KERN_DEBUG "%s checking dsp %s\n",
			    __func__, dsp->name);
one_member:
		/* remove HFC conference if enabled */
		if (dsp->hfc_conf >= 0) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s removing %s from HFC conf %d "
				    "because dsp is split\n", __func__,
				    dsp->name, dsp->hfc_conf);
			dsp_cmx_hw_message(dsp, MISDN_CTRL_HFC_CONF_SPLIT,
			    0, 0, 0, 0);
			dsp->hfc_conf = -1;
		}
		/* process hw echo */
		if (dsp->features.pcm_banks < 1)
			return;
		if (!dsp->echo.software && !dsp->echo.hardware) {
			/* NO ECHO: remove PCM slot if assigned */
			if (dsp->pcm_slot_tx >= 0 || dsp->pcm_slot_rx >= 0) {
				if (dsp_debug & DEBUG_DSP_CMX)
					printk(KERN_DEBUG "%s removing %s from"
					    " PCM slot %d (TX) %d (RX) because"
					    " dsp is split (no echo)\n",
					    __func__, dsp->name,
					    dsp->pcm_slot_tx, dsp->pcm_slot_rx);
				dsp_cmx_hw_message(dsp, MISDN_CTRL_HFC_PCM_DISC,
				    0, 0, 0, 0);
				dsp->pcm_slot_tx = -1;
				dsp->pcm_bank_tx = -1;
				dsp->pcm_slot_rx = -1;
				dsp->pcm_bank_rx = -1;
			}
			return;
		}
		/* echo is enabled, find out if we use soft or hardware */
		dsp->echo.software = dsp->tx_data;
		dsp->echo.hardware = 0;
		/* ECHO: already echo */
		if (dsp->pcm_slot_tx >= 0 && dsp->pcm_slot_rx < 0 &&
		    dsp->pcm_bank_tx == 2 && dsp->pcm_bank_rx == 2) {
			dsp->echo.hardware = 1;
			return;
		}
		/* ECHO: if slot already assigned */
		if (dsp->pcm_slot_tx >= 0) {
			dsp->pcm_slot_rx = dsp->pcm_slot_tx;
			dsp->pcm_bank_tx = 2; /* 2 means loop */
			dsp->pcm_bank_rx = 2;
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s refresh %s for echo using slot %d\n",
				    __func__, dsp->name,
				    dsp->pcm_slot_tx);
			dsp_cmx_hw_message(dsp, MISDN_CTRL_HFC_PCM_CONN,
			    dsp->pcm_slot_tx, 2, dsp->pcm_slot_rx, 2);
			dsp->echo.hardware = 1;
			return;
		}
		/* ECHO: find slot */
		dsp->pcm_slot_tx = -1;
		dsp->pcm_slot_rx = -1;
		memset(freeslots, 1, sizeof(freeslots));
		list_for_each_entry(finddsp, &dsp_ilist, list) {
			if (finddsp->features.pcm_id == dsp->features.pcm_id) {
				if (finddsp->pcm_slot_rx >= 0 &&
				    finddsp->pcm_slot_rx < sizeof(freeslots))
					freeslots[finddsp->pcm_slot_rx] = 0;
				if (finddsp->pcm_slot_tx >= 0 &&
				    finddsp->pcm_slot_tx < sizeof(freeslots))
					freeslots[finddsp->pcm_slot_tx] = 0;
			}
		}
		i = 0;
		ii = dsp->features.pcm_slots;
		while (i < ii) {
			if (freeslots[i])
				break;
			i++;
		}
		if (i == ii) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s no slot available for echo\n",
				    __func__);
			/* no more slots available */
			dsp->echo.software = 1;
			return;
		}
		/* assign free slot */
		dsp->pcm_slot_tx = i;
		dsp->pcm_slot_rx = i;
		dsp->pcm_bank_tx = 2; /* loop */
		dsp->pcm_bank_rx = 2;
		if (dsp_debug & DEBUG_DSP_CMX)
			printk(KERN_DEBUG
			    "%s assign echo for %s using slot %d\n",
			    __func__, dsp->name, dsp->pcm_slot_tx);
		dsp_cmx_hw_message(dsp, MISDN_CTRL_HFC_PCM_CONN,
		    dsp->pcm_slot_tx, 2, dsp->pcm_slot_rx, 2);
		dsp->echo.hardware = 1;
		return;
	}

	/* conf gets updated (all members) */
	if (dsp_debug & DEBUG_DSP_CMX)
		printk(KERN_DEBUG "%s checking conference %d\n",
		    __func__, conf->id);

	if (list_empty(&conf->mlist)) {
		printk(KERN_ERR "%s: conference whithout members\n",
		    __func__);
		return;
	}
	member = list_entry(conf->mlist.next, struct dsp_conf_member, list);
	same_hfc = member->dsp->features.hfc_id;
	same_pcm = member->dsp->features.pcm_id;
	/* check all members in our conference */
	list_for_each_entry(member, &conf->mlist, list) {
		/* check if member uses mixing */
		if (member->dsp->tx_mix) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s dsp %s cannot form a conf, because "
				    "tx_mix is turned on\n", __func__,
				    member->dsp->name);
conf_software:
			list_for_each_entry(member, &conf->mlist, list) {
				dsp = member->dsp;
				/* remove HFC conference if enabled */
				if (dsp->hfc_conf >= 0) {
					if (dsp_debug & DEBUG_DSP_CMX)
						printk(KERN_DEBUG
						    "%s removing %s from HFC "
						    "conf %d because not "
						    "possible with hardware\n",
						    __func__,
						    dsp->name,
						    dsp->hfc_conf);
					dsp_cmx_hw_message(dsp,
					    MISDN_CTRL_HFC_CONF_SPLIT,
					    0, 0, 0, 0);
					dsp->hfc_conf = -1;
				}
				/* remove PCM slot if assigned */
				if (dsp->pcm_slot_tx >= 0 ||
				    dsp->pcm_slot_rx >= 0) {
					if (dsp_debug & DEBUG_DSP_CMX)
						printk(KERN_DEBUG "%s removing "
						    "%s from PCM slot %d (TX)"
						    " slot %d (RX) because not"
						    " possible with hardware\n",
						    __func__,
						    dsp->name,
						    dsp->pcm_slot_tx,
						    dsp->pcm_slot_rx);
					dsp_cmx_hw_message(dsp,
					    MISDN_CTRL_HFC_PCM_DISC,
					    0, 0, 0, 0);
					dsp->pcm_slot_tx = -1;
					dsp->pcm_bank_tx = -1;
					dsp->pcm_slot_rx = -1;
					dsp->pcm_bank_rx = -1;
				}
			}
			conf->hardware = 0;
			conf->software = 1;
			return;
		}
		/* check if member has echo turned on */
		if (member->dsp->echo.hardware || member->dsp->echo.software) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s dsp %s cannot form a conf, because "
				    "echo is turned on\n", __func__,
				    member->dsp->name);
			goto conf_software;
		}
		/* check if member has tx_mix turned on */
		if (member->dsp->tx_mix) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s dsp %s cannot form a conf, because "
				    "tx_mix is turned on\n",
				    __func__, member->dsp->name);
			goto conf_software;
		}
		/* check if member changes volume at an not suppoted level */
		if (member->dsp->tx_volume) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s dsp %s cannot form a conf, because "
				    "tx_volume is changed\n",
				    __func__, member->dsp->name);
			goto conf_software;
		}
		if (member->dsp->rx_volume) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s dsp %s cannot form a conf, because "
				    "rx_volume is changed\n",
				    __func__, member->dsp->name);
			goto conf_software;
		}
		/* check if tx-data turned on */
		if (member->dsp->tx_data) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s dsp %s tx_data is turned on\n",
				    __func__, member->dsp->name);
			tx_data = 1;
		}
		/* check if pipeline exists */
		if (member->dsp->pipeline.inuse) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s dsp %s cannot form a conf, because "
				    "pipeline exists\n", __func__,
				    member->dsp->name);
			goto conf_software;
		}
		/* check if encryption is enabled */
		if (member->dsp->bf_enable) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG "%s dsp %s cannot form a "
				    "conf, because encryption is enabled\n",
				    __func__, member->dsp->name);
			goto conf_software;
		}
		/* check if member is on a card with PCM support */
		if (member->dsp->features.pcm_id < 0) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s dsp %s cannot form a conf, because "
				    "dsp has no PCM bus\n",
				    __func__, member->dsp->name);
			goto conf_software;
		}
		/* check if relations are on the same PCM bus */
		if (member->dsp->features.pcm_id != same_pcm) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s dsp %s cannot form a conf, because "
				    "dsp is on a different PCM bus than the "
				    "first dsp\n",
				    __func__, member->dsp->name);
			goto conf_software;
		}
		/* determine if members are on the same hfc chip */
		if (same_hfc != member->dsp->features.hfc_id)
			same_hfc = -1;
		/* if there are members already in a conference */
		if (current_conf < 0 && member->dsp->hfc_conf >= 0)
			current_conf = member->dsp->hfc_conf;
		/* if any member is not in a conference */
		if (member->dsp->hfc_conf < 0)
			all_conf = 0;

		memb++;
	}

	/* if no member, this is an error */
	if (memb < 1)
		return;

	/* one member */
	if (memb == 1) {
		if (dsp_debug & DEBUG_DSP_CMX)
			printk(KERN_DEBUG
			    "%s conf %d cannot form a HW conference, "
			    "because dsp is alone\n", __func__, conf->id);
		conf->hardware = 0;
		conf->software = 0;
		member = list_entry(conf->mlist.next, struct dsp_conf_member,
			list);
		dsp = member->dsp;
		goto one_member;
	}

	/*
	 * ok, now we are sure that all members are on the same pcm.
	 * now we will see if we have only two members, so we can do
	 * crossconnections, which don't have any limitations.
	 */

	/* if we have only two members */
	if (memb == 2) {
		member = list_entry(conf->mlist.next, struct dsp_conf_member,
			list);
		nextm = list_entry(member->list.next, struct dsp_conf_member,
			list);
		/* remove HFC conference if enabled */
		if (member->dsp->hfc_conf >= 0) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s removing %s from HFC conf %d because "
				    "two parties require only a PCM slot\n",
				    __func__, member->dsp->name,
				    member->dsp->hfc_conf);
			dsp_cmx_hw_message(member->dsp,
			    MISDN_CTRL_HFC_CONF_SPLIT, 0, 0, 0, 0);
			member->dsp->hfc_conf = -1;
		}
		if (nextm->dsp->hfc_conf >= 0) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s removing %s from HFC conf %d because "
				    "two parties require only a PCM slot\n",
				    __func__, nextm->dsp->name,
				    nextm->dsp->hfc_conf);
			dsp_cmx_hw_message(nextm->dsp,
			    MISDN_CTRL_HFC_CONF_SPLIT, 0, 0, 0, 0);
			nextm->dsp->hfc_conf = -1;
		}
		/* if members have two banks (and not on the same chip) */
		if (member->dsp->features.pcm_banks > 1 &&
		    nextm->dsp->features.pcm_banks > 1 &&
		    member->dsp->features.hfc_id !=
		    nextm->dsp->features.hfc_id) {
			/* if both members have same slots with crossed banks */
			if (member->dsp->pcm_slot_tx >= 0 &&
			    member->dsp->pcm_slot_rx >= 0 &&
			    nextm->dsp->pcm_slot_tx >= 0 &&
			    nextm->dsp->pcm_slot_rx >= 0 &&
			    nextm->dsp->pcm_slot_tx ==
			    member->dsp->pcm_slot_rx &&
			    nextm->dsp->pcm_slot_rx ==
			    member->dsp->pcm_slot_tx &&
			    nextm->dsp->pcm_slot_tx ==
			    member->dsp->pcm_slot_tx &&
			    member->dsp->pcm_bank_tx !=
			    member->dsp->pcm_bank_rx &&
			    nextm->dsp->pcm_bank_tx !=
			    nextm->dsp->pcm_bank_rx) {
				/* all members have same slot */
				if (dsp_debug & DEBUG_DSP_CMX)
					printk(KERN_DEBUG
					    "%s dsp %s & %s stay joined on "
					    "PCM slot %d bank %d (TX) bank %d "
					    "(RX) (on different chips)\n",
					    __func__,
					    member->dsp->name,
					    nextm->dsp->name,
					    member->dsp->pcm_slot_tx,
					    member->dsp->pcm_bank_tx,
					    member->dsp->pcm_bank_rx);
				conf->hardware = 0;
				conf->software = 1;
				return;
			}
			/* find a new slot */
			memset(freeslots, 1, sizeof(freeslots));
			list_for_each_entry(dsp, &dsp_ilist, list) {
				if (dsp != member->dsp &&
				    dsp != nextm->dsp &&
				    member->dsp->features.pcm_id ==
				    dsp->features.pcm_id) {
					if (dsp->pcm_slot_rx >= 0 &&
					    dsp->pcm_slot_rx <
					    sizeof(freeslots))
						freeslots[dsp->pcm_slot_rx] = 0;
					if (dsp->pcm_slot_tx >= 0 &&
					    dsp->pcm_slot_tx <
					    sizeof(freeslots))
						freeslots[dsp->pcm_slot_tx] = 0;
				}
			}
			i = 0;
			ii = member->dsp->features.pcm_slots;
			while (i < ii) {
				if (freeslots[i])
					break;
				i++;
			}
			if (i == ii) {
				if (dsp_debug & DEBUG_DSP_CMX)
					printk(KERN_DEBUG
					    "%s no slot available for "
					    "%s & %s\n", __func__,
					    member->dsp->name,
					    nextm->dsp->name);
				/* no more slots available */
				goto conf_software;
			}
			/* assign free slot */
			member->dsp->pcm_slot_tx = i;
			member->dsp->pcm_slot_rx = i;
			nextm->dsp->pcm_slot_tx = i;
			nextm->dsp->pcm_slot_rx = i;
			member->dsp->pcm_bank_rx = 0;
			member->dsp->pcm_bank_tx = 1;
			nextm->dsp->pcm_bank_rx = 1;
			nextm->dsp->pcm_bank_tx = 0;
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s adding %s & %s to new PCM slot %d "
				    "(TX and RX on different chips) because "
				    "both members have not same slots\n",
				    __func__,
				    member->dsp->name,
				    nextm->dsp->name,
				    member->dsp->pcm_slot_tx);
			dsp_cmx_hw_message(member->dsp, MISDN_CTRL_HFC_PCM_CONN,
			    member->dsp->pcm_slot_tx, member->dsp->pcm_bank_tx,
			    member->dsp->pcm_slot_rx, member->dsp->pcm_bank_rx);
			dsp_cmx_hw_message(nextm->dsp, MISDN_CTRL_HFC_PCM_CONN,
			    nextm->dsp->pcm_slot_tx, nextm->dsp->pcm_bank_tx,
			    nextm->dsp->pcm_slot_rx, nextm->dsp->pcm_bank_rx);
			conf->hardware = 1;
			conf->software = tx_data;
			return;
		/* if members have one bank (or on the same chip) */
		} else {
			/* if both members have different crossed slots */
			if (member->dsp->pcm_slot_tx >= 0 &&
			    member->dsp->pcm_slot_rx >= 0 &&
			    nextm->dsp->pcm_slot_tx >= 0 &&
			    nextm->dsp->pcm_slot_rx >= 0 &&
			    nextm->dsp->pcm_slot_tx ==
			    member->dsp->pcm_slot_rx &&
			    nextm->dsp->pcm_slot_rx ==
			    member->dsp->pcm_slot_tx &&
			    member->dsp->pcm_slot_tx !=
			    member->dsp->pcm_slot_rx &&
			    member->dsp->pcm_bank_tx == 0 &&
			    member->dsp->pcm_bank_rx == 0 &&
			    nextm->dsp->pcm_bank_tx == 0 &&
			    nextm->dsp->pcm_bank_rx == 0) {
				/* all members have same slot */
				if (dsp_debug & DEBUG_DSP_CMX)
					printk(KERN_DEBUG
					    "%s dsp %s & %s stay joined on PCM "
					    "slot %d (TX) %d (RX) on same chip "
					    "or one bank PCM)\n", __func__,
					    member->dsp->name,
					    nextm->dsp->name,
					    member->dsp->pcm_slot_tx,
					    member->dsp->pcm_slot_rx);
				conf->hardware = 0;
				conf->software = 1;
				return;
			}
			/* find two new slot */
			memset(freeslots, 1, sizeof(freeslots));
			list_for_each_entry(dsp, &dsp_ilist, list) {
				if (dsp != member->dsp &&
				    dsp != nextm->dsp &&
				    member->dsp->features.pcm_id ==
				    dsp->features.pcm_id) {
					if (dsp->pcm_slot_rx >= 0 &&
					    dsp->pcm_slot_rx <
					    sizeof(freeslots))
						freeslots[dsp->pcm_slot_rx] = 0;
					if (dsp->pcm_slot_tx >= 0 &&
					    dsp->pcm_slot_tx <
					    sizeof(freeslots))
						freeslots[dsp->pcm_slot_tx] = 0;
				}
			}
			i1 = 0;
			ii = member->dsp->features.pcm_slots;
			while (i1 < ii) {
				if (freeslots[i1])
					break;
				i1++;
			}
			if (i1 == ii) {
				if (dsp_debug & DEBUG_DSP_CMX)
					printk(KERN_DEBUG
					    "%s no slot available "
					    "for %s & %s\n", __func__,
					    member->dsp->name,
					    nextm->dsp->name);
				/* no more slots available */
				goto conf_software;
			}
			i2 = i1+1;
			while (i2 < ii) {
				if (freeslots[i2])
					break;
				i2++;
			}
			if (i2 == ii) {
				if (dsp_debug & DEBUG_DSP_CMX)
					printk(KERN_DEBUG
					    "%s no slot available "
					    "for %s & %s\n",
					    __func__,
					    member->dsp->name,
					    nextm->dsp->name);
				/* no more slots available */
				goto conf_software;
			}
			/* assign free slots */
			member->dsp->pcm_slot_tx = i1;
			member->dsp->pcm_slot_rx = i2;
			nextm->dsp->pcm_slot_tx = i2;
			nextm->dsp->pcm_slot_rx = i1;
			member->dsp->pcm_bank_rx = 0;
			member->dsp->pcm_bank_tx = 0;
			nextm->dsp->pcm_bank_rx = 0;
			nextm->dsp->pcm_bank_tx = 0;
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s adding %s & %s to new PCM slot %d "
				    "(TX) %d (RX) on same chip or one bank "
				    "PCM, because both members have not "
				    "crossed slots\n", __func__,
				    member->dsp->name,
				    nextm->dsp->name,
				    member->dsp->pcm_slot_tx,
				    member->dsp->pcm_slot_rx);
			dsp_cmx_hw_message(member->dsp, MISDN_CTRL_HFC_PCM_CONN,
			    member->dsp->pcm_slot_tx, member->dsp->pcm_bank_tx,
			    member->dsp->pcm_slot_rx, member->dsp->pcm_bank_rx);
			dsp_cmx_hw_message(nextm->dsp, MISDN_CTRL_HFC_PCM_CONN,
			    nextm->dsp->pcm_slot_tx, nextm->dsp->pcm_bank_tx,
			    nextm->dsp->pcm_slot_rx, nextm->dsp->pcm_bank_rx);
			conf->hardware = 1;
			conf->software = tx_data;
			return;
		}
	}

	/*
	 * if we have more than two, we may check if we have a conference
	 * unit available on the chip. also all members must be on the same
	 */

	/* if not the same HFC chip */
	if (same_hfc < 0) {
		if (dsp_debug & DEBUG_DSP_CMX)
			printk(KERN_DEBUG
			    "%s conference %d cannot be formed, because "
			    "members are on different chips or not "
			    "on HFC chip\n",
			    __func__, conf->id);
		goto conf_software;
	}

	/* for more than two members.. */

	/* if all members already have the same conference */
	if (all_conf)
		return;

	/*
	 * if there is an existing conference, but not all members have joined
	 */
	if (current_conf >= 0) {
join_members:
		list_for_each_entry(member, &conf->mlist, list) {
			/* if no conference engine on our chip, change to
			 * software */
			if (!member->dsp->features.hfc_conf)
				goto conf_software;
			/* in case of hdlc, change to software */
			if (member->dsp->hdlc)
				goto conf_software;
			/* join to current conference */
			if (member->dsp->hfc_conf == current_conf)
				continue;
			/* get a free timeslot first */
			memset(freeslots, 1, sizeof(freeslots));
			list_for_each_entry(dsp, &dsp_ilist, list) {
				/*
				 * not checking current member, because
				 * slot will be overwritten.
				 */
				if (
				    dsp != member->dsp &&
				/* dsp must be on the same PCM */
				    member->dsp->features.pcm_id ==
				    dsp->features.pcm_id) {
					/* dsp must be on a slot */
					if (dsp->pcm_slot_tx >= 0 &&
					    dsp->pcm_slot_tx <
					    sizeof(freeslots))
						freeslots[dsp->pcm_slot_tx] = 0;
					if (dsp->pcm_slot_rx >= 0 &&
					    dsp->pcm_slot_rx <
					    sizeof(freeslots))
						freeslots[dsp->pcm_slot_rx] = 0;
				}
			}
			i = 0;
			ii = member->dsp->features.pcm_slots;
			while (i < ii) {
				if (freeslots[i])
					break;
				i++;
			}
			if (i == ii) {
				/* no more slots available */
				if (dsp_debug & DEBUG_DSP_CMX)
					printk(KERN_DEBUG
					    "%s conference %d cannot be formed,"
					    " because no slot free\n",
					    __func__, conf->id);
				goto conf_software;
			}
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "%s changing dsp %s to HW conference "
				    "%d slot %d\n", __func__,
				    member->dsp->name, current_conf, i);
			/* assign free slot & set PCM & join conf */
			member->dsp->pcm_slot_tx = i;
			member->dsp->pcm_slot_rx = i;
			member->dsp->pcm_bank_tx = 2; /* loop */
			member->dsp->pcm_bank_rx = 2;
			member->dsp->hfc_conf = current_conf;
			dsp_cmx_hw_message(member->dsp, MISDN_CTRL_HFC_PCM_CONN,
			    i, 2, i, 2);
			dsp_cmx_hw_message(member->dsp,
			    MISDN_CTRL_HFC_CONF_JOIN, current_conf, 0, 0, 0);
		}
		return;
	}

	/*
	 * no member is in a conference yet, so we find a free one
	 */
	memset(freeunits, 1, sizeof(freeunits));
	list_for_each_entry(dsp, &dsp_ilist, list) {
		/* dsp must be on the same chip */
		if (dsp->features.hfc_id == same_hfc &&
		    /* dsp must have joined a HW conference */
		    dsp->hfc_conf >= 0 &&
		    /* slot must be within range */
		    dsp->hfc_conf < 8)
			freeunits[dsp->hfc_conf] = 0;
	}
	i = 0;
	ii = 8;
	while (i < ii) {
		if (freeunits[i])
			break;
		i++;
	}
	if (i == ii) {
		/* no more conferences available */
		if (dsp_debug & DEBUG_DSP_CMX)
			printk(KERN_DEBUG
			    "%s conference %d cannot be formed, because "
			    "no conference number free\n",
			    __func__, conf->id);
		goto conf_software;
	}
	/* join all members */
	current_conf = i;
	goto join_members;
}


int
dsp_cmx_conf(struct dsp *dsp, u32 conf_id)
{
	int err;
	struct dsp_conf *conf;
	struct dsp_conf_member	*member;

	/* if conference doesn't change */
	if (dsp->conf_id == conf_id)
		return 0;

	/* first remove us from current conf */
	if (dsp->conf_id) {
		if (dsp_debug & DEBUG_DSP_CMX)
			printk(KERN_DEBUG "removing us from conference %d\n",
				dsp->conf->id);
		/* remove us from conf */
		conf = dsp->conf;
		err = dsp_cmx_del_conf_member(dsp);
		if (err)
			return err;
		dsp->conf_id = 0;

		/* update hardware */
		dsp_cmx_hardware(NULL, dsp);

		/* conf now empty? */
		if (list_empty(&conf->mlist)) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "conference is empty, so we remove it.\n");
			err = dsp_cmx_del_conf(conf);
			if (err)
				return err;
		} else {
			/* update members left on conf */
			dsp_cmx_hardware(conf, NULL);
		}
	}

	/* if split */
	if (!conf_id)
		return 0;

	/* now add us to conf */
	if (dsp_debug & DEBUG_DSP_CMX)
		printk(KERN_DEBUG "searching conference %d\n",
			conf_id);
	conf = dsp_cmx_search_conf(conf_id);
	if (!conf) {
		if (dsp_debug & DEBUG_DSP_CMX)
			printk(KERN_DEBUG
			    "conference doesn't exist yet, creating.\n");
		/* the conference doesn't exist, so we create */
		conf = dsp_cmx_new_conf(conf_id);
		if (!conf)
			return -EINVAL;
	} else if (!list_empty(&conf->mlist)) {
		member = list_entry(conf->mlist.next, struct dsp_conf_member,
			list);
		if (dsp->hdlc && !member->dsp->hdlc) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "cannot join transparent conference.\n");
			return -EINVAL;
		}
		if (!dsp->hdlc && member->dsp->hdlc) {
			if (dsp_debug & DEBUG_DSP_CMX)
				printk(KERN_DEBUG
				    "cannot join hdlc conference.\n");
			return -EINVAL;
		}
	}
	/* add conference member */
	err = dsp_cmx_add_conf_member(dsp, conf);
	if (err)
		return err;
	dsp->conf_id = conf_id;

	/* if we are alone, we do nothing! */
	if (list_empty(&conf->mlist)) {
		if (dsp_debug & DEBUG_DSP_CMX)
			printk(KERN_DEBUG
			    "we are alone in this conference, so exit.\n");
		/* update hardware */
		dsp_cmx_hardware(NULL, dsp);
		return 0;
	}

	/* update members on conf */
	dsp_cmx_hardware(conf, NULL);

	return 0;
}

#ifdef CMX_DELAY_DEBUG
int delaycount;
static void
showdelay(struct dsp *dsp, int samples, int delay)
{
	char bar[] = "--------------------------------------------------|";
	int sdelay;

	delaycount += samples;
	if (delaycount < 8000)
		return;
	delaycount = 0;

	sdelay = delay * 50 / (dsp_poll << 2);

	printk(KERN_DEBUG "DELAY (%s) %3d >%s\n", dsp->name, delay,
		sdelay > 50 ? "..." : bar + 50 - sdelay);
}
#endif

void
dsp_cmx_receive(struct dsp *dsp, struct sk_buff *skb)
{
	u8 *d, *p;
	int len = skb->len;
	struct mISDNhead *hh = mISDN_HEAD_P(skb);
	int w, i, ii;

	/* check if we have sompen */
	if (len < 1)
		return;

	/* half of the buffer should be larger than maximum packet size */
	if (len >= CMX_BUFF_HALF) {
		printk(KERN_ERR
		    "%s line %d: packet from card is too large (%d bytes). "
		    "please make card send smaller packets OR increase "
		    "CMX_BUFF_SIZE\n", __FILE__, __LINE__, len);
		return;
	}

	/*
	 * initialize pointers if not already -
	 * also add delay if requested by PH_SIGNAL
	 */
	if (dsp->rx_init) {
		dsp->rx_init = 0;
		if (dsp->features.unordered) {
			dsp->rx_R = (hh->id & CMX_BUFF_MASK);
			if (dsp->cmx_delay)
				dsp->rx_W = (dsp->rx_R + dsp->cmx_delay)
					& CMX_BUFF_MASK;
			else
				dsp->rx_W = (dsp->rx_R + (dsp_poll >> 1))
					& CMX_BUFF_MASK;
		} else {
			dsp->rx_R = 0;
			if (dsp->cmx_delay)
				dsp->rx_W = dsp->cmx_delay;
			else
				dsp->rx_W = dsp_poll >> 1;
		}
	}
	/* if frame contains time code, write directly */
	if (dsp->features.unordered) {
		dsp->rx_W = (hh->id & CMX_BUFF_MASK);
		/* printk(KERN_DEBUG "%s %08x\n", dsp->name, hh->id); */
	}
	/*
	 * if we underrun (or maybe overrun),
	 * we set our new read pointer, and write silence to buffer
	 */
	if (((dsp->rx_W-dsp->rx_R) & CMX_BUFF_MASK) >= CMX_BUFF_HALF) {
		if (dsp_debug & DEBUG_DSP_CLOCK)
			printk(KERN_DEBUG
			    "cmx_receive(dsp=%lx): UNDERRUN (or overrun the "
			    "maximum delay), adjusting read pointer! "
			    "(inst %s)\n", (u_long)dsp, dsp->name);
		/* flush rx buffer and set delay to dsp_poll / 2 */
		if (dsp->features.unordered) {
			dsp->rx_R = (hh->id & CMX_BUFF_MASK);
			if (dsp->cmx_delay)
				dsp->rx_W = (dsp->rx_R + dsp->cmx_delay)
					& CMX_BUFF_MASK;
				dsp->rx_W = (dsp->rx_R + (dsp_poll >> 1))
					& CMX_BUFF_MASK;
		} else {
			dsp->rx_R = 0;
			if (dsp->cmx_delay)
				dsp->rx_W = dsp->cmx_delay;
			else
				dsp->rx_W = dsp_poll >> 1;
		}
		memset(dsp->rx_buff, dsp_silence, sizeof(dsp->rx_buff));
	}
	/* if we have reached double delay, jump back to middle */
	if (dsp->cmx_delay)
		if (((dsp->rx_W - dsp->rx_R) & CMX_BUFF_MASK) >=
		    (dsp->cmx_delay << 1)) {
			if (dsp_debug & DEBUG_DSP_CLOCK)
				printk(KERN_DEBUG
				    "cmx_receive(dsp=%lx): OVERRUN (because "
				    "twice the delay is reached), adjusting "
				    "read pointer! (inst %s)\n",
				    (u_long)dsp, dsp->name);
		/* flush buffer */
		if (dsp->features.unordered) {
			dsp->rx_R = (hh->id & CMX_BUFF_MASK);
			dsp->rx_W = (dsp->rx_R + dsp->cmx_delay)
				& CMX_BUFF_MASK;
		} else {
			dsp->rx_R = 0;
			dsp->rx_W = dsp->cmx_delay;
		}
		memset(dsp->rx_buff, dsp_silence, sizeof(dsp->rx_buff));
	}

	/* show where to write */
#ifdef CMX_DEBUG
	printk(KERN_DEBUG
	    "cmx_receive(dsp=%lx): rx_R(dsp)=%05x rx_W(dsp)=%05x len=%d %s\n",
	    (u_long)dsp, dsp->rx_R, dsp->rx_W, len, dsp->name);
#endif

	/* write data into rx_buffer */
	p = skb->data;
	d = dsp->rx_buff;
	w = dsp->rx_W;
	i = 0;
	ii = len;
	while (i < ii) {
		d[w++ & CMX_BUFF_MASK] = *p++;
		i++;
	}

	/* increase write-pointer */
	dsp->rx_W = ((dsp->rx_W+len) & CMX_BUFF_MASK);
#ifdef CMX_DELAY_DEBUG
	showdelay(dsp, len, (dsp->rx_W-dsp->rx_R) & CMX_BUFF_MASK);
#endif
}


static void
dsp_cmx_send_member(struct dsp *dsp, int len, s32 *c, int members)
{
	struct dsp_conf *conf = dsp->conf;
	struct dsp *member, *other;
	register s32 sample;
	u8 *d, *p, *q, *o_q;
	struct sk_buff *nskb, *txskb;
	int r, rr, t, tt, o_r, o_rr;
	int preload = 0;
	struct mISDNhead *hh, *thh;
	int tx_data_only = 0;

	/* don't process if: */
	if (!dsp->b_active) { /* if not active */
		dsp->last_tx = 0;
		return;
	}
	if (((dsp->conf && dsp->conf->hardware) || /* hardware conf */
	    dsp->echo.hardware) && /* OR hardware echo */
	    dsp->tx_R == dsp->tx_W && /* AND no tx-data */
	    !(dsp->tone.tone && dsp->tone.software)) { /* AND not soft tones */
		if (!dsp->tx_data) { /* no tx_data for user space required */
			dsp->last_tx = 0;
			return;
		}
		if (dsp->conf && dsp->conf->software && dsp->conf->hardware)
			tx_data_only = 1;
		if (dsp->conf->software && dsp->echo.hardware)
			tx_data_only = 1;
	}

#ifdef CMX_DEBUG
	printk(KERN_DEBUG
	    "SEND members=%d dsp=%s, conf=%p, rx_R=%05x rx_W=%05x\n",
	    members, dsp->name, conf, dsp->rx_R, dsp->rx_W);
#endif

	/* preload if we have delay set */
	if (dsp->cmx_delay && !dsp->last_tx) {
		preload = len;
		if (preload < 128)
			preload = 128;
	}

	/* PREPARE RESULT */
	nskb = mI_alloc_skb(len + preload, GFP_ATOMIC);
	if (!nskb) {
		printk(KERN_ERR
		    "FATAL ERROR in mISDN_dsp.o: cannot alloc %d bytes\n",
		    len + preload);
		return;
	}
	hh = mISDN_HEAD_P(nskb);
	hh->prim = PH_DATA_REQ;
	hh->id = 0;
	dsp->last_tx = 1;

	/* set pointers, indexes and stuff */
	member = dsp;
	p = dsp->tx_buff; /* transmit data */
	q = dsp->rx_buff; /* received data */
	d = skb_put(nskb, preload + len); /* result */
	t = dsp->tx_R; /* tx-pointers */
	tt = dsp->tx_W;
	r = dsp->rx_R; /* rx-pointers */
	rr = (r + len) & CMX_BUFF_MASK;

	/* preload with silence, if required */
	if (preload) {
		memset(d, dsp_silence, preload);
		d += preload;
	}

	/* PROCESS TONES/TX-DATA ONLY */
	if (dsp->tone.tone && dsp->tone.software) {
		/* -> copy tone */
		dsp_tone_copy(dsp, d, len);
		dsp->tx_R = 0; /* clear tx buffer */
		dsp->tx_W = 0;
		goto send_packet;
	}
	/* if we have tx-data but do not use mixing */
	if (!dsp->tx_mix && t != tt) {
		/* -> send tx-data and continue when not enough */
#ifdef CMX_TX_DEBUG
	sprintf(debugbuf, "TX sending (%04x-%04x)%p: ", t, tt, p);
#endif
		while (r != rr && t != tt) {
#ifdef CMX_TX_DEBUG
			if (strlen(debugbuf) < 48)
				sprintf(debugbuf+strlen(debugbuf), " %02x",
				    p[t]);
#endif
			*d++ = p[t]; /* write tx_buff */
			t = (t+1) & CMX_BUFF_MASK;
			r = (r+1) & CMX_BUFF_MASK;
		}
		if (r == rr) {
			dsp->tx_R = t;
#ifdef CMX_TX_DEBUG
	printk(KERN_DEBUG "%s\n", debugbuf);
#endif
			goto send_packet;
		}
	}
#ifdef CMX_TX_DEBUG
	printk(KERN_DEBUG "%s\n", debugbuf);
#endif

	/* PROCESS DATA (one member / no conf) */
	if (!conf || members <= 1) {
		/* -> if echo is NOT enabled */
		if (!dsp->echo.software) {
			/* -> send tx-data if available or use 0-volume */
			while (r != rr && t != tt) {
				*d++ = p[t]; /* write tx_buff */
				t = (t+1) & CMX_BUFF_MASK;
				r = (r+1) & CMX_BUFF_MASK;
			}
			if (r != rr) {
				if (dsp_debug & DEBUG_DSP_CLOCK)
					printk(KERN_DEBUG "%s: RX empty\n",
						__func__);
				memset(d, dsp_silence, (rr-r)&CMX_BUFF_MASK);
			}
		/* -> if echo is enabled */
		} else {
			/*
			 * -> mix tx-data with echo if available,
			 * or use echo only
			 */
			while (r != rr && t != tt) {
				*d++ = dsp_audio_mix_law[(p[t]<<8)|q[r]];
				t = (t+1) & CMX_BUFF_MASK;
				r = (r+1) & CMX_BUFF_MASK;
			}
			while (r != rr) {
				*d++ = q[r]; /* echo */
				r = (r+1) & CMX_BUFF_MASK;
			}
		}
		dsp->tx_R = t;
		goto send_packet;
	}
	/* PROCESS DATA (two members) */
#ifdef CMX_CONF_DEBUG
	if (0) {
#else
	if (members == 2) {
#endif
		/* "other" becomes other party */
		other = (list_entry(conf->mlist.next,
		    struct dsp_conf_member, list))->dsp;
		if (other == member)
			other = (list_entry(conf->mlist.prev,
			    struct dsp_conf_member, list))->dsp;
		o_q = other->rx_buff; /* received data */
		o_rr = (other->rx_R + len) & CMX_BUFF_MASK;
			/* end of rx-pointer */
		o_r = (o_rr - rr + r) & CMX_BUFF_MASK;
			/* start rx-pointer at current read position*/
		/* -> if echo is NOT enabled */
		if (!dsp->echo.software) {
			/*
			 * -> copy other member's rx-data,
			 * if tx-data is available, mix
			 */
			while (o_r != o_rr && t != tt) {
				*d++ = dsp_audio_mix_law[(p[t]<<8)|o_q[o_r]];
				t = (t+1) & CMX_BUFF_MASK;
				o_r = (o_r+1) & CMX_BUFF_MASK;
			}
			while (o_r != o_rr) {
				*d++ = o_q[o_r];
				o_r = (o_r+1) & CMX_BUFF_MASK;
			}
		/* -> if echo is enabled */
		} else {
			/*
			 * -> mix other member's rx-data with echo,
			 * if tx-data is available, mix
			 */
			while (r != rr && t != tt) {
				sample = dsp_audio_law_to_s32[p[t]] +
				    dsp_audio_law_to_s32[q[r]] +
				    dsp_audio_law_to_s32[o_q[o_r]];
				if (sample < -32768)
					sample = -32768;
				else if (sample > 32767)
					sample = 32767;
				*d++ = dsp_audio_s16_to_law[sample & 0xffff];
				    /* tx-data + rx_data + echo */
				t = (t+1) & CMX_BUFF_MASK;
				r = (r+1) & CMX_BUFF_MASK;
				o_r = (o_r+1) & CMX_BUFF_MASK;
			}
			while (r != rr) {
				*d++ = dsp_audio_mix_law[(q[r]<<8)|o_q[o_r]];
				r = (r+1) & CMX_BUFF_MASK;
				o_r = (o_r+1) & CMX_BUFF_MASK;
			}
		}
		dsp->tx_R = t;
		goto send_packet;
	}
#ifdef DSP_NEVER_DEFINED
	}
#endif
	/* PROCESS DATA (three or more members) */
	/* -> if echo is NOT enabled */
	if (!dsp->echo.software) {
		/*
		 * -> substract rx-data from conf-data,
		 * if tx-data is available, mix
		 */
		while (r != rr && t != tt) {
			sample = dsp_audio_law_to_s32[p[t]] + *c++ -
			    dsp_audio_law_to_s32[q[r]];
			if (sample < -32768)
				sample = -32768;
			else if (sample > 32767)
				sample = 32767;
			*d++ = dsp_audio_s16_to_law[sample & 0xffff];
			    /* conf-rx+tx */
			r = (r+1) & CMX_BUFF_MASK;
			t = (t+1) & CMX_BUFF_MASK;
		}
		while (r != rr) {
			sample = *c++ - dsp_audio_law_to_s32[q[r]];
			if (sample < -32768)
				sample = -32768;
			else if (sample > 32767)
				sample = 32767;
			*d++ = dsp_audio_s16_to_law[sample & 0xffff];
			    /* conf-rx */
			r = (r+1) & CMX_BUFF_MASK;
		}
	/* -> if echo is enabled */
	} else {
		/*
		 * -> encode conf-data, if tx-data
		 * is available, mix
		 */
		while (r != rr && t != tt) {
			sample = dsp_audio_law_to_s32[p[t]] + *c++;
			if (sample < -32768)
				sample = -32768;
			else if (sample > 32767)
				sample = 32767;
			*d++ = dsp_audio_s16_to_law[sample & 0xffff];
			    /* conf(echo)+tx */
			t = (t+1) & CMX_BUFF_MASK;
			r = (r+1) & CMX_BUFF_MASK;
		}
		while (r != rr) {
			sample = *c++;
			if (sample < -32768)
				sample = -32768;
			else if (sample > 32767)
				sample = 32767;
			*d++ = dsp_audio_s16_to_law[sample & 0xffff];
			    /* conf(echo) */
			r = (r+1) & CMX_BUFF_MASK;
		}
	}
	dsp->tx_R = t;
	goto send_packet;

send_packet:
	/*
	 * send tx-data if enabled - don't filter,
	 * becuase we want what we send, not what we filtered
	 */
	if (dsp->tx_data) {
		if (tx_data_only) {
			hh->prim = DL_DATA_REQ;
			hh->id = 0;
			/* queue and trigger */
			skb_queue_tail(&dsp->sendq, nskb);
			schedule_work(&dsp->workq);
			/* exit because only tx_data is used */
			return;
		} else {
			txskb = mI_alloc_skb(len, GFP_ATOMIC);
			if (!txskb) {
				printk(KERN_ERR
				    "FATAL ERROR in mISDN_dsp.o: "
				    "cannot alloc %d bytes\n", len);
			} else {
				thh = mISDN_HEAD_P(txskb);
				thh->prim = DL_DATA_REQ;
				thh->id = 0;
				memcpy(skb_put(txskb, len), nskb->data+preload,
					len);
				/* queue (trigger later) */
				skb_queue_tail(&dsp->sendq, txskb);
			}
		}
	}

	/* send data only to card, if we don't just calculated tx_data */
	/* adjust volume */
	if (dsp->tx_volume)
		dsp_change_volume(nskb, dsp->tx_volume);
	/* pipeline */
	if (dsp->pipeline.inuse)
		dsp_pipeline_process_tx(&dsp->pipeline, nskb->data,
			nskb->len);
	/* crypt */
	if (dsp->bf_enable)
		dsp_bf_encrypt(dsp, nskb->data, nskb->len);
	/* queue and trigger */
	skb_queue_tail(&dsp->sendq, nskb);
	schedule_work(&dsp->workq);
}

static u32	jittercount; /* counter for jitter check */
struct timer_list dsp_spl_tl;
u32	dsp_spl_jiffies; /* calculate the next time to fire */
static u16	dsp_count; /* last sample count */
static int	dsp_count_valid ; /* if we have last sample count */

void
dsp_cmx_send(void *arg)
{
	struct dsp_conf *conf;
	struct dsp_conf_member *member;
	struct dsp *dsp;
	int mustmix, members;
	static s32 mixbuffer[MAX_POLL+100];
	s32 *c;
	u8 *p, *q;
	int r, rr;
	int jittercheck = 0, delay, i;
	u_long flags;
	u16 length, count;

	/* lock */
	spin_lock_irqsave(&dsp_lock, flags);

	if (!dsp_count_valid) {
		dsp_count = mISDN_clock_get();
		length = dsp_poll;
		dsp_count_valid = 1;
	} else {
		count = mISDN_clock_get();
		length = count - dsp_count;
		dsp_count = count;
	}
	if (length > MAX_POLL + 100)
		length = MAX_POLL + 100;
	/* printk(KERN_DEBUG "len=%d dsp_count=0x%x\n", length, dsp_count); */

	/*
	 * check if jitter needs to be checked (this is every second)
	 */
	jittercount += length;
	if (jittercount >= 8000) {
		jittercount -= 8000;
		jittercheck = 1;
	}

	/* loop all members that do not require conference mixing */
	list_for_each_entry(dsp, &dsp_ilist, list) {
		if (dsp->hdlc)
			continue;
		conf = dsp->conf;
		mustmix = 0;
		members = 0;
		if (conf) {
			members = count_list_member(&conf->mlist);
#ifdef CMX_CONF_DEBUG
			if (conf->software && members > 1)
#else
			if (conf->software && members > 2)
#endif
				mustmix = 1;
		}

		/* transmission required */
		if (!mustmix) {
			dsp_cmx_send_member(dsp, length, mixbuffer, members);

			/*
			 * unused mixbuffer is given to prevent a
			 * potential null-pointer-bug
			 */
		}
	}

	/* loop all members that require conference mixing */
	list_for_each_entry(conf, &conf_ilist, list) {
		/* count members and check hardware */
		members = count_list_member(&conf->mlist);
#ifdef CMX_CONF_DEBUG
		if (conf->software && members > 1) {
#else
		if (conf->software && members > 2) {
#endif
			/* check for hdlc conf */
			member = list_entry(conf->mlist.next,
				struct dsp_conf_member, list);
			if (member->dsp->hdlc)
				continue;
			/* mix all data */
			memset(mixbuffer, 0, length*sizeof(s32));
			list_for_each_entry(member, &conf->mlist, list) {
				dsp = member->dsp;
				/* get range of data to mix */
				c = mixbuffer;
				q = dsp->rx_buff;
				r = dsp->rx_R;
				rr = (r + length) & CMX_BUFF_MASK;
				/* add member's data */
				while (r != rr) {
					*c++ += dsp_audio_law_to_s32[q[r]];
					r = (r+1) & CMX_BUFF_MASK;
				}
			}

			/* process each member */
			list_for_each_entry(member, &conf->mlist, list) {
				/* transmission */
				dsp_cmx_send_member(member->dsp, length,
				    mixbuffer, members);
			}
		}
	}

	/* delete rx-data, increment buffers, change pointers */
	list_for_each_entry(dsp, &dsp_ilist, list) {
		if (dsp->hdlc)
			continue;
		p = dsp->rx_buff;
		q = dsp->tx_buff;
		r = dsp->rx_R;
		/* move receive pointer when receiving */
		if (!dsp->rx_is_off) {
			rr = (r + length) & CMX_BUFF_MASK;
			/* delete rx-data */
			while (r != rr) {
				p[r] = dsp_silence;
				r = (r+1) & CMX_BUFF_MASK;
			}
			/* increment rx-buffer pointer */
			dsp->rx_R = r; /* write incremented read pointer */
		}

		/* check current rx_delay */
		delay = (dsp->rx_W-dsp->rx_R) & CMX_BUFF_MASK;
		if (delay >= CMX_BUFF_HALF)
			delay = 0; /* will be the delay before next write */
		/* check for lower delay */
		if (delay < dsp->rx_delay[0])
			dsp->rx_delay[0] = delay;
		/* check current tx_delay */
		delay = (dsp->tx_W-dsp->tx_R) & CMX_BUFF_MASK;
		if (delay >= CMX_BUFF_HALF)
			delay = 0; /* will be the delay before next write */
		/* check for lower delay */
		if (delay < dsp->tx_delay[0])
			dsp->tx_delay[0] = delay;
		if (jittercheck) {
			/* find the lowest of all rx_delays */
			delay = dsp->rx_delay[0];
			i = 1;
			while (i < MAX_SECONDS_JITTER_CHECK) {
				if (delay > dsp->rx_delay[i])
					delay = dsp->rx_delay[i];
				i++;
			}
			/*
			 * remove rx_delay only if we have delay AND we
			 * have not preset cmx_delay AND
			 * the delay is greater dsp_poll
			 */
			if (delay > dsp_poll && !dsp->cmx_delay) {
				if (dsp_debug & DEBUG_DSP_CLOCK)
					printk(KERN_DEBUG
					    "%s lowest rx_delay of %d bytes for"
					    " dsp %s are now removed.\n",
					    __func__, delay,
					    dsp->name);
				r = dsp->rx_R;
				rr = (r + delay - (dsp_poll >> 1))
					& CMX_BUFF_MASK;
				/* delete rx-data */
				while (r != rr) {
					p[r] = dsp_silence;
					r = (r+1) & CMX_BUFF_MASK;
				}
				/* increment rx-buffer pointer */
				dsp->rx_R = r;
				    /* write incremented read pointer */
			}
			/* find the lowest of all tx_delays */
			delay = dsp->tx_delay[0];
			i = 1;
			while (i < MAX_SECONDS_JITTER_CHECK) {
				if (delay > dsp->tx_delay[i])
					delay = dsp->tx_delay[i];
				i++;
			}
			/*
			 * remove delay only if we have delay AND we
			 * have enabled tx_dejitter
			 */
			if (delay > dsp_poll && dsp->tx_dejitter) {
				if (dsp_debug & DEBUG_DSP_CLOCK)
					printk(KERN_DEBUG
					    "%s lowest tx_delay of %d bytes for"
					    " dsp %s are now removed.\n",
					    __func__, delay,
					    dsp->name);
				r = dsp->tx_R;
				rr = (r + delay - (dsp_poll >> 1))
					& CMX_BUFF_MASK;
				/* delete tx-data */
				while (r != rr) {
					q[r] = dsp_silence;
					r = (r+1) & CMX_BUFF_MASK;
				}
				/* increment rx-buffer pointer */
				dsp->tx_R = r;
				    /* write incremented read pointer */
			}
			/* scroll up delays */
			i = MAX_SECONDS_JITTER_CHECK - 1;
			while (i) {
				dsp->rx_delay[i] = dsp->rx_delay[i-1];
				dsp->tx_delay[i] = dsp->tx_delay[i-1];
				i--;
			}
			dsp->tx_delay[0] = CMX_BUFF_HALF; /* (infinite) delay */
			dsp->rx_delay[0] = CMX_BUFF_HALF; /* (infinite) delay */
		}
	}

	/* if next event would be in the past ... */
	if ((s32)(dsp_spl_jiffies+dsp_tics-jiffies) <= 0)
		dsp_spl_jiffies = jiffies + 1;
	else
		dsp_spl_jiffies += dsp_tics;

	dsp_spl_tl.expires = dsp_spl_jiffies;
	add_timer(&dsp_spl_tl);

	/* unlock */
	spin_unlock_irqrestore(&dsp_lock, flags);
}

void
dsp_cmx_transmit(struct dsp *dsp, struct sk_buff *skb)
{
	u_int w, ww;
	u8 *d, *p;
	int space; /* todo: , l = skb->len; */
#ifdef CMX_TX_DEBUG
	char debugbuf[256] = "";
#endif

	/* check if there is enough space, and then copy */
	w = dsp->tx_W;
	ww = dsp->tx_R;
	p = dsp->tx_buff;
	d = skb->data;
	space = (ww - w - 1) & CMX_BUFF_MASK;
	/* write-pointer should not overrun nor reach read pointer */
	if (space < skb->len) {
		/* write to the space we have left */
		ww = (ww - 1) & CMX_BUFF_MASK; /* end one byte prior tx_R */
		if (dsp_debug & DEBUG_DSP_CLOCK)
			printk(KERN_DEBUG "%s: TX overflow space=%d skb->len="
			    "%d, w=0x%04x, ww=0x%04x\n", __func__, space,
			    skb->len, w, ww);
	} else
		/* write until all byte are copied */
		ww = (w + skb->len) & CMX_BUFF_MASK;
	dsp->tx_W = ww;

	/* show current buffer */
#ifdef CMX_DEBUG
	printk(KERN_DEBUG
	    "cmx_transmit(dsp=%lx) %d bytes to 0x%x-0x%x. %s\n",
	    (u_long)dsp, (ww-w)&CMX_BUFF_MASK, w, ww, dsp->name);
#endif

	/* copy transmit data to tx-buffer */
#ifdef CMX_TX_DEBUG
	sprintf(debugbuf, "TX getting (%04x-%04x)%p: ", w, ww, p);
#endif
	while (w != ww) {
#ifdef CMX_TX_DEBUG
		if (strlen(debugbuf) < 48)
			sprintf(debugbuf+strlen(debugbuf), " %02x", *d);
#endif
		p[w] = *d++;
		w = (w+1) & CMX_BUFF_MASK;
	}
#ifdef CMX_TX_DEBUG
	printk(KERN_DEBUG "%s\n", debugbuf);
#endif

}

void
dsp_cmx_hdlc(struct dsp *dsp, struct sk_buff *skb)
{
	struct sk_buff *nskb = NULL;
	struct dsp_conf_member *member;
	struct mISDNhead *hh;

	/* not if not active */
	if (!dsp->b_active)
		return;

	/* check if we have sompen */
	if (skb->len < 1)
		return;

	/* no conf */
	if (!dsp->conf) {
		/* in case of software echo */
		if (dsp->echo.software) {
			nskb = skb_clone(skb, GFP_ATOMIC);
			if (nskb) {
				hh = mISDN_HEAD_P(nskb);
				hh->prim = PH_DATA_REQ;
				hh->id = 0;
				skb_queue_tail(&dsp->sendq, nskb);
				schedule_work(&dsp->workq);
			}
		}
		return;
	}
	/* in case of hardware conference */
	if (dsp->conf->hardware)
		return;
	list_for_each_entry(member, &dsp->conf->mlist, list) {
		if (dsp->echo.software || member->dsp != dsp) {
			nskb = skb_clone(skb, GFP_ATOMIC);
			if (nskb) {
				hh = mISDN_HEAD_P(nskb);
				hh->prim = PH_DATA_REQ;
				hh->id = 0;
				skb_queue_tail(&member->dsp->sendq, nskb);
				schedule_work(&member->dsp->workq);
			}
		}
	}
}


