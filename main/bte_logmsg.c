/*****************************************************************************
**                                                                           
**  Name:          bte_logmsg.c                                                
**                                                                           
**  Description: Contains the LogMsg wrapper routines for BTE.  It routes calls
**               the appropriate application's LogMsg equivalent.
**                                                                           
**  Copyright (c) 2001-2011, WIDCOMM Inc., All Rights Reserved.             
**  WIDCOMM Bluetooth Core. Proprietary and confidential.                    
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "gki.h"
#include "bte.h"

#include "bte_appl.h"

#if MMI_INCLUDED == TRUE
#include "mmi.h"
#endif

/* always enable trace framework */

#include "btu.h"
#include "l2c_api.h"
#if (RFCOMM_INCLUDED==TRUE)
#include "port_api.h"
#endif
#if (OBX_INCLUDED==TRUE)
#include "obx_api.h"
#endif
#if (AVCT_INCLUDED==TRUE)
#include "avct_api.h"
#endif
#if (AVDT_INCLUDED==TRUE)
#include "avdt_api.h"
#endif
#if (AVRC_INCLUDED==TRUE)
#include "avrc_api.h"
#endif
#if (AVDT_INCLUDED==TRUE)
#include "avdt_api.h"
#endif
#if (A2D_INCLUDED==TRUE)
#include "a2d_api.h"
#endif
#if (BIP_INCLUDED==TRUE)
#include "bip_api.h"
#endif
#if (BNEP_INCLUDED==TRUE)
#include "bnep_api.h"
#endif
#if (BPP_INCLUDED==TRUE)
#include "bpp_api.h"
#endif
#include "btm_api.h"
#if (DUN_INCLUDED==TRUE)
#include "dun_api.h"
#endif
#if (GAP_INCLUDED==TRUE)
#include "gap_api.h"
#endif
#if (GOEP_INCLUDED==TRUE)
#include "goep_util.h"
#endif
#if (HCRP_INCLUDED==TRUE)
#include "hcrp_api.h"
#endif
#if (PAN_INCLUDED==TRUE)
#include "pan_api.h"
#endif
#include "sdp_api.h"
#if (VDP_INCLUDED==TRUE)
#include "vdp_api.h"
#endif

#if (BLE_INCLUDED==TRUE)
#include "gatt_api.h"
#endif

    /* LayerIDs for BTA, currently everything maps onto appl_trace_level */
#if (BTA_INCLUDED==TRUE)
#include "bta_api.h"
#endif


#if defined(__CYGWIN__) || defined(__linux__)
#undef RPC_INCLUDED
#define RPC_INCLUDED TRUE

#include <sys/time.h>
#include <time.h>

#if (defined(ANDROID_USE_LOGCAT) && (ANDROID_USE_LOGCAT==TRUE))	
#define LOG_TAG "BTLD"

#ifndef LINUX_NATIVE
#include <cutils/log.h>
#define LOGI0(s) __android_log_write(ANDROID_LOG_INFO, NULL, s)
#define LOGD0(s) __android_log_write(ANDROID_LOG_DEBUG, NULL, s)
#define LOGW0(s) __android_log_write(ANDROID_LOG_WARN, NULL, s)
#define LOGE0(s) __android_log_write(ANDROID_LOG_ERROR, NULL, s)

#else
#undef ANDROID_USE_LOGCAT
#endif

#endif


//#include "btl_cfg.h"
#define BTL_GLOBAL_PROP_TRC_FLAG "TRC_BTAPP"


#ifndef BTE_LOG_BUF_SIZE
#define BTE_LOG_BUF_SIZE  1024
#endif
#define BTE_LOG_MAX_SIZE  (BTE_LOG_BUF_SIZE - 12)


//#define BTE_MAP_TRACE_LEVEL FALSE
/* map by default BTE trace levels onto android trace levels */
#ifndef BTE_MAP_TRACE_LEVEL
#define BTE_MAP_TRACE_LEVEL TRUE
#endif

