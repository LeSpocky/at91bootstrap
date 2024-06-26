// Copyright (C) 2012 Microchip Technology Inc. and its subsidiaries
//
// SPDX-License-Identifier: MIT

#include "hardware.h"
#include "arch/at91_slowclk.h"
#include "timer.h"
#include "backup.h"

#if !defined(CONFIG_SAMA5D4) && !defined(CONFIG_SAMA5D2) \
	&& !defined(CONFIG_SAMA7G5) && !defined(CONFIG_SAMA7D65)
#define SWITCH_TO_OSC32_EXPECTED_SCKCR_VALUE (AT91C_SLCKSEL_OSCSEL | AT91C_SLCKSEL_OSC32EN)
#else
#define SWITCH_TO_OSC32_EXPECTED_SCKCR_VALUE AT91C_SLCKSEL_OSCSEL
#endif

int slowclk_enable_osc32(void)
{
#if !defined(CONFIG_SAMA5D4) && !defined(CONFIG_SAMA5D2) \
	&& !defined(CONFIG_SAMA7G5) && !defined(CONFIG_SAMA7D65)
	unsigned int reg;

	/*
	 * Enable the 32768 Hz oscillator by setting the bit OSC32EN to 1
	 */
	reg = readl(AT91C_BASE_SCKCR);
	/* Do nothing if aleady set. */
	if (reg & AT91C_SLCKSEL_OSC32EN)
		return 0;
	reg |= AT91C_SLCKSEL_OSC32EN;
	writel(reg, AT91C_BASE_SCKCR);
#endif

	/* start a internal timer */
	start_interval_timer();

	return 0;
}

static void slowclk_wait_osc32_stable(void)
{
	/* VDDBU keeps feeding oscilator. No need for wait here. */
	if (backup_resume())
		return;

	/*
	 * Wait 32768 Hz Startup Time for clock stabilization (software loop)
	 * wait about 1s (1300ms)
	 */
	wait_interval_timer(1300);
}

#if !defined(CONFIG_SAMA5D4) && !defined(CONFIG_SAMA5D2) \
	&& !defined(CONFIG_SAM9X60) && !defined(CONFIG_SAM9X7) \
	&& !defined(CONFIG_SAMA7G5) && !defined(CONFIG_SAMA7D65)
static void slowclk_disable_rc32(void)
{
	unsigned int reg;

	/*
	 * Disable the 32kHz RC oscillator by setting the bit RCEN to 0
	 */
	reg = readl(AT91C_BASE_SCKCR);
	/* Do nothing if aleady set. */
	if ((reg & AT91C_SLCKSEL_RCEN) == 0)
		return;
	reg &= ~AT91C_SLCKSEL_RCEN;
	writel(reg, AT91C_BASE_SCKCR);
}
#endif /* #if !defined(CONFIG_SAMA5D4) && !defined(CONFIG_SAMA5D2) \
	&& !defined(CONFIG_SAM9X60) && !defined(CONFIG_SAM9X7) \
	&& !defined(CONFIG_SAMA7G5) && !defined(CONFIG_SAMA7D65)
	*/

static int slowclk_select_osc32(void)
{
	unsigned int reg;

	/*
	 * Switching from internal 32kHz RC oscillator to 32768 Hz oscillator
	 * by setting the bit OSCSEL to 1
	 */
	reg = readl(AT91C_BASE_SCKCR);

	/* Do nothing if aleady set. */
	if (reg & AT91C_SLCKSEL_OSCSEL)
		return 0;

	reg |= AT91C_SLCKSEL_OSCSEL;
	writel(reg, AT91C_BASE_SCKCR);

	/*
	 * Waiting 5 slow clock cycles for internal resynchronization
	 * 5 slow clock cycles = ~153 us (5 / 32768)
	 */
	udelay(153);

	return 0;
}

int slowclk_switch_osc32(void)
{
	unsigned int reg = readl(AT91C_BASE_SCKCR);

	if (reg == SWITCH_TO_OSC32_EXPECTED_SCKCR_VALUE)
		/*
		 * The slowclk configuration register is already set
		 * correctly: the osc32 is already selected and running.
		 * Skip the configuration.
		 */
		return 0;
	slowclk_wait_osc32_stable();

	slowclk_select_osc32();

#if !defined(CONFIG_SAMA5D4) && !defined(CONFIG_SAMA5D2) \
	&& !defined(CONFIG_SAM9X60) && !defined(CONFIG_SAM9X7) \
	&& !defined(CONFIG_SAMA7G5) && !defined(CONFIG_SAMA7D65)
	slowclk_disable_rc32();
#endif

	return 0;
}

