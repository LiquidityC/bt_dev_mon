#include "file_reader.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct DeviceList {
	BTDevice *devices;
	size_t capacity;
	size_t len;
} DeviceList;

static int dl_init(DeviceList *list)
{
	memset(list, 0, sizeof(DeviceList));
	list->capacity = 1;
	list->len = 0;
	list->devices = malloc(sizeof(BTDevice));
	if (!list->devices) {
		return -1;
	} else {
		return 0;
	}
}

static int add_device(DeviceList *list, const char *label, const char *bdaddr)
{
	if (list->len == list->capacity) {
		BTDevice *new = realloc(list->devices, list->capacity * 2 * sizeof(BTDevice));
		if (!new) {
			return -1;
		}
		list->devices = new;
		list->capacity *= 2;
	}

	BTDevice *device = &list->devices[list->len++];
	strncpy(device->label, label, BT_DEV_LABEL_LEN + 1);
	strncpy(device->bdaddr, bdaddr, BT_DEV_BDADDR_LEN + 1);

	return 0;
}

BTDevice *read_config_file(const char *fpath, size_t *count)
{
	DeviceList list = { 0 };
	FILE *fp = NULL;

	fp = fopen(CONFIG_FILE_PATH, "r");
	if (fp == NULL) {
		fprintf(stderr, "File: %s\n", CONFIG_FILE_PATH);
		perror("fopen");
		goto out;
	}

	if (dl_init(&list) != 0) {
		perror("dl_init");
		goto out;
	}

	char label[BT_DEV_LABEL_LEN + 1];
	char bdaddr[BT_DEV_BDADDR_LEN + 1];

	while (fscanf(fp, "%" BT_DEV_LABEL_LEN_S "s %" BT_DEV_BDADDR_LEN_S "s\n", label, bdaddr) == 2) {
		printf("Reading device: %s | %s\n", label, bdaddr);
		if (add_device(&list, label, bdaddr) != 0) {
			perror("add_device");
			goto out;
		}
	}

out:
	if (fp) {
		fclose(fp);
	}
	*count = list.len;
	return list.devices;
}