// #define BTE_ANDROID_INTERNAL_TIMESTAMP TRUE
/* by default no internal timestamp. adb logcate -v time allows having timestamps */
#ifndef BTE_ANDROID_INTERNAL_TIMESTAMP
#define BTE_ANDROID_INTERNAL_TIMESTAMP FALSE
#endif
#if (BTE_ANDROID_INTERNAL_TIMESTAMP==TRUE)
#define MSG_BUFFER_OFFSET strlen(buffer)
#else
#define MSG_BUFFER_OFFSET 0
#endif

//#define DBG_TRACE

#if defined( DBG_TRACE )
#define DBG_TRACE_API0( m ) BT_TRACE_0( TRACE_LAYER_HCI, TRACE_TYPE_API, m )
#define DBG_TRACE_WARNING2( m, p0, p1 ) BT_TRACE_2( TRACE_LAYER_BTM, (TRACE_ORG_APPL|TRACE_TYPE_WARNING), m, p0, p1 )
#else
#define DBG_TRACE_API0( m )
#define DBG_TRACE_WARNING2( m, p0, p1 )
#endif
#define DBG_TRACE_DEBUG2( m, p0, p1 ) BT_TRACE_2( TRACE_LAYER_BTM, (TRACE_ORG_APPL|TRACE_TYPE_DEBUG), m, p0, p1 )

void
LogMsg(UINT32 trace_set_mask, const char *fmt_str, ...)
{
	static char buffer[BTE_LOG_BUF_SIZE];

	va_list ap;
#if (BTE_ANDROID_INTERNAL_TIMESTAMP==TRUE)
	struct timeval tv;
	struct timezone tz;
	struct tm *tm;
	time_t t;
	
	gettimeofday(&tv, &tz);
	time(&t);
	tm = localtime(&t);

    sprintf(buffer, "%02d:%02d:%02d.%03d ", tm->tm_hour, tm->tm_min, tm->tm_sec,
        tv.tv_usec / 1000);
#endif
	va_start(ap, fmt_str);
	vsnprintf(&buffer[MSG_BUFFER_OFFSET], BTE_LOG_MAX_SIZE, fmt_str, ap);
	va_end(ap);

#if (defined(ANDROID_USE_LOGCAT) && (ANDROID_USE_LOGCAT==TRUE))	
#if (BTE_MAP_TRACE_LEVEL==TRUE)
    switch ( TRACE_GET_TYPE(trace_set_mask) )
    {
        case TRACE_TYPE_ERROR:
            LOGE0(buffer);
            break;
        case TRACE_TYPE_WARNING:
            LOGW0(buffer);
            break;
        case TRACE_TYPE_API:
        case TRACE_TYPE_EVENT:
            LOGI0(buffer);
            break;
        case TRACE_TYPE_DEBUG:
            LOGD0(buffer);
            break;
        default:
            LOGE0(buffer);      /* we should never get this */
            break;
    }
#else
    LOGI0(buffer);
#endif
#else
	write(2, buffer, strlen(buffer));
	write(2, "\n", 1);
#endif	
}

void
ScrLog(UINT32 trace_set_mask, const char *fmt_str, ...)
{
	static char buffer[BTE_LOG_BUF_SIZE];

	va_list ap;
	struct timeval tv;
	struct timezone tz;
	struct tm *tm;
	time_t t;
	
	gettimeofday(&tv, &tz);
	time(&t);
	tm = localtime(&t);

        sprintf(buffer, "%02d:%02d:%02d.%03ld ", tm->tm_hour, tm->tm_min, tm->tm_sec,
        tv.tv_usec / 1000);
	
	va_start(ap, fmt_str);
	vsnprintf(&buffer[strlen(buffer)], BTE_LOG_MAX_SIZE, fmt_str, ap);
	va_end(ap);

#if (defined(ANDROID_USE_LOGCAT) && (ANDROID_USE_LOGCAT==TRUE))	
    LOGI0(buffer);
#else
	write(2, buffer, strlen(buffer));
	write(2, "\n", 1);
#endif	
}

