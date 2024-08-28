// SPDX-License-Identifier: MIT
/*
 * Copyright 2006-2012 Red Hat, Inc.
 * Copyright 2018-2020 Cisco Systems, Inc. and/or its affiliates. All rights reserved.
 *
 * Author: Adam Jackson <ajax@nwnk.net>
 * Maintainer: Hans Verkuil <hverkuil-cisco@xs4all.nl>
 */
#include "ediddecode.h"

#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static char *manufacturer_name(const unsigned char *x)
{
	static char name[4];

	name[0] = ((x[0] & 0x7c) >> 2) + '@';
	name[1] = ((x[0] & 0x03) << 3) + ((x[1] & 0xe0) >> 5) + '@';
	name[2] = (x[1] & 0x1f) + '@';
	name[3] = 0;

	if (!isupper(name[0]) || !isupper(name[1]) || !isupper(name[2]))
		fail("Manufacturer name field contains garbage.\n");

	return name;
}

/* extract a string from a detailed subblock, checking for termination */
char *extract_string(const unsigned char *x, unsigned len)
{
	static char s[EDID_PAGE_SIZE];
	bool seen_newline = false;
	unsigned i;

	memset(s, 0, sizeof(s));

	for (i = 0; i < len; i++) {
		if (seen_newline) {
			if (x[i] != 0x20) {
				fail("Non-space after newline.\n");
				return s;
			}
		} else if (isgraph(x[i]) || x[i] == 0x20) {
			s[i] = x[i];
		} else if (x[i] == 0x0a) {
			seen_newline = true;
			if (!i)
				fail("Empty string.\n");
			else if (s[i - 1] == 0x20)
				fail("One or more trailing spaces.\n");
		} else {
			fail("Non-printable character.\n");
			return s;
		}
	}
	/* Does the string end with a space? */
	if (!seen_newline && s[len - 1] == 0x20)
		fail("One or more trailing spaces.\n");

	return s;
}


void edid_state::detailed_block(const unsigned char *x,QString &str)
{
	static const unsigned char zero_descr[18] = { 0 };
	unsigned cnt;
	unsigned i;

	base.detailed_block_cnt++;
	if (x[0] || x[1]) {
		// detailed_timings("    ", x);
		return;
	}

	data_block = "Display Descriptor #" + std::to_string(base.detailed_block_cnt);
	/* Monitor descriptor block, not detailed timing descriptor. */
	if (x[2] != 0) {
		/* 1.3, 3.10.3 */
		fail("Monitor descriptor block has byte 2 nonzero (0x%02x).\n", x[2]);
	}
	if ((base.edid_minor < 4 || x[3] != 0xfd) && x[4] != 0x00) {
		/* 1.3, 3.10.3 */
		fail("Monitor descriptor block has byte 4 nonzero (0x%02x).\n", x[4]);
	}

	base.seen_non_detailed_descriptor = true;
	if (base.edid_minor == 0)
		fail("Has descriptor blocks other than detailed timings.\n");

	if (!memcmp(x, zero_descr, sizeof(zero_descr))) {
		// data_block = "Empty Descriptor";
		return;
	}

	switch (x[3]) {
	case 0x0e:
		// detailed_epi(x);
		return;
	case 0x10:

		return;
	case 0xf7:

		return;
	case 0xf8:
		// data_block = "CVT 3 Byte Timing Codes";
		return;
	case 0xf9:
		// data_block = "Display Color Management Data";
		return;
	case 0xfa:
		// data_block = "Standard Timing Identifications";
		return;
	case 0xfb: {

		// data_block = "Color Point Data";
		return;
	}
	case 0xfc:
		data_block = "Display Product Name";
		base.has_name_descriptor = 1;
		printf("    %s: '%s'\n", data_block.c_str(), extract_string(x + 5, 13));
		return;
	case 0xfd:
		// detailed_display_range_limits(x);
		return;
	case 0xfe:
		if (!base.has_spwg || base.detailed_block_cnt < 3) {
			data_block = "Alphanumeric Data String";
            str = QString(extract_string(x + 5, 13));
			printf("    %s: '%s'\n", data_block.c_str(),  extract_string(x + 5, 13));
			return;
		}
		// 	data_block = "SPWG Descriptor #3";

		return;
	case 0xff: {
		// data_block = "Display Product Serial Number";
		return;
	}
	default:
		printf("    %s Display Descriptor (0x%02hhx):",
		       x[3] <= 0x0f ? "Manufacturer-Specified" : "Unknown", x[3]);
		// hex_block(" ", x + 2, 16);
		if (x[3] > 0x0f)
			fail("Unknown Type 0x%02hhx.\n", x[3]);
		return;
	}
}

