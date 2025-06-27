#ifndef WINDOWSONLY_H
#define WINDOWSONLY_H

#ifdef _WIN32

    #include <iostream>
    #include <windows.h>
    #include <mmsystem.h>

    #include "Util.h"

    inline void EnableHighResolutionSleeps()
    {
        ASSERT_LOG(
            timeBeginPeriod(1) == TIMERR_NOERROR,
            "Failed to set resolution of timer. Sleeps will be inaccurate. Expect jitteryness."
        );
    }

    inline void DisableHighResolutionSleeps()
    {
        timeEndPeriod(1); //We don't really care if this calls fails, windows will clean up everything for us anyway
    }
#else
    inline void EnableHighResolutionSleeps()
    {

    }

    inline void DisableHighResolutionSleeps()
    {

    }
#endif

#endif //WINDOWSONLY_H
