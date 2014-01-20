/*
 * Copyright 2000-2013
 * Giuseppe Pagano, Seco s.r.l., giuseppe.pagano@seco.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>

#define DEF_bootargs_defaults "console=ttymxc1,115200 root=/dev/mmcblk0p${sd_part} rootwait rw fixrtc rootflags=barrier=1 fbmem=24M video=mxcfb0:dev=hdmi,1920x1080M@60,if=RGB24"
#define DEF_bootcmd_defaults "plotmsg -u Booting from mmcblk0p${sd_part}. Please wait...; mmc dev 0; ext2load mmc 0:${sd_part} 0x10800000 /boot/uImage; bootm 0x10800000"

#define msleep(a) udelay(a * 1000)
int default_boot = 1;

void do_show_environment(int ch, char *bootvars) {
	char tempstr[500];
	char settempstr[500];
	char env_var_name[300];

	sprintf(env_var_name, "%s_run_%d", bootvars, ch);
	if (getenv(env_var_name) != NULL) {
		strcpy (tempstr, getenv(env_var_name));
		printf("  %s\n", tempstr);
	} else {
		sprintf(env_var_name, "%s_%d", bootvars, ch);
		if (getenv(env_var_name) != NULL) {
			strcpy (tempstr, getenv(env_var_name));
			printf("  setenv %s '%s'\n", bootvars, tempstr);
		} else {
			sprintf(env_var_name, "sd_part_%d", ch);
			if (getenv(env_var_name) != NULL) {
				strcpy(tempstr, getenv(env_var_name));
				printf("  setenv sd_part '%s'\n", tempstr);
			} else {
				printf("  setenv sd_part '%d'\n", ch);
			}
			if (strcmp(bootvars, "bootargs") == 0) {
				if (getenv("bootargs_defaults") != NULL) {
					strcpy(tempstr, strdup(getenv("bootargs_defaults")));
					printf("  bootargs_defaults='%s'\n", tempstr);
				} else {
					strcpy(tempstr, DEF_bootargs_defaults);
				}
				sprintf(settempstr, "  setenv bootargs '%s'\n", tempstr);
				printf(settempstr);
			} else {
				if (getenv("bootcmd_defaults") != NULL) {
					strcpy(tempstr, strdup(getenv("bootcmd_defaults")));
					printf("  bootcmd_defaults='%s'\n", tempstr);
				} else {
					strcpy(tempstr, DEF_bootcmd_defaults);
				}
				sprintf(settempstr, "  setenv bootcmd '%s'\n", tempstr);
				printf(settempstr);
			}
		}
	}
}

void do_detailed_environment(void) {
	int counter = 1;
	char *boot_descr;
	char boot_descr_opt[300];

	sprintf(boot_descr_opt, "boot_descr_1");
	printf ("\nShowing multiboot possible action.\n");

	while (getenv(boot_descr_opt) != NULL) {
		sprintf(boot_descr_opt, "boot_descr_%d", counter);
		boot_descr = strdup(getenv(boot_descr_opt));
		if (getenv(boot_descr_opt) == NULL) {
			break;
		}
		printf("%d) %s.\n", counter, boot_descr);
		do_show_environment(counter, "bootargs");
		do_show_environment(counter, "bootcmd");
		printf("  boot\n");
		counter++;
	}
	return;
}

int show_multioption(void) {
	int counter = 1;
	char *boot_descr;
	char  boot_descr_opt[300];
	char *default_boot_str;

	sprintf(boot_descr_opt, "boot_descr_1");
	printf ("Chose system you want to boot.\n");

	if (getenv("default_boot") != NULL) {
		default_boot_str = strdup(getenv("default_boot"));
		default_boot = default_boot_str[0] - '0';
	}

	while (getenv(boot_descr_opt) != NULL) {
		sprintf(boot_descr_opt, "boot_descr_%d", counter);
		boot_descr = strdup(getenv(boot_descr_opt));
		if (getenv(boot_descr_opt) == NULL) {
			printf("  p) Show what would be done.\n");
			printf("  q) Quit.\n");
			break;
		}
		if (counter == default_boot)
			printf("> %d) %s.\n", counter, boot_descr);
		else
			printf("  %d) %s.\n", counter, boot_descr);
		counter++;
	}
	if ((counter > 1) && (default_boot >= counter)) {
		printf("Warn ! Invalid default_boot value: ->%c<- (non numeric or not existing entry).\n", default_boot_str[0]);
	        default_boot = 1;
	}
	printf ("  > ");
	return --counter;
}

int do_detailed_help(void) {

	printf("\n\tHow to setup u-boot environment for multiboot utility.\n");

	printf("\nStep 1) Define as much \"boot_descr_X\" variable as you need assigning to it\n");
	printf("\ta title rapresenting bootable system number X. \n");
	printf("\t(Eg. \"setenv boot_descr_1 Linux\").\n");

	printf("\nStep 2) Define default booting entry via \"default_boot\" variable.\n");
	printf("\t(Eg. \"setenv default_boot 2\").\n");

	printf("\nStep 3) For each configured bootable system there are 3 levels of variable\n");
	printf("\tdefinitions which override each other in the following order:\n");
	printf("\t  \"bootargs_run_X\" OVERRIDES \"bootargs_X\" OVERRIDES \"sd_part_X\"\n");
	printf("\t  \"bootcmd_run_X\" OVERRIDES \"bootcmd_X\" OVERRIDES \"sd_part_X\"\n");
	printf("\tIf one of the described variable is missing the next one will be used.\n");  

	printf("\nHere the meaning of multiboot configuration variable:\n");  

	printf("  - bootargs_run_X (where X can be 1, 2, 3, ...)\n");
	printf(" String contained in \"bootargs_run_X\" will be executed inside multiboot\n");
	printf(" environment with the following command: \"run bootargs_run_X\".\n");

	printf("  - bootargs_X (where X can be 1, 2, 3, ...)\n");
	printf(" String contained in \"bootargs_X\" will be adopted as bootargs environment.\n");

	printf("  - sd_part_X (where X can be 1, 2, 3, ...)\n");
	printf(" uSD partition number \"sd_part_X\" to be used as root device filesystem.\n");

	printf("  - bootargs_defaults\n");
	printf(" Current value for bootargs_defaults is:\n");
	if (getenv("bootargs_defaults") != NULL)
		printf("  bootargs_defaults='%s'\n", getenv("bootargs_defaults"));
	else
		printf("  bootargs_defaults='%s'\n", DEF_bootargs_defaults);
	printf("\n");

	printf("  - bootcmd_run_X (where X can be 1, 2, 3, ...)\n");
	printf(" String contained in \"bootcmd_run_X\" will be executed inside multiboot\n");
	printf(" environment with the following command: \"run bootcmd_run_X\".\n");

	printf("  - bootcmd_X (where X can be 1, 2, 3, ...)\n");
	printf(" String contained in \"bootcmd_X\" will be used as bootcmd environment.\n");

	printf("  - sd_part_X (where X can be 1, 2, 3, ...)\n");
	printf(" uSD partition number \"sd_part_X\" will be used to load uImage from.\n");

	printf("  - bootcmd_default\n");
	printf(" Current value for bootcmd_default is:\n");
	if (getenv("bootcmd_defaults") != NULL)
		printf("  bootcmd_defaults='%s'\n", getenv("bootcmd_defaults"));
	else
		printf("  bootcmd_defaults='%s'\n", DEF_bootcmd_defaults);
	printf("\n");


	printf("If your system need multiboot customized environment it will be necessary to\n");
	printf("define some boot*_X variable or change \"bootargs_defaults\" and \"bootcmd_default\".\n");
	printf("When no variable is defined, multiboot will use /dev/sda1 for system 1,\n");
	printf("use /dev/sda2 for system 2, /dev/sda3 for system 3, and so on.\n");

	return 0;
}

void do_set_environment(char ch, char *bootvars) {
	char tempstr[500];
	char settempstr[500];
	char env_var_name[300];

	sprintf(env_var_name, "%s_run_%c", bootvars, ch);
	if (getenv(env_var_name) != NULL) {
		strcpy (tempstr, getenv(env_var_name));
		run_command(tempstr, 0);
	} else {
		sprintf(env_var_name, "%s_%c", bootvars, ch);
		if (getenv(env_var_name) != NULL) {
			strcpy (tempstr, getenv(env_var_name));
			setenv(bootvars, tempstr);
		} else {
			sprintf(env_var_name, "sd_part_%c", ch);
			if (getenv(env_var_name) != NULL) {
				strcpy(tempstr, getenv(env_var_name));
				setenv("sd_part", tempstr);
			} else {
				setenv("sd_part", ch);
			}
			if (strcmp(bootvars, "bootargs") == 0) {
				if (getenv("bootargs_defaults") != NULL) {
					strcpy(tempstr, strdup(getenv("bootargs_defaults")));
				} else {
					strcpy(tempstr, DEF_bootargs_defaults);
				}
				sprintf(settempstr, "setenv bootargs %s", tempstr);
				run_command(settempstr, 0);
			} else {
				if (getenv("bootcmd_defaults") != NULL) {
					strcpy(tempstr, strdup(getenv("bootcmd_defaults")));
				} else {
					strcpy(tempstr, DEF_bootcmd_defaults);
				}
				sprintf(settempstr, "setenv bootcmd %s", tempstr);
				run_command(settempstr, 0);
			}
		}
	}
	return;
}

void do_chooseboot(int sec) {
	int timeout;
	int first_round = 0;
	int num_of_possibility;
	char n[1];
	char ch;

	do {
		if (first_round > 0)
			printf ("\nInvalid option. Valid choice are from 1 to %d.\n", num_of_possibility);
		num_of_possibility = show_multioption();
		sprintf(n, "%d", num_of_possibility);
		ch = (char)(((int)'0') + default_boot);
		
		if (num_of_possibility == 0) {
			printf("No bootable system defined: \"boot_descr_1\" is empty! \n  Giving up.\n");
			return;
		}

		if ( sec == 99 ) {
			ch = getc();
			printf("%c\n", ch);
		} else {
			timeout = sec * 10;
			while (timeout > 0) {
				if (tstc()) {
					ch = getc();
					printf("%c\n", ch);
					break;
				}
				msleep(100);
				timeout--;
			}
		}

		if (ch == 'q')
			return;

		if (ch == 'p') {
			sec = 99;
			do_detailed_environment();
		}
		first_round = 1;
	} while ((ch < '1') || (ch > n[0]));

	do_set_environment(ch, "bootargs");
	do_set_environment(ch, "bootcmd");

	run_command("boot", 0);
}

static int do_multiboot(cmd_tbl_t *cmd, int flag, int argc, char * const argv[])
{
	int sec = 5;

        if (argc > 2)
                return cmd_usage(cmd);

        if (argc == 2 && ((strcmp(argv[1], "help") == 0) || strcmp(argv[1], "h") == 0))  {
                return cmd_usage(cmd);
        }

        if (argc == 2 && (strcmp(argv[1], "d") == 0))  {
		return do_detailed_help();
        }
	
	if (argc == 2 && ((argv[1] >= '0') || (argv[1] <= '9')))
		sec = argv[1][0] - '0';

        if (argc == 2 && (strcmp(argv[1], "W") == 0))
		sec = 99;

        printf ("\nUDOO interactive multiboot utility.\n");

	do_chooseboot(sec);
	return 0;
}

U_BOOT_CMD(
	multiboot,	CONFIG_SYS_MAXARGS,	1,	do_multiboot,
	"manage a multiboot startup",
	"[args..]\n"
	" - d Print detailed help.\n - [0-9, W] Seconds before default entry (if empty 5 sec). W = Wait forever"
);
