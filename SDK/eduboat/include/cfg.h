/**
 * @file cfg.h
 * @author Scott CJX (scottcjx.w@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-08-02
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _CFG
#define _CFG

#include <MsgPack.h>
#include <stdint.h>

#define AP_CHANNEL 1

static struct Message
{
    uint8_t x;
    uint8_t y;

    MSGPACK_DEFINE(x, y);
} message;


#endif /* _CFG */