/*
 * The sRGB chromaticities are (x, y):
 * red:   0.640,  0.330
 * green: 0.300,  0.600
 * blue:  0.150,  0.060
 * white: 0.3127, 0.3290
 */
static const unsigned char srgb_chromaticity[10] = {
	0xee, 0x91, 0xa3, 0x54, 0x4c, 0x99, 0x26, 0x0f, 0x50, 0x54
};



void edid_state::parse_base_block(const unsigned char *x,QString &str)
{
	time_t the_time;
	struct tm *ptm;
	int analog;
	unsigned col_x, col_y;
	bool has_preferred_timing = false;

	data_block = "EDID Structure Version & Revision";
    printf("  %s: %hhu.%hhu\n", data_block.c_str(), x[0x12], x[0x13]);
	if (x[0x12] == 1) {
		base.edid_minor = x[0x13];
		if (base.edid_minor > 4)
			warn("Unknown EDID minor version %u, assuming 1.4 conformance.\n", base.edid_minor);
		if (base.edid_minor < 3)
			warn("EDID 1.%u is deprecated, do not use.\n", base.edid_minor);
	} else {
		fail("Unknown EDID major version.\n");
	}

	base.serial_number = x[0x0c] + (x[0x0d] << 8) +	(x[0x0e] << 16) + (x[0x0f] << 24);


	data_block = "Vendor & Product Identification";
    printf("  %s:\n", data_block.c_str());
    printf("    Manufacturer: %s\n    Model: %x\n",       manufacturer_name(x + 0x08),      (x[0x0a] + (x[0x0b] << 8)));
	if (base.serial_number) {
		unsigned sn = base.serial_number;
		printf("    Serial Number: %u (0x%08x)\n", sn, sn);

		// This is a list of known dummy values that are often used in EDIDs:
		switch (sn) {
		case 1:
		case 0x01010101:
		case 1010101:
		case 0x5445:
		case 0x80000000:
		case 20000080:
		case 8888:
		case 6666:
			warn("The serial number is one of the known dummy values, it should probably be set to 0.\n");
			break;
		}
	}

	time(&the_time);
	ptm = localtime(&the_time);

	base.week = x[0x10];
	base.year = x[0x11];
	unsigned char week = base.week;
	int year = 1990 + base.year;

	if (week) {
		if (base.edid_minor <= 3 && week == 0xff)
			fail("EDID 1.3 does not support week 0xff.\n");
		// The max week is 53 in EDID 1.3 and 54 in EDID 1.4.
		// No idea why there is a difference.
		if (base.edid_minor <= 3 && week == 54)
			fail("EDID 1.3 does not support week 54.\n");
		if (week != 0xff && week > 54)
			fail("Invalid week of manufacture (> 54).\n");
	}
	if (year - 1 > ptm->tm_year + 1900)
		fail("The year is more than one year in the future.\n");

	if (week == 0xff)
		printf("    Model year: %d\n", year);
	else if (replace_unique_ids)
		printf("    Made in: 2000\n");
	else if (week)
		printf("    Made in: week %hhu of %d\n", week, year);
	else
		printf("    Made in: %d\n", year);

	/* display section */

	// data_block = "Basic Display Parameters & Features";
	// printf("  %s:\n", data_block.c_str());
	// if (x[0x14] & 0x80) {
	// 	analog = 0;
	// 	printf("    Digital display\n");
	// 	if (base.edid_minor >= 4) {
	// 		if ((x[0x14] & 0x70) == 0x00)
	// 			printf("    Color depth is undefined\n");
	// 		else if ((x[0x14] & 0x70) == 0x70)
	// 			fail("Color Bit Depth set to reserved value.\n");
	// 		else
	// 			printf("    Bits per primary color channel: %u\n",
	// 			       ((x[0x14] & 0x70) >> 3) + 4);

	// 		printf("    ");
	// 		switch (x[0x14] & 0x0f) {
	// 		case 0x00: printf("Digital interface is not defined\n"); break;
	// 		case 0x01: printf("DVI interface\n"); break;
	// 		case 0x02: printf("HDMI-a interface\n"); break;
	// 		case 0x03: printf("HDMI-b interface\n"); break;
	// 		case 0x04: printf("MDDI interface\n"); break;
	// 		case 0x05: printf("DisplayPort interface\n"); break;
	// 		default:
	// 			   printf("Unknown interface: 0x%02x\n", x[0x14] & 0x0f);
	// 			   fail("Digital Video Interface Standard set to reserved value 0x%02x.\n", x[0x14] & 0x0f);
	// 			   break;
	// 		}
	// 	} else if (base.edid_minor >= 2) {
	// 		if (x[0x14] & 0x01) {
	// 			printf("    DFP 1.x compatible TMDS\n");
	// 		}
	// 		if (x[0x14] & 0x7e)
	// 			fail("Digital Video Interface Standard set to reserved value 0x%02x.\n", x[0x14] & 0x7e);
	// 	} else if (x[0x14] & 0x7f) {
	// 		fail("Digital Video Interface Standard set to reserved value 0x%02x.\n", x[0x14] & 0x7f);
	// 	}
	// } else {
	// 	static const char * const voltages[] = {
	// 		"0.700 : 0.300 : 1.000 V p-p",
	// 		"0.714 : 0.286 : 1.000 V p-p",
	// 		"1.000 : 0.400 : 1.400 V p-p",
	// 		"0.700 : 0.000 : 0.700 V p-p"
	// 	};
	// 	unsigned voltage = (x[0x14] & 0x60) >> 5;
	// 	unsigned sync = (x[0x14] & 0x0f);

	// 	analog = 1;
	// 	printf("    Analog display\n");
	// 	printf("    Signal Level Standard: %s\n", voltages[voltage]);

	// 	if (x[0x14] & 0x10)
	// 		printf("    Blank-to-black setup/pedestal\n");
	// 	else
	// 		printf("    Blank level equals black level\n");

	// 	if (sync)
	// 		printf("    Sync:%s%s%s%s\n",
	// 		       sync & 0x08 ? " Separate" : "",
	// 		       sync & 0x04 ? " Composite" : "",
	// 		       sync & 0x02 ? " SyncOnGreen" : "",
	// 		       sync & 0x01 ? " Serration" : "");
	// }

	if (x[0x15] && x[0x16]) {
		printf("    Maximum image size: %u cm x %u cm\n", x[0x15], x[0x16]);
		base.max_display_width_mm = x[0x15] * 10;
		base.max_display_height_mm = x[0x16] * 10;
		image_width = base.max_display_width_mm * 10;
		image_height = base.max_display_height_mm * 10;
		if (x[0x15] < 10 || x[0x16] < 10)
			warn("Dubious maximum image size (%ux%u is smaller than 10x10 cm).\n",
			     x[0x15], x[0x16]);
	}
	// else if (base.edid_minor >= 4 && (x[0x15] || x[0x16])) {
	// 	if (x[0x15])
	// 		printf("    Aspect ratio: %.2f (landscape)\n", (x[0x15] + 99) / 100.0);
	// 	else
	// 		printf("    Aspect ratio: %.2f (portrait)\n", 100.0 / (x[0x16] + 99));
	// } else {
	// 	/* Either or both can be zero for 1.3 and before */
	// 	printf("    Image size is variable\n");
	// }

	// if (x[0x17] == 0xff)
	// 	printf("    Gamma is defined in an extension block\n");
	// else
	// 	printf("    Gamma: %.2f\n", (x[0x17] + 100.0) / 100.0);

	// if (x[0x18] & 0xe0) {
	// 	printf("    DPMS levels:");
	// 	if (x[0x18] & 0x80) printf(" Standby");
	// 	if (x[0x18] & 0x40) printf(" Suspend");
	// 	if (x[0x18] & 0x20) printf(" Off");
	// 	printf("\n");
	// }

	// if (analog || base.edid_minor < 4) {
	// 	printf("    ");
	// 	switch (x[0x18] & 0x18) {
	// 	case 0x00: printf("Monochrome or grayscale display\n"); break;
	// 	case 0x08: printf("RGB color display\n"); break;
	// 	case 0x10: printf("Non-RGB color display\n"); break;
	// 	case 0x18: printf("Undefined display color type\n"); break;
	// 	}
	// } else {
	// 	printf("    Supported color formats: RGB 4:4:4");
	// 	if (x[0x18] & 0x08)
	// 		printf(", YCrCb 4:4:4");
	// 	if (x[0x18] & 0x10)
	// 		printf(", YCrCb 4:2:2");
	// 	printf("\n");
	// }

	// if (x[0x18] & 0x04) {
	// 	printf("    Default (sRGB) color space is primary color space\n");
	// 	if (memcmp(x + 0x19, srgb_chromaticity, sizeof(srgb_chromaticity)))
	// 		fail("sRGB is signaled, but the chromaticities do not match.\n");
	// 	if (x[0x17] != 120)
	// 		warn("sRGB is signaled, but the gamma != 2.2.\n");
	// 	base.uses_srgb = true;
	// } else if (!memcmp(x + 0x19, srgb_chromaticity, sizeof(srgb_chromaticity))) {
	// 	fail("The chromaticities match sRGB, but sRGB is not signaled.\n");
	// 	base.uses_srgb = true;
	// }

	// if (base.edid_minor >= 4) {
	// 	/* 1.4 always has a preferred timing and this bit means something else. */
	// 	has_preferred_timing = true;
	// 	base.preferred_is_also_native = x[0x18] & 0x02;
	// 	printf("    First detailed timing %s the native pixel format and preferred refresh rate\n",
	// 	       base.preferred_is_also_native ? "includes" : "does not include");
	// } else {
	// 	if (x[0x18] & 0x02) {
	// 		printf("    First detailed timing is the preferred timing\n");
	// 		has_preferred_timing = true;
	// 		// 1.3 recommends that the preferred timing corresponds to the
	// 		// native timing, but it is not a requirement.
	// 		// That said, we continue with the assumption that it actually
	// 		// is the native timing.
	// 		base.preferred_is_also_native = true;
	// 	} else if (base.edid_minor == 3) {
	// 		fail("EDID 1.3 requires that the first detailed timing is the preferred timing.\n");
	// 	}
	// }

	if (x[0x18] & 0x01) {
		if (base.edid_minor >= 4) {
			base.supports_continuous_freq = true;
			printf("    Display supports continuous frequencies\n");
		} else {
			printf("    Supports GTF timings within operating range\n");
			base.supports_gtf = true;
		}
	}

	// data_block = "Color Characteristics";
	// printf("  %s:\n", data_block.c_str());
	// col_x = (x[0x1b] << 2) | (x[0x19] >> 6);
	// col_y = (x[0x1c] << 2) | ((x[0x19] >> 4) & 3);
	// printf("    Red  : 0.%04u, 0.%04u\n",
	//        (col_x * 10000) / 1024, (col_y * 10000) / 1024);
	// col_x = (x[0x1d] << 2) | ((x[0x19] >> 2) & 3);
	// col_y = (x[0x1e] << 2) | (x[0x19] & 3);
	// printf("    Green: 0.%04u, 0.%04u\n",
	//        (col_x * 10000) / 1024, (col_y * 10000) / 1024);
	// col_x = (x[0x1f] << 2) | (x[0x1a] >> 6);
	// col_y = (x[0x20] << 2) | ((x[0x1a] >> 4) & 3);
	// printf("    Blue : 0.%04u, 0.%04u\n",
	//        (col_x * 10000) / 1024, (col_y * 10000) / 1024);
	// col_x = (x[0x21] << 2) | ((x[0x1a] >> 2) & 3);
	// col_y = (x[0x22] << 2) | (x[0x1a] & 3);
	// printf("    White: 0.%04u, 0.%04u\n",
	//        (col_x * 10000) / 1024, (col_y * 10000) / 1024);

	// data_block = "Established Timings I & II";
	// if (x[0x23] || x[0x24] || x[0x25]) {
	// 	printf("  %s:\n", data_block.c_str());
	// 	for (unsigned i = 0; i < ARRAY_SIZE(established_timings12); i++) {
	// 		if (x[0x23 + i / 8] & (1 << (7 - i % 8))) {
	// 			unsigned char dmt_id = established_timings12[i].dmt_id;
	// 			const struct timings *t;
	// 			char type[16];

	// 			if (dmt_id) {
	// 				sprintf(type, "DMT 0x%02x", dmt_id);
	// 				// t = find_dmt_id(dmt_id);
	// 			} else {
	// 				t = &established_timings12[i].t;
	// 				sprintf(type, "%-8s", established_timings12[i].type);
	// 			}
	// 		}
	// 	}
	// } else {
	// 	printf("  %s: none\n", data_block.c_str());
	// }
	base.has_640x480p60_est_timing = x[0x23] & 0x20;

	// data_block = "Standard Timings";
	// bool found = false;
	// for (unsigned i = 0; i < 8; i++) {
	// 	if (x[0x26 + i * 2] != 0x01 || x[0x26 + i * 2 + 1] != 0x01) {
	// 		found = true;
	// 		break;
	// 	}
	// }
	// if (found) {
	// 	printf("  %s:\n", data_block.c_str());
	// 	for (unsigned i = 0; i < 8; i++)
	// 		print_standard_timing("    ", x[0x26 + i * 2], x[0x26 + i * 2 + 1]);
	// } else {
	// 	printf("  %s: none\n", data_block.c_str());
	// }

	// /* 18 byte descriptors */
	// if (has_preferred_timing && !x[0x36] && !x[0x37])
	// 	fail("Missing preferred timing.\n");

	// /* Look for SPWG Noteboook Panel EDID data blocks */
	// if ((x[0x36] || x[0x37]) &&
	//     (x[0x48] || x[0x49]) &&
	//     !x[0x5a] && !x[0x5b] && x[0x5d] == 0xfe &&
	//     !x[0x6c] && !x[0x6d] && x[0x6f] == 0xfe &&
	//     (x[0x79] == 1 || x[0x79] == 2) && x[0x7a] <= 1)
	// 	base.has_spwg = true;

	// for (unsigned i = 0; i < (base.has_spwg ? 2 : 4); i++)
	// 	if (x[0x36 + i * 18] || x[0x37 + i * 18])
	// 		cta.preparsed_total_dtds++;

	data_block = "Detailed Timing Descriptors";
	printf("  %s:\n", data_block.c_str());
    detailed_block(x + 0x36,str);
    detailed_block(x + 0x48,str);
    detailed_block(x + 0x5a,str);
    detailed_block(x + 0x6c,str);
	// base.has_spwg = false;
	// if (!base.preferred_is_also_native) {
	// 	cta.native_timings.clear();
	// 	base.preferred_timing = timings_ext();
	// }

	data_block = block;
	// if (x[0x7e])
	// 	printf("  Extension blocks: %u\n", x[0x7e]);

	block = block_name(0x00);
	data_block.clear();
	// do_checksum("", x, EDID_PAGE_SIZE);
	// if (base.edid_minor >= 3) {
	// 	if (!base.has_name_descriptor)
	// 		msg(base.edid_minor >= 4, "Missing Display Product Name.\n");
	// 	if ((base.edid_minor == 3 || base.supports_continuous_freq) &&
	// 	    !base.has_display_range_descriptor)
	// 		fail("Missing Display Range Limits Descriptor.\n");
	// }
}