/* Switch from 32768 Hz Crystal Oscillator to Internal 32 kHz RC Oscillator */
int slowclk_switch_rc32(void)
{
	unsigned int reg;

#if !defined(CONFIG_SAMA5D4) && !defined(CONFIG_SAMA5D2) \
	&& !defined(CONFIG_SAM9X60) && !defined(CONFIG_SAM9X7) \
	&& !defined(CONFIG_SAMA7G5) && !defined(CONFIG_SAMA7D65)
	/* Enable the internal 32 kHz RC oscillator for low power by writing a 1 to the RCEN bit. */
	reg = readl(AT91C_BASE_SCKCR);
	reg |= AT91C_SLCKSEL_RCEN;
	writel(reg, AT91C_BASE_SCKCR);

	/* Wait internal 32 kHz RC startup time for clock stabilization (software loop). */
	/* 500 us */
	udelay(500);
#endif

	/* Switch from 32768 Hz oscillator to internal RC by writing a 0 to the OSCSEL bit. */
	reg = readl(AT91C_BASE_SCKCR);
	reg &= ~AT91C_SLCKSEL_OSCSEL;
	writel(reg, AT91C_BASE_SCKCR);

	/* Wait 5 slow clock cycles for internal resynchronization. */
	/* 5 slow clock cycles = ~153 us (5 / 32768) */
	udelay(153);

#if !defined(CONFIG_SAMA5D4) && !defined(CONFIG_SAMA5D2) \
	&& !defined(CONFIG_SAMA7G5) && !defined(CONFIG_SAMA7D65)
	/* Disable the 32768 Hz oscillator by writing a 0 to the OSC32EN bit. */
	reg = readl(AT91C_BASE_SCKCR);
	reg &= ~AT91C_SLCKSEL_OSC32EN;
	writel(reg, AT91C_BASE_SCKCR);
#endif

	return 0;
}

#ifdef CONFIG_SCLK_BYPASS
static int slowclk_osc32_bypass(void)
{
#if !defined(CONFIG_SAMA5D4) && !defined(CONFIG_SAMA5D2) \
	&& !defined(CONFIG_SAMA7G5) && !defined(CONFIG_SAMA7D65)
	unsigned int reg;

	/* Bypass the 32kHz oscillator by using an external clock
	 * Set OSC32BYP=1 and OSC32EN=0 atomically
	 */
	reg = readl(AT91C_BASE_SCKCR);
	/* 32768 Hz oscillator is bypassed, accept an external slow clock on XIN32 */
	reg |= AT91C_SLCKSEL_OSC32BYP;
	/* 32768 Hz oscillator is disabled. */
	reg &= ~AT91C_SLCKSEL_OSC32EN;
	writel(reg, AT91C_BASE_SCKCR);

	/*
	 * Switching from internal 32kHz RC oscillator to 32768 Hz oscillator
	 * by setting the bit OSCSEL to 1
	 */
	reg = readl(AT91C_BASE_SCKCR);
	reg |= AT91C_SLCKSEL_OSCSEL;
	writel(reg, AT91C_BASE_SCKCR);
#endif

	/*
	 * Waiting 5 slow clock cycles for internal resynchronization
	 * 5 slow clock cycles = ~153 us (5 / 32768)
	 */
	udelay(153);

#if !defined(CONFIG_SAMA5D4) && !defined(CONFIG_SAMA5D2) \
	&& !defined(CONFIG_SAM9X60) && !defined(CONFIG_SAM9X7) \
	&& !defined(CONFIG_SAMA7G5) && !defined(CONFIG_SAMA7D65)

	/*
	 * Disable the 32kHz RC oscillator by setting the bit RCEN to 0
	 */
	slowclk_disable_rc32();
#endif /* #if !defined(CONFIG_SAMA5D4) && !defined(CONFIG_SAMA5D2) \
	&& !defined(CONFIG_SAM9X60) && !defined(CONFIG_SAM9X7) \
	&& !defined(CONFIG_SAMA7G5) && !defined(CONFIG_SAMA7D65)
	*/

	return 0;
}

/* Accept an external slow clock on XIN32 */
int slowclk_switch_osc32_bypass(void)
{
	/*
	 * On Atmel sama5d3x, if no quartz is connected to the 32768 Hz oscillator,
	 * the SoC must not be resetted while OSC32BYP=1 and OSC32EN=1 and OSCSEL=1,
	 * because it would cause the SoC to be blocked in reset.
	 */

	/* switch to internal RC32 ocsillator, in order to be sure to
	 * have a reliable source for slow clock, at any moment during
	 * the following steps
	 */
	slowclk_switch_rc32();

	/* Enable OSC32, as it is needed for power supply of the osc by-pass cell
	 */
	slowclk_enable_osc32();

	slowclk_osc32_bypass();

	return 0;
}
#endif /* CONFIG_SCLK_BYPASS */
