

#ifndef _LOG_H_
#define _LOH_H_

#include "Segger_RTT/SEGGER_RTT.h"

#define LOG_DEBUG 1

#if LOG_DEBUG

/* Segger RTT ��֧�ָ�������ӡ */
#define LOG_PROTO(type,color,format,...)            \
        SEGGER_RTT_printf(0,"  %s%s"format"\r\n%s", \
                          color,                    \
                          type,                     \
                          ##__VA_ARGS__,            \
                          RTT_CTRL_RESET)

/* ����*/
#define LOG_CLEAR() SEGGER_RTT_WriteString(0, "  "RTT_CTRL_CLEAR)

/* ����ɫ��־��� */
#define LOG(format,...) LOG_PROTO("","",format,##__VA_ARGS__)

/* ����ɫ��ʽ��־��� */
#define LOGI(format,...) LOG_PROTO("I: ", RTT_CTRL_TEXT_BRIGHT_GREEN , format, ##__VA_ARGS__)
#define LOGW(format,...) LOG_PROTO("W: ", RTT_CTRL_TEXT_BRIGHT_YELLOW, format, ##__VA_ARGS__)
#define LOGE(format,...) LOG_PROTO("E: ", RTT_CTRL_TEXT_BRIGHT_RED   , format, ##__VA_ARGS__)

#else
#define LOG_CLEAR()
#define LOG
#define LOGI
#define LOGW
#define LOGE

#endif

#endif // !_LOG_H_
