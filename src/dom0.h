/*
 * Copyright (c) 2024 EPAM Systems
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DOM0_XT_SRC_DOM0_H_
#define ZEPHYR_DOM0_XT_SRC_DOM0_H_

/**
 * @brief Dom0 domain's configuration data structure.
 *
 * Hold Dom0 application specific configuration data for Xen guest domains.
 */
struct dom0_domain_cfg {
	struct xen_domain_cfg *domain_cfg; /**< xenlib domain cfg */

	/**
	 * @brief Dom0 configuration init callback.
	 *
	 * Callback that allows to execute some init routine before starting
	 * domain. Such routine can be parsing pv parameters.
	 */
	void (*init)(void);
	const char *image_kernel_path; /**< disk: domain kernel binary path */
	const char *image_dt_path; /**< disk: domain partial device-tree (PDT) binary path */
};

#if defined(CONFIG_DOM_STORAGE_FATFS_ENABLE)
#define DISK_DRIVE_NAME CONFIG_SDMMC_VOLUME_NAME
#define DISK_MOUNT_PT "/"CONFIG_DOM_STORAGE_FATFS_LOGICAL_DRIVE_NAME":"
#define DISK_BIN_PATH DISK_MOUNT_PT"/"CONFIG_DOM_STORAGE_FATFS_DIR

int storage_init(void);
int storage_image_kernel_read(uint8_t *buf, size_t bufsize, uint64_t offset, void *image_info);
ssize_t storage_image_kernel_get_size(void *image_info, uint64_t *size);
int storage_image_dt_read(uint8_t *buf, size_t bufsize, uint64_t offset, void *image_info);
int storage_image_dt_get_size(void *image_info, size_t *size);
#else

static inline int storage_init(void)
{
	return 0;
}
#endif /* CONFIG_DOM_STORAGE_FATFS_ENABLE */

#endif /* ZEPHYR_DOM0_XT_SRC_DOM0_H_ */
