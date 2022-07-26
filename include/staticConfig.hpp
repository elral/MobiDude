//  2022 maddsua | https://gitlab.com/maddsua
//	No warranties are given, etc...
//	This file is a component of the AVR Firmware Uploader

#ifndef STATCFG
#define STATCFG

#define VER_AUTHSTAMP	"2022 maddsua"

#define windowSizeX			360
#define windowSizeY			360

#define totalBoardFields	30
#define totalBoards			((totalBoardFields)/(3))
#define boardsParamSize		16
#define scanPorts			64
#define comPortNameSize		16

#define progbar_steps		1000
#define progbar_timer_step	30
#define dude_run_timeout	35000

#define dudecmdlen			1024
#define binfilenamelen		256
#define packagecfglen		128

#define EC_DUDE_MAIN		1
#define EC_DUDE_NOEXEC		552
#define EC_DUDE_TIMEOUT		553

#define IMG_BANNER			1000
#define IMG_FOLDER			1001
#define IMG_HELP			1002
#define IMG_CANCEL			1003
#define IMG_BOARD			1004

#define ID_TIMER			13

#define GUI_GROUP_MAIN		25
#define GUI_STAT_TIP_MCU	26
#define GUI_STAT_TIP_PROG	27
#define GUI_STAT_TIP_SPEED	28
#define GUI_STAT_TIP_COM	29
#define GUI_STAT_FILE		30
#define GUI_STAT_BOARD		31
#define GUI_BOX_MCU			32
#define GUI_BOX_PRG			33
#define GUI_BOX_SPD			34
#define GUI_BOX_COM			35
#define GUI_BTN_OPEN		36
#define GUI_BTN_FLASH		37
#define GUI_BTN_SVCFG		38
#define GUI_BTN_HELP		39
#define GUI_BTN_CANCEL		40

#define configFileHeader	"#AVRBINARYUPLOADCONFIGFILE\nBIN="


#define msg_nofilesel	"No binary file selected"
#define msg_nocomport	"No serial ports was found on this PC"




#endif
