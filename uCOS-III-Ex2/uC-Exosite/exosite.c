/*
*********************************************************************************************************
*                                               Exosite
*                                         Exosite Device Cloud
*
*                                      (c) Copyright 2011, Exosite
*                                          All Rights Reserved
*
*
* File    : EXOSITE.C
* By      : CSR
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include  "rdk_meta.h"
#include "stdlib.h"
#include  <includes.h>
//#include  <iodefine.h>  // alternate MAC address retrieval
#include <lib_ascii.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
#define RX_SIZE     50 // must be at least 12 in order to read HTTP status header
#define OSN_SIZE    30
#define OSV_SIZE    16

#define MAC_LENGTH  12
#define CIK_LENGTH  40
#define PID_LENGTH  9
#define PID         "YRDKRX62N"

/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
static CPU_SIZE_T    OSN_LENGTH;
static CPU_CHAR      OSN[OSN_SIZE]; // buffer size is fixed, length is arbitrary
static CPU_SIZE_T    OSV_LENGTH;
static CPU_CHAR      OSV[OSV_SIZE]; // buffer size is fixed, length is arbitrary
static CPU_CHAR      MAC[MAC_LENGTH];
static CPU_CHAR      CIK[CIK_LENGTH];
static NET_IP_ADDR   IP;
static NET_PORT_NBR  PORT;

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static void init_mac_address(NET_IF_NBR if_nbr);
static void init_flash_content(void);
static void activate_device(void);
static void update_m2ip(void);

static void socket_close(NET_SOCK_ID sock);
static NET_SOCK_ID socket_open(void);
static CPU_SIZE_T socket_recv(NET_SOCK_ID sock, CPU_CHAR *rx_buf, CPU_SIZE_T rx_buflen);
static CPU_SIZE_T socket_send(NET_SOCK_ID sock, CPU_CHAR *tx_buf, CPU_SIZE_T tx_buflen);

static CPU_SIZE_T url_encode(CPU_CHAR *buf, CPU_SIZE_T bufsize, CPU_CHAR *str, CPU_SIZE_T strlen);

CPU_BOOLEAN Exosite_Update(void);

/*
*********************************************************************************************************
*********************************************************************************************************
*/
CPU_BOOLEAN Exosite_Reinit(void)
{
    CPU_SIZE_T i;

    update_m2ip();
    activate_device();

    for (i = 0; i < CIK_LENGTH; i++)
    {
        if (!(CIK[i] >= 'a' && CIK[i] <= 'f' || CIK[i] >= '0' && CIK[i] <= '9'))
        {
            return DEF_FALSE;
        }
    }

    return DEF_TRUE;
}

/*
*********************************************************************************************************
*********************************************************************************************************
*/
CPU_BOOLEAN Exosite_Init(CPU_CHAR * pOS, CPU_CHAR * pVer, NET_IF_NBR if_nbr)
{
    init_mac_address(if_nbr);
    init_flash_content();

    Str_Copy_N(OSN, pOS, OSN_SIZE);
    OSN[OSN_SIZE - 1] = 0;
    OSN_LENGTH = Str_Len(OSN);

    Str_Copy_N(OSV, pVer, OSV_SIZE);
    OSV[OSV_SIZE - 1] = 0;
    OSV_LENGTH = Str_Len(OSV);

    return Exosite_Reinit();
}

/*
*********************************************************************************************************
* Get MAC address
*********************************************************************************************************
*/
CPU_BOOLEAN Exosite_GetMAC(CPU_CHAR * pMAC)
{
    CPU_INT16U index = 0;
    while(index < MAC_LENGTH)
    {
        pMAC[index] = MAC[index];
        index++;
    }
    pMAC[index] = ASCII_CHAR_NULL;
    return DEF_TRUE;
}

/*
*********************************************************************************************************
* Temporarily set the used CIK to something else
* or reset it to value stored in flash by calling with NULL
*********************************************************************************************************
*/
void Exosite_UseCIK(CPU_CHAR * pCIK)
{
    if (NULL != pCIK && 0 != *pCIK)
    {
        Str_Copy_N(CIK, pCIK, CIK_LENGTH);
    }
    else
    {
        rdk_meta *meta_info = (rdk_meta *)RDK_META_LOCATION;
        Str_Copy_N(CIK, (CPU_CHAR*)meta_info->cik, CIK_LENGTH);
    }
}

