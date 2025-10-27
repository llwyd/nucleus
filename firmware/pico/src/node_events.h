#ifndef NODE_EVENTS_H_
#define NODE_EVENTS_H_

#include "state.h"

#define EVENTS(EVNT) \
    EVNT( Tick ) \
    EVNT( ReadSensor ) \
    EVNT( WifiCheckStatus ) \
    EVNT( WifiConnected ) \
    EVNT( WifiDisconnected ) \
    EVNT( TCPRetryConnect ) \
    EVNT( RetryCounterIncrement ) \
    EVNT( TCPConnected ) \
    EVNT( TCPDisconnected ) \
    EVNT( MessageReceived ) \
    EVNT( MQTTRetryConnect ) \
    EVNT( AccelDataReady ) \
    EVNT( AccelMotion ) \
    EVNT( DNSReceived ) \
    EVNT( DNSRetryRequest ) \
    EVNT( NTPReceived ) \
    EVNT( UDPReceived ) \
    EVNT( NTPRetryRequest ) \
    EVNT( TCPReceived ) \
    EVNT( AlarmElapsed ) \
    EVNT( HandleCommand ) \
    EVNT( TCPRetryClose ) \
    EVNT( AckReceived ) \
    EVNT( AckTimeout ) \
    EVNT( PCBInvalid ) \
    EVNT( GPIOAEvent ) \
    EVNT( GPIOBEvent ) \
    EVNT( PQResend ) \
    EVNT( UptimeRequest ) \
    EVNT( Resync ) \

GENERATE_EVENTS( EVENTS );

#endif /* NODE_EVENTS_H_ */
