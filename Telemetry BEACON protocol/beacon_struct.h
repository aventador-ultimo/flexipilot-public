#ifndef __INCLUDED_BEACON_STRUCT
#define __INCLUDED_BEACON_STRUCT

#pragma pack(1)

/**
* AGL=Above Ground Level, approximated by ATL for flat areas
* ATL=Above Takeoff Point, barometric
* MSL=GPS above Mean Sea Level using geoid
* Add 20000 to all altitudes before sending, substract when receiving.
* Altitude range -20000m...+45535m
* ATL may start high during airdrop
*
* Units, unless otherwise specified:
* Speeds (incl. windspeeds) in km/h
* Climbrates in m/s
* Altitudes by default in m above mean Sea Level GPS WGS84 geoid model, shifted by 20000
* Floating point altitudes without offset.
* Pressures in Pascals
* Latitude/Longitude in floating point decimal format
* Angles in degrees
* Angular rates degrees/s
* Duration in s, sometimes is negative if estimated event has occured (u16 is 18h+, float unlimited)
* Distances in km, unlimited precision
* Temperatures in Celsius
*/

static const float32_t BEACON_ALTIOFFSET=20000.0f;
const uint8_t BEACON_JOKER=0xB9u;//only used by BEACON2
const uint8_t BEACON_UNION_TYPE_INVALID=0xFFu;
uint8_t BEACON_UNION_TYPE=BEACON_UNION_TYPE_INVALID;
uint8_t BEACON_UNION_SENDERID=0u;
uint8_t BEACON_UNION_MESGCNTR=0u;

/////////////////////////////////////////////////////////////////////////////

/**
* ID=0
*/
struct BEACON_PING
{
    float32_t age;//s since last received ack
    float32_t quality;//0-100%
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=1
*/
struct BEACON_SITUATION
{
    float32_t latitude;//decimal representation, must display 6 dec. places
    float32_t longitude;//decimal representatio, must display 6 dec. places
    uint16_t altitude_atl;//m
    uint16_t altitude_msl;//m
    uint16_t altitude_wgs;//m
    uint16_t airspeed;//in 0.1 km/h, 65000 is 6500km/h
    uint16_t groundspeed;//in 0.1 km/h, 65000 is 6500 km/h
    int16_t climbrate;//in 0.1m/s, -32000 is -3200 m/s
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=2
*/
struct BEACON_ORIENTATION
{
    int16_t roll;//[angle*10] -180..180     //display must accept both -90..90 and -180..180
    int16_t rollrate;//[rate*10] -500..500 deg/s
    int16_t pitch;//[angle*10] -180..180    //display must accept both -90..90 and -180..180
    int16_t pitchrate;//[rate*10] -500..500 deg/s
    uint16_t heading;//[angle*10] 0..359
    int16_t headrate;//[rate*10] -500..500 deg/s
    uint16_t course;//[angle*10] 0..359     //course over ground
    int16_t turnrate;//[rate*10]-500..500 deg/s,                     includes wind influence
    int16_t gravroll;//[angle*10] -180..180     //display must accept both -90..90 and -180..180
    int8_t gfactor;//[Gs*10] -12.8..12.7G, in -120 is -12G,  10 is 1G
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=3
* wptid is not the number in cycle,
*/
struct BEACON_NAVIGATION
{
    /**
    * bit0: autopilot on
    * bit1: assisted mode
    * bit2: going home
    * bit3: loiter
    * bit4: parachute deployed
    * bit5: motor1 enabled
    * bit6: motor2 enabled
    * bit7: motor1 running
    * bit8: motor2 running
    * bit9: mission done
    */
    uint16_t mode;
    uint16_t ncycles;//[0..max] decrementing counter for flight patterns or remaining mission duration
    uint16_t wptid;//0-4095 to be supported initially, bits 13|14|15 reserved
    float32_t wpt_latitude;//[decimal] destination, decimal representation, must display 6 dec. places
    float32_t wpt_longitude;//[decimal] destination, decimal representation, must display 6 dec. places
    uint16_t wpt_altitude_msl;//m
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=4
* This PID can represent any type of feedback loop.
* Proposed PID IDs:
*  1 - RUDD2TURN
*  2 - TURN2HEAD
*  3 - THR2ALTI
*  4 - THRB2ALTI
*  5 - RUDD2ROLL
*  6 - ROLL2HEAD
*  7 - ELEV2ALTI
*  8 - ELEV2PITCH
*  9 - PITCH2ALTI
* 10 - ELEV2CLIMB
* 11 - CLIMB2ALTI
*/
struct BEACON_PID
{
    uint8_t pidid;//typically up to 3-4 PIDs at once, only one (rarely two) to be displayed
    float32_t target; //desired value
    float32_t input;  //actual value
    float32_t pterm;  //evaluated proportional term
    float32_t iterm;  //evaluated integral term
    float32_t dterm;  //evaluated differential term
    float32_t ffterm; //evaluated feed forward term
    float32_t output; //after saturation, slewrate etc
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=5
*/
struct BEACON_METEO
{
    float32_t airpressure;   //pascals, around 98000
    uint16_t altitude_std;   //[m] ISA pressure altitude
    uint16_t altitude_dens;  //[m] density altitude
    uint16_t altitude_atl;   //[m+20000] barometric calibrated altitude
    uint16_t windheading;    //0..359
    uint8_t windspeed;       //0..255 km/h
    int16_t airtemp;         //0.01C -274C..320C
    int16_t dewpoint;        //0.01C -274C..320C
    int16_t frostpoint;      //0.01C -274C..320C
    uint8_t relhumidity;     //0-100%
    uint16_t altitude_cloud; //[m] using ISA
    uint16_t altitude_frost; //[m] using ISA
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=6
*/
struct BEACON_DATETIME
{
    uint8_t time_format;//0=UTC

