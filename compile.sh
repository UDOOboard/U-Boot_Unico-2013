#!/bin/bash

type_udoo="UDOO"

type_cpu_q="QUAD"
type_cpu_dl="DUAL_LITE"

type_os_linux="LINUX"
type_os_android="ANDROID"

CONFIG_FILE="./tools/sconfig"

env_MMC="MMC"

BACKTITLE='U-boot UDOO config'

SELECTION=""
SEL_ITEM=1
SELECTION_COMP=""
SUBSEL=""
EXIT_RESPONCE=0
EXIT=0

# Default values
MEM_SIZE=0
MEM_TYPE=0
BOARD=${type_udoo}
CPU_TYPE=${type_cpu_q}
OS_TYPE=${type_os_android}
ENV_DEV=${env_MMC}
CLEAN="CLEAN"
COMPILER_PATH="/usr/bin/arm-linux-gnueabi-"
UBOOT_VER="imx6_udoo"

SUFFIX=""

#################################################################
#																#
#					CONFIG FILE FUNCTION						#
#																#
#################################################################

set_ConfFile() {
	echo "MEMORY_SIZE $MEM_SIZE" > $CONFIG_FILE
	echo "MEMORY_TYPE $MEM_TYPE" >> $CONFIG_FILE
	echo "BOARD_TYPE $BOARD" >> $CONFIG_FILE
	echo "CPU_TYPE $CPU_TYPE" >> $CONFIG_FILE
	echo "OS_TYPE $OS_TYPE" >> $CONFIG_FILE
	echo "ENV_DEVICE $ENV_DEV" >> $CONFIG_FILE
	echo "CLEAN_OP $CLEAN" >> $CONFIG_FILE
	echo "COMPILER $COMPILER_PATH" >> $CONFIG_FILE
	echo "UBOOT_VER $UBOOT_VER" >> $CONFIG_FILE
}

set_from_ConfFile() {
	if [[ -e $CONFIG_FILE ]]; then
		VAR=$(cat $CONFIG_FILE | grep "MEMORY_SIZE" | awk '{print $2}')
		if [[ "${VAR}" != "" ]]; then
			MEM_SIZE=$VAR
		fi
		VAR=$(cat $CONFIG_FILE | grep "MEMORY_TYPE" | awk '{print $2}')
		if [[ "${VAR}" != "" ]]; then
			MEM_TYPE=$VAR
		fi
		VAR=$(cat $CONFIG_FILE | grep "BOARD_TYPE" | awk '{print $2}')
		if [[ "${VAR}" != "" ]]; then
			BOARD=$VAR
		fi
		VAR=$(cat $CONFIG_FILE | grep "CPU_TYPE" | awk '{print $2}')
		if [[ "${VAR}" != "" ]]; then
			CPU_TYPE=$VAR
		fi
		VAR=$(cat $CONFIG_FILE | grep "OS_TYPE" | awk '{print $2}')
		if [[ "${VAR}" != "" ]]; then
			OS_TYPE=$VAR
		fi
		VAR=$(cat $CONFIG_FILE | grep "ENV_DEVICE" | awk '{print $2}')
		if [[ "${VAR}" != "" ]]; then
			ENV_DEV=$VAR
		fi
		VAR=$(cat $CONFIG_FILE | grep "CLEAN_OP" | awk '{print $2}')
		if [[ "${VAR}" != "" ]]; then
			CLEAN=$VAR
		fi
		VAR=$(cat $CONFIG_FILE | grep "COMPILER" | awk '{print $2}')
		if [[ "${VAR}" != "" ]]; then
			COMPILER_PATH=$VAR
		fi
		VAR=$(cat $CONFIG_FILE | grep "UBOOT_VER" | awk '{print $2}')
		if [[ "${VAR}" != "" ]]; then
			UBOOT_VER=$VAR
		fi
	else
		echo "WARNING: Configuration file not found!"
		set_ConfFile
	fi
}


#################################################################
#																#
#						GRAPHIC FUNCTION						#
#																#
#################################################################


main_view() {
	# open fd
	exec 3>&1
	 
	# Store data to $VALUES variable
	SELECTION=$(dialog --title "Main Menu" \
			--backtitle "$BACKTITLE" \
			--ok-label "Select" \
			--default-item $SEL_ITEM \
			--cancel-label "Exit" \
			--menu "Please choose an operation:" 25 60 10 \
			1 "DDR Size -->" \
			2 "DDR Type -->" \
			3 "Board Type -->" \
			4 "CPU type -->" \
			5 "OS type -->" \
			6 "Environment device -->" \
			7 "Extra options -->" \
			8 "Compiler options -->" \
			2>&1 1>&3)
	 
	# close fd
	exec 3>&-
	
	if [[ "${SELECTION}" == "" ]]; then
		EXIT=1
 	fi
	# display values just entered
#	echo "$SELECTION"
	
}


