#define _POSIX_C_SOURCE 200809L
#include "sockgames.h"
#include "aux/pterm.h"
#include "aux/foreach.h"

static
sg_table_map_t _sg_snake_map[] = {
	{'#' ,ANSI_TEXT(ANSI_BG_GREY_1()   " ")},
	{'X' ,ANSI_TEXT(ANSI_FG_RED_1()    "X")},
	{'1' ,ANSI_TEXT(ANSI_FG_YELLOW_1() "o")},
	{'>' ,ANSI_TEXT(ANSI_FG_YELLOW_1() "-")},
	{'^' ,ANSI_TEXT(ANSI_FG_YELLOW_1() ":")},
	{'<' ,ANSI_TEXT(ANSI_FG_YELLOW_1() "-")},
	{'V' ,ANSI_TEXT(ANSI_FG_YELLOW_1() ":")},
	{'\0',""}
};

err_t
sg_snake_init(sg_snake_t *_snake)
{
	memset(_snake, 0, sizeof(sg_snake_t));
	
	sg_table_init (&_snake->table);
	sg_clock_init (&_snake->clock, SG_SNAKE_GAME_IMPL.us_rate, 100000);
	
	sg_points_init(_snake->points, SG_SNAKE_MAX_PLAYERS);
	memset(_snake->moves, SG_MOVE_ARROW_NOTHING, sizeof(_snake->moves));
	memset(_snake->has_head,0, sizeof(_snake->has_head));
	return 0;
}

void
sg_snake_clean(sg_snake_t *_snake)
{
	sg_points_clean(_snake->points, SG_SNAKE_MAX_PLAYERS);
	sg_clock_clean(&_snake->clock);
	sg_table_clean(&_snake->table);
}

int
sg_snake_print (sg_snake_t *_snake, char *_buffer, size_t _len)
{
	return sg_table_print(&_snake->table, _buffer, _len);
}

static err_t
sg_snake_update_tail(sg_snake_t *_snake, sg_coor_t _coor,int _length)
{
	const struct {
		unsigned char chr;
		sg_coor_t ptr;
	} surrounds[] = {
		{'V' ,{ 0,-1}},
		{'^' ,{ 0, 1}},
		{'>' ,{-1, 0}},
		{'<' ,{ 1, 0}},
		{'\0',{ 0, 0}},
	};
	for (int i=0; surrounds[i].chr; i++) {
		sg_coor_t npos = sg_coor_near(_coor, surrounds[i].ptr);
		unsigned char    nchr = sg_table_get(&_snake->table, npos, '#');
		if(surrounds[i].chr==nchr) {
			if(_length<0) {
				sg_table_set(&_snake->table, npos, ' ');
			}
			return sg_snake_update_tail(_snake, npos, _length-1);
		}
	}
	return 0;
}


static err_t
_sg_snake_apply(void *_sg_game, int _player_count)
{
	sg_snake_t *_snake = _sg_game;
	for (int p=0; p<_player_count; p++) {
		
		/* Skip players without heads. */
		if (!_snake->has_head[p]) {
			continue;
		}
		sg_coor_t *head = &_snake->head[p];
		
		/* Get next position. */
		sg_coor_t diff = {-1,-1};
		unsigned char char_repl;
		switch (_snake->moves[p]) {
		case SG_MOVE_ARROW_UP:    diff = SG_COOR_UP();    char_repl = '^'; break;
		case SG_MOVE_ARROW_DOWN:  diff = SG_COOR_DOWN();  char_repl = 'V'; break;
		case SG_MOVE_ARROW_LEFT:  diff = SG_COOR_LEFT();  char_repl = '<'; break;
		case SG_MOVE_ARROW_RIGHT: diff = SG_COOR_RIGHT(); char_repl = '>'; break;
		case SG_MOVE_ARROW_NOTHING: break;
		}
		
		/* Skip players not moving. */
		if(diff.x == -1 && diff.y == -1) {
			continue;
		}
		
		/* Flip when invalid. */
		sg_coor_t next = sg_coor_near(*head,diff);
		
		/* Next character. */
		unsigned char char_next = sg_table_get(&_snake->table, next, '#');
		
		/* Flip if required. */
		if (char_next == 'V' && _snake->moves[p] == SG_MOVE_ARROW_UP) {
			diff = SG_COOR_DOWN();
			char_repl = 'V';
		} else if (char_next == '^' && _snake->moves[p] == SG_MOVE_ARROW_DOWN) {
			diff = SG_COOR_UP();
			char_repl = '^';
		} else if (char_next == '>' && _snake->moves[p] == SG_MOVE_ARROW_LEFT) {
			diff = SG_COOR_RIGHT();
			char_repl = '>';
		} else if (char_next == '<' && _snake->moves[p] == SG_MOVE_ARROW_RIGHT) {
			diff = SG_COOR_LEFT();
			char_repl = '<';
		}
		next = sg_coor_near(*head, diff);
		char_next = sg_table_get(&_snake->table, next, '#');
		
		/* Set character. */
		sg_table_set(&_snake->table, *head, char_repl);
		
		/* If next character is an obstacle, finish. */
		if (char_next != ' ' && char_next != 'X') {
			sg_table_set(&_snake->table, *head, '$');
			_snake->has_head[p] = 0;
			continue;
		}
		
		/* Point up. */
		if (char_next == 'X') {
			_snake->points[p].number++;
			for (int j=0; j<10; j++) {
				sg_coor_t np = sg_coor_random(&_snake->table);
				unsigned char nc = sg_table_get(&_snake->table, np, '#');
				if(nc == ' ') {
					sg_table_set(&_snake->table, np, 'X');
					break;
				}
			}
		}
		
		/* Update tail. */
		sg_snake_update_tail(_snake, *head, _snake->points[p].number+5);
		
		/* Advance head. */
		sg_table_set(&_snake->table, next, sg_player_character(p));
		*head = next;
	}
	return 0;
}


