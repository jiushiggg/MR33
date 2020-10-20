/*
 * update_mr33.c
 *
 *  Created on: 2020Äê10ÔÂ15ÈÕ
 *      Author: gaolongfei
 */

#include "update_mr33.h"
#include "update_type.h"
#include "rf_handle.h"
#include "debug.h"


int8_t set_wk_handle(void** addr, rf_parse_st* info)
{
    set_wkup_st *set = (set_wkup_st*)addr;
    TRACE();

    if((set->cmd==CMD_SET_WKUP_TRN)||(set->cmd==CMD_SET_WKUP_BDC))
    {
        pdebug("set wkup trn & bdc, loop:%d\r\n", set_loop_times);
        pinfoEsl("sw0 bg\r\n");
        wakeup_start(set_addr, set_len, 0);
    }
    else if((set->cmd==CMD_SET_WKUP_GLB) || (set->cmd==CMD_SET_WKUP_CH))
    {
        pinfoEsl("sw1 bg\r\n");
        pdebug("set wkup glb & ch\r\n");
        wakeup_start(set_addr, set_len, 1);

    }
    else if (set_cmd == CMD_SET_LED_FLASH)
    {
        pinfo("sw2 bg\r\n");
        set_wakeup_led_flash(set_addr, &frame1_addr, updata_table->data, set_len);
    }

}

int8_t group_wk_handle(void** addr, rf_parse_st* info)
{

}

int8_t frame1_handle(void** addr, rf_parse_st* info)
{

}

int8_t sleep_handle(void* addr, rf_parse_st* info)
{

}

int8_t updata_handle(void** addr, rf_parse_st* info)
{

}

int8_t query_handle(void** addr, rf_parse_st* info)
{

}