ddr_size_view() {
	# open fd
	exec 3>&1
	VAL=(off off)
	VAL[$MEM_SIZE]=on
	# Store data to $VALUES variable
	SELECTION=$(dialog --title "DDR configuration" \
			--backtitle "$BACKTITLE" \
			--ok-label "Select" \
			--cancel-label "Exit" \
			--default-item $MEM_SIZE \
			--radiolist "Select DDR type:" 20 80 10 \
			0 "1Giga, bus size 64, active CS = 1 (256Mx4)" ${VAL[0]} \
			2>&1 1>&3)
	 
	# close fd
	exec 3>&-
	
	if [[ "${SELECTION}" == "" ]]; then
		echo "not select"
	else
		MEM_SIZE=$SELECTION
	fi	
}

#		1 "2Giga, bus size 64, active CS = 1 (512Mx4)" ${VAL[1]} \

ddr_type_view() {
	# open fd
	exec 3>&1
	VAL=(off)
	VAL[$MEM_TYPE]=on
	# Store data to $VALUES variable
	SELECTION=$(dialog --title "DDR type" \
			--backtitle "$BACKTITLE" \
			--ok-label "Select" \
			--cancel-label "Exit" \
			--default-item $MEM_TYPE \
			--radiolist "Select DRR type:" 20 60 10 \
 			0 "DDR3 Low Voltage" ${VAL[0]} \
			2>&1 1>&3)
	 
	# close fd
	exec 3>&-
	
	if [[ "${SELECTION}" == "" ]]; then
		echo "not select"
	else
		MEM_TYPE=$SELECTION
	fi	
}

board_type_view() {
	# open fd
	exec 3>&1
	VAL=(off)
	case "$BOARD" in
			"$type_udoo") VAL[0]=on;;
	esac
	# Store data to $VALUES variable
	SELECTION=$(dialog --title "Board type" \
			--backtitle "$BACKTITLE" \
			--ok-label "Ok" \
			--cancel-label "Exit" \
			--default-item $BOARD \
			--radiolist "Select Board type:" 20 60 10 \
			$type_udoo "UDOO board" ${VAL[0]} \
			2>&1 1>&3)
	 
	# close fd
	exec 3>&-
	 
	if [[ "${SELECTION}" == "" ]]; then
		echo "not select"
	else
		BOARD=$SELECTION
	fi	
}

cpu_type_view() {
	# open fd
	exec 3>&1
	VAL=(off off)
	case "$CPU_TYPE" in
			"$type_cpu_q") VAL[0]=on;;
			"$type_cpu_dl") VAL[1]=on;;
	esac
	# Store data to $VALUES variable
	SELECTION=$(dialog --title "CPU type" \
			--backtitle "$BACKTITLE" \
			--ok-label "Ok" \
			--cancel-label "Exit" \
			--radiolist "Select CPU type:" 20 60 10 \
			$type_cpu_q "iMX6 QUAD" ${VAL[0]} \
			$type_cpu_dl "iMX6 DUAL LITE" ${VAL[1]} \
			2>&1 1>&3)
	 
	# close fd
	exec 3>&-
	 
	if [[ "${SELECTION}" == "" ]]; then
		echo "not select"
	else
		CPU_TYPE=$SELECTION
	fi	
}

os_type_view() {
	# open fd
	exec 3>&1
	VAL=(off off)
	case "$OS_TYPE" in
			"$type_os_linux") VAL[0]=on;;
			"$type_os_android") VAL[1]=on;;
	esac
	# Store data to $VALUES variable
	SELECTION=$(dialog --title "OS type" \
			--backtitle "$BACKTITLE" \
			--ok-label "Ok" \
			--cancel-label "Exit" \
			--radiolist "Select OS type:" 20 60 10 \
			$type_os_linux "Linux" ${VAL[0]} \
			2>&1 1>&3)
#			$type_os_android "Android" ${VAL[1]} \
	 
	# close fd
	exec 3>&-
	 
	if [[ "${SELECTION}" == "" ]]; then
		echo "not select"
	else
		OS_TYPE=$SELECTION
	fi	
}

