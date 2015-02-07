
#ifndef _MT6573_RECOVERY_H_
#define _MT6573_RECOVERY_H_

static const int MISC_PAGES = 3;
static const int MISC_COMMAND_PAGE = 1;  // bootloader command is this page

#define MT6573_REBOOT_FLAG    0xFFFF
#define MT6573_NORMAL_FLAG    0x0000

struct misc_message {
    char command[32];
    char status[32];
    char recovery[1024];
};

#endif

