#ifndef __INCLUDED_BEACON_SENDER
#define __INCLUDED_BEACON_SENDER

#include "crc16.h"
#include "beacon_struct.h"

/////////////////////////////////////////////////////////////////////////////

void BEACON_PrintUINT8(uint8_t value)
{
    BEACON_PrintChar(value);
    //printf("[0x%02X=%02u]", value, value);
    if(value==BEACON_JOKER)
    {
        BEACON_PrintChar(value);
    }
}

/////////////////////////////////////////////////////////////////////////////

void BEACON_PrintSerialised(const uint8_t data[], uint8_t size, uint8_t * checkA, uint8_t * checkB)
{
    for (uint8_t i=0u; i<size; i++)
    {
        BEACON_PrintUINT8(data[i]);
    }
    fletcher16(checkA , checkB, data, size);
}

/////////////////////////////////////////////////////////////////////////////

void BEACON_SEND_MESSAGE(
    uint8_t sender_id,
    uint8_t * mesg_counter,
    uint8_t message_id,
    const uint8_t data[],
    uint8_t datasize
)
{
    uint8_t checkA=0u;
    uint8_t checkB=0u;
    BEACON_PrintUINT8(sender_id);
    fletcher16(&checkA , &checkB, &sender_id, 1u);
    {
        const uint8_t temp=(message_id<<4u) | ((*mesg_counter)&0x0Fu);
        BEACON_PrintUINT8( temp );
        fletcher16(&checkA , &checkB, &temp, 1u);
    }
    BEACON_PrintSerialised(data, datasize, &checkA , &checkB);
    BEACON_PrintUINT8(checkA);
    BEACON_PrintUINT8(checkB);
    BEACON_PrintChar(BEACON_JOKER);

    //Those two guarantee flush even if the frame was corrupted:
    BEACON_PrintChar(' ');
    BEACON_PrintChar(BEACON_JOKER);

    (*mesg_counter)++;
    if ((*mesg_counter)>0x0Fu)
    {
        (*mesg_counter)=0u;
    }
}

/////////////////////////////////////////////////////////////////////////////

void BEACON_SEND_PING(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_PING & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 0u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_SITUATION(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_SITUATION & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 1u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_ORIENTATION(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_ORIENTATION & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 2u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_NAVIGATION(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_NAVIGATION & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 3u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_PID(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_PID & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 4u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_METEO(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_METEO & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 5u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_DATETIME(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_DATETIME & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 6u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_MISSIONSAFETY(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_MISSIONSAFETY & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 7u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_PERFORMANCE(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_PERFORMANCE & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 8u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_ENGINE(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_ENGINE & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 9u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_FORMATION(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_FORMATION & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 10u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_STABHEAD(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_STABHEAD & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 11u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_IMPACT(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_IMPACT & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 12u, (uint8_t *)&data, sizeof(data) );
}

void BEACON_SEND_RECEPTION(uint8_t sender_id, uint8_t * mesg_counter, const BEACON_RECEPTION & data)
{
    BEACON_SEND_MESSAGE(sender_id, mesg_counter, 13u, (uint8_t *)&data, sizeof(data) );
}

/////////////////////////////////////////////////////////////////////////////

#endif //__INCLUDED_BEACON_SENDER

