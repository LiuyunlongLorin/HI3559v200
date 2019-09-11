# use HIARCH indicate chip, hi3556av100/hi3559av100
export HIARCH?=hi3559v200

####### cpu type ,use dual/linux ###########
export MAPI_CPU_TYPE?=dual


####### select sensor  Cable(lane_divide_mode) for your sample ########
#LANE_DIVIDE_MODE_0/LANE_DIVIDE_MODE_1/LANE_DIVIDE_MODE_2
SENSOR_CABLE_TYPE ?= LANE_DIVIDE_MODE_0

####### select sensor type for your sample ########
#IMX377/IMX335/IMX458/IMX307/BT656/OS04B10/OV12870

CFG_SENSOR_TYPE0 ?=IMX458
CFG_SENSOR_TYPE1 ?=

####### select log level for your sample ########
#MAPI_LOG_NONE/MAPI_LOG_ERR/MAPI_LOG_ALL
CFG_LOG_LEVEL ?= MAPI_LOG_ALL

####### support stitch###########
CFG_SUPPORT_STITCH = n
####### support photo###########
CFG_SUPPORT_PHOTO_POST_PROCESS = n
####### support dis###########
CFG_SUPPORT_DIS ?= y
####### support gyro###########
CFG_SUPPORT_GYRO ?= n
####### support hdmi###########
CFG_SUPPORT_HDMI ?= y