env_dev_view() {
	# open fd
	exec 3>&1
	VAL=(off)
	case "$ENV_DEV" in
		 "${env_MMC}") VAL[0]=on;; 
	esac
	# Store data to $VALUES variable
	SELECTION=$(dialog --title "Environment device" \
			--backtitle "$BACKTITLE" \
			--ok-label "Ok" \
			--cancel-label "Exit" \
			--default-item $ENV_DEV \
			--radiolist "Select device for environmnet storing:" 20 60 10 \
			$env_MMC "SD/MMC as environment device"  ${VAL[0]} \
			2>&1 1>&3)
	 
	# close fd
	exec 3>&-
	
	if [[ "${SELECTION}" == "" ]]; then
		echo "not select"
	else
		ENV_DEV=$SELECTION
	fi	
}

extra_view() {
	# open fd
	exec 3>&1
	VAL=(off off)
	if [[ "${CLEAN}" == "CLEAN" ]]; then
		VAL[0]=on   
	fi
	# Store data to $VALUES variable
	SELECTION=$(dialog --title "Extra option" \
			--backtitle "$BACKTITLE" \
			--ok-label "Ok" \
			--cancel-label "Cancel" \
			--checklist "General settings:" 20 60 10 \
			CLEAN "Clear befor compile" ${VAL[0]} \
			2>&1 1>&3)
	 
	# close fd
	exec 3>&-
	CLEAN="NOCLEAN"
	if [[ "${SELECTION}" == "" ]]; then
		echo "not select"
	else
		str=${SELECTION//\"/""}
		IFS=' ' read -a array <<< "$str"
		for i in ${array[*]}; do
			if [[ "$i" == "CLEAN" ]]; then
				CLEAN="${i}"	
			fi
		done	
	fi	
}

compile_view() {
	EXIT_COMP=0
	while [[ $EXIT_COMP -ne 1 ]]; do
		# open fd
		exec 3>&1
		
		SELECTION_COMP=$(dialog --title "Compile options" \
				--backtitle "$BACKTITLE" \
				--ok-label "Ok" \
				--cancel-label "Cancel" \
				--backtitle "$BACKTITLE" \
				--ok-label "Select" \
				--cancel-label "Exit" \
				--menu "Please choose an operation:" 25 60 10 \
				1 "Compiler path" \
				2 "u-boot version" \
				2>&1 1>&3)	

		case "$SELECTION_COMP" in 
			"1") SUBSEL=$(dialog --title "" \
					--backtitle "$BACKTITLE" \
					--nocancel \
					--inputbox "Enter Cross Compiler path here" 8 60 "$COMPILER_PATH" \
					2>&1 1>&3)
					if [[ "${SUBSEL}" != "" ]]; then
						COMPILER_PATH=$SUBSEL		
					fi;;
			"2") SUBSEL=$(dialog --title "" \
					--backtitle "$BACKTITLE" \
					--nocancel \
					--inputbox "Enter u-boot version here" 8 60 "$UBOOT_VER" \
					2>&1 1>&3)
					if [[ "${SUBSEL}" != "" ]]; then
						UBOOT_VER=$SUBSEL		
					fi;;
			  *) EXIT_COMP=1
		esac
	
		# close fd
		exec 3>&-	

	done
}

function exit_view () {
	dialog --title "" \
			--backtitle "$BACKTITLE" \
			--extra-button \
			--extra-label "Yes, with Compile" \
			--cancel-label "No" \
			--yesno "Want to save before exiting?" 5 70
	EXIT_RESPONCE=$?
}

#################################################################
#																#
#						COMPILE FUNCTION						#
#																#
#################################################################

#	elif [ "${MEM_SIZE}" == "1" ]; then
#		echo "RAM size selected: 2Giga, bus size 64, active CS = 1 (512Mx4)"
#		SUFFIX=${SUFFIX}-2GB

function check_mem_size () {
	echo ""
	if [ "${MEM_SIZE}" == "0" ]; then
		echo "RAM size selected: 1Giga, bus size 64, active CS = 1 (256Mx4)"
		SUFFIX=${SUFFIX}-256MBx4
	else
		echo "ERROR: wrong DDR size selected"
		exit 0
	fi
	echo ""
}

function check_mem_type () {
	echo ""
	if [ "${MEM_TYPE}" == "0" ]; then
		echo "RAM type selected: Low Voltage DDR3"
		SUFFIX=${SUFFIX}-LDDR3
	else
		echo "ERROR: wrong DDR type selected"
		exit 0
	fi
	echo ""
}

