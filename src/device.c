/**********************************************************************
 * INCLUDES
 */
#include "tl_common.h"
#include "device.h"
#include "zb_api.h"
#include "zcl_include.h"
#include "bdb.h"
#include "ota.h"
#include "device.h"
#if ZBHCI_EN
#include "zbhci.h"
#endif

#include "app_ui.h"
#include "zcl_relative_humidity.h"
#include "app_i2c.h"
#include "sensor.h"
#include "lcd.h"
#include "reporting.h"
#include "ext_ota.h"

/**********************************************************************
 * LOCAL CONSTANTS
 */


/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * GLOBAL VARIABLES
 */
app_ctx_t g_sensorAppCtx;
extern u8 lcd_version;
extern u8 sensor_version;


#ifdef ZCL_OTA
extern ota_callBack_t sensorDevice_otaCb;

//running code firmware information
ota_preamble_t sensorDevice_otaInfo = {
	.fileVer 			= FILE_VERSION,
	.imageType 			= IMAGE_TYPE,
	.manufacturerCode 	= MANUFACTURER_CODE_TELINK,
};
#endif


//Must declare the application call back function which used by ZDO layer
const zdo_appIndCb_t appCbLst = {
	bdb_zdoStartDevCnf,//start device cnf cb
	NULL,//reset cnf cb
	NULL,//device announce indication cb
	sensorDevice_leaveIndHandler,//leave ind cb
	sensorDevice_leaveCnfHandler,//leave cnf cb
	NULL,//nwk update ind cb
	NULL,//permit join ind cb
	NULL,//nlme sync cnf cb
	NULL,//tc join ind cb
	NULL,//tc detects that the frame counter is near limit
};


/**
 *  @brief Definition for bdb commissioning setting
 */
bdb_commissionSetting_t g_bdbCommissionSetting = {
	.linkKey.tcLinkKey.keyType = SS_GLOBAL_LINK_KEY,
	.linkKey.tcLinkKey.key = (u8 *)tcLinkKeyCentralDefault,       		//can use unique link key stored in NV

	.linkKey.distributeLinkKey.keyType = MASTER_KEY,
	.linkKey.distributeLinkKey.key = (u8 *)linkKeyDistributedMaster,  	//use linkKeyDistributedCertification before testing

	.linkKey.touchLinkKey.keyType = MASTER_KEY,
	.linkKey.touchLinkKey.key = (u8 *)touchLinkKeyMaster,   			//use touchLinkKeyCertification before testing

#if TOUCHLINK_SUPPORT
	.touchlinkEnable = 1,												/* enable touch-link */
#else
	.touchlinkEnable = 0,												/* disable touch-link */
#endif
	.touchlinkChannel = DEFAULT_CHANNEL, 								/* touch-link default operation channel for target */
	.touchlinkLqiThreshold = 0xA0,			   							/* threshold for touch-link scan req/resp command */
};

/**********************************************************************
 * LOCAL VARIABLES
 */

//const scomfort_t def_cmf = {
_attribute_data_retention_ scomfort_t cmf = {
    .t = {2100,2600}, // x0.01 C
    .h = {3000,6000}  // x0.01 %
};


/**********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      stack_init
 *
 * @brief   This function initialize the ZigBee stack and related profile. If HA/ZLL profile is
 *          enabled in this application, related cluster should be registered here.
 *
 * @param   None
 *
 * @return  None
 */
void stack_init(void)
{
	zb_init();
	zb_zdoCbRegister((zdo_appIndCb_t *)&appCbLst);
}

_attribute_ram_code_
u8 is_comfort(s16 t, u16 h) {
	u8 ret = 0;
	if (t >= cmf.t[0] && t <= cmf.t[1] && h >= cmf.h[0] && h <= cmf.h[1])
		ret = 1;
	return ret;
}

