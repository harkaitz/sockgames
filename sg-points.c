#include "sockgames.h"

void
sg_points_init(sg_points_t _points[], int _count)
{
	for (int i=0; i<_count; i++) {
		_points[i].number = 0;
	}
}

void
sg_points_clean (sg_points_t _points[], int _count)
{}

void
sg_points_read_table (sg_points_t _points[], int _count, sg_table_t *_table)
{
	char prefix[32];
	for (int i=0; i<_count; i++) {
		snprintf(prefix, sizeof(prefix), "Points %c", sg_player_character(i));
		sg_table_get_number(_table, prefix, &_points[i].number);
	}
}

void
sg_points_refresh (sg_points_t _points[], int _count, pbuffer_t *_buffer, int _human)
{
	for (int i=0; i<_count; i++) {
		pbuffer_printf(
		    _buffer,
		    0,
		    "%sPoints %c: %li" "\n",
		    (_human)?"   ":"#",
		    sg_player_character(i),
		    _points[i].number
		);
	}
}