/*
*********************************************************************************************************
* Store new CIK in flash
*********************************************************************************************************
*/
void Exosite_SetCIK(CPU_CHAR * pCIK)
{
    rdk_meta *meta_info = (rdk_meta *)RDK_META_LOCATION;

    if (0 != Str_Cmp_N((CPU_CHAR*)meta_info->cik, pCIK, CIK_LENGTH))
    {
        rdk_meta_write(pCIK, CIK_LENGTH, meta_info->cik);
    }
    Str_Copy_N(CIK, pCIK, CIK_LENGTH);
}

/*
*********************************************************************************************************
* NOTE: 'pkey' must be urlencoded
*       'pbuf' is returned urlencoded and must be decoded by the caller.
*********************************************************************************************************
*/
CPU_SIZE_T Exosite_Read(CPU_CHAR *pkey, CPU_CHAR *pbuf, CPU_SIZE_T buflen)
{
    NET_SOCK_ID  sock;
    CPU_SIZE_T   len, klen, vlen, rxlen;
    CPU_CHAR    *p,  *pcheck;
    CPU_CHAR     rx[RX_SIZE];

    vlen = 0;
    klen = Str_Len(pkey);
    pcheck = pkey;

    sock = socket_open();

    if (-1 == sock)
    {
        return vlen;
    }

    if (
        24         != socket_send(sock, "GET /api:v1/stack/alias?", 24) ||
        klen       != socket_send(sock, pkey, klen) ||
        12         != socket_send(sock, "= HTTP/1.1\r\n", 12) ||
        22         != socket_send(sock, "Host: m2.exosite.com\r\n", 22) ||
        15         != socket_send(sock, "X-Exosite-CIK: ", 15) ||
        CIK_LENGTH != socket_send(sock, CIK, CIK_LENGTH) ||
        2          != socket_send(sock, "\r\n", 2) ||
        58         != socket_send(sock, "Accept: application/x-www-form-urlencoded; charset=utf-8\r\n", 58) ||
        2          != socket_send(sock, "\r\n", 2)
    )
    {
        socket_close(sock);
        return vlen;
    }

    // @HTTP/x.x NNN@
    rxlen = socket_recv(sock, rx, 12);

    if (12 == rxlen && '2' == rx[9] && '0' == rx[10] && '0' == rx[11])
    {
        CPU_CHAR crlf = 0;

        do
        {
            rxlen = socket_recv(sock, rx, RX_SIZE);
            len = rxlen;
            p = rx;

            // Find 4 consecutive \r or \n - should be: \r\n\r\n
            while (0 < len && 4 > crlf)
            {
                if ('\r' == *p || '\n' == *p)
                {
                    ++crlf;
                }
                else
                {
                    crlf = 0;
                }
                ++p;
                --len;
            }

            // The body is "<key>=<value>"
            if (0 < len && 4 == crlf && buflen > vlen)
            {
                // Move past "<key>"
                while (0 < len && 0 != *pcheck)
                {
                    if (*pcheck == *p)
                    {
                        ++pcheck;
                    }
                    else
                    {
                        pcheck = pkey;
                    }
                    ++p;
                    --len;
                }

                // Match '=',  we should now have '<key>='
                if (0 < len && 0 == *pcheck && '=' == *p)
                {
                    ++p;
                    --len;
                }

                // read in the rest of the body as the value
                while (0 < len && buflen > vlen)
                {
                    pbuf[vlen++] = *p++;
                    --len;
                }
            }
        } while (RX_SIZE == rxlen);
    }

    socket_close(sock);
    return vlen;
}

