#pragma once

#define VER_AUTHSTAMP		"2024 elral"

#define windowSizeX			400
#define windowSizeY			360

#define scanPorts			64
#define comPortNameSize		16

#define progbar_steps		1000
#define progbar_timer_step	30
#define dude_run_timeout	60000

#define dudecmdlen			1024
#define terminalcmdlen		1024
#define binfilenamelen		256

#define EC_DUDE_MAIN		1
#define EC_DUDE_NOEXEC		552
#define EC_DUDE_TIMEOUT		553

#define EC_TERMINAL_MAIN	1
#define EC_TERMINAL_NOEXEC	552

#define IMG_BANNER			1000
#define IMG_FOLDER			1001
#define IMG_HELP			1002
#define IMG_CANCEL			1003

#define ID_TIMER_AVRDUDE	13
#define ID_TIMER_TERMINAL   14
#define ID_TIMER_GETINFO    15

#define GUI_GROUP_MAIN		25
#define GUI_STAT_TIP_MCU	26
#define GUI_STAT_TIP_COM	27
#define GUI_STAT_FILE		28
#define GUI_BOX_MCU			29
#define GUI_BOX_COM			30
#define GUI_BTN_OPEN		31
#define GUI_BTN_FLASH		32
#define GUI_BTN_TERMINAL	33
#define GUI_BTN_GETINFO     34
#define GUI_BTN_CANCEL		35
#define GUI_GROUP_LAST		36


#define configFileHeader	"#AVRBINARYUPLOADCONFIGFILE\nBIN="

#define PRODUCT_NAME		"MobiDude Firmware Uploader 1.0.7"