function check_board_type () {
	echo ""
	if [ "${BOARD}" == "${type_udoo}" ]; then
		echo "Board type selected: UDOO"
		SUFFIX=${SUFFIX}-Q7
	else
		echo "ERROR: wrong board type selected"
		exit 0
	fi
	echo ""
}

function check_cpu_type () {
	echo ""
    if [ "${CPU_TYPE}" == "${type_cpu_q}" ]; then
        echo "make udoo_quad_config"
		SUFFIX=${SUFFIX}-QD
    elif [ "${CPU_TYPE}" == "${type_cpu_dl}" ]; then
        echo "make udoo_dl_config"
		SUFFIX=${SUFFIX}-DL
	else
		echo "ERROR: No CPU Type selected "
	fi
}

function check_os_type () {
	echo ""
    if [ "${OS_TYPE}" == "${type_os_linux}" ]; then
        echo "OS type: Linux"
		SUFFIX=${SUFFIX}-LNX
    elif [ "${OS_TYPE}" == "${type_os_android}" ]; then
        echo "OS type: Android"
		SUFFIX=${SUFFIX}-ADR
	else
		echo "ERROR: No OS Type selected "
	fi
}
		
function check_env_device_type () {
	echo ""
	if [ "${ENV_DEV}" == "${env_MMC}" ]; then
		echo "Environment selected: MMC"
	else
		echo "ERROR: wrong environment selected"
		exit 0
	fi
	echo  ""
}

function make_cpu_type () {
	echo ""
	if [ "${BOARD}" == "${type_udoo}" ]; then
	    if [ "${CPU_TYPE}" == "${type_cpu_q}" ]; then
	        make udoo_quad_config  
	    elif [ "${CPU_TYPE}" == "${type_cpu_dl}" ]; then
	        make udoo_dl_config  
	    fi 
	fi
}

function check_compile_invironment () {
	echo ""
	if [ "${ARCH}" == "" ]; then
		export ARCH=arm
	fi
	if [ "${CROSS_COMPILE}" == "" ]; then
		export CROSS_COMPILE=$COMPILER_PATH
	fi
}

function compile () {
	
	check_compile_invironment

	SUFFIX=""	
	#N.B. don't change this calling order
	check_board_type	
	check_cpu_type
	check_os_type
	check_mem_size	
	check_mem_type
	check_env_device_type

	if [ "${CLEAN}" == "CLEAN" ]; then
		echo ""
		echo "Select Clean operation!"
		echo ""
		make distclean
		sleep 1
	fi
	
	make_cpu_type
	
	SUFFIX=-imx6${SUFFIX}-${UBOOT_VER}
	echo ${SUFFIX} > ./tools/suffix
	sleep 2
	
	let "MEM_SIZE=MEM_SIZE+2"
	let "MEM_TYPE=MEM_TYPE+1"
	
	make DDR_SIZE=${MEM_SIZE} DDR_TYPE=${MEM_TYPE} BOARD_TYPE=${BOARD} CPU_TYPE=${CPU_TYPE} OS_TYPE=${OS_TYPE} ENV_DEVICE=ENV_${ENV_DEV}
}

#################################################################
#																#
#																#
#################################################################

function help_view () {
	echo "U-boot compiler"
	echo "Usage: $0 [-c for configuration option]"
	echo
}

set_from_ConfFile
while getopts ":m:b:p:dch" optname; do
	case "$optname" in
		"c") while [[ $EXIT -ne 1 ]]; do
				main_view
				SEL_ITEM=$SELECTION
				case "$SELECTION" in
					"1") ddr_size_view;;
					"2") ddr_type_view;;
					"3") board_type_view;;
					"4") cpu_type_view;;
					"5") os_type_view;;
					"6") env_dev_view;;
					"7") extra_view;;
					"8") compile_view;;
					  *) echo "" ;;
				esac
			done

			exit_view
			case "${EXIT_RESPONCE}" in
				  "0") set_ConfFile; clear; echo "Configuration saved!";;
				  "1") clear; echo "Configuration not saved!";;
				  "3") set_ConfFile; clear; echo "Configuration saved!"; compile;;
				"255") clear;  echo "Configuration not saved!";;
					*) clear;
			esac
			exit 0;;

		  h) help_view
			 exit 0;;	
		  *) echo "ERROR: option not valid!"
			 help_view
			 exit 1;;
	esac
done

#if no any option is present, the compilation start directly
compile