//-------------------------

#define STR(x) #x
#define STRING(x) STR(x)

static edid_state state;

static unsigned char edid[EDID_PAGE_SIZE * EDID_MAX_BLOCKS];
static bool odd_hex_digits;

enum output_format {
	OUT_FMT_DEFAULT,
	OUT_FMT_HEX,
	OUT_FMT_RAW,
	OUT_FMT_CARRAY,
	OUT_FMT_XML,
};




static std::string s_msgs[EDID_MAX_BLOCKS + 1][2];

void msg(bool is_warn, const char *fmt, ...)
{
	char buf[1024] = "";
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	if (is_warn)
		state.warnings++;
	else
		state.failures++;
	if (state.data_block.empty())
		s_msgs[state.block_nr][is_warn] += std::string("  ") + buf;
	else
		s_msgs[state.block_nr][is_warn] += "  " + state.data_block + ": " + buf;

	// if (options[OptCheckInline])
		printf("%s: %s", is_warn ? "WARN" : "FAIL", buf);
}

// static void show_msgs(bool is_warn)
// {
// 	printf("\n%s:\n\n", is_warn ? "Warnings" : "Failures");
// 	for (unsigned i = 0; i < state.num_blocks; i++) {
// 		if (s_msgs[i][is_warn].empty())
// 			continue;
// 		printf("Block %u, %s:\n%s",
// 		       i, block_name(edid[i * EDID_PAGE_SIZE]).c_str(),
// 		       s_msgs[i][is_warn].c_str());
// 	}
// 	if (s_msgs[EDID_MAX_BLOCKS][is_warn].empty())
// 		return;
// 	printf("EDID:\n%s",
// 	       s_msgs[EDID_MAX_BLOCKS][is_warn].c_str());
// }


