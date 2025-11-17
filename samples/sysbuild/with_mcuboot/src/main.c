/*
 * Copyright (c) 2022 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>

#ifdef CONFIG_MPU_REGIONS_PRINT 
#include <zephyr/arch/arm/mpu/arm_mpu_regions_print.h>
#endif

int main(void)
{
	printk("Address of sample %p\n", (void *)__rom_region_start);
	printk("Hello sysbuild with mcuboot! %s\n", CONFIG_BOARD);

#ifdef CONFIG_MPU_REGIONS_PRINT 
	arm_mpu_regions_print();
#endif

	return 0;
}
