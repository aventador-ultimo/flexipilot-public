#ifndef __INCLUDED_BEACON_PARSER
#define __INCLUDED_BEACON_PARSER

#include "crc16.h"
#include "beacon_struct.h"

static const uint8_t BEACON_BUFFER_MAXSIZE=43u;
static uint8_t BEACON_BUFFER[BEACON_BUFFER_MAXSIZE];//max(BEACON_mesglentable)*1+4+1
static uint8_t BEACON_BUFFER_USED=0u;
static const uint8_t BEACON_MIN_VALID_MESG_SIZE=5u;//sender, mesgid+counter, payload, 2xcrc, all *1=5 characters
static const uint8_t BEACON_mesglentable[]=
{
    sizeof(BEACON_PING),
    sizeof(BEACON_SITUATION),
    sizeof(BEACON_ORIENTATION),
    sizeof(BEACON_NAVIGATION),
    sizeof(BEACON_PID),
    sizeof(BEACON_METEO),
    sizeof(BEACON_DATETIME),
    sizeof(BEACON_MISSIONSAFETY),
    sizeof(BEACON_PERFORMANCE),
    sizeof(BEACON_ENGINE),
    sizeof(BEACON_FORMATION),
    sizeof(BEACON_STABHEAD),
    sizeof(BEACON_IMPACT),
    sizeof(BEACON_RECEPTION)
};

/////////////////////////////////////////////////////////////////////////////

uint8_t hexchar2nibble(char_t chr)
{
    if ( (chr>='0') && (chr<='9') )
    {
        return chr-'0';
    }
    if ( (chr>='A') && (chr<='F') )
    {
        return (uint8_t)((chr-'A')+0x0Au);
    }
    return 0u;
}

/////////////////////////////////////////////////////////////////////////////

uint8_t hexchar2byte(char_t chr1, char_t chr2)
{
    return (hexchar2nibble(chr1)<<4u) | hexchar2nibble(chr2);
}

/////////////////////////////////////////////////////////////////////////////

void BEACON_HandleReceive2(uint8_t chr, bool_t execute_parser)
{
    if(execute_parser==false)
    {
        if(BEACON_BUFFER_USED<BEACON_BUFFER_MAXSIZE)
        {
            BEACON_BUFFER[BEACON_BUFFER_USED]=chr;
            BEACON_BUFFER_USED++;
            //printf("{0x%02X}", (unsigned)chr);
        }
        else if (BEACON_BUFFER_USED>=BEACON_BUFFER_MAXSIZE)
        {
            execute_parser=true;
        }
    }

    if (execute_parser==true)
    {
        if (BEACON_BUFFER_USED > BEACON_MIN_VALID_MESG_SIZE)
        {
            //Parsing
            const uint8_t sender_id=BEACON_BUFFER[0u];
            if (sender_id>0u)
            {
                const uint8_t temp=BEACON_BUFFER[1u];
                const uint8_t message_id=(temp>>4u);
                if (message_id<sizeof(BEACON_mesglentable))
                {
                    const uint8_t message_counter=(temp&0x0Fu);// now unused
                    const uint8_t mesgsize=BEACON_mesglentable[message_id];
                    const uint8_t mesglen=mesgsize+4u;
                    if (BEACON_BUFFER_USED>=mesglen)
                    {
                        const uint8_t crcpos=mesglen-2u;
                        const uint8_t received_checkA=BEACON_BUFFER[crcpos];
                        const uint8_t received_checkB=BEACON_BUFFER[crcpos+1u];
                        uint8_t * unionpointer=(uint8_t *)(&BEACON_UNION);
                        for (uint8_t w=0u, r=2u; w<mesgsize; w++, r++)
                        {
                            unionpointer[w]=BEACON_BUFFER[r];
                        }
                        uint8_t checkA=0u;
                        uint8_t checkB=0u;
                        fletcher16(&checkA, &checkB, &sender_id, 1u);
                        fletcher16(&checkA, &checkB, &temp, 1u);
                        fletcher16(&checkA, &checkB, unionpointer, mesgsize);
                        if ( (checkA==received_checkA) && (checkB==received_checkB) )
                        {
                            BEACON_UNION_TYPE=message_id;
                            BEACON_UNION_SENDERID=sender_id;
                            BEACON_UNION_MESGCNTR=message_counter;
                        }
                    }
                }
            }
        }
        BEACON_BUFFER_USED=0;
    }
}

/////////////////////////////////////////////////////////////////////////////

