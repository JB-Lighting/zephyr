/*
 * Copyright 2025 JB-Lighting Lichtanlagentechnik GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "cortex_m/arm_mpu_internal.h"

#include <zephyr/arch/arm/mpu/arm_mpu_regions_print.h>

LOG_MODULE_REGISTER(mpu_print, CONFIG_LOG_DEFAULT_LEVEL);

void print_size_human_readable(uint32_t size)
{
	if (size == 0) {
		LOG_RAW("4 GB\n");
		return;
	}

	const char *units[] = {"B", "KB", "MB", "GB"};
	int unit_index = 0;

	uint32_t display_size = size;

	while (display_size % 1024 == 0 && unit_index < 3) {
		display_size /= 1024;
		unit_index++;
	}
	LOG_RAW("%u %s\n", display_size, units[unit_index]);
}

const char *get_ap_priviledged(const uint32_t ap)
{
	switch (ap) {
	case 0:
		return "NA";
	case 1:
	case 2:
	case 3:
		return "RW";
	case 5:
	case 6:
		return "RO";
	default:
		return "Res";
	}
}
const char *get_ap_unpriviledged(const uint32_t ap)
{
	switch (ap) {
	case 0:
	case 1:
	case 5:
		return "NA";
	case 2:
	case 6:
		return "RO";
	case 3:
		return "RW";
	default:
		return "Res";
	}
}

typedef struct {
	uint32_t tex;
	uint32_t c;
	uint32_t b;
	const char *type;
	const char *desc;
} mem_types_t;

typedef struct {
	uint32_t start;
	const char *name;
} known_mem_regions_t;

mem_types_t mem_types[] = {
	{0, 0, 0, "Strongly-ordered", "Strongly-ordered"},
	{0, 0, 1, "Device", "Shared Device"},
	{0, 1, 0, "Normal", "Write-Through No Write-Allocate"},
	{0, 1, 1, "Normal", "Write-Back, no Write-Allocate"},
	{1, 0, 0, "Normal", "Non-cacheable"},
	{1, 0, 1, "Reserved", "Reserved"},
	{1, 1, 0, "Undefined", "Undefined"},
	{1, 1, 1, "Normal", "Write-Back, Write and Read Allocate"},
	{2, 0, 0, "Device", "Non-Shareable device"},
};

known_mem_regions_t known_mem_regions[] = {
	{0x00000000, "ITCM/All"},
	{0x08000000, "Flash"},
	{0x8fff000, "OTP area"},
	{0x8fff800, "Flash Read Only (ID, Trim, Stack ID)"},
	{0x20000000, "DTCM"},
	{0x24000000, "AXI SRAM"},
	{0x30000000, "AHB SRAM1"},
	{0x30004000, "AHB SRAM2"},
	{0x38800000, "BB SRAM"},
	{0x40000000, "Peripherals"},
	{0x70000000, "XSPI2"},
	{0x90000000, "XSPI1"},
};

void arm_mpu_regions_print()
{
	void *ret_addr = __builtin_return_address(0);

	LOG_RAW("Printing MPU Regions Configuration (called from %p):\n", ret_addr);

	for (uint8_t i = 0; i < get_num_regions(); i++) {
		if (is_enabled_region(i)) {
			const uint32_t base = mpu_region_get_base(i);
			const uint32_t size = mpu_region_get_size(i);
			const uint32_t rasr = MPU->RASR;
			const uint32_t rasr_xn = (rasr & MPU_RASR_XN_Msk) >> MPU_RASR_XN_Pos;
			const uint32_t rasr_ap = (rasr & MPU_RASR_AP_Msk) >> MPU_RASR_AP_Pos;
			const uint32_t rasr_tex = (rasr & MPU_RASR_TEX_Msk) >> MPU_RASR_TEX_Pos;
			const uint32_t rasr_s = (rasr & MPU_RASR_S_Msk) >> MPU_RASR_S_Pos;
			const uint32_t rasr_c = (rasr & MPU_RASR_C_Msk) >> MPU_RASR_C_Pos;
			const uint32_t rasr_b = (rasr & MPU_RASR_B_Msk) >> MPU_RASR_B_Pos;
			const uint32_t rasr_srd = (rasr & MPU_RASR_SRD_Msk) >> MPU_RASR_SRD_Pos;

			LOG_RAW("MPU Region %d:\n", i);

			LOG_RAW("   Base = 0x%x", base);
			for (size_t kmr = 0; kmr < ARRAY_SIZE(known_mem_regions); kmr++) {
				if (known_mem_regions[kmr].start == base) {
					LOG_RAW(" - Start of %s", known_mem_regions[kmr].name);
					break;
				}
			}
			LOG_RAW("\n");

			LOG_RAW("   Size = ");
			print_size_human_readable(size);

			LOG_RAW("   AP: Priv=%s, Unpriv=%s\n", get_ap_priviledged(rasr_ap),
				get_ap_unpriviledged(rasr_ap));

			LOG_RAW("   Attributes: XN%u TEX%u S%u C%u B%u SRD%x", rasr_xn, rasr_tex,
				rasr_s, rasr_c, rasr_b, rasr_srd);
			for (size_t mt = 0; mt < ARRAY_SIZE(mem_types); mt++) {
				if (mem_types[mt].tex == rasr_tex && mem_types[mt].c == rasr_c &&
				    mem_types[mt].b == rasr_b) {
					LOG_RAW(" - Memory Type: %s (%s)", mem_types[mt].type,
						mem_types[mt].desc);
					break;
				}
			}
			LOG_RAW("\n");

		} else {
			LOG_RAW("MPU Region %d: Disabled\n", i);
		}
		k_msleep(100);
	}
}
