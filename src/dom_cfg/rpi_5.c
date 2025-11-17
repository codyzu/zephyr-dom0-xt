/*
 * Copyright (c) 2024 EPAM Systems
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <domain.h>
#include <zephyr/xen/public/domctl.h>
#include <string.h>
#include <xl_parser.h>
#include "../dom0.h"

/*
 * Example Xen guest domains configuration file for "rpi_5" which contains
 * - domu_cfg_0 as DomD example which uses real HW "/soc/gpio@7d517c00" and can be used to run
 *   samples/basic/blinky sample.
 * - domu_cfg_1 as DomU, no real HW. It can be used to run any Zephyr example not accessing HW.
 *   For example samples/synchronization
 * - domu_cfg_2 as DomU, no real HW.
 *   The Unikernel helloworld monkey example is used as guest domain. Its supported only with
 *   storage enabled.
 */

#if defined(CONFIG_DOM_STORAGE_FATFS_ENABLE)
static struct xen_domain_cfg domu_cfg_2 = {
	.name = "helloworld_xen-arm64",
	.mem_kb = 16384,
	.flags = (XEN_DOMCTL_CDF_hvm | XEN_DOMCTL_CDF_hap),
	.max_evtchns = 10,
	.max_vcpus = 1,
	.gnt_frames = 32,
	.max_maptrack_frames = 1,
	.gic_version = XEN_DOMCTL_CONFIG_GIC_V2,
	.tee_type = XEN_DOMCTL_CONFIG_TEE_NONE,
	.cmdline = "",
	.ssidref = 12,

	.load_image_bytes = storage_image_kernel_read,
	.get_image_size = storage_image_kernel_get_size,

};
#endif /* CONFIG_DOM_STORAGE_FATFS_ENABLE */

#if !defined(CONFIG_DOM_STORAGE_FATFS_ENABLE)
extern char __img_domu1_start[];
extern char __img_domu1_end[];
extern char __dtb_domu1_start[];
extern char __dtb_domu1_end[];

static int load_domu_image_bytes(uint8_t *buf, size_t bufsize, uint64_t offset, void *image_info)
{
	ARG_UNUSED(image_info);

	memcpy(buf, __img_domu1_start + offset, bufsize);
	return 0;
}

static ssize_t get_domu_image_size(void *image_info, uint64_t *size)
{
	ARG_UNUSED(image_info);

	*size = __img_domu1_end - __img_domu1_start;
	return 0;
}
#endif /* CONFIG_DOM_STORAGE_FATFS_ENABLE */

/*
 * Linux PV DomU guests (generic Arm64 DomU and Web DomU)
 */
#if defined(CONFIG_DOM_CFG_LINUX_PV_DOMAIN)
static const char *params_vif =
	"vif=[ 'backend=1,bridge=xenbr0,mac=08:00:27:ff:cb:ce,ip=192.168.0.2 "
	"255.255.255.0 192.168.0.1' ]";

static const char *params_vbd =
	"disk=[ 'backend=1, format=raw, vdev=xvda, "
	"access=rw, target=/etc/u-boot-initial-env' ]";

static const char *params_vif_web =
	"vif=[ 'backend=1,bridge=xenbr0,mac=08:00:27:ff:cb:cf,ip=192.168.0.3 "
	"255.255.255.0 192.168.0.1' ]";

static const char *params_vbd_web =
	"disk=[ 'backend=1, format=raw, vdev=xvda, "
	"access=rw, target=/etc/u-boot-initial-env' ]";

/*
 * Generic Linux PV DomU (used for the "classic" Linux PV demo)
 */
static struct xen_domain_cfg domu_cfg_3 = {
	.name = "linux_pv_domu",
	.mem_kb = 256 * 1024,
	.flags = (XEN_DOMCTL_CDF_hvm | XEN_DOMCTL_CDF_hap),
	.max_evtchns = 10,
	.max_vcpus = 2,
	.gnt_frames = 32,
	.max_maptrack_frames = 1,
	.gic_version = XEN_DOMCTL_CONFIG_GIC_V2,
	.tee_type = XEN_DOMCTL_CONFIG_TEE_NONE,
	.cmdline = "root=/dev/ram0 rootwait console=hvc0 clk_ignore_unused",
	.ssidref = 12,

	.load_image_bytes = storage_image_kernel_read,
	.get_image_size = storage_image_kernel_get_size,
};

/*
 * Web DomU: identical to linux_pv_domu, but with a different name and MAC/IP.
 * The initramfs runs BusyBox httpd and serves static content from /var/www.
 */
static struct xen_domain_cfg domu_cfg_4 = {
	.name = "linux_pv_domu_web",
	.mem_kb = 256 * 1024,
	.flags = (XEN_DOMCTL_CDF_hvm | XEN_DOMCTL_CDF_hap),
	.max_evtchns = 10,
	.max_vcpus = 2,
	.gnt_frames = 32,
	.max_maptrack_frames = 1,
	.gic_version = XEN_DOMCTL_CONFIG_GIC_V2,
	.tee_type = XEN_DOMCTL_CONFIG_TEE_NONE,
	.cmdline = "root=/dev/ram0 rootwait console=hvc0 clk_ignore_unused",
	.ssidref = 12,

	.load_image_bytes = storage_image_kernel_read,
	.get_image_size = storage_image_kernel_get_size,
};

static void pv_domu_init(void)
{
	parse_one_record_and_fill_cfg(params_vif, &domu_cfg_3.back_cfg);
	parse_one_record_and_fill_cfg(params_vbd, &domu_cfg_3.back_cfg);
}