// void replace_checksum(unsigned char *x, size_t len)
// {
// 	unsigned char sum = 0;
// 	unsigned i;

// 	for (i = 0; i < len - 1; i++)
// 		sum += x[i];
// 	x[len - 1] = -sum & 0xff;
// }

void do_checksum(const char *prefix, const unsigned char *x, size_t len, unsigned unused_bytes)
{
	unsigned char check = x[len - 1];
	unsigned char sum = 0;
	unsigned i;

	for (i = 0; i < len - 1; i++)
		sum += x[i];

	printf("%sChecksum: 0x%02hhx", prefix, check);
	if ((unsigned char)(check + sum) != 0) {
		printf(" (should be 0x%02x)", -sum & 0xff);
		fail("Invalid checksum 0x%02x (should be 0x%02x).\n",
		     check, -sum & 0xff);
	}
	if (unused_bytes)
		printf("  Unused space in Extension Block: %u byte%s",
		       unused_bytes, unused_bytes > 1 ? "s" : "");
	printf("\n");
}

unsigned gcd(unsigned a, unsigned b)
{
	while (b) {
		unsigned t = b;

		b = a % b;
		a = t;
	}
	return a;
}

void calc_ratio(struct timings *t)
{
	unsigned d = gcd(t->hact, t->vact);

	if (d == 0) {
		t->hratio = t->vratio = 0;
		return;
	}
	t->hratio = t->hact / d;
	t->vratio = t->vact / d;

	if (t->hratio == 8 && t->vratio == 5) {
		t->hratio = 16;
		t->vratio = 10;
	}
}