static err_t
_sg_snake_refresh (void *_sg_game, unsigned long _us_time, sg_player_t _players[], int _count)
{
	err_t e;
	sg_snake_t *_snake = _sg_game;
	
	/* Update clock. */
	sg_clock_update(&_snake->clock, _us_time);
	
	/* For each tick forward. */
	while (sg_clock_get_tick(&_snake->clock)) {
		
		/* Get moves. */
		int player_count = (_count<8)?_count:8;
		for(int p = 0; p<player_count; p++) {
			if(sg_move_get_arrow(&_players[p].io[0], &_snake->moves[p])) {
				sg_move_reset(&_players[p].io[0]);
			}
		}
		
		/* Perform table update. */
		_sg_snake_apply(_sg_game, player_count);
		
		/* Send table. */
		for(int p = 0; p<player_count; p++) {
			/* Print title. */
			if(_players[p].human) {
				pbuffer_printf(
				    &_players[p].io[1], 0,
				    ANSI_CLEAR_SCREEN() ANSI_CLEAR_POSITION()
				);
			}
			/* Print table. */
			e = sg_table_refresh(
			    &_snake->table,
			    &_players[p].io[1],0,
			    _players[p].human,
			    _sg_snake_map
			);
			if(e<0/*err*/) {
				return e;
			}
			/* Print time. */
			sg_clock_refresh(&_snake->clock, &_players[p].io[1], _players[p].human);
			/* Print points. */
			sg_points_refresh(_snake->points, player_count, &_players[p].io[1], _players[p].human);
		}
	}
	return 0;
}

static err_t
_sg_snake_load (void *_sg_game, char _save[])
{
	err_t e;
	sg_snake_t *_snake = _sg_game;
	
	/* Load table. */
	e = sg_table_read_b(&_snake->table, _save);
	if(e<0/*err*/) return e;
	
	/* Set points. */
	sg_points_read_table(_snake->points, SG_SNAKE_MAX_PLAYERS, &_snake->table);
	
	/* Search heads. */
	for (int p=0; p<SG_SNAKE_MAX_PLAYERS; p++) {
		_snake->has_head[p] = sg_table_search(
		    &_snake->table,
		    sg_player_character(p),
		    &_snake->head[p]
		);
	}
	
	return 0;
}

static bool
_sg_snake_finished(void *_sg_game)
{
	sg_snake_t *snake = _sg_game;
	for (int p=0; p<SG_SNAKE_MAX_PLAYERS; p++) {
		if(snake->has_head[p]) {
			return 0;
		}
	}
	return 1;
}

static const char SG_SNAKE_MAP[] =
    " *********************** SOCKGAMES SNAKE *********************** " "\n"
    "|###############################################################|" "\n"
    "|# #                                                         # #|" "\n"
    "|###      X                                                  ###|" "\n"
    "|# #                                   X                     # #|" "\n"
    "|###        X                                                ###|" "\n"
    "|# #                          X                   X          # #|" "\n"
    "|###                                                         ###|" "\n"
    "|# #                  X                                      # #|" "\n"
    "|###                                    X                    ###|" "\n"
    "|# #     X                ####  ####                         # #|" "\n"
    "|###                      #    1   #              X          ###|" "\n"
    "|# #                      #        #   X                     # #|" "\n"
    "|###                      #        #                         ###|" "\n"
    "|# #          X                                              # #|" "\n"
    "|###                           X                             ###|" "\n"
    "|# #                                            X            # #|" "\n"
    "|###                                                         ###|" "\n"
    "|# #                                                         # #|" "\n"
    "|###              X             X              X             ###|" "\n"
    "|# #                                                         # #|" "\n"
    "|###                                                         ###|" "\n"
    "|###############################################################|" "\n"
    "."                                                                 "\n"
    "#Points 1: 0"                                                      "\n"
    ;

struct sg_game_i SG_SNAKE_GAME_IMPL = {
	.name = "snake",
	.description =
	"The player controls a dot on a bordered plane. As it moves forward, it leaves a" "\n"
	"trail behind, resembling a moving snake. The player loses when the snake runs "  "\n"
	"into the screen border, a trail or other obstacle, or itself.",
	.init     = (void*)sg_snake_init,
	.clean    = (void*)sg_snake_clean,
	.print    = (void*)sg_snake_print,
	.refresh  = _sg_snake_refresh,
	.load     = _sg_snake_load,
	.finished = _sg_snake_finished,
	.world    = SG_SNAKE_MAP,
	.us_rate  = 50000
};





