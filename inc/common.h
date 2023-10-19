#ifndef _COMMON_H_
#define _COMMON_H_

#define BT_DEV_LABEL_LEN 200
#define BT_DEV_BDADDR_LEN 18

typedef struct BTDevice {
	char label[BT_DEV_LABEL_LEN];
	char bdaddr[BT_DEV_BDADDR_LEN];
} BTDevice;

#endif // _COMMON_H_