    uint16_t year;
    uint8_t month;
    uint8_t day;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    uint16_t millisecond;
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=7
*/
struct BEACON_MISSIONSAFETY
{
    float32_t missiontime;//s
    int16_t time2nofuel;//s might be negative in estimation
    uint16_t time2takeoffpos;//s
    uint16_t time2besthome;//s
    uint16_t time2ground;//s
    uint16_t volt1;//V*1000, 0..65.535
    uint16_t volt2;//V*1000, 0..65.535
    uint16_t heading2takeoffpos;//[deg*10]
    uint16_t heading2besthome;//[deg*10]
    float32_t dist2takeoffpos;//km
    float32_t dist2besthome;//km
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=8
*/
struct BEACON_PERFORMANCE
{
    float32_t distflown;//km
    int16_t climbrate;//m/s*10, -3200...3200
    float32_t total_energy_avg;//Joules/mass
    float32_t total_energy;//Joules/mass
    uint16_t total_watt;//Watt*10
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=9
*/
struct BEACON_ENGINE
{
    uint8_t engineid;//1 or 2
    uint16_t proprpm;//revolutions/min including windmilling
    uint16_t engrpm;//revolutions/min induced by ESC or measured at prop
    uint16_t watt;//[W*10], max is 65535=6.5355KW
    uint16_t amp;//[W*100], max is 65535=655.35A
    uint16_t volt;//[V*1000] 0..65.535 //note: voltage=watt/amp, but must be known also when no current flows
    float32_t ahconsumed;//Ah consumed so far
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=10
* wptid is not the number in cycle,
* note: current altitude target for PID is a different thing from 'actual altitude'
*/
struct BEACON_FORMATION
{
    uint16_t mode;//bit0: autopilot on, bit1: rethome, bit2: aborted.landed, bit3:takeoff, other reserved

    float32_t act_latitude;//[decimal] actual, decimal representation, must display 6 dec. places
    float32_t act_longitude;//[decimal] actual, decimal representation, must display 6 dec. places
    uint16_t act_altitude_msl;//[m]
    uint16_t act_course;//[deg*10] 0..359     //display must accept 360, includes wind influence
    uint16_t act_groundspeed;//in 0.1 km/h, 65000 is 6500km/h

    float32_t src_latitude;//[decimal] source, decimal representation, must display 6 dec. places
    float32_t src_longitude;//[decimal] source, decimal representation, must display 6 dec. places
    uint16_t src_altitude_msl;//[m] source

    float32_t dest_latitude;//[decimal] destination, decimal representation, must display 6 dec. places
    float32_t dest_longitude;//[decimal] destination, decimal representation, must display 6 dec. places
    uint16_t dest_altitude_msl;//[m] source
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=11
* Lookat says where the stabilised head wants to look.
*/
struct BEACON_STABHEAD
{
    uint8_t stabid;//1 or 2
    uint8_t mode;
    float32_t lookat_latitude;//[decimal] actual, decimal representation, must display 6 dec. places
    float32_t lookat_longitude;//[decimal] actual, decimal representation, must display 6 dec. places
    float32_t lookat_altitude_msl;//[m] actual
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=12
* Projected impact position:
* 0 - extrapolated glide path
* 1 - after parachute deployment
* 2 - payload delivery
*/
struct BEACON_IMPACT
{
    uint8_t impact_type;//0-2
    float32_t impact_latitude;//[decimal] estimated, decimal representation, must display 6 dec. places
    float32_t impact_longitude;//[decimal] estimated, decimal representation, must display 6 dec. places
    uint16_t impact_altitude_msl;//[m]
    float32_t time2ground;//[s]
};

/////////////////////////////////////////////////////////////////////////////

/**
* ID=13
* Status of receivers
*/
struct BEACON_RECEPTION
{
    ///bit4 GPGGA dgps info (0-1) 0-off(GPGGA<=1), 1-on(GPGGA>=2)
    ///bit3 GPGSA fix3d selector (0-1) 0-man, 1-auto
    ///bit2 GPRMC fix available (0-1) 0-Void 1-Active
    ///bit1,0 GPGSA fix type (1-3) 1-notav 2=2D 3=3D
    uint8_t gps_mode;
    uint8_t gps_nsats_view;//0-16
    uint8_t gps_nsats_used;//0-16
    uint8_t gps_pdop;//0-25.5
    float32_t gps_age;//time to last good measurement

    uint8_t rc_rssi;//0-255
    uint8_t modem_rssi;//0-255

    int8_t servo[12u];//-125..125%
};

/////////////////////////////////////////////////////////////////////////////

union BEACON_UNION_t
{
    BEACON_PING ping;
    BEACON_SITUATION situation;
    BEACON_ORIENTATION orientation;
    BEACON_NAVIGATION navigation;
    BEACON_PID pid;
    BEACON_METEO meteo;
    BEACON_DATETIME datetime;
    BEACON_MISSIONSAFETY missionsafety;
    BEACON_PERFORMANCE performance;
    BEACON_ENGINE engine;
    BEACON_FORMATION formation;
    BEACON_STABHEAD stabhead;
    BEACON_IMPACT impact;
    BEACON_RECEPTION reception;
    BEACON_UNION_t();
};

BEACON_UNION_t::BEACON_UNION_t()
{
    (void)memset(this, 0, sizeof(*this));
}

static BEACON_UNION_t BEACON_UNION;

/////////////////////////////////////////////////////////////////////////////

#endif //__INCLUDED_BEACON_STRUCT