void BEACON_HandleReceive(uint8_t chr)
{
    static bool_t last_chr_valid=false;
    static uint8_t last_chr=0;
    if(last_chr_valid==true)
    {
        if(chr==BEACON_JOKER)
        {
            if(last_chr==BEACON_JOKER)
            {
                BEACON_HandleReceive2(last_chr, false);//dual joker, use as data
                last_chr_valid=false;
            }
            else
            {
                BEACON_HandleReceive2(last_chr, false);
                last_chr=chr;
                last_chr_valid=true;
            }
        }
        else
        {
            if(last_chr==BEACON_JOKER)
            {
                BEACON_HandleReceive2(BEACON_JOKER, true);
                last_chr=chr;
                last_chr_valid=true;
            }
            else
            {
                BEACON_HandleReceive2(last_chr, false);
                last_chr=chr;
                last_chr_valid=true;
            }
        }
    }
    else
    {
        last_chr=chr;
        last_chr_valid=true;
    }
}

/////////////////////////////////////////////////////////////////////////////

void BEACON_PrintParsed(
    const BEACON_UNION_t & parsedstruct,
    uint8_t structtype,
    char buffer[],
    unsigned buffer_size
)
{
    int printpos=0;
    switch (structtype)
    {
        case 0u:
        {
            const BEACON_PING * data=(const BEACON_PING *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "age=%.3fs\n", data->age);
            printpos+=snprintf(&buffer[printpos], buffer_size, "quality=%.1f%%\n", data->quality);
        }
        break;
        case 1u:
        {
            const BEACON_SITUATION * data=(const BEACON_SITUATION *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "latitude =%10.6f\n", data->latitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "longitude=%10.6f\n", data->longitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "altitude_atl=%.0fm\n", data->altitude_atl-BEACON_ALTIOFFSET);
            printpos+=snprintf(&buffer[printpos], buffer_size, "altitude_msl=%.0fm\n", data->altitude_msl-BEACON_ALTIOFFSET);
            printpos+=snprintf(&buffer[printpos], buffer_size, "altitude_wgs=%.0fm\n", data->altitude_wgs-BEACON_ALTIOFFSET);
            printpos+=snprintf(&buffer[printpos], buffer_size, "airspeed=%.1fkm/h\n", data->airspeed/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "groundspeed=%.1fkm/h\n", data->groundspeed/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "climbrate=%.1fm/s\n", data->climbrate/10.0f);
        }
        break;
        case 2u:
        {
            const BEACON_ORIENTATION * data=(const BEACON_ORIENTATION *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "roll=%.1fdeg\n", data->roll/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "rollrate=%.1fdeg/s\n", data->rollrate/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "pitch=%.1fdeg\n", data->pitch/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "pitchrate=%.1fdeg/s\n", data->pitchrate/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "heading=%.1fdeg\n", data->heading/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "headrate=%.1fdeg/s\n", data->headrate/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "course=%.1fdeg\n", data->course/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "turnrate=%.1fdeg/s\n", data->turnrate/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "gravroll=%.1fdeg\n", data->gravroll/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "gfactor=%.1fG\n", data->gfactor/10.0f);
        }
        break;
        case 3u:
        {
            const BEACON_NAVIGATION * data=(const BEACON_NAVIGATION *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "auto=%u assisted=%u goinghome=%u loiter=%u parachuted=%u motor1on=%u motor2on=%u\n",
                               (data->mode&1u),
                               (data->mode&2u),
                               (data->mode&4u),
                               (data->mode&8u),
                               (data->mode&16u),
                               (data->mode&64u),
                               (data->mode&128u)
                              );
            printpos+=snprintf(&buffer[printpos], buffer_size, "ncycles=%u\n", data->ncycles);
            printpos+=snprintf(&buffer[printpos], buffer_size, "wptid=%u\n", data->wptid);
            printpos+=snprintf(&buffer[printpos], buffer_size, "wpt_latitude =%10.6f\n", data->wpt_latitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "wpt_longitude=%10.6f\n", data->wpt_longitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "wpt_altitude_msl=%.0fm\n", data->wpt_altitude_msl-BEACON_ALTIOFFSET);
        }
        break;
        case 4u:
        {
            const BEACON_PID * data=(const BEACON_PID *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "pidid=%u\n", data->pidid);
            printpos+=snprintf(&buffer[printpos], buffer_size, "target=%f\n", data->target);
            printpos+=snprintf(&buffer[printpos], buffer_size, "input=%f\n", data->input);
            printpos+=snprintf(&buffer[printpos], buffer_size, "pterm=%f\n", data->pterm);
            printpos+=snprintf(&buffer[printpos], buffer_size, "iterm=%f\n", data->iterm);
            printpos+=snprintf(&buffer[printpos], buffer_size, "dterm=%f\n", data->dterm);
            printpos+=snprintf(&buffer[printpos], buffer_size, "ffterm=%f\n", data->ffterm);
            printpos+=snprintf(&buffer[printpos], buffer_size, "output=%f\n", data->output);
        }
        break;
        case 5u:
        {
            const BEACON_METEO * data=(const BEACON_METEO *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "airpressure=%.2fPa\n", data->airpressure);
            printpos+=snprintf(&buffer[printpos], buffer_size, "altitude_std=%.0fm\n", data->altitude_std-BEACON_ALTIOFFSET);
            printpos+=snprintf(&buffer[printpos], buffer_size, "altitude_dens=%.0fm\n", data->altitude_dens-BEACON_ALTIOFFSET);
            printpos+=snprintf(&buffer[printpos], buffer_size, "altitude_atl=%.0fm\n", data->altitude_atl-BEACON_ALTIOFFSET);
            printpos+=snprintf(&buffer[printpos], buffer_size, "windheading=%udeg\n", data->windheading);
            printpos+=snprintf(&buffer[printpos], buffer_size, "windspeed=%ukm/h\n", data->windspeed);
            printpos+=snprintf(&buffer[printpos], buffer_size, "airtemp=%.2fC\n", data->airtemp/100.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "dewpoint=%.2fC\n", data->dewpoint/100.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "frostpoint=%.2fC\n", data->frostpoint/100.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "relhumidity=%u%%\n", data->relhumidity);
            printpos+=snprintf(&buffer[printpos], buffer_size, "altitude_cloud=%.0fm\n", data->altitude_cloud-BEACON_ALTIOFFSET);
            printpos+=snprintf(&buffer[printpos], buffer_size, "altitude_frost=%.0fm\n", data->altitude_frost-BEACON_ALTIOFFSET);
        }
        break;
        case 6u:
        {
            const BEACON_DATETIME * data=(const BEACON_DATETIME *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "time_format=%u\n", data->time_format);
            printpos+=snprintf(&buffer[printpos], buffer_size, "date=%04u-%02u-%02u\n", data->year, data->month, data->day);
            printpos+=snprintf(&buffer[printpos], buffer_size, "time=%02u:%02u:%02u.%03u\n", data->hour, data->minute, data->second, data->millisecond);
        }
        break;
        case 7u:
        {
            const BEACON_MISSIONSAFETY * data=(const BEACON_MISSIONSAFETY *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "missiontime=%.3fs\n", data->missiontime);
            printpos+=snprintf(&buffer[printpos], buffer_size, "time2nofuel=%ds\n", data->time2nofuel);//Is singed!
            printpos+=snprintf(&buffer[printpos], buffer_size, "time2takeoffpos=%us\n", data->time2takeoffpos);
            printpos+=snprintf(&buffer[printpos], buffer_size, "time2besthome=%us\n", data->time2besthome);
            printpos+=snprintf(&buffer[printpos], buffer_size, "time2ground=%us\n", data->time2ground);
            printpos+=snprintf(&buffer[printpos], buffer_size, "volt1=%.3fV\n", data->volt1/1000.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "volt2=%.3fV\n", data->volt2/1000.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "heading2takeoffpos=%.1fdeg\n", data->heading2takeoffpos/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "heading2besthome=%.1fdeg\n", data->heading2besthome/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "dist2takeoffpos=%.3fkm\n", data->dist2takeoffpos);
            printpos+=snprintf(&buffer[printpos], buffer_size, "dist2besthome=%.3fkm\n", data->dist2besthome);
        }
        break;
        case 8u:
        {
            const BEACON_PERFORMANCE * data=(const BEACON_PERFORMANCE *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "distflown=%.3fkm\n", data->distflown);
            printpos+=snprintf(&buffer[printpos], buffer_size, "climbrate=%.1fm/s\n", data->climbrate/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "total_energy_avg=%.1f\n", data->total_energy_avg);
            printpos+=snprintf(&buffer[printpos], buffer_size, "total_energy=%.1f\n", data->total_energy);
            printpos+=snprintf(&buffer[printpos], buffer_size, "total_watt=%.1f\n", data->total_watt/10.0f);
            /*
            printpos+=snprintf(&buffer[printpos], buffer_size, "glideratio=%d\n", data->glideratio);
            printpos+=snprintf(&buffer[printpos], buffer_size, "glideratio2takeoffpos=%d\n", data->glideratio2takeoffpos);
            printpos+=snprintf(&buffer[printpos], buffer_size, "glideratio2besthome=%d\n", data->glideratio2besthome);
            */
        }
        break;
        case 9u:
        {
            const BEACON_ENGINE * data=(const BEACON_ENGINE *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "engineid=%u\n", data->engineid);
            printpos+=snprintf(&buffer[printpos], buffer_size, "proprpm=%uRPM\n", data->proprpm);
            printpos+=snprintf(&buffer[printpos], buffer_size, "engrpm=%uRPM\n", data->engrpm);
            printpos+=snprintf(&buffer[printpos], buffer_size, "watt=%.1fW\n", data->watt/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "amp=%.2fA\n", data->amp/100.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "volt=%.3fV\n", data->volt/1000.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "ahconsumed=%.3fAh\n", data->ahconsumed);
        }
        break;
        case 10u:
        {
            const BEACON_FORMATION * data=(const BEACON_FORMATION *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "mode=%u\n", data->mode);

            printpos+=snprintf(&buffer[printpos], buffer_size, "act_latitude =%10.6f\n", data->act_latitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "act_longitude=%10.6f\n", data->act_longitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "act_altitude_msl=%.0fm\n", data->act_altitude_msl-BEACON_ALTIOFFSET);
            printpos+=snprintf(&buffer[printpos], buffer_size, "act_course=%.1f\n", data->act_course/10.0f);
            printpos+=snprintf(&buffer[printpos], buffer_size, "act_groundspeed=%.1f\n", data->act_groundspeed/10.0f);

            printpos+=snprintf(&buffer[printpos], buffer_size, "src_latitude =%10.6f\n", data->src_latitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "src_longitude=%10.6f\n", data->src_longitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "src_altitude_msl=%.0fm\n", data->src_altitude_msl-BEACON_ALTIOFFSET);

            printpos+=snprintf(&buffer[printpos], buffer_size, "dest_latitude =%10.6f\n", data->dest_latitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "dest_longitude=%10.6f\n", data->dest_longitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "dest_altitude_msl=%.0fm\n", data->dest_altitude_msl-BEACON_ALTIOFFSET);
        }
        break;
        case 11u:
        {
            const BEACON_STABHEAD * data=(const BEACON_STABHEAD *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "stabid=%u\n", data->stabid);
            printpos+=snprintf(&buffer[printpos], buffer_size, "mode=%u\n", data->mode);
            printpos+=snprintf(&buffer[printpos], buffer_size, "lookat_latitude =%10.6f\n", data->lookat_latitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "lookat_longitude=%10.6f\n", data->lookat_longitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "lookat_altitude_msl=%.0fm\n", data->lookat_altitude_msl-BEACON_ALTIOFFSET);
        }
        break;
        case 12u:
        {
            const BEACON_IMPACT * data=(const BEACON_IMPACT *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "impact_type=%u\n", data->impact_type);
            printpos+=snprintf(&buffer[printpos], buffer_size, "impact_latitude =%10.6f\n", data->impact_latitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "impact_longitude=%10.6f\n", data->impact_longitude);
            printpos+=snprintf(&buffer[printpos], buffer_size, "impact_altitude_msl=%.0fm\n", data->impact_altitude_msl-BEACON_ALTIOFFSET);
            printpos+=snprintf(&buffer[printpos], buffer_size, "time2ground=%.3fs\n", data->time2ground);
        }
        break;
        case 13u:
        {
            const BEACON_RECEPTION * data=(const BEACON_RECEPTION *)&parsedstruct;
            printpos+=snprintf(&buffer[printpos], buffer_size, "gps_mode=%u\n", data->gps_mode);
            printpos+=snprintf(&buffer[printpos], buffer_size, "gps_nsats_view=%u\n", data->gps_nsats_view);
            printpos+=snprintf(&buffer[printpos], buffer_size, "gps_nsats_used=%u\n", data->gps_nsats_used);
            printpos+=snprintf(&buffer[printpos], buffer_size, "gps_pdop=%.1fm\n", data->gps_pdop/10.0);
            printpos+=snprintf(&buffer[printpos], buffer_size, "gps_age=%.3fs\n", data->gps_age);
            printpos+=snprintf(&buffer[printpos], buffer_size, "rc_rssi=%u\n", data->rc_rssi);
            printpos+=snprintf(&buffer[printpos], buffer_size, "modem_rssi=%u\n", data->modem_rssi);
            for(unsigned s=0; s<12; s++)
            {
                printpos+=snprintf(&buffer[printpos], buffer_size, "servo%u=%+d%%\n", s+1, data->servo[s]);
            }
        }
        break;
    };
}

/////////////////////////////////////////////////////////////////////////////

#endif //__INCLUDED_BEACON_PARSER
