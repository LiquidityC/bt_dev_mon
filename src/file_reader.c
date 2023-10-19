#include "file_reader.h"
#include <stdlib.h>
#include <string.h>

BTDevice *read_config_file(const char *fpath, size_t *count)
{
	(void)fpath;
	*count = 1;
	BTDevice *devices = calloc(*count, sizeof(BTDevice));

	if (devices != NULL) {
		strncpy(devices[0].label, "Linus", BT_DEV_LABEL_LEN);
		strncpy(devices[0].bdaddr, "B8:90:47:25:49:00", BT_DEV_BDADDR_LEN);
	} else {
		*count = 0;
	}

	return devices;
}
