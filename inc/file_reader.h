#ifndef _FILE_READER_H_
#define _FILE_READER_H_

#include <stddef.h>

#include "defines.h"
#include "common.h"

#ifdef RELEASE_BUILD
#define CONFIG_FOLDER "/etc/bt_mon/"
#else
#define CONFIG_FOLDER "./"
#endif

#define CONFIG_FILE_PATH CONFIG_FOLDER "bt_dev_mon.conf"

/**
 * \brief Read config file
 *
 * \param fpath The path to the config file
 * \param count The number of devices read
 * \returns A pointer to a list of devices; this needs to be free'd after use.
 */
BTDevice *read_config_file(const char *fpath, size_t *count);

#endif // _FILE_READER_H_
