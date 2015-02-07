

#ifndef _PPC64_HVCSERVER_H
#define _PPC64_HVCSERVER_H
#ifdef __KERNEL__

#include <linux/list.h>

/* Converged Location Code length */
#define HVCS_CLC_LENGTH	79

struct hvcs_partner_info {
	struct list_head node;
	uint32_t unit_address;
	uint32_t partition_ID;
	char location_code[HVCS_CLC_LENGTH + 1]; /* CLC + 1 null-term char */
};

extern int hvcs_free_partner_info(struct list_head *head);
extern int hvcs_get_partner_info(uint32_t unit_address,
		struct list_head *head, unsigned long *pi_buff);
extern int hvcs_register_connection(uint32_t unit_address,
		uint32_t p_partition_ID, uint32_t p_unit_address);
extern int hvcs_free_connection(uint32_t unit_address);

#endif /* __KERNEL__ */
#endif /* _PPC64_HVCSERVER_H */