std::string edid_state::dtd_type(unsigned cnt)
{
	unsigned len = std::to_string(cta.preparsed_total_dtds).length();
	char buf[16];
	sprintf(buf, "DTD %*u", len, cnt);
	return buf;
}


static void or_str(std::string &s, const std::string &flag, unsigned &num_flags)
{
	if (!num_flags)
		s = flag;
	else if (num_flags % 2 == 0)
		s = s + " | \\\n\t\t" + flag;
	else
		s = s + " | " + flag;
	num_flags++;
}



std::string utohex(unsigned char x)
{
	char buf[10];

	sprintf(buf, "0x%02hhx", x);
	return buf;
}

std::string ouitohex(unsigned oui)
{
	char buf[32];

	sprintf(buf, "%02X-%02X-%02X", (oui >> 16) & 0xff, (oui >> 8) & 0xff, oui & 0xff);
	return buf;
}

bool memchk(const unsigned char *x, unsigned len, unsigned char v)
{
	for (unsigned i = 0; i < len; i++)
		if (x[i] != v)
			return false;
	return true;
}

static bool edid_add_byte(const char *s, bool two_digits = true)
{
	char buf[3];

	if (state.edid_size == sizeof(edid))
		return false;
	buf[0] = s[0];
	buf[1] = two_digits ? s[1] : 0;
	buf[2] = 0;
	edid[state.edid_size++] = strtoul(buf, NULL, 16);
	return true;
}