/* this function should go into BTAPP_DM for example */
BT_API UINT8 BTAPP_SetTraceLevel( UINT8 new_level )
{
    if (new_level != 0xFF)
        appl_trace_level = new_level;

    return (appl_trace_level);
}

BTU_API UINT8 BTU_SetTraceLevel( UINT8 new_level )
{
    if (new_level != 0xFF)
        btu_cb.trace_level = new_level;

    return (btu_cb.trace_level);
}




/********************************************************************************
 **
 **    Function Name:    BTA_SysSetTraceLevel
 **
 **    Purpose:          set or reads the different Trace Levels of layer IDs (see bt_trace.h,
 **                      BTTRC_ID_xxxx
 **
 **    Input Parameters: Array with trace layers to set to a given level or read. a layer ID of 0
 **                      defines the end of the list
 **                      WARNING: currently type should be 0-5! or FF for reading!!!!
 **
 **    Returns:
 **                      input array with trace levels for given layer id
 **
 *********************************************************************************/
BT_API tBTTRC_LEVEL * BTA_SysSetTraceLevel(tBTTRC_LEVEL * p_levels)
{
    const tBTTRC_FUNC_MAP *p_f_map;
    tBTTRC_LEVEL *p_l = p_levels;

    DBG_TRACE_API0( "BTA_SysSetTraceLevel()" );

    while (0 != p_l->layer_id)
    {
        p_f_map = &bttrc_set_level_map[0];
        
        while (0 != p_f_map->layer_id_start)
        {
            printf("BTA_SysSetTraceLevel - trace id in map start = %d end= %d,  paramter id = %d\r\n", p_f_map->layer_id_start, p_f_map->layer_id_end, p_l->layer_id );
            /* as p_f_map is ordered by increasing layer id, go to next map entry as long end id
             * is smaller */
            //if (p_f_map->layer_id_end < p_l->layer_id)
            //{
                //p_f_map++;
            //}
            //else
            {
                /* check if layer_id actually false into a range or if it is note existing in the  map */
                if ((NULL != p_f_map->p_f) && (p_f_map->layer_id_start <= p_l->layer_id) && (p_f_map->layer_id_end >= p_l->layer_id) )
                {
                    DBG_TRACE_DEBUG2( "BTA_SysSetTraceLevel( id:%d, level:%d ): setting/reading",
                            p_l->layer_id, p_l->type );
                    p_l->type = p_f_map->p_f(p_l->type);
                    break;
                }
                else
                {
                    DBG_TRACE_WARNING2( "BTA_SysSetTraceLevel( id:%d, level:%d ): MISSING Set function OR ID in map!",
                            p_l->layer_id, p_l->type );
                }
                /* set/read next trace level by getting out ot map loop */
                //p_l++;
                //break;
            }
            p_f_map++;
        }
        //if (0 == p_f_map->layer_id_start)
        {
            DBG_TRACE_WARNING2( "BTA_SysSetTraceLevel( id:%d, level:%d ): ID NOT FOUND in map. Skip to next",
                    p_l->layer_id, p_l->type );
            p_l++;
        }
    }

    return p_levels;
} /* BTA_SysSetTraceLevel() */