void read_sensor_and_save() {
	read_sensor();
#ifdef ZCL_THERMOSTAT_UI_CFG
	if (g_zcl_thermostatUICfgAttrs.displayMode == 2) {
		// (°F) = (Temperature in degrees Celsius (°C) * 9/5) + 32.
		show_big_number_x10((measured_data.temp * 9 / 50) + 320, 2); // convert C to F
	} else
		g_zcl_thermostatUICfgAttrs.displayMode = 1;
		show_big_number_x10(measured_data.temp / 10, 1);
#else
	g_zcl_temperatureAttrs.measuredValue = measured_data.temp;

	show_big_number_x10(measured_data.temp / 10, 1);
#endif
	g_zcl_temperatureAttrs.measuredValue = measured_data.temp;
    g_zcl_relHumidityAttrs.measuredValue = measured_data.humi;

    g_zcl_powerAttrs.batteryVoltage = (u8)(measured_data.battery_mv / 100);
    // batteryPercentage = level in zigbee sepulkas
    measured_data.battery_level = (measured_data.battery_mv - BATTERY_SAFETY_THRESHOLD)/4;
    if(measured_data.battery_level > 200)
    	measured_data.battery_level = 200;
    g_zcl_powerAttrs.batteryPercentage = (u8)measured_data.battery_level;


#if BOARD == BOARD_CGDK2
    show_small_number_x10(measured_data.humi / 10, 1);
    show_battery_symbol(true);
#else
    show_small_number(measured_data.humi / 100, 1);
    show_battery_symbol(g_zcl_powerAttrs.batteryPercentage < 10);
#endif
#if defined(SHOW_SMILEY)
    show_smiley(
        is_comfort(measured_data.temp, measured_data.humi) ? 1 : 2
    );
#endif
    update_lcd();

    g_sensorAppCtx.readSensorTime = clock_time();
}

/*********************************************************************
 * @fn      user_app_init
 *
 * @brief   This function initialize the application(Endpoint) information for this node.
 *
 * @param   None
 *
 * @return  None
 */
void user_app_init(void)
{
	// factoryRst_init();
#if ZCL_POLL_CTRL_SUPPORT
	af_powerDescPowerModeUpdate(POWER_MODE_RECEIVER_COMES_PERIODICALLY);
#else
	af_powerDescPowerModeUpdate(POWER_MODE_RECEIVER_COMES_WHEN_STIMULATED);
#endif
    /* Initialize ZCL layer */
	/* Register Incoming ZCL Foundation command/response messages */
	zcl_init(sensorDevice_zclProcessIncomingMsg);

	/* Register endPoint */
	af_endpointRegister(SENSOR_DEVICE_ENDPOINT, (af_simple_descriptor_t *)&sensorDevice_simpleDesc, zcl_rx_handler, NULL);

	zcl_reportingTabInit();

	/* Register ZCL specific cluster information */
	zcl_register(SENSOR_DEVICE_ENDPOINT, SENSOR_DEVICE_CB_CLUSTER_NUM, (zcl_specClusterInfo_t *)g_sensorDeviceClusterList);

#if ZCL_OTA_SUPPORT
    ota_init(OTA_TYPE_CLIENT, (af_simple_descriptor_t *)&sensorDevice_simpleDesc, &sensorDevice_otaInfo, &sensorDevice_otaCb);
#endif
    if(!zb_isDeviceJoinedNwk())
    	g_sensorAppCtx.bindTime = clock_time() | 1;
    // read sensors
	read_sensor_and_save();
}

void app_task(void)
{
	tack_keys();

	if(clock_time_exceed(g_sensorAppCtx.readSensorTime, READ_SENSOR_TIMER*1000)){
		read_sensor_and_save();
	}

	if(bdb_isIdle()){
		// report handler
		if(zb_isDeviceJoinedNwk()){
			if(!g_sensorAppCtx.timerLedEvt)
				show_ble_symbol(false);
			g_sensorAppCtx.bindTime = 0;
			if(zcl_reportingEntryActiveNumGet()){
				reportNoMinLimit();
				//start report timer
				app_reportAttrTimerStart();
			}else{
				//stop report timer
			}
		} else {
			if(!g_sensorAppCtx.timerLedEvt)
				show_ble_symbol(true);
			if(g_sensorAppCtx.bindTime) {
				if(clock_time_exceed(g_sensorAppCtx.bindTime,	45 *1000 * 1000)) { // 45 sec
					show_blink_screen();
					drv_pm_sleep(PM_SLEEP_MODE_DEEPSLEEP, PM_WAKEUP_SRC_PAD, 0);
				}
			}
		}
#if PM_ENABLE
		drv_pm_lowPowerEnter();
#endif
	}
}


