#ifndef _NETSIM_H_
#define _NETSIM_H_

#include <stdlib.h>  // for size_t

/** You are to fill in details to describe what you would do on a packet receipt.
 * The data buffer will not survive past the function call's return;
 * if you want to keep it or any part of it, you'll need to make a copy
 * (e.g., with malloc and memcpy).
 */
void recvd(size_t len, void* data);

/** Use to send messages to the server.
 * The first byte must be a checksum.
 */
void send(size_t len, const void* _data);

/**
    You may invoke this to ask to have `callback` invoked after `ms` milliseconds (`ms * 0.001` seconds).

    On success, returns a positive identifier that may be used in `clearTimeout`.
    On failure, returns a special error code:
    
    - `0` if `callback` is `NULL`
    - `-1` if `ms` is too far in the future (more than a minute)
    - `-2` if you have too many pending callbacks scheduled (based on an implementation-defined limit guaranteed to be at least 16)
 */    
int setTimeout(void (*callback)(void *), unsigned long ms, void *arg);

/**
    You may invoke this to ask to have `callback` invoked after `ms` milliseconds (`ms * 0.001` seconds).

    On success, returns a positive identifier that may be used in `clearTimeout`.
    On failure, returns a special error code:
    
    - `0` if `callback` is `NULL`
    - `-1` if `ms` is too far in the future (more than a minute)
    - `-2` if you have too many pending callbacks scheduled (based on an implementation-defined limit guaranteed to be at least 16)
 */    
int setTimeout(void (*callback)(void *), unsigned long ms, void *arg);

/**
    If `timeoutID` was returned by `setTimeout` and the callback has not yet been invoked,
    unschedules that callback and returns its argument.
    Otherwise, does nothing and returns NULL.
 */
void *clearTimeout(int timeoutID);

/**
    If there are no pending timeouts, returns immediately.
    Otherwise, blocks until there are not scheduled timeouts left.
  */
void waitForAllTimeoutsAndMessagesThenExit(void);

#endif