/* make sure list is order by increasing layer id!!! */
const tBTTRC_FUNC_MAP bttrc_set_level_map[] = {
    { BTTRC_ID_STK_BTU, BTTRC_ID_STK_HCI, (const tBTTRC_SET_TRACE_LEVEL *)BTU_SetTraceLevel, "TRC_HCI" },
    { BTTRC_ID_STK_L2CAP, BTTRC_ID_STK_L2CAP, (const tBTTRC_SET_TRACE_LEVEL *)L2CA_SetTraceLevel, "TRC_L2CAP" },
#if (RFCOMM_INCLUDED==TRUE)
    { BTTRC_ID_STK_RFCOMM, BTTRC_ID_STK_RFCOMM_DATA, (const tBTTRC_SET_TRACE_LEVEL *)PORT_SetTraceLevel, "TRC_RFCOMM" },
#endif
#if (OBX_INCLUDED==TRUE)
    { BTTRC_ID_STK_OBEX, BTTRC_ID_STK_OBEX, (const tBTTRC_SET_TRACE_LEVEL *)OBX_SetTraceLevel, "TRC_OBEX" },
#endif
#if (AVCT_INCLUDED==TRUE)
    { BTTRC_ID_STK_AVCT, BTTRC_ID_STK_AVCT, (tBTTRC_SET_TRACE_LEVEL *)NULL, "TRC_AVCT" },
#endif
#if (AVDT_INCLUDED==TRUE)
    { BTTRC_ID_STK_AVDT, BTTRC_ID_STK_AVDT, (const tBTTRC_SET_TRACE_LEVEL *)AVDT_SetTraceLevel, "TRC_AVDT" },
#endif
#if (AVRC_INCLUDED==TRUE)
    { BTTRC_ID_STK_AVRC, BTTRC_ID_STK_AVRC, (const tBTTRC_SET_TRACE_LEVEL *)AVRC_SetTraceLevel, "TRC_AVRC" },
#endif
#if (AVDT_INCLUDED==TRUE)
    { BTTRC_ID_AVDT_SCB, BTTRC_ID_AVDT_CCB, (tBTTRC_SET_TRACE_LEVEL *)NULL, "TRC_AVDT_SCB" },
#endif
#if (A2D_INCLUDED==TRUE)
    { BTTRC_ID_STK_A2D, BTTRC_ID_STK_A2D, (const tBTTRC_SET_TRACE_LEVEL *)A2D_SetTraceLevel, "TRC_A2D" },
#endif
#if (BIP_INCLUDED==TRUE)
    { BTTRC_ID_STK_BIP, BTTRC_ID_STK_BIP, (const tBTTRC_SET_TRACE_LEVEL *)BIP_SetTraceLevel, "TRC_BIP" },
#endif
#if (BNEP_INCLUDED==TRUE)
    { BTTRC_ID_STK_BNEP, BTTRC_ID_STK_BNEP, (const tBTTRC_SET_TRACE_LEVEL *)BNEP_SetTraceLevel, "TRC_BNEP" },
#endif
#if (BPP_INCLUDED==TRUE)
    { BTTRC_ID_STK_BPP, BTTRC_ID_STK_BPP, (const tBTTRC_SET_TRACE_LEVEL *)BPP_SetTraceLevel, "TRC_BPP" },
#endif
    { BTTRC_ID_STK_BTM_ACL, BTTRC_ID_STK_BTM_SEC, (const tBTTRC_SET_TRACE_LEVEL *)BTM_SetTraceLevel, "TRC_BTM" },
#if (DUN_INCLUDED==TRUE)
    { BTTRC_ID_STK_DUN, BTTRC_ID_STK_DUN, (const tBTTRC_SET_TRACE_LEVEL *)DUN_SetTraceLevel, "TRC_DUN" },
#endif
#if (GAP_INCLUDED==TRUE)
    { BTTRC_ID_STK_GAP, BTTRC_ID_STK_GAP, (const tBTTRC_SET_TRACE_LEVEL *)GAP_SetTraceLevel, "TRC_GAP" },
#endif
#if (GOEP_INCLUDED==TRUE)
    { BTTRC_ID_STK_GOEP, BTTRC_ID_STK_GOEP, (const tBTTRC_SET_TRACE_LEVEL *)GOEP_SetTraceLevel, "TRC_GOEP" },
#endif
#if (HCRP_INCLUDED==TRUE)
    { BTTRC_ID_STK_HCRP, BTTRC_ID_STK_HCRP, (const tBTTRC_SET_TRACE_LEVEL *)HCRP_SetTraceLevel, "TRC_HCRP" },
#endif
#if (PAN_INCLUDED==TRUE)
    { BTTRC_ID_STK_PAN, BTTRC_ID_STK_PAN, (const tBTTRC_SET_TRACE_LEVEL *)PAN_SetTraceLevel, "TRC_PAN" },
#endif
#if (SAP_SERVER_INCLUDED==TRUE)
    { BTTRC_ID_STK_SAP, BTTRC_ID_STK_SAP, (tBTTRC_SET_TRACE_LEVEL *)NULL, "TRC_SAP" },
#endif
    { BTTRC_ID_STK_SDP, BTTRC_ID_STK_SDP, (const tBTTRC_SET_TRACE_LEVEL *)SDP_SetTraceLevel, "TRC_SDP" },
#if (VDP_INCLUDED==TRUE)
    { BTTRC_ID_STK_VDP, BTTRC_ID_STK_VDP, (const tBTTRC_SET_TRACE_LEVEL *)VDP_SetTraceLevel, "TRC_VDP" },
#endif
#if (BLE_INCLUDED==TRUE)
    { BTTRC_ID_STK_GATT, BTTRC_ID_STK_GATT, (const tBTTRC_SET_TRACE_LEVEL *)GATT_SetTraceLevel , "TRC_GATT" },
#endif
#if (BLE_INCLUDED==TRUE)
    { BTTRC_ID_STK_SMP, BTTRC_ID_STK_SMP, (const tBTTRC_SET_TRACE_LEVEL *)SMP_SetTraceLevel , "TRC_SMP" },
#endif

    /* LayerIDs for BTA, currently everything maps onto appl_trace_level. BTL_GLOBAL_PROP_TRC_FLAG
     * serves as flag in property. if present, the whole table is scanned. */
#if (BTA_INCLUDED==TRUE)
    { BTTRC_ID_BTA_ACC, BTTRC_ID_BTAPP, (const tBTTRC_SET_TRACE_LEVEL *)BTAPP_SetTraceLevel, BTL_GLOBAL_PROP_TRC_FLAG },
#endif

#if 0
    {BTTRC_ID_BTA_HF                    39         /* headset/handsfree AG & HF */
    {BTTRC_ID_BTA_AV                    40         /* Advanced audio */
    {BTTRC_ID_BTA_BIP                   41         /* Basic Imaging Client */
    {BTTRC_ID_BTA_BP                    42         /* Basic Printing Client */
    {BTTRC_ID_BTA_CTP                   43         /* cordless telephony profile */
    {BTTRC_ID_BTA_DG                    44         /* data gateway */
    {BTTRC_ID_BTA_DM                    45         /* device manager */
    {BTTRC_ID_BTA_FM                    46
    {BTTRC_ID_BTA_FS                    47         /* File System */
    {BTTRC_ID_BTA_FTP                   48         /* file transfer client & server */
    {BTTRC_ID_BTA_HID                   49         /* hidc & hidd */
    {BTTRC_ID_BTA_JV                    50         /* java connector */
    {BTTRC_ID_BTA_OPP                   51         /* object push client */
    {BTTRC_ID_BTA_PAN                   52         /* Personal Area Networking */
    {BTTRC_ID_BTA_PR                    53         /* Printer module */
    {BTTRC_ID_BTA_SC                    54         /* SIM Card Access module */
    {BTTRC_ID_BTA_SS                    55         /* synchronization module */
    {BTTRC_ID_BTA_SYS                   56         /* system manager */
    {BTTRC_ID_BTA_SSR                   57         /* GPS sensor */
    {BTTRC_ID_BTA_ME                    58         /* message equipement server/client */

    /* LayerIDs for BT APP */
    { BTTRC_ID_BTAPP, BTTRC_ID_BTAPP, (const tBTTRC_SET_TRACE_LEVEL *)BTAPP_SetTraceLevel, "BTAPP" },
#endif

    { 0, 0, NULL, "" }
};

const UINT16 bttrc_map_size = sizeof(bttrc_set_level_map)/sizeof(tBTTRC_FUNC_MAP);
#endif

/********************************************************************************
 **
 **    Function Name:     BTE_InitTraceLevels
 **
 **    Purpose:           This function can be used to set the boot time reading it from the
 **                       platform.
 **                       WARNING: it is called under BTU context and it blocks the BTU task
 **                       till it returns (sync call)
 **
 **    Input Parameters:  None, platform to provide levels
 **    Returns:
 **                       Newly set levels, if any!
 **
 *********************************************************************************/
BT_API void BTE_InitTraceLevels( void )
{
    /* read and set trace levels from android property system and call the different
     * XXX_SetTraceLevel().
     */
#if ( BT_USE_TRACES==TRUE )
    /* read runtime trace settings after init of control block */

    // BLUEDROID MOD
    //if ( !btl_cfg_get_trace_prop() )
    {
#if defined(BTL_CFG_USE_CONF_FILE) && (BTL_CFG_USE_CONF_FILE==TRUE)
        if (NULL!=bte_appl_cfg.p_conf_params)
        {
            if ( 0 > btl_cfg_set_by_idx_conf( &conf_table, bte_appl_cfg.p_conf_params,
                                              BTL_CFG_CONF_TRACE) )
            {
                BT_TRACE_0( TRACE_LAYER_NONE, TRACE_TYPE_DEBUG, "[bttrc] using compile default trace settings" );
            }
            /* free up property settings data from conf file as needed anymore */
            GKI_os_free(bte_appl_cfg.p_conf_params);
            bte_appl_cfg.p_conf_params = NULL;
            BT_TRACE_0( TRACE_LAYER_NONE, TRACE_TYPE_DEBUG, "BTE_InitTraceLevels(): freed p_conf_params" );
        }
        else
        {
            BT_TRACE_0( TRACE_LAYER_NONE, TRACE_TYPE_DEBUG, "[bttrc] using compile default trace settings" );
        }
#else
        BT_TRACE_0( TRACE_LAYER_NONE, TRACE_TYPE_DEBUG, "[bttrc] using compile default trace settings" );
#endif
    }
#endif
}


/********************************************************************************
 **
 **    Function Name:   LogMsg_0
 **
 **    Purpose:  Encodes a trace message that has no parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_0(UINT32 trace_set_mask, const char *fmt_str) {
    LogMsg(trace_set_mask, fmt_str);
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_1
 **
 **    Purpose:  Encodes a trace message that has one parameter argument
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_1(UINT32 trace_set_mask, const char *fmt_str, UINT32 p1) {

    LogMsg(trace_set_mask, fmt_str, p1);
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_2
 **
 **    Purpose:  Encodes a trace message that has two parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_2(UINT32 trace_set_mask, const char *fmt_str, UINT32 p1, UINT32 p2) {
    LogMsg(trace_set_mask, fmt_str, p1, p2);
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_3
 **
 **    Purpose:  Encodes a trace message that has three parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_3(UINT32 trace_set_mask, const char *fmt_str, UINT32 p1, UINT32 p2,
        UINT32 p3) {
    LogMsg(trace_set_mask, fmt_str, p1, p2, p3);
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_4
 **
 **    Purpose:  Encodes a trace message that has four parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_4(UINT32 trace_set_mask, const char *fmt_str, UINT32 p1, UINT32 p2,
        UINT32 p3, UINT32 p4) {
    LogMsg(trace_set_mask, fmt_str, p1, p2, p3, p4);
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_5
 **
 **    Purpose:  Encodes a trace message that has five parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_5(UINT32 trace_set_mask, const char *fmt_str, UINT32 p1, UINT32 p2,
        UINT32 p3, UINT32 p4, UINT32 p5) {
    LogMsg(trace_set_mask, fmt_str, p1, p2, p3, p4, p5);
}

/********************************************************************************
 **
 **    Function Name:   LogMsg_6
 **
 **    Purpose:  Encodes a trace message that has six parameter arguments
 **
 **    Input Parameters:  trace_set_mask: tester trace type.
 **                       fmt_str: displayable string.
 **    Returns:
 **                      Nothing.
 **
 *********************************************************************************/
void LogMsg_6(UINT32 trace_set_mask, const char *fmt_str, UINT32 p1, UINT32 p2,
        UINT32 p3, UINT32 p4, UINT32 p5, UINT32 p6) {
    LogMsg(trace_set_mask, fmt_str, p1, p2, p3, p4, p5, p6);
}