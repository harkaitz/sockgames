#define _POSIX_C_SOURCE 200809L /* strtok_r */
#include "sockgames.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "aux/foreach.h"

/* ----------------------------------------------------------------
 * ---- TABLE ----------------------------------------------------
 * ---------------------------------------------------------------- */

void
sg_table_init(sg_table_t *_table)
{
	for (int y=0; y<SG_TABLE_MAX_Y; y++) {
		for (int x=0; x<SG_TABLE_MAX_X; x++) {
			_table->world[y][x] = ' ';
		}
	}
	_table->x_len = 0;
	_table->y_len = 0;
}

void
sg_table_clean(sg_table_t *_table __attribute__((unused)))
{}

int
sg_table_print(sg_table_t *_table, char *_buffer, size_t _len)
{
	int pos = 0;
	size_t rlen = _table->y_len*(_table->x_len+4)+1;
	
	if (_len>rlen) {
		return rlen;
	}
	for (int y=0; y<_table->y_len; y++) {
		_buffer[pos++]='|';
		for (int x=0; x<_table->x_len; x++) {
			_buffer[pos++]=_table->world[y][x];
		}
		_buffer[pos++]='|';
		_buffer[pos++]='\r';
		_buffer[pos++]='\n';
	}
	return pos;
}

err_t
sg_table_read_b(sg_table_t *_table, char _world_map[])
{
	int stage = 0;
	
	for (char *lb,*l = strtok_r(_world_map, "\n", &lb); l; l = strtok_r(NULL, "\n", &lb)) {
		if(l[0]=='|' && stage != 2) {
			stage = 1;
			int x = 0;
			for(char *chr = l+1; (*chr) && (*chr != '|'); chr++) {
				_table->world[_table->y_len][x++] = *chr;
			}
			if(x > _table->x_len) {
				_table->x_len = x;
			}
			_table->y_len++;
		} else if(stage == 0) {
			_table->h[_table->h_count++] = l;
		} else if(stage == 1) {
			_table->b[_table->b_count++] = l;
			stage = 2;
		} else if(stage == 2) {
			_table->b[_table->b_count++] = l;
		}
		if(_table->b_count==SG_TABLE_MAX_HEADERS) {
			return perr(-1, "Too much headers.");
		}
	}
	
	return 0;
}

err_t
sg_table_refresh(sg_table_t *_table, pbuffer_t *_out, int _side, int _human, sg_table_map_t _opt_map[])
{
	/* Write headers.  */
	for (int i=0; i<_table->h_count; i++) {
		if(_table->b[i][0]!='#') {
			pbuffer_printf(
			    _out, 0,
			    "%s%s\n",
			    (_human)?"   ":"",
			    _table->h[i]
			);
		}
	}
	
	/* Write table. */
	char *chr; size_t len; size_t i = 0;
	pbuffer_position(_out, &chr, &len);
	
	for (int y=0; y<(_table->y_len); y++) {
		if(_human) {
			chr[i++] = ' '; if (i>=len) return -1;
			chr[i++] = ' '; if (i>=len) return -1;
			chr[i++] = ' '; if (i>=len) return -1;
		}
		chr[i++] = '|'; if (i>=len) return -1;
		
		for (int x=0; x<_table->x_len; x++) {
			sg_table_map_t *map = NULL;
			for (sg_table_map_t *m=_opt_map; _human && _opt_map && m->fr; m++) {
				if (_table->world[y][x]==m->fr) {
					map = m;
					break;
				}
			}
			if (map) {
				for (const char *c = (map->to); *c; c++) {
					chr[i++]=*c;
					if (i>=len) return -1;
				}
			} else {
				chr[i++]=_table->world[y][x];
				if (i>=len) return -1;
			}
			if (i>=len) {
				return -1;
			}
		}
		chr[i++]='|';  if (i>=len) return -1;
		chr[i++]='\n'; if (i>=len) return -1;
	}
	_out->pos+=i;
	
	/* Write botton. */
	for(int i=0; i<_table->b_count; i++) {
		if(_table->b[i][0]!='#') {
			pbuffer_printf(
			    _out, 0,
			    "%s%s\n",
			    _human ? "   " : "",
			    _table->b[i]
			);
		}
	}
	
	return i;
}

err_t
sg_table_get_number(sg_table_t *_table, char const _prefix[], long *_number)
{
	err_t e;
	char const *string = NULL;
	e = sg_table_get_string(_table, _prefix, &string);
	if(e<0/*err*/) { return e; }
	*_number = atol(string);
	return 0;
}

err_t
sg_table_get_string(sg_table_t *_table, char const _prefix[],char const **_string)
{
	size_t len = strlen(_prefix);
	for (int i=0; i<_table->b_count; i++) {
		if (_table->b[i][0]!='#') {
			continue;
		}
		if(strncasecmp(_table->b[i]+1, _prefix, len)) {
			continue;
		}
		char const *value = strchr(_table->b[i]+1+len, ':');
		if(value == NULL) {
			continue;
		}
		value++;
		*_string = value;
		return 0;
	}
	return perr(-1, "Prefix `%s` not found.", prefix);
}

/* ----------------------------------------------------------------
 * ---- COORDINATES -----------------------------------------------
 * ---------------------------------------------------------------- */
sg_coor_t
sg_coor(int _x,int _y)
{
	sg_coor_t c = {_x,_y};
	return c;
}

sg_coor_t
sg_coor_near(sg_coor_t _coor, sg_coor_t _diff)
{
	sg_coor_t c = _coor;
	c.x += _diff.x; c.y += _diff.y;
	return c;
}

bool
sg_coor_is_valid(sg_coor_t _coor, const sg_table_t *_opt_table)
{
	if (_coor.x < 0 || _coor.y < 0) {
		return 0;
	}
	if (_opt_table) {
		if (_coor.x >= _opt_table->x_len) return 0;
		if (_coor.y >= _opt_table->y_len) return 0;
	}
	return 1;
}

bool
sg_table_search(const sg_table_t *_table, unsigned char _byte, sg_coor_t *_out_pos)
{
	int x,y;
	for (y=0; y<_table->y_len; y++) {
		for (x=0; x<_table->x_len; x++) {
			if (_byte == _table->world[y][x]) {
				_out_pos->x = x;
				_out_pos->y = y;
				return 1;
			}
		}
	}
	return 0;
}

unsigned char
sg_table_get(const sg_table_t *_table, sg_coor_t _pos, unsigned char _limit)
{
	if (!sg_coor_is_valid(_pos,_table)) {
		return _limit;
	}
	return _table->world[_pos.y][_pos.x];
}

void
sg_table_set(sg_table_t *_table, sg_coor_t _pos, unsigned char _chr)
{
	if (sg_coor_is_valid(_pos, _table)) {
		_table->world[_pos.y][_pos.x] = _chr;
	}
}

sg_coor_t
sg_coor_random(const sg_table_t *table)
{
	int x = rand() % table->x_len;
	int y = rand() % table->y_len;
	return sg_coor(x, y);
}