void sensorDeviceSysException(void)
{
#if DEBUG_ENABLE
	extern volatile u16 T_evtExcept[4];
	show_big_number_x10(T_evtExcept[0], 0);
#if BOARD == BOARD_CGDK2
	show_small_number_x10(T_evtExcept[1], false);
#else
	show_small_number(T_evtExcept[1], false);
#endif
	update_lcd();
	drv_pm_sleep(PM_SLEEP_MODE_DEEPSLEEP, 0, 20*1000);
#else
	zb_resetDevice();
#endif
}

char int_to_hex(u8 num){
	char digits[] = "0123456789ABCDEF";
	if (num > 15) return digits[0];
	return digits[num];
}

void populate_sw_build() {
	g_zcl_basicAttrs.swBuildId[1] = int_to_hex(STACK_RELEASE>>4);
	g_zcl_basicAttrs.swBuildId[2] = int_to_hex(STACK_RELEASE & 0xf);
	g_zcl_basicAttrs.swBuildId[3] = int_to_hex(STACK_BUILD>>4);
	g_zcl_basicAttrs.swBuildId[4] = int_to_hex(STACK_BUILD & 0xf);
	g_zcl_basicAttrs.swBuildId[6] = int_to_hex(APP_RELEASE>>4);
	g_zcl_basicAttrs.swBuildId[7] = int_to_hex(APP_RELEASE & 0xf);
	g_zcl_basicAttrs.swBuildId[8] = int_to_hex(APP_BUILD>>4);
	g_zcl_basicAttrs.swBuildId[9] = int_to_hex(APP_BUILD & 0xf);
}