static void pv_domu_web_init(void)
{
	parse_one_record_and_fill_cfg(params_vif_web, &domu_cfg_4.back_cfg);
	parse_one_record_and_fill_cfg(params_vbd_web, &domu_cfg_4.back_cfg);
}

#endif /* CONFIG_DOM_CFG_LINUX_PV_DOMAIN */

/* Zephyr DomU used for the sync demo */
static struct xen_domain_cfg domu_cfg_1 = {
	.name = "rpi_5_domu",
	.mem_kb = 16384,
	.flags = (XEN_DOMCTL_CDF_hvm | XEN_DOMCTL_CDF_hap),
	.max_evtchns = 10,
	.max_vcpus = 1,
	.gnt_frames = 32,
	.max_maptrack_frames = 1,
	.gic_version = XEN_DOMCTL_CONFIG_GIC_V2,
	.tee_type = XEN_DOMCTL_CONFIG_TEE_NONE,
	.cmdline = "",
	.ssidref = 12,

#if !defined(CONFIG_DOM_STORAGE_FATFS_ENABLE)
	.load_image_bytes = load_domu_image_bytes,
	.get_image_size = get_domu_image_size,

	.dtb_start = __dtb_domu1_start,
	.dtb_end = __dtb_domu1_end,
#else
	.load_image_bytes = storage_image_kernel_read,
	.get_image_size = storage_image_kernel_get_size,
#endif /* CONFIG_DOM_STORAGE_FATFS_ENABLE */
};

static struct xen_domain_iomem domd_iomems[] = {
	{.first_mfn = 0x107D517, .nr_mfns = 0x1, .first_gfn = 0},
};

static char *domd_dt_passthrough[] = {
	"/soc/gpio@7d517c00",
};

#if !defined(CONFIG_DOM_STORAGE_FATFS_ENABLE)
extern char __img_domu0_start[];
extern char __img_domu0_end[];
extern char __dtb_domu0_start[];
extern char __dtb_domu0_end[];

static int load_domd_image_bytes(uint8_t *buf, size_t bufsize, uint64_t offset, void *image_info)
{
	ARG_UNUSED(image_info);

	memcpy(buf, __img_domu0_start + offset, bufsize);
	return 0;
}

static ssize_t get_domd_image_size(void *image_info, uint64_t *size)
{
	ARG_UNUSED(image_info);

	*size = __img_domu0_end - __img_domu0_start;
	return 0;
}
#endif /* CONFIG_DOM_STORAGE_FATFS_ENABLE */

/* Driver domain (DomD) */
static struct xen_domain_cfg domu_cfg_0 = {
	.name = "rpi_5_domd",
	.mem_kb = 16384,
	.flags = (XEN_DOMCTL_CDF_hvm | XEN_DOMCTL_CDF_hap),
	.max_evtchns = 10,
	.max_vcpus = 1,
	.gnt_frames = 32,
	.max_maptrack_frames = 1,
	.gic_version = XEN_DOMCTL_CONFIG_GIC_V2,
	.tee_type = XEN_DOMCTL_CONFIG_TEE_NONE,
	.cmdline = "",
	.ssidref = 12,

	.iomems = domd_iomems,
	.nr_iomems = ARRAY_SIZE(domd_iomems),

	.dt_passthrough = domd_dt_passthrough,
	.nr_dt_passthrough = ARRAY_SIZE(domd_dt_passthrough),

#if !defined(CONFIG_DOM_STORAGE_FATFS_ENABLE)
	.load_image_bytes = load_domd_image_bytes,
	.get_image_size = get_domd_image_size,

	.dtb_start = __dtb_domu0_start,
	.dtb_end = __dtb_domu0_end,
#else
	.load_image_bytes = storage_image_kernel_read,
	.get_image_size = storage_image_kernel_get_size,
#endif /* CONFIG_DOM_STORAGE_FATFS_ENABLE */

};

/* Dom0less domain configuration table */
struct dom0_domain_cfg domain_cfgs[] = {
	{
		.domain_cfg = &domu_cfg_0,
#if defined(CONFIG_DOM_STORAGE_FATFS_ENABLE)
		.image_kernel_path = DISK_BIN_PATH"z_blinky.bin",
#endif /* CONFIG_DOM_STORAGE_FATFS_ENABLE */
	},
	{
		.domain_cfg = &domu_cfg_1,
#if defined(CONFIG_DOM_STORAGE_FATFS_ENABLE)
		.image_kernel_path = DISK_BIN_PATH"z_sync.bin",
#endif /* CONFIG_DOM_STORAGE_FATFS_ENABLE */
	},
#if defined(CONFIG_DOM_STORAGE_FATFS_ENABLE)
	{
		.domain_cfg = &domu_cfg_2,
		.image_kernel_path = DISK_BIN_PATH "helloworld_xen-arm64",
	},
#endif /* CONFIG_DOM_STORAGE_FATFS_ENABLE */
#if defined(CONFIG_DOM_CFG_LINUX_PV_DOMAIN)
	/*
	 * Linux PV domains:
	 *  - linux_pv_domu: generic Linux PV demo
	 *  - linux_pv_domu_web: web DomU serving BusyBox httpd content
	 */
	{
		.domain_cfg = &domu_cfg_3,
		.image_kernel_path = DISK_BIN_PATH "linux-pv-image",
		.init = pv_domu_init,
	},
	{
		.domain_cfg = &domu_cfg_4,
		.image_kernel_path = DISK_BIN_PATH "linux-pv-web-image",
		.init = pv_domu_web_init,
	},
#endif /* CONFIG_DOM_CFG_LINUX_PV_DOMAIN */
	{ 0 },
};
