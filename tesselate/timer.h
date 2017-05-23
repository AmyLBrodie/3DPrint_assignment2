#ifndef TimerC
#define TimerC
/**
 * @file
 *
 * Fairly accurate timing routines.
*/

#include <sys/time.h>

/**
 * Timer with millisecond accuracy based on gettimeofday calls
 */
class Timer
{

private:
    struct timeval tbegin;  ///< starting time in unix timeofday format
    struct timeval tend;    ///< stopping time in unix timeofday format
    struct timezone zone;   ///< time zone required for gettimeofday

public:

    /// Start timing
    void start();

    /**
     * Stop timing
     * @pre Expects a previous call to start for this timer
     */
    void stop();

    /**
     * Get the elapsed time in seconds between calls to @a start and @a stop
     * @returns elapsed time in seconds
     */
    float peek();
};

#endif