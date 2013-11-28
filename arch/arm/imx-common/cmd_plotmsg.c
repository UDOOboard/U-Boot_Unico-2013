/*
 * Copyright (C) 2013 Seco USA Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <bmp_logo.h>
#include <video.h>

#define MAX_LEN		160
#define X_MARGIN	10
#define Y_PADDING	20

static int do_plotmsg(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i = 1;
	static int y = 30;
	int x;
	char msg[MAX_LEN];

	if (argc == 1)
		return CMD_RET_USAGE;

	if (y < BMP_LOGO_HEIGHT)
		x = BMP_LOGO_WIDTH + X_MARGIN;
	else
		x = X_MARGIN;

	msg[0] = 0;

	if ((strcmp(argv[1], "-r") == 0) && (y > 30)) {
		int f;
		i++;
		y -= Y_PADDING;
		for (f=0; f < MAX_LEN/10; f++)
			strcat(msg, "          ");
		udoo_video_drawstring(x, y, msg);
		strcpy(msg, argv[i]);
		i++;
	}

	if (strcmp(argv[1], "-u") == 0) {
		i++;
		x = X_MARGIN;
		y = ((BMP_LOGO_HEIGHT / Y_PADDING) * Y_PADDING) + Y_PADDING;
		strcpy(msg, argv[i]);
		i++;
	}
		
	while(i < argc) {
		if ((strlen(msg) + strlen(argv[i])) < MAX_LEN) {
			if  (i == 1) {
				strcpy(msg, argv[i]);
			} else {
				strcat(msg, " ");
				strcat(msg, argv[i]);
			}
		} else
			printf("Warn: Max text length reached (max %d characters permetted).\n", MAX_LEN);
		i++;
	}

	udoo_video_drawstring(x, y, msg);
	y += Y_PADDING;
	return 0;
}

U_BOOT_CMD(plotmsg, 50, 1, do_plotmsg,
	"Plot text message on current display (HDMI or LVDS).\n" \
	  "\t -r Overwrite last message.\n" \
	  "\t -u Start writing just under Logo.",
	"[-r|-u] message ..."
);