static bool extract_edid_quantumdata(const char *start)
{
	/* Parse QuantumData 980 EDID files */
	do {
		start = strstr(start, ">");
		if (!start)
			return false;
		start++;
		for (unsigned i = 0; start[i] && start[i + 1] && i < 256; i += 2)
			if (!edid_add_byte(start + i))
				return false;
		start = strstr(start, "<BLOCK");
	} while (start);
	return state.edid_size;
}

static const char *ignore_chars = ",:;";

static bool extract_edid_hex(const char *s, bool require_two_digits = true)
{
	for (; *s; s++) {
		if (isspace(*s) || strchr(ignore_chars, *s))
			continue;

		if (*s == '0' && tolower(s[1]) == 'x') {
			s++;
			continue;
		}

		/* Read one or two hex digits from the log */
		if (!isxdigit(s[0])) {
			if (state.edid_size && state.edid_size % 128 == 0)
				break;
			return false;
		}
		if (require_two_digits && !isxdigit(s[1])) {
			odd_hex_digits = true;
			return false;
		}
		if (!edid_add_byte(s, isxdigit(s[1])))
			return false;
		if (isxdigit(s[1]))
			s++;
	}
	return state.edid_size;
}

static bool extract_edid_xrandr(const char *start)
{
	static const char indentation1[] = "                ";
	static const char indentation2[] = "\t\t";
	/* Used to detect that we've gone past the EDID property */
	static const char half_indentation1[] = "        ";
	static const char half_indentation2[] = "\t";
	const char *indentation;
	const char *s;

	for (;;) {
		unsigned j;

		/* Get the next start of the line of EDID hex, assuming spaces for indentation */
		s = strstr(start, indentation = indentation1);
		/* Did we skip the start of another property? */
		if (s && s > strstr(start, half_indentation1))
			break;

		/* If we failed, retry assuming tabs for indentation */
		if (!s) {
			s = strstr(start, indentation = indentation2);
			/* Did we skip the start of another property? */
			if (s && s > strstr(start, half_indentation2))
				break;
		}

		if (!s)
			break;

		start = s + strlen(indentation);

		for (j = 0; j < 16; j++, start += 2) {
			/* Read a %02x from the log */
			if (!isxdigit(start[0]) || !isxdigit(start[1])) {
				if (j)
					break;
				return false;
			}
			if (!edid_add_byte(start))
				return false;
		}
	}
	return state.edid_size;
}

static bool extract_edid_xorg(const char *start)
{
	bool find_first_num = true;

	for (; *start; start++) {
		if (find_first_num) {
			const char *s;

			/* skip ahead to the : */
			s = strstr(start, ": \t");
			if (!s)
				s = strstr(start, ":     ");
			if (!s)
				break;
			start = s;
			/* and find the first number */
			while (!isxdigit(start[1]))
				start++;
			find_first_num = false;
			continue;
		} else {
			/* Read a %02x from the log */
			if (!isxdigit(*start)) {
				find_first_num = true;
				continue;
			}
			if (!edid_add_byte(start))
				return false;
			start++;
		}
	}
	return state.edid_size;
}

static bool extract_edid(int fd, FILE *errorr)
{
	std::vector<char> edid_data;
	char buf[EDID_PAGE_SIZE];

	for (;;) {
		ssize_t i = read(fd, buf, sizeof(buf));

		if (i < 0)
			return false;
		if (i == 0)
			break;
		edid_data.insert(edid_data.end(), buf, buf + i);
	}

	if (edid_data.empty()) {
		state.edid_size = 0;
		return false;
	}
	// Ensure it is safely terminated by a 0 char
	edid_data.push_back('\0');

	const char *data = &edid_data[0];
	const char *start;

	/* Look for edid-decode output */
	start = strstr(data, "EDID (hex):");
	if (!start)
		start = strstr(data, "edid-decode (hex):");
	if (start)
		return extract_edid_hex(strchr(start, ':'));

	/* Look for C-array */
	start = strstr(data, "unsigned char edid[] = {");
	if (start)
		return extract_edid_hex(strchr(start, '{') + 1, false);

	/* Look for QuantumData EDID output */
	start = strstr(data, "<BLOCK");
	if (start)
		return extract_edid_quantumdata(start);

	/* Look for xrandr --verbose output (lines of 16 hex bytes) */
	start = strstr(data, "EDID_DATA:");
	if (!start)
		start = strstr(data, "EDID:");
	if (start)
		return extract_edid_xrandr(start);

	/* Look for an EDID in an Xorg.0.log file */
	start = strstr(data, "EDID (in hex):");
	if (start)
		start = strstr(start, "(II)");
	if (start)
		return extract_edid_xorg(start);

	unsigned i;

	/* Is the EDID provided in hex? */
	for (i = 0; i < 32 && (isspace(data[i]) || strchr(ignore_chars, data[i]) ||
			       tolower(data[i]) == 'x' || isxdigit(data[i])); i++);

	if (i == 32)
		return extract_edid_hex(data);

	// Drop the extra '\0' byte since we now assume binary data
	edid_data.pop_back();

	/* Assume binary */
	if (edid_data.size() > sizeof(edid)) {
//		fprintf(error, "Binary EDID length %zu is greater than %zu.\n",
//			edid_data.size(), sizeof(edid));
		return false;
	}
	memcpy(edid, data, edid_data.size());
	state.edid_size = edid_data.size();
	return true;
}

