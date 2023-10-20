#ifndef _COMMON_H_
#define _COMMON_H_

#define BT_DEV_LABEL_LEN 199
#define BT_DEV_BDADDR_LEN 17

#define BT_DEV_LABEL_LEN_S "199"
#define BT_DEV_BDADDR_LEN_S "17"

typedef struct BTDevice {
	char label[BT_DEV_LABEL_LEN + 1];
	char bdaddr[BT_DEV_BDADDR_LEN + 1];
} BTDevice;

#endif // _COMMON_H_