/*
*********************************************************************************************************
* NOTE: 'pkey' and 'pval' must be urlencoded
*********************************************************************************************************
*/
CPU_BOOLEAN Exosite_Write(CPU_CHAR *pkey, CPU_CHAR *pval)
{
    CPU_BOOLEAN  success = DEF_FALSE;
    NET_SOCK_ID  sock;
    CPU_SIZE_T   len, slen, klen, vlen, rxlen;
    CPU_CHAR     length[4];
    CPU_CHAR     rx[RX_SIZE];

    klen = Str_Len(pkey);
    vlen = Str_Len(pval);
    len =  klen + 1 + vlen; // "<pkey>","=","<pval>"

    if (0 == Str_FmtNbr_Int32U (
        (CPU_INT32U)  len,
        (CPU_INT08U)  3,
        (CPU_INT08U)  10,
        (CPU_CHAR)    '\0',
        (CPU_BOOLEAN) DEF_YES,
        (CPU_BOOLEAN) DEF_YES,
        (CPU_CHAR*)   length
    ))
    {
        return success;
    }
    slen = Str_Len_N(length, 3);

    sock = socket_open();

    if (-1 == sock)
    {
        return success;
    }

    if (
        35         != socket_send(sock, "POST /api:v1/stack/alias HTTP/1.1\r\n", 35) ||
        22         != socket_send(sock, "Host: m2.exosite.com\r\n", 22) ||
        15         != socket_send(sock, "X-Exosite-CIK: ", 15) ||
        CIK_LENGTH != socket_send(sock, CIK, CIK_LENGTH) ||
        2          != socket_send(sock, "\r\n", 2) ||
        64         != socket_send(sock, "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n", 64) ||
        16         != socket_send(sock, "Content-Length: ", 16) ||
        slen       != socket_send(sock, length, slen) ||
        4          != socket_send(sock, "\r\n\r\n", 4) ||
        klen       != socket_send(sock, pkey, klen) ||
        1          != socket_send(sock, "=", 1) ||
        vlen       != socket_send(sock, pval, vlen)
    )
    {
        socket_close(sock);
        return success;
    }

    // @HTTP/x.x NNN@
    rxlen = socket_recv(sock, rx, 12);

    if (12 == rxlen && '2' == rx[9] && '0' == rx[10] && '4' == rx[11])
    {
        success = DEF_TRUE;
    }

    socket_close(sock);
    return success;
}

/*
*********************************************************************************************************
* NOTE: each key and value in 'pkeys' and 'pvalues' must be urlencoded
*********************************************************************************************************
*/
CPU_BOOLEAN Exosite_Write_Batch(CPU_CHAR **pkeys, CPU_CHAR **pvalues, CPU_SIZE_T count)
{
    CPU_BOOLEAN  success = DEF_FALSE;
    NET_SOCK_ID  sock;
    CPU_SIZE_T   len, slen, rxlen;
    CPU_CHAR     length[4];
    CPU_CHAR     rx[RX_SIZE];
    CPU_SIZE_T   i;

    if (0 == count)
    {
        return success;
    }

    len = 0;
    for (i = 0; i < count; i++)
    {
        len +=  Str_Len(pkeys[i]) + 1 + Str_Len(pvalues[i]); // "<key>","=","<val>"
    }
    len += count - 1; // number of '&' chars.

    if (0 == Str_FmtNbr_Int32U (
        (CPU_INT32U)  len,
        (CPU_INT08U)  3,
        (CPU_INT08U)  10,
        (CPU_CHAR)    '\0',
        (CPU_BOOLEAN) DEF_YES,
        (CPU_BOOLEAN) DEF_YES,
        (CPU_CHAR*)   length
    ))
    {
        return success;
    }
    slen = Str_Len_N(length, 3);

    sock = socket_open();

    if (-1 == sock)
    {
        return success;
    }

    if (
        35         != socket_send(sock, "POST /api:v1/stack/alias HTTP/1.1\r\n", 35) ||
        22         != socket_send(sock, "Host: m2.exosite.com\r\n", 22) ||
        15         != socket_send(sock, "X-Exosite-CIK: ", 15) ||
        CIK_LENGTH != socket_send(sock, CIK, CIK_LENGTH) ||
        2          != socket_send(sock, "\r\n", 2) ||
        64         != socket_send(sock, "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n", 64) ||
        16         != socket_send(sock, "Content-Length: ", 16) ||
        slen       != socket_send(sock, length, slen) ||
        4          != socket_send(sock, "\r\n\r\n", 4)
    )
    {
        socket_close(sock);
        return success;
    }

    i = 0;
    socket_send(sock, pkeys[i], Str_Len(pkeys[i]));
    socket_send(sock, "=", 1);
    socket_send(sock, pvalues[i], Str_Len(pvalues[i]));
    for (i = 1; i < count; i++)
    {
        socket_send(sock, "&", 1);
        socket_send(sock, pkeys[i], Str_Len(pkeys[i]));
        socket_send(sock, "=", 1);
        socket_send(sock, pvalues[i], Str_Len(pvalues[i]));
    }

    // @HTTP/x.x NNN@
    rxlen = socket_recv(sock, rx, 12);

    if (12 == rxlen && '2' == rx[9] && '0' == rx[10] && '4' == rx[11])
    {
        success = DEF_TRUE;
    }

    socket_close(sock);
    return success;
}