static int edid_from_file(const char *from_file, FILE *error)
{
	int flags = O_RDONLY;

	int fd;

	if (!strcmp(from_file, "-")) {
		from_file = "stdin";
		fd = 0;
	} else if ((fd = open(from_file, flags)) == -1) {
		perror(from_file);
		return -1;
	}

	odd_hex_digits = false;
    if (!extract_edid(fd, error)) {
		if (!state.edid_size) {
			fprintf(error, "EDID of '%s' was empty.\n", from_file);
			return -1;
		}
		fprintf(error, "EDID extract of '%s' failed: ", from_file);
		if (odd_hex_digits)
			fprintf(error, "odd number of hexadecimal digits.\n");
		else
			fprintf(error, "unknown format.\n");
		return -1;
	}
	if (state.edid_size % EDID_PAGE_SIZE) {
		fprintf(error, "EDID length %u is not a multiple of %u.\n",
			state.edid_size, EDID_PAGE_SIZE);
		return -1;
	}
	state.num_blocks = state.edid_size / EDID_PAGE_SIZE;
	if (fd != 0)
		close(fd);

	if (memcmp(edid, "\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00", 8)) {
		fprintf(error, "No EDID header found in '%s'.\n", from_file);
		return -1;
	}
	return 0;
}

int edid_decode_alphanumeric_data_string(const char *from_file, QString &str)
{
    int ret = edid_from_file(from_file, stdout);
    if (ret) {
        return ret;
    }
    return state.parse_edid(str);
}

/* generic extension code */

std::string block_name(unsigned char block)
{
	char buf[10];

	switch (block) {
	case 0x00: return "Base EDID";
	case 0x02: return "CTA-861 Extension Block";
	case 0x10: return "Video Timing Extension Block";
	case 0x20: return "EDID 2.0 Extension Block";
	case 0x40: return "Display Information Extension Block";
	case 0x50: return "Localized String Extension Block";
	case 0x60: return "Microdisplay Interface Extension Block";
	case 0x70: return "DisplayID Extension Block";
	case 0xf0: return "Block Map Extension Block";
	case 0xff: return "Manufacturer-Specific Extension Block";
	default:
		sprintf(buf, " 0x%02x", block);
		return std::string("Unknown EDID Extension Block") + buf;
	}
}




int edid_state::parse_edid(QString &str)
{

	block = block_name(0x00);
//    printf("Block %u, %s:\n", block_nr, block.c_str());
    parse_base_block(edid,str);

	// for (unsigned i = 1; i < num_blocks; i++) {
	// 	block_nr++;
	// 	printf("\n----------------\n");
	// 	parse_extension(edid + i * EDID_PAGE_SIZE);
	// }

	// block = "";
	// block_nr = EDID_MAX_BLOCKS;

	// if (cta.has_svrs)
	// 	cta_resolve_svrs();

	// if (options[OptPreferredTimings])
		// print_preferred_timings();

	// print_native_res();



	// check_base_block(edid);
	// if (has_cta)
	// 	check_cta_blocks();
	// if (has_dispid)
	// 	check_displayid_blocks();

	printf("\n----------------\n");


//	printf("\nEDID conformity: %s\n", failures ? "FAIL" : "PASS");
	return failures ? -2 : 0;
}

#ifndef __EMSCRIPTEN__

static unsigned char crc_calc(const unsigned char *b)
{
	unsigned char sum = 0;
	unsigned i;

	for (i = 0; i < 127; i++)
		sum += b[i];
	return 256 - sum;
}

static int crc_ok(const unsigned char *b)
{
	return crc_calc(b) == b[127];
}

static void hexdumpedid(FILE *f, const unsigned char *edid, unsigned size)
{
	unsigned b, i, j;

	for (b = 0; b < size / 128; b++) {
		const unsigned char *buf = edid + 128 * b;

		if (b)
			fprintf(f, "\n");
		for (i = 0; i < 128; i += 0x10) {
			fprintf(f, "%02x", buf[i]);
			for (j = 1; j < 0x10; j++) {
				fprintf(f, " %02x", buf[i + j]);
			}
			fprintf(f, "\n");
		}
		if (!crc_ok(buf))
			fprintf(f, "Block %u has a checksum error (should be 0x%02x).\n",
				b, crc_calc(buf));
	}
}



static int edid_to_file(const char *to_file)
{
	FILE *out;

	// if (!strcmp(to_file, "-")) {
	// 	to_file = "stdout";
	// 	out = stdout;
	// } else 
	if ((out = fopen(to_file, "w")) == NULL) {
		perror(to_file);
		return -1;
	}
		hexdumpedid(out, edid, state.edid_size);

		fclose(out);
	return 0;
}

#else


#endif
