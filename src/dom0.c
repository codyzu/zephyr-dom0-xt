/*
 * Copyright (c) 2024 EPAM Systems
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <domain.h>
#include <errno.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

LOG_MODULE_REGISTER(dom0);

#include "dom0.h"

extern struct dom0_domain_cfg domain_cfgs[];

#define DOM0_AUTOSTART_CMD_LEN 64
/* Give the Zephyr shell time to initialize before executing commands. */
#define DOM0_AUTOSTART_DELAY_MS 500U

static bool dom0_has_autostart(void)
{
	int i = 0;

	while (domain_cfgs[i].domain_cfg) {
		if (domain_cfgs[i].autostart) {
			return true;
		}
		i++;
	}

	return false;
}

static int dom0_autostart_domain(struct dom0_domain_cfg *cfg)
{
	char cmd[DOM0_AUTOSTART_CMD_LEN];
	int len;
	int ret;

	if (!cfg->domain_cfg || !cfg->domain_cfg->name || !cfg->domain_cfg->name[0]) {
		LOG_ERR("autostart: invalid domain configuration entry");
		return -EINVAL;
	}

	if (cfg->autostart_domid == 0U) {
		LOG_ERR("autostart: domid is not set for %s", cfg->domain_cfg->name);
		return -EINVAL;
	}

	len = snprintf(cmd, sizeof(cmd), "xu create %s -d %u%s",
		       cfg->domain_cfg->name,
		       (unsigned int)cfg->autostart_domid,
		       cfg->autostart_create_paused ? " -p" : "");
	if ((len < 0) || (len >= DOM0_AUTOSTART_CMD_LEN)) {
		LOG_ERR("autostart: create command truncated for %s", cfg->domain_cfg->name);
		return -ENOSPC;
	}

	ret = shell_execute_cmd(NULL, cmd);
	if (ret) {
		LOG_ERR("autostart: \"%s\" failed (%d)", cmd, ret);
		return ret;
	}

	LOG_INF("autostart: created %s (domid %u)", cfg->domain_cfg->name,
		(unsigned int)cfg->autostart_domid);

	if (!cfg->autostart_unpause) {
		return 0;
	}

	len = snprintf(cmd, sizeof(cmd), "xu unpause %u", (unsigned int)cfg->autostart_domid);
	if ((len < 0) || (len >= DOM0_AUTOSTART_CMD_LEN)) {
		LOG_ERR("autostart: unpause command truncated for %s", cfg->domain_cfg->name);
		return -ENOSPC;
	}

	ret = shell_execute_cmd(NULL, cmd);
	if (ret) {
		LOG_ERR("autostart: \"%s\" failed (%d)", cmd, ret);
		return ret;
	}

	LOG_INF("autostart: unpaused %s (domid %u)", cfg->domain_cfg->name,
		(unsigned int)cfg->autostart_domid);

	return 0;
}

static void dom0_autostart_domains(void)
{
	int i = 0;

	while (domain_cfgs[i].domain_cfg) {
		if (domain_cfgs[i].autostart) {
			(void)dom0_autostart_domain(&domain_cfgs[i]);
		}
		i++;
	}
}

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
	bool autostart_needed = dom0_has_autostart();

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

	if (!ret && autostart_needed) {
		k_msleep(DOM0_AUTOSTART_DELAY_MS);
		dom0_autostart_domains();
	}

exit_err:
	return ret;
}