/*
*********************************************************************************************************
*********************************************************************************************************
*/
static void activate_device(void)
{
    NET_SOCK_ID  sock;
    CPU_SIZE_T   len, slen, rxlen, osnlen, osvlen;
    CPU_CHAR     length[4];
    CPU_CHAR     NCIK[CIK_LENGTH];
    CPU_CHAR     rx[RX_SIZE];
    CPU_CHAR    *p, *pOSN, *pOSV;
    rdk_meta *meta_info = (rdk_meta *)RDK_META_LOCATION;
    
    pOSN = malloc(OSN_LENGTH * 3 + 1);
    osnlen = url_encode(pOSN, OSN_LENGTH * 3 + 1, OSN, OSN_LENGTH);
    pOSV = malloc(OSV_LENGTH * 3 + 1);
    osvlen = url_encode(pOSV, OSV_LENGTH * 3 + 1, OSV, OSV_LENGTH);

    len  = 6 + PID_LENGTH;        // "model=",PID
    len += 4 + MAC_LENGTH;        // "&sn=",MAC
    len += 5 + osnlen;            // "&osn=",OSN
    len += 5 + osvlen;            // "&osv=",OSV
    len += 5 + RDK_META_MFR_SIZE; // "&mfr=",MFRDATA
    
    if (0 == Str_FmtNbr_Int32U (
        (CPU_INT32U)  len,
        (CPU_INT08U)  3,
        (CPU_INT08U)  10,
        (CPU_CHAR)    '\0',
        (CPU_BOOLEAN) DEF_YES,
        (CPU_BOOLEAN) DEF_YES,
        (CPU_CHAR*)   length
    ))
    {
        return;
    }
    slen = Str_Len_N(length, 3);

    sock = socket_open();

    if (-1 == sock)
    {
        return;
    }

    if (
        35                != socket_send(sock, "POST /provision/activate HTTP/1.1\r\n", 35) ||
        22                != socket_send(sock, "Host: m2.exosite.com\r\n", 22) ||
        64                != socket_send(sock, "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n", 64) ||
        35                != socket_send(sock, "Accept: text/plain; charset=utf-8\r\n", 35) ||
        16                != socket_send(sock, "Content-Length: ", 16) ||
        slen              != socket_send(sock, length, slen) ||
        4                 != socket_send(sock, "\r\n\r\n", 4) ||
        14                != socket_send(sock, "vendor=renesas", 14) ||
        7                 != socket_send(sock, "&model=", 7) ||
        PID_LENGTH        != socket_send(sock, PID, PID_LENGTH) ||
        4                 != socket_send(sock, "&sn=", 4) ||
        MAC_LENGTH        != socket_send(sock, MAC, MAC_LENGTH) ||
        5                 != socket_send(sock, "&osn=", 5) ||
        osnlen            != socket_send(sock, pOSN, osnlen) ||
        5                 != socket_send(sock, "&osv=", 5) ||
        osvlen            != socket_send(sock, pOSV, osvlen) ||
        5                 != socket_send(sock, "&mfr=", 5) ||
        RDK_META_MFR_SIZE != socket_send(sock, meta_info->mfr, RDK_META_MFR_SIZE)
    )
    {
        socket_close(sock);
        return;
    }
    free(pOSN);
    free(pOSV);

    // @HTTP/x.x NNN@
    rxlen = socket_recv(sock, rx, 12);

    if (12 == rxlen && '2' == rx[9] && '0' == rx[10] && '0' == rx[11])
    {
        CPU_CHAR crlf = 0;
        CPU_CHAR ciklen = 0;

        do
        {
            rxlen = socket_recv(sock, rx, RX_SIZE);
            len = rxlen;
            p = rx;

            // Find 4 consecutive \r or \n - should be: \r\n\r\n
            while (0 < len && 4 > crlf)
            {
                if ('\r' == *p || '\n' == *p)
                {
                    ++crlf;
                }
                else
                {
                    crlf = 0;
                }
                ++p;
                --len;
            }

            // The body is the CIK
            if (0 < len && 4 == crlf && CIK_LENGTH > ciklen)
            {
                // TODO, be more robust - match Content-Length header value to CIK_LENGTH
                CPU_CHAR need, part;
                need = CIK_LENGTH - ciklen;
                part = need < len ? need : len;
                Str_Copy_N(NCIK + ciklen, p, part);
                ciklen += part;
            }
        } while (RX_SIZE == rxlen);

        if (CIK_LENGTH == ciklen)
        {
            Exosite_SetCIK(NCIK);
        }
    }

    socket_close(sock);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*/
void update_m2ip(void)
{
    NET_SOCK_ID  sock;
    CPU_SIZE_T   len, rxlen, IPLEN = 23; // 23 => 3*6+5 => "nnn,nnn,nnn,nnn,nnn,nnn"
    CPU_CHAR    *p;
    CPU_CHAR     rx[RX_SIZE], ip[IPLEN];

    sock = socket_open();

    if (-1 == sock)
    {
        return;
    }

    if (
        18 != socket_send(sock, "GET /ip HTTP/1.1\r\n", 18) ||
        22 != socket_send(sock, "Host: m2.exosite.com\r\n", 22) ||
        35 != socket_send(sock, "Accept: text/plain; charset=utf-8\r\n", 35) ||
        2  != socket_send(sock, "\r\n", 2)
    )
    {
        socket_close(sock);
        return;
    }

    // @HTTP/x.x NNN@
    rxlen = socket_recv(sock, rx, 12);

    if (12 == rxlen && '2' == rx[9] && '0' == rx[10] && '0' == rx[11])
    {
        CPU_CHAR crlf = 0;
        CPU_CHAR iplen = 0;

        do
        {
            rxlen = socket_recv(sock, rx, RX_SIZE);
            len = rxlen;
            p = rx;

            // Find 4 consecutive \r or \n - should be: \r\n\r\n
            while (0 < len && 4 > crlf)
            {
                if ('\r' == *p || '\n' == *p)
                {
                    ++crlf;
                }
                else
                {
                    crlf = 0;
                }
                ++p;
                --len;
            }

            // The body is "nnn,nnn,nnn,nnn,nnn,nnn"
            if (0 < len && 4 == crlf && IPLEN > iplen)
            {
                // TODO, be more robust - match Content-Length header value to IPLEN
                CPU_CHAR need, part;
                need = IPLEN - iplen;
                part = need < len ? need : len;
                Str_Copy_N(ip + iplen, p, part);
                iplen += part;
            }
        } while (RX_SIZE == rxlen);

        if (0 < iplen && IPLEN >= iplen)
        {
            CPU_CHAR server_ip[6];
            CPU_CHAR i;

            p = ip;

            for (i = 0; i < 6 || iplen > 0; i++)
            {
                if (*p >= '0' && *p <= '9')
                {
                    server_ip[i] = *p++ - '0';
                    if (0 == --iplen) break;
                }
                if (*p >= '0' && *p <= '9')
                {
                    server_ip[i] *= 10;
                    server_ip[i] += *p++ - '0';
                    if (0 == --iplen) break;
                }
                if (*p >= '0' && *p <= '9')
                {
                    server_ip[i] *= 10;
                    server_ip[i] += *p++ - '0';
                    --iplen;
                }
                if (iplen > 0)
                {
                    if (5 == i || ',' != *p++)
                    {
                        break;
                    }
                    --iplen;
                }
            }

            if (6 == i)
            {
                rdk_meta *meta_info = (rdk_meta *)RDK_META_LOCATION;

                if (0 != Str_Cmp_N((CPU_CHAR*)meta_info->server, server_ip, 6))
                {
                    rdk_meta_write(server_ip, 6, meta_info->server);

                    // Convert stored copy to something usable
                    IP   =  server_ip[0] * 16777216
                          + server_ip[1] * 65536
                          + server_ip[2] * 256
                          + server_ip[3] * 1;
                    PORT =  server_ip[4] * 256
                          + server_ip[5] * 1;
                }
            }
        }
    }

    socket_close(sock);
    return;
}

/*
*********************************************************************************************************
*********************************************************************************************************
*/
static void init_flash_content(void)
{
    rdk_meta *meta_info;

    rdk_meta_init();

    meta_info = (rdk_meta *)RDK_META_LOCATION;

    if (0 != Str_Cmp_N((CPU_CHAR*)meta_info->mark, EXOMARK, Str_Len(EXOMARK)))
    {
        rdk_meta_defaults();
    }

    // Get local copy as it can be set temporarily
    Str_Copy_N(CIK, (CPU_CHAR*)meta_info->cik, CIK_LENGTH);

    // Convert stored copy to something usable
    IP   =  meta_info->server[0] * 16777216
          + meta_info->server[1] * 65536
          + meta_info->server[2] * 256
          + meta_info->server[3] * 1;
    PORT =  meta_info->server[4] * 256
          + meta_info->server[5] * 1;
}

/*
*********************************************************************************************************
*********************************************************************************************************
*/
static void init_mac_address(NET_IF_NBR if_nbr)
{
    CPU_CHAR    hex[] = "0123456789abcdef";
    NET_ERR     err;
    CPU_INT08U  addr_hw[6];
    CPU_INT08U  addr_hw_len = 6;

    NetIF_AddrHW_Get(
        (NET_IF_NBR  ) if_nbr,
        (CPU_INT08U *)&addr_hw[0],
        (CPU_INT08U *)&addr_hw_len,
        (NET_ERR    *)&err
    );

    if (NET_IF_ERR_NONE != err)
    {
        return;
    }

    MAC[0]  = hex[addr_hw[0] >> 4];
    MAC[1]  = hex[addr_hw[0] & 15];
    MAC[2]  = hex[addr_hw[1] >> 4];
    MAC[3]  = hex[addr_hw[1] & 15];
    MAC[4]  = hex[addr_hw[2] >> 4];
    MAC[5]  = hex[addr_hw[2] & 15];
    MAC[6]  = hex[addr_hw[3] >> 4];
    MAC[7]  = hex[addr_hw[3] & 15];
    MAC[8]  = hex[addr_hw[4] >> 4];
    MAC[9]  = hex[addr_hw[4] & 15];
    MAC[10] = hex[addr_hw[5] >> 4];
    MAC[11] = hex[addr_hw[5] & 15];
}

/*
static void init_mac_address(void)
{
    CPU_CHAR   hex[] = "0123456789abcdef";
    CPU_INT32U _mahr = ETHERC.MAHR;
    CPU_INT16U _malr = ETHERC.MALR.BIT.MA;

    MAC[0]  = hex[(_mahr >> 28) & 15];
    MAC[1]  = hex[(_mahr >> 24) & 15];
    MAC[2]  = hex[(_mahr >> 20) & 15];
    MAC[3]  = hex[(_mahr >> 16) & 15];
    MAC[4]  = hex[(_mahr >> 12) & 15];
    MAC[5]  = hex[(_mahr >> 8)  & 15];
    MAC[6]  = hex[(_mahr >> 4)  & 15];
    MAC[7]  = hex[(_mahr >> 0)  & 15];
    MAC[8]  = hex[(_malr >> 12) & 15];
    MAC[9]  = hex[(_malr >> 8)  & 15];
    MAC[10] = hex[(_malr >> 4)  & 15];
    MAC[11] = hex[(_malr >> 0)  & 15];
}
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*/
static void socket_close(NET_SOCK_ID sock)
{
    NET_ERR err;
    NetSock_CfgTimeoutConnCloseSet(sock, 3000, &err);
    NetSock_Close(sock, &err);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*/
static NET_SOCK_ID socket_open(void)
{
    NET_SOCK_ID       sock;
    NET_SOCK_ADDR_IP  sock_addr_ip;
    NET_ERR           err;

    Mem_Clr(
        (void *)    &sock_addr_ip,
        (CPU_SIZE_T) sizeof(sock_addr_ip)
    );

    sock_addr_ip.AddrFamily = NET_SOCK_ADDR_FAMILY_IP_V4;
    sock_addr_ip.Addr       = NET_UTIL_HOST_TO_NET_32(IP);
    sock_addr_ip.Port       = NET_UTIL_HOST_TO_NET_16(PORT);

    sock = NetSock_Open(
        NET_SOCK_ADDR_FAMILY_IP_V4,
        NET_SOCK_TYPE_STREAM,
        NET_SOCK_PROTOCOL_TCP,
        &err
    );

    if (NET_SOCK_ERR_NONE != err)
    {
        return -1;
    }

    NetSock_CfgTimeoutConnReqSet(sock, 2000, &err);

    if (NET_SOCK_ERR_NONE != err)
    {
        NetSock_Close(sock, &err);
        return -1;
    }

    NetSock_Conn(
        (NET_SOCK_ID)       sock,
        (NET_SOCK_ADDR *)  &sock_addr_ip,
        (NET_SOCK_ADDR_LEN) sizeof(sock_addr_ip),
        (NET_ERR *)        &err
    );

    if (NET_SOCK_ERR_NONE != err)
    {
        NetSock_Close(sock, &err);
        return -1;
    }

    return sock;
}

/*
*********************************************************************************************************
*********************************************************************************************************
*/
static CPU_SIZE_T socket_recv(NET_SOCK_ID sock, CPU_CHAR *rx_buf, CPU_SIZE_T rx_buflen)
{
    NET_SOCK_RTN_CODE  rtn_code;
    NET_ERR            err;

    NetSock_CfgTimeoutRxQ_Set(sock, 5000, &err);

    rtn_code = NetSock_RxData(
        (NET_SOCK_ID) sock,
        (void *)      rx_buf,
        (CPU_INT16U)  rx_buflen,
        (CPU_INT16S)  NET_SOCK_FLAG_NONE,
                     &err
    );

    if (0 >= rtn_code || NET_SOCK_ERR_NONE != err)
    {
        return 0;
    }

    return (CPU_SIZE_T)rtn_code;
}

/*
*********************************************************************************************************
*********************************************************************************************************
*/
static CPU_SIZE_T socket_send(NET_SOCK_ID sock, CPU_CHAR *tx_buf, CPU_SIZE_T tx_buflen)
{
    NET_SOCK_RTN_CODE  rtn_code;
    NET_ERR            err;
    CPU_SIZE_T         tx_size = 0;

    while (tx_size < tx_buflen)
    {
        NetSock_CfgTimeoutTxQ_Set(sock, 3000, &err);

        rtn_code = NetSock_TxData(
            (NET_SOCK_ID) sock,
            (void *)     (tx_buf + tx_size),
            (CPU_INT16U) (tx_buflen - tx_size),
            (CPU_INT16S)  NET_SOCK_FLAG_NONE,
                         &err
        );

        if (0 >= rtn_code || NET_SOCK_ERR_NONE != err)
        {
            break;
        }

        tx_size += (CPU_SIZE_T)rtn_code;
    }

    return tx_size;
}


/*
*********************************************************************************************************
* Snagged from: http://www.geekhideout.com/urlcode.shtml  Thanks!
* Modified to use CPU_XXX types and lib_ascii and to accept input/output buffers
*********************************************************************************************************
*/

/* Converts a hex character to its integer value */
static CPU_CHAR from_hex(CPU_CHAR ch)
{
    return ASCII_IsDig(ch) ? ch - '0' : ASCII_ToLower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
static CPU_CHAR to_hex(CPU_CHAR code)
{
    static CPU_CHAR hex[] = "0123456789abcdef";
    return hex[code & 15];
}

/* Returns a url-encoded version of str */
static CPU_SIZE_T url_encode(CPU_CHAR *buf, CPU_SIZE_T bufsize, CPU_CHAR *str, CPU_SIZE_T strlen)
{
    CPU_SIZE_T len = 0;
    CPU_CHAR *pstr = str, *pbuf = buf;

    // leave room for terminating char. No one would ever pass in zero of course.</sarcasm>
    --bufsize;

    while (strlen && *pstr && len < bufsize)
    {
        if (ASCII_IsAlphaNum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
        {
            *pbuf++ = *pstr;
            len += 1;
        }
        else if (*pstr == ' ')
        {
            *pbuf++ = '+';
            len += 1;
        }
        else if (len + 3 > bufsize)
        {
            break;
        }
        else
        {
            *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
            len += 3;
        }
        ++pstr;
        --strlen;
    }
    *pbuf = '\0';

    return len;
}

/* Returns a url-decoded version of str */
static CPU_SIZE_T url_decode(CPU_CHAR *buf, CPU_SIZE_T bufsize, CPU_CHAR *str, CPU_SIZE_T strlen)
{
    CPU_SIZE_T len = 0;
    CPU_CHAR *pstr = str, *pbuf = buf;

    // leave room for terminating char. No one would ever pass in zero of course.</sarcasm>
    --bufsize;

    while (strlen && *pstr && len < bufsize)
    {
        if (*pstr == '%')
        {
            if (!(2 < strlen && pstr[1] && pstr[2]))
            {
                break;
            }

            *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
            pstr += 2;
            strlen -= 2;
        }
        else if (*pstr == '+')
        {
            *pbuf++ = ' ';
        }
        else
        {
            *pbuf++ = *pstr;
        }
        ++pstr;
        --strlen;
        ++len;
    }
    *pbuf = '\0';

    return len;
}
