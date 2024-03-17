#include "sockgames.h"
#include <sys/select.h>
#include <strings.h>
#include "aux/pterm.h"
#include "aux/foreach.h"

void
sg_player_init(sg_player_t *_player)
{
	_player->active = 0;
	_player->human  = 0;
	_player->scount = 0;
	_player->fd[0] = -1;
	_player->fd[1] = -1;
	_player->io[0] = pbuffer_create(_player->i_data, sizeof(_player->i_data));
	_player->io[1] = pbuffer_create(_player->o_data, sizeof(_player->o_data));
}

void
sg_player_set_stdio(sg_player_t *_player)
{
	_player->active = 1;
	_player->human  = isatty(1);
	_player->fd[0]  = 0;
	_player->fd[1]  = 1;
}

void
sg_player_clean (sg_player_t *_player __attribute__((unused)))
{}

void
sg_player_close (sg_player_t *_player)
{
	if(_player->fd[0]!=-1) close(_player->fd[0]);
	if(_player->fd[1]!=-1) close(_player->fd[1]);
	_player->fd[0] = -1;
	_player->fd[1] = -1;
	_player->active = 0;
}

err_t
sg_game_start(void *_sg_game, struct sg_game_i *_intf, char const _save[], char _buffer[], size_t _buffer_len)
{
	/* Select game to copy. */
	char const *initial = _save ? _save : _intf->world;
	
	/* Copy to storage. */
	size_t len = strlen(initial);
	if (len>=_buffer_len) {
		return perr(-1, "Save game too large.");
	}
	memcpy(_buffer, initial, len);
	_buffer[len]='\0';
	
	/* Load. */
	if (_intf->load && _intf->world) {
		err_t err = _intf->load(_sg_game, _buffer);
		if(err<0) return err;
	}
	
	return 0;
}

char
sg_game_loop (void *_sg_game, struct sg_game_i *_intf, sg_player_t _players[], int _count)
{
	/* Global status. */
	unsigned long us_time = 0;
	unsigned long us_rate = _intf->us_rate;
	struct timeval timeout = {0,us_rate};
	
	/* Save and configure terminal. */
	struct termios term;
	pterm_save_config(&term);
	pterm_set_gaming_mode();
	
	/* Game loop. */
	char quit = 0; int stopped = 0;
	while (!quit) {
		int max = 0;
		fd_set rset,wset;
		FD_ZERO(&rset); FD_ZERO(&wset);
		
		sg_player_t *player;
		AR_FOREACH(_players, _count, player) {
			if (player->active==0) {
				continue;
			}
			if (player->fd[0]!=-1) {
				if(player->fd[0] > max) {
					max = player->fd[0];
				}
				FD_SET(player->fd[0], &rset);
			}
			if (player->io[1].pos && player->fd[1]!=-1) {
				if(player->fd[1] > max) {
					max = player->fd[1];
				}
				FD_SET(player->fd[1], &wset);
			}
			player->scount++;
		}
		
		/* Run select. */
		int finished = (_intf->finished && _intf->finished(_sg_game))?1:0;
		
		int ret = select(max+1, &rset, &wset, NULL, (finished)?NULL:&timeout);
		
		/* Select fail. */
		if (ret == -1) {
			quit = 'e';
			break;
		}
		
		/* Refresh. */
		if(timeout.tv_usec == 0) {
			timeout.tv_sec  = 0;
			timeout.tv_usec = us_rate;
			if (stopped == 0) {
				us_time += us_rate;
				if (_intf->refresh) {
					_intf->refresh(_sg_game, us_time, _players, _count);
				}
			}
		}
		
		/* I/O */
		AR_FOREACH(_players, _count, player) {
			if (FD_ISSET(player->fd[0], &rset)) {
				int res = pbuffer_read(&player->io[0], player->fd[0],1);
				if (res==-1||res==0) {
					sg_player_close(player);
				}
				
				if (player->fd[0]==0) {
					switch(player->io[0].str[player->io[0].pos-1]) {
					case 'q': quit = 'q'; break;
					case 'r': quit = 'r'; break;
					case 's': stopped = !stopped; break;
					}
				}
				
			}
			if (FD_ISSET(player->fd[1], &wset)) {
				if(player->fd[1]==1 && player->human) {
					pbuffer_printf(&player->io[1], 0,
					    "   Type [q] to quit."    "\n"
					    "   Type [r] to restart." "\n"
					    "   Type [s] to stop"     "\n");
					
				}
				int res = pbuffer_write(&player->io[1],player->fd[1]);
				if(res==-1||res==0) {
					sg_player_close(player);
				}
			}
		}
	}
	/* Restore terminal. */
	pterm_restore_config(&term);
	return quit;
}

bool
sg_game_finished (void *_sg_game, struct sg_game_i *_intf)
{
	if(_intf->finished && _intf->finished(_sg_game)) {
		return 1;
	} else {
		return 0;
	}
}

struct sg_game_i *
sg_game_search(struct sg_game_i **_games, char const _name[])
{
	for (size_t i=0; _games[i]; i++) {
		if (!strcasecmp(_games[i]->name, _name)) {
			return _games[i];
		}
	}
	return NULL;
}
