/*
 * Copyright (c) 2024 EPAM Systems
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <domain.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/init.h>

LOG_MODULE_REGISTER(dom0);

#include "dom0.h"

extern struct dom0_domain_cfg domain_cfgs[];

#ifndef STRINGIFY
#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x) STRINGIFY_INNER(x)
#endif

#define DOM0_AUTOSTART_DELAY_MS 500U
#define DOM0_WEB_DOMAIN_ID 4U

#if defined(CONFIG_DOM_CFG_LINUX_PV_DOMAIN)
static int dom0_autostart_linux_pv_web(const struct device *dev)
{
    ARG_UNUSED(dev);
    int ret;

    k_msleep(DOM0_AUTOSTART_DELAY_MS);
    LOG_INF("Autostarting linux_pv_domu_web (delay %u ms, domid %u)",
            DOM0_AUTOSTART_DELAY_MS, DOM0_WEB_DOMAIN_ID);

    // ret = shell_execute_cmd(NULL, "xu create linux_pv_domu_web -d " STRINGIFY(DOM0_WEB_DOMAIN_ID) " -p");
    ret = shell_execute_cmd(NULL, "xu create linux_pv_domu_web");
    if (ret) {
        LOG_ERR("Failed to create linux_pv_domu_web (%d)", ret);
        return ret;
    }

    // ret = shell_execute_cmd(NULL, "xu unpause " STRINGIFY(DOM0_WEB_DOMAIN_ID));
    // if (ret) {
    //     LOG_ERR("Failed to unpause linux_pv_domu_web (%d)", ret);
    //     return ret;
    // }

    LOG_INF("linux_pv_domu_web started successfully");
    return 0;
}

SYS_INIT(dom0_autostart_linux_pv_web, APPLICATION, 99);
#endif

int domain_get_user_cfg_count(void)
{
	int i = 0;

	while (domain_cfgs[i].domain_cfg) {
		i++;
	}

	return i;
}

struct xen_domain_cfg *domain_get_user_cfg(int index)
{
	if (index < domain_get_user_cfg_count()) {
		return domain_cfgs[index].domain_cfg;
	}

	return NULL;
}

int main(void)
{
	int ret;
	int i = 0;

	ret = storage_init();
	if (ret) {
		goto exit_err;
	}

	/* It's required to init xenlib struct xen_domain_cfg->image_info with pointer
	 * at struct dom0_domain_cfg, so it can be passed in binary images loading callbacks like
	 * .load_image_bytes()/get_image_size(). It's the only way to pass app data
	 * back from xenlib.
	 */
	while (domain_cfgs[i].domain_cfg) {
		if (domain_cfgs[i].init) {
			domain_cfgs[i].init();
		}
		domain_cfgs[i].domain_cfg->image_info = &domain_cfgs[i];
		i++;
	}

exit_err:
	return ret;
}