void populate_date_code() {
	u8 month;
	if (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n') month = 1;
	else if (__DATE__[0] == 'F') month = 2;
	else if (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r') month = 3;
	else if (__DATE__[0] == 'A' && __DATE__[1] == 'p') month = 4;
	else if (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y') month = 5;
	else if (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n') month = 6;
	else if (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l') month = 7;
	else if (__DATE__[0] == 'A' && __DATE__[1] == 'u') month = 8;
	else if (__DATE__[0] == 'S') month = 9;
	else if (__DATE__[0] == 'O') month = 10;
	else if (__DATE__[0] == 'N') month = 11;
	else if (__DATE__[0] == 'D') month = 12;

	g_zcl_basicAttrs.dateCode[1] = __DATE__[7];
	g_zcl_basicAttrs.dateCode[2] = __DATE__[8];
	g_zcl_basicAttrs.dateCode[3] = __DATE__[9];
	g_zcl_basicAttrs.dateCode[4] = __DATE__[10];
	g_zcl_basicAttrs.dateCode[5] = '0' + month / 10;
	g_zcl_basicAttrs.dateCode[6] = '0' + month % 10;
	g_zcl_basicAttrs.dateCode[7] = __DATE__[4] >= '0' ? (__DATE__[4]) : '0';
	g_zcl_basicAttrs.dateCode[8] = __DATE__[5];
}


void populate_hw_version() {
/*
 HW  | LCD I2C addr | SHTxxx I2C addr | Note
-----|--------------|-----------------|---------
B1.4 | 0x3C         | 0x70   (SHTC3)  |
B1.5 | UART!        | 0x70   (SHTC3)  |
B1.6 | UART!        | 0x44   (SHT4x)  |
B1.7 | 0x3C         | 0x44   (SHT4x)  | Test   original string HW
B1.9 | 0x3E         | 0x44   (SHT4x)  |
B2.0 | 0x3C         | 0x44   (SHT4x)  | Test   original string HW
*/
    if (i2c_address_lcd == B14_I2C_ADDR) {
        if (sensor_version == 0)
            g_zcl_basicAttrs.hwVersion = 14;
        else if (sensor_version == 1)
            g_zcl_basicAttrs.hwVersion = 20;
    } else if (i2c_address_lcd == B16_I2C_ADDR) {
        if (sensor_version == 0)
            g_zcl_basicAttrs.hwVersion = 15;
        else if (sensor_version == 1)
            g_zcl_basicAttrs.hwVersion = 16;
    } else if (i2c_address_lcd == B19_I2C_ADDR) {
        g_zcl_basicAttrs.hwVersion = 19;
    }
}

/*********************************************************************
 * @fn      user_init
 *
 * @brief   User level initialization code.
 *
 * @param   isRetention - if it is waking up with ram retention.
 *
 * @return  None
 */
u16 reportableChange[4];

void user_init(bool isRetention)
{
#if PA_ENABLE
	rf_paInit(PA_TX, PA_RX);
#endif

#if ZBHCI_EN
	zbhciInit();
#endif

	if(!isRetention){

//		test_first_ota();

		/* Populate properties with compiled-in values */
		populate_sw_build();
		populate_date_code();

		/* Initialize Stack */
		stack_init();

		init_lcd();
		init_sensor();

		populate_hw_version();

#if DEBUG_ENABLE
		/* Register except handler for test */
		sys_exceptHandlerRegister(sensorDeviceSysException);
#endif

		/* Initialize user application */
		user_app_init();

		/* User's Task */
#if ZBHCI_EN
		ev_on_poll(EV_POLL_HCI, zbhciTask);
#endif
		ev_on_poll(EV_POLL_IDLE, app_task);

		/* Load the pre-install code from flash */
		if(bdb_preInstallCodeLoad(&g_sensorAppCtx.tcLinkKey.keyType, g_sensorAppCtx.tcLinkKey.key) == RET_OK){
			g_bdbCommissionSetting.linkKey.tcLinkKey.keyType = g_sensorAppCtx.tcLinkKey.keyType;
			g_bdbCommissionSetting.linkKey.tcLinkKey.key = g_sensorAppCtx.tcLinkKey.key;
		}

		/* Set default reporting configuration */
		reportableChange[0] = 0;
        bdb_defaultReportingCfg(
			SENSOR_DEVICE_ENDPOINT,
			HA_PROFILE_ID,
			ZCL_CLUSTER_GEN_POWER_CFG,
			ZCL_ATTRID_BATTERY_VOLTAGE,
			60,
			3600,
			(u8 *)&reportableChange[0]
		);
        reportableChange[1] = 0;
        bdb_defaultReportingCfg(
			SENSOR_DEVICE_ENDPOINT,
			HA_PROFILE_ID,
			ZCL_CLUSTER_GEN_POWER_CFG,
			ZCL_ATTRID_BATTERY_PERCENTAGE_REMAINING,
			60,
			3600,
			(u8 *)&reportableChange[1]
		);
        reportableChange[2] = 10;
		bdb_defaultReportingCfg(
			SENSOR_DEVICE_ENDPOINT,
			HA_PROFILE_ID,
			ZCL_CLUSTER_MS_TEMPERATURE_MEASUREMENT,
			ZCL_TEMPERATURE_MEASUREMENT_ATTRID_MEASUREDVALUE,
			25,
			120,
			(u8 *)&reportableChange[2]
		);
        reportableChange[3] = 100;
		bdb_defaultReportingCfg(
			SENSOR_DEVICE_ENDPOINT,
			HA_PROFILE_ID,
			ZCL_CLUSTER_MS_RELATIVE_HUMIDITY,
			ZCL_RELATIVE_HUMIDITY_ATTRID_MEASUREDVALUE,
			25,
			120,
			(u8 *)&reportableChange[3]
		);

		/* Initialize BDB */
		u8 repower = drv_pm_deepSleep_flag_get() ? 0 : 1;
		bdb_init((af_simple_descriptor_t *)&sensorDevice_simpleDesc, &g_bdbCommissionSetting, &g_zbDemoBdbCb, repower);
	}else{
		/* Re-config phy when system recovery from deep sleep with retention */
		mac_phyReconfig();
	}
}
