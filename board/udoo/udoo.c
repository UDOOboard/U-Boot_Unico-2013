/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <malloc.h>
#include <asm/arch/mx6-pins.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/sata.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <micrel.h>
#include <miiphy.h>
#include <netdev.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mxc_hdmi.h>
#include <command.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |                   \
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define WDT_EN		IMX_GPIO_NR(5, 4)
#define WDT_TRG		IMX_GPIO_NR(3, 19)

int dram_init(void)
{
	gd->ram_size = (phys_size_t)CONFIG_DDR_MB * 1024 * 1024;

	return 0;
}

static iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_EIM_D26__UART2_TXD | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D27__UART2_RXD | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__USDHC3_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__USDHC3_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__USDHC3_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__USDHC3_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__USDHC3_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__USDHC3_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

static iomux_v3_cfg_t const wdog_pads[] = {
	MX6_PAD_EIM_A24__GPIO_5_4 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D19__GPIO_3_19,
};

static iomux_v3_cfg_t const lvds_pads[] = {
	MX6_PAD_GPIO_2__GPIO_1_2 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_GPIO_4__GPIO_1_4 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int mx6_rgmii_rework(struct phy_device *phydev)
{
	/* 
	 * Bug: Apparently uDoo does not works with Gigabit switches...
	 * Limiting speed to 10/100Mbps, and setting master mode, seems to 
	 * be the only way to have a successfull PHY auto negotiation.
	 * How to fix: Understand why Linux kernel do not have this issue.
	 */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_CTRL1000, 0x1c00);

        /* control data pad skew - devaddr = 0x02, register = 0x04 */
        ksz9031_phy_extended_write(phydev, 0x02, 
				   MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW, 
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
        /* rx data pad skew - devaddr = 0x02, register = 0x05 */
        ksz9031_phy_extended_write(phydev, 0x02, 
				   MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW, 
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
        /* tx data pad skew - devaddr = 0x02, register = 0x05 */
        ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
        /* gtx and rx clock pad skew - devaddr = 0x02, register = 0x08 */

        ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x03FF);
	return 0;
}

static iomux_v3_cfg_t const enet_pads1[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__ENET_RGMII_TXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__ENET_RGMII_TD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__ENET_RGMII_TD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__ENET_RGMII_TD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__ENET_RGMII_TD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RXC__ENET_RGMII_RXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	/* RGMII reset */
	MX6_PAD_EIM_D23__GPIO_3_23		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* alimentazione ethernet*/
	MX6_PAD_EIM_EB3__GPIO_2_31		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 32 - 1 - (MODE0) all */
	MX6_PAD_RGMII_RD0__GPIO_6_25		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 31 - 1 - (MODE1) all */
	MX6_PAD_RGMII_RD1__GPIO_6_27		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 28 - 1 - (MODE2) all */
	MX6_PAD_RGMII_RD2__GPIO_6_28		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 27 - 1 - (MODE3) all */
	MX6_PAD_RGMII_RD3__GPIO_6_29		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* pin 33 - 1 - (CLK125_EN) 125Mhz clockout enabled */
	MX6_PAD_RGMII_RX_CTL__GPIO_6_24		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const enet_pads2[] = {
	MX6_PAD_RGMII_RD0__ENET_RGMII_RD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__ENET_RGMII_RD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__ENET_RGMII_RD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__ENET_RGMII_RD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads1, ARRAY_SIZE(enet_pads1));
	udelay(20);
	gpio_direction_output(IMX_GPIO_NR(2, 31), 1); /* Power on enet */

	gpio_direction_output(IMX_GPIO_NR(3, 23), 0); /* assert PHY rst */

	gpio_direction_output(IMX_GPIO_NR(6, 24), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 25), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 27), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 28), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 29), 1);
	udelay(1000);

	gpio_set_value(IMX_GPIO_NR(3, 23), 1); /* deassert PHY rst */

	/* Need delay 100ms to exit from reset. */
	udelay(1000 * 100);

	gpio_free(IMX_GPIO_NR(6, 24));
	gpio_free(IMX_GPIO_NR(6, 25));
	gpio_free(IMX_GPIO_NR(6, 27));
	gpio_free(IMX_GPIO_NR(6, 28));
	gpio_free(IMX_GPIO_NR(6, 29));

	imx_iomux_v3_setup_multiple_pads(enet_pads2, ARRAY_SIZE(enet_pads2));
}

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
}

static void setup_iomux_wdog(void)
{
	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));
	gpio_direction_output(WDT_TRG, 0);
	gpio_direction_output(WDT_EN, 1);
	gpio_direction_input(WDT_TRG);
}

static struct fsl_esdhc_cfg usdhc_cfg = { USDHC3_BASE_ADDR };

int board_mmc_getcd(struct mmc *mmc)
{
	return 1; /* Always present */
}

int board_eth_init(bd_t *bis)
{
	uint32_t base = IMX_FEC_BASE;
	struct mii_dev *bus = NULL;
	struct phy_device *phydev = NULL;
	int ret;

	setup_iomux_enet();

#ifdef CONFIG_FEC_MXC
	bus = fec_get_miibus(base, -1);
	if (!bus)
		return 0;
	/* scan phy 4,5,6,7 */
	phydev = phy_find_by_mask(bus, (0xf << 4), PHY_INTERFACE_MODE_RGMII);

	if (!phydev) {
		free(bus);
		return 0;
	}
	printf("using phy at %d\n", phydev->addr);
	ret  = fec_probe(bis, -1, base, bus, phydev);
	if (ret) {
		printf("FEC MXC: %s:failed\n", __func__);
		free(phydev);
		free(bus);
	}
#endif
	return 0;
}

int board_mmc_init(bd_t *bis)
{
	imx_iomux_v3_setup_multiple_pads(usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
	usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	usdhc_cfg.max_bus_width = 4;

	return fsl_esdhc_initialize(bis, &usdhc_cfg);
}

#if defined(CONFIG_VIDEO_IPUV3)

struct display_info_t {
        int     bus;
        int     addr;
        int     pixfmt;
        int     (*detect)(struct display_info_t const *dev);
        void    (*enable)(struct display_info_t const *dev);
        struct  fb_videomode mode;
};

static int detect_hdmi(struct display_info_t const *dev)
{
        struct hdmi_regs *hdmi  = (struct hdmi_regs *)HDMI_ARB_BASE_ADDR;
        return readb(&hdmi->phy_stat0) & HDMI_DVI_STAT;
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
        imx_enable_hdmi_phy();
}

static int detect_lvds(struct display_info_t const *dev)
{
        return 0;
}

static void do_enable_lvds(struct display_info_t const *dev)
{
	imx_iomux_v3_setup_multiple_pads(lvds_pads, ARRAY_SIZE(lvds_pads));
	gpio_direction_output(IMX_GPIO_NR(1, 2), 1); /* LVDS power On */
	gpio_direction_output(IMX_GPIO_NR(1, 4), 1); /* LVDS backlight On */
        return;
}

static struct display_info_t const displays[] = {{
        .bus    = -1,
        .addr   = -1,
        .pixfmt = IPU_PIX_FMT_RGB666,
        .detect = detect_lvds,
        .enable = do_enable_lvds,
        .mode   = {
		// Rif. 800x480 Panel UMSH-8596MD-20T
		// To activate write "setenv panel LDB-WVGA" or leave empty.
		.name           = "LDB-WVGA",
		.refresh        = 60,
		.xres           = 800,
		.yres           = 480,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
        .bus    = -1,
        .addr   = 0,
        .pixfmt = IPU_PIX_FMT_RGB24,
        .detect = detect_hdmi,
        .enable = do_enable_hdmi,
        .mode   = {
		.name           = "HDMI",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
        .bus    = -1,
        .addr   = -1,
        .pixfmt = IPU_PIX_FMT_RGB666,
        .detect = detect_lvds,
        .enable = do_enable_lvds,
        .mode   = {
		// Rif. Panel 1024x768 - UMSH-8596MD-15T - G156XW01V0
		// To activate write "setenv panel LDB-XGA".
		.name           = "LDB-XGA",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
        .bus    = -1,
        .addr   = -1,
        .pixfmt = IPU_PIX_FMT_RGB666,
        .detect = detect_lvds,
        .enable = do_enable_lvds,
        .mode   = {
		// Rif. 1366x768 Panel CHIMEI M156B3-LA1
		// To activate write "setenv panel LDB-WXGA".
		.name           = "LDB-WXGA",
		.refresh        = 59,
		.xres           = 1368,
		.yres           = 768,
		.pixclock       = 13890,
		.left_margin    = 93,
		.right_margin   = 33,
		.upper_margin   = 22,
		.lower_margin   = 7,
		.hsync_len      = 40,
		.vsync_len      = 4,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, 
};

int board_video_skip(void)
{
        int i;
        int ret;
        char const *panel = getenv("panel");

        if (!panel) {
                for (i = 0; i < ARRAY_SIZE(displays); i++) {
                        struct display_info_t const *dev = displays+i;
                        if (dev->detect(dev)) {
                                panel = dev->mode.name;
                                printf("auto-detected panel %s\n", panel);
                                break;
                        }
                }
                if (!panel) {
                        panel = displays[0].mode.name;
                        printf("No panel detected: default to %s\n", panel);
                        i = 0;
                }
        } else {
                for (i = 0; i < ARRAY_SIZE(displays); i++) {
                        if (!strcmp(panel, displays[i].mode.name))
                                break;
                }
        }
        if (i < ARRAY_SIZE(displays)) {
                ret = ipuv3_fb_init(&displays[i].mode, 0,
                                    displays[i].pixfmt);
                if (!ret) {
                        displays[i].enable(displays+i);
                        printf("Display: %s (%ux%u)\n",
                               displays[i].mode.name,
                               displays[i].mode.xres,
                               displays[i].mode.yres);
                } else {
                        printf("LCD %s cannot be configured: %d\n",
                               displays[i].mode.name, ret);
                }
        } else {
                printf("unsupported panel %s\n", panel);
                ret = -EINVAL;
        }
        return (0 != ret);
}

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	imx_setup_hdmi();

	/* Turn on LDB0,IPU DI0 clocks */
	reg = __raw_readl(&mxc_ccm->CCGR3);
	reg |=  MXC_CCM_CCGR3_LDB_DI0_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* set LDB0, LDB1 clk select to 011/011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 |MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3<<MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
		|IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
		|IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
		|IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
		|IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
		|IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
		|IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
		|IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
		|IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~(IOMUXC_GPR3_LVDS0_MUX_CTL_MASK
			|IOMUXC_GPR3_HDMI_MUX_CTL_MASK))
		| (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
		<<IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);
}

/*
 * Show device feature strings on current display
 * around uDOO Logo.
 */
void show_boot_messages(void) 
{
	int i;
	ulong cycles = 0;
	int repeatable;
	char *plotmsg_cmd[2];
#if defined(CONFIG_MX6DL)
	char *boot_messages[7] = {
"UDOO Board 2013",
"CPU Freescale i.MX6 DualLite 1GHz",
"dual ARMv7 Cortex-A9 core",
"1GB RAM DDR3",
"Vivante GC880 GPU",
"Atmel SAM3X8E ARM Cortex-M3 CPU",
"Arduino-compatible R3 1.0 pinout",
};
#else
	char *boot_messages[7] = {
"UDOO Board 2013",
"CPU Freescale i.MX6 Quad/Dual 1GHz",
"quad/dual ARMv7 Cortex-A9 core",
"1GB RAM DDR3",
"Vivante GC2000 / GC880",
"Atmel SAM3X8E ARM Cortex-M3 CPU",
"Arduino-compatible R3 1.0 pinout",
};
#endif

	for (i=0; i<7; i++) {
		plotmsg_cmd[0] = "plotmsg";
		plotmsg_cmd[1] = boot_messages[i];
		cmd_process(0, 2, plotmsg_cmd, &repeatable, &cycles);
	}
}
#endif /* CONFIG_VIDEO_IPUV3 */

int board_early_init_f(void)
{
#if defined(CONFIG_VIDEO_IPUV3)
        setup_display();
#endif
	setup_iomux_wdog();
	setup_iomux_uart();

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
#if defined(CONFIG_VIDEO_IPUV3)
	show_boot_messages();
#endif
        return 1;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_CMD_SATA
	sata_setup();
#endif
	return 0;
}

int checkboard(void)
{
	puts("Board: UDOO\n");

	return 0;
}
