/*
 * timer.c
 *
 * Created: 7/20/2019 2:08:27 AM
 *  Author: hamster
 */ 

#include <atmel_start.h>
#include <timer.h>

// Second/millisecond time since system boot.
static uint16_t timeElapsed = 0;


/**
 * Get the time elapsed compared to a time stamp
 */
void timeElapsedSinceTS(uint16_t *ts, uint16_t *diff){
	TIME_STRUCT now;

	timeGetElapsed(&now);
	// Check for rollover:
	if (ts->MILLISECONDS > 999){
		ts->MILLISECONDS = 999; // shouldn't happen...
	}
	if (((now.SECONDS == ts->SECONDS) && (now.MILLISECONDS < ts->MILLISECONDS)) || now.SECONDS < ts->SECONDS){
		// then there was a rollover: 'now' is less than 'then' (the value in ts)
		// find the time from 'then' to the rollover time at 0xFFFFFFFF seconds and 999 msec
		// add that to the 'now' current time.
		diff->SECONDS = now.SECONDS + (0xFFFFFFFF - ts->SECONDS);
		diff->MILLISECONDS = now.MILLISECONDS + (1000 - ts->MILLISECONDS);
		if (diff->MILLISECONDS > 999){ // msec rolled over...
			diff->MILLISECONDS -= 1000;
			diff->SECONDS += 1;
		}
	}
	else{
		if (now.MILLISECONDS < ts->MILLISECONDS){
			// borrow a second...
			diff->MILLISECONDS = now.MILLISECONDS + (1000 - ts->MILLISECONDS);
			diff->SECONDS = now.SECONDS - ts->SECONDS - 1;
		}
		else{
			diff->MILLISECONDS = now.MILLISECONDS - ts->MILLISECONDS;
			diff->SECONDS = now.SECONDS - ts->SECONDS;
		}
	}
}

/**
 * Determine if timeout milliseconds have passed since the timestamp
 */
bool timeIsTimeout(uint16_t *ts, uint16_t timeout){
	uint16_t diff;

	timeElapsedSinceTS(ts, &diff);
	return (diff.MILLISECONDS >= timeout);
}

/**
 * Determine if timeout milliseconds has not passed since timestamp
 */
bool timeIsNotTimeout(uint16_t *ts, uint16_t timeout){
	return(!timeIsTimeout(ts, timeout));
}


/**
 * Add a 1ms tick to the system time
 */
void timeTick1ms(void){
		timeElapsed++;
}

/**
 * Get the difference between two timestamps
 */
void timeDiff(TIME_STRUCT *oldTime, TIME_STRUCT *newTime, TIME_STRUCT *diff){
	if(newTime->MILLISECONDS < oldTime->MILLISECONDS){
		// Do we need to borrow from SECONDS?
		diff->MILLISECONDS = newTime->MILLISECONDS + 1000 - oldTime->MILLISECONDS;
		diff->SECONDS      = newTime->SECONDS      -    1 - oldTime->SECONDS;
	}
	else{
		// No need to borrow. Straight forward subtraction.
		diff->MILLISECONDS = newTime->MILLISECONDS        - oldTime->MILLISECONDS;
		diff->SECONDS      = newTime->SECONDS             - oldTime->SECONDS;
	}
}

/**
 * Get the system time
 */
void timeGetElapsed(uint16_t *ts){
	*ts = timeElapsed;
}