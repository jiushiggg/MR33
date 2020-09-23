#include "common.h"
#include "crc16.h"
#include <string.h>
#include "debug.h"
#include "t2gupdata.h"

UINT16 cal_crc16(UINT8 ctrl, const UINT8 *eslid, const UINT8 *pdata, UINT8 len)
{
	UINT16 crc = 0;
	
	crc = CRC16_CaculateStepByStep(crc, &ctrl, sizeof(ctrl));
	crc = CRC16_CaculateStepByStep(crc, pdata, len);
	crc = CRC16_CaculateStepByStep(crc, eslid, 4);
	
	return crc;
}

INT32 make_sleep_data(UINT8 *eslid, UINT8 x, UINT8 *pdata, UINT8 len)
{
	UINT16 crc = 0;
	
	if(len < 3)
	{
		return -1;
	}
	
	memset(pdata, 0, len);
	pdata[0] = CTRL_SLEEP | (eslid[x] & 0x1F);
	crc = cal_crc16(pdata[0], eslid, pdata+1, len-3); // 3 = 1 byte ctrl + 2 bytes crc
	pdata[len-2] = crc % 256;
	pdata[len-1] = crc / 256;
	
	return (INT32)len;
}

// for g3
void g3_make_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len)
{
	UINT16 crc = 0;
	
	pdata[0] = (3 << 5) | (first_pkg_data[0] & 0x1F);
	pdata[1] = slot;
	memcpy(&pdata[2], &pkg_num, sizeof(pkg_num));
	memcpy(&pdata[4], first_pkg_data, 6);
	crc = cal_crc16(pdata[0], eslid, pdata+1, 23);
	
	pdata[24] = crc % 256;
	pdata[25] = crc / 256;
}

UINT8 g3_check_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len)
{
	UINT8 ret = 0;
	UINT16 crc1 = 0, crc2 = 0;
	
	crc1 = pdata[24]+pdata[25]*256;
	crc2 = cal_crc16(pdata[0], eslid, pdata+1, 23);
	
	if(crc1 != crc2) //check crc
	{
		pinfo("crc:%d,%d\r\n", crc1, crc2);
		goto done;
	}
	
	if(pdata[0] != ((3 << 5) | (first_pkg_data[0] & 0x1F))) //check ctrl
	{
		pinfo("ctl:%d\r\n", pdata[0]);
		goto done;
	}
	
	if(pdata[1] != (UINT8)(slot+1)) //check sid
	{
		pinfo("ap:%d, %d,esl:%d\r\n", slot,slot+1, pdata[1]);
		goto done;
	}
	
	ret = 1;
	
done:
	return ret;
}
//for t2g
void t2g_make_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len)
{
    st_t2g_ap_query *p = (st_t2g_ap_query*)pdata;

    p->ctrl = T2G_AP_QUERY_CTRL;
    p->sid = first_pkg_data[1];
    p->slot = slot;
    p->total_packet = pkg_num;
    memcpy(&p->reserve, first_pkg_data, 6);
    p->crc = cal_crc16(p->ctrl, eslid, pdata+1, 23);

}

UINT8 t2g_check_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len)
{
    UINT8 ret = 0;
    UINT16 crc2 = 0;
    st_t2g_esl_rsp *p = (st_t2g_esl_rsp*)pdata;

    crc2 = cal_crc16(p->ctrl, eslid, pdata+1, 23);

    if(p->crc != crc2)                  //check crc
    {
        pdebug("crc error:%x, %x\r\n", p->crc, crc2);
        goto done;
    }

    if(p->ctrl != T2G_AP_RESPONSE_CTRL)    //check ctrl
    {
        pdebug("crc ctrl:%x\r\n", p->ctrl);
        goto done;
    }
    if (p->sid != first_pkg_data[1]){   //check sid
        pdebug("crc sid:%d,%d\r\n", p->sid, first_pkg_data[1]);
        goto done;
    }

    if(p->slot != (slot+1))             //check slot
    {
        pdebug("crc slot:%d, %d\r\n", p->slot, slot);
        goto done;
    }

    ret = 1;

done:
    return ret;
}
INT32 t2g_make_sleep_data(UINT8 *eslid, UINT8 x, UINT8 *pdata, UINT8 len)
{
    st_t2g_esl_sleep *p = (st_t2g_esl_sleep*)pdata;
    if(len < 3)
    {
        return -1;
    }

    memset(pdata, 0, len);
    p->ctrl = T2G_SLEEP_CTRL;
    //p->total_pkg =
    p->crc = cal_crc16(p->ctrl, eslid, pdata+1, len-3); // 3 = 1 byte ctrl + 2 bytes crc

    return (INT32)len;
}


// for g2
void g2_make_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len)
{
	UINT16 crc = 0;
	
	memset(pdata, 0, len);
	pdata[0] = (3 << 5) | (first_pkg_data[0] & 0x1F);
	pdata[1] = slot;
	memcpy(&pdata[2], &pkg_num, sizeof(pkg_num));
	memcpy(&pdata[4], first_pkg_data, 6);
	crc = cal_crc16(pdata[0], eslid, pdata+2, 22);
	pdata[24] = crc % 256;
	pdata[25] = crc / 256;
}

UINT8 g2_check_link_query(UINT8 *eslid, UINT16 pkg_num, UINT8 slot, UINT8 *first_pkg_data, UINT8 *pdata, UINT8 len)
{
	UINT8 ret = 0;
	UINT16 crc1 = 0, crc2 = 0;
	
	crc1 = pdata[24]+pdata[25]*256;
	crc2 = cal_crc16(pdata[0], eslid, pdata+2, 22);
	
	if(crc1 != crc2) //check crc
	{
		goto done;
	}
	
	if(pdata[0] != ((3 << 5) | (first_pkg_data[0] & 0x1F))) //check ctrl
	{
		goto done;
	}
	
	if(pdata[1] != (slot+1)) //check sid
	{
		goto done;
	}

	ret = 1;
	
done:
	return ret;
}

