#include "sockgames.h"

err_t
sg_clock_init(sg_clock_t *_clock, unsigned long _min_period, unsigned long _period)
{
	_clock->last_update  = 0;
	_clock->period       = _period;
	_clock->min_period   = _min_period;
	_clock->ticks        = 0;
	return 0;
}

void
sg_clock_clean(sg_clock_t *_clock __attribute__((unused))) {}

err_t
sg_clock_set_2x(sg_clock_t *_clock)
{
	unsigned long new_period = _clock->period/2;
	if(new_period<_clock->min_period) return perr(-1,"Period too small.");
	_clock->period = new_period;
	return 0;
}

void
sg_clock_update(sg_clock_t *_clock, unsigned long _ctime)
{
	if(_clock->last_update==0) {
		_clock->last_update = _ctime;
	} else {
		unsigned long passed_time = (_ctime - _clock->last_update);
		int ticks = passed_time / _clock->period;
		if(ticks) {
			_clock->last_update += ticks * _clock->period;
			_clock->ticks       += ticks;
		}
	}
}

bool
sg_clock_get_tick(sg_clock_t *_clock)
{
	if(_clock->ticks) {
		_clock->ticks--;
		return 1;
	} else {
		return 0;
	}
}

void
sg_clock_refresh (sg_clock_t *_clock, pbuffer_t *_buffer, int _human)
{
	pbuffer_printf(
	    _buffer,
	    0,
	    "%sTime    : %lims" "\n",
	    (_human)?"   ":"#",
	    _clock->last_update/1000
	);
}
