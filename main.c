#include "sockgames.h"
#include <stdio.h>
#include <libgen.h>
#include <strings.h>

#define SOCKGAMES_GENERAL_HELP                                                        \
    "Usage: %s [GAME|FILE|list]"                                                 "\n" \
    ""                                                                           "\n" \
    "SockGames is a text based popular game collection. It targets bored system" "\n" \
    "administrators and AI researchers. It provides a text based interface to"   "\n" \
    "enable both humans and robots to play without discrimination."              "\n" \
    ""                                                                           "\n"

#define SOCKGAMES_SPECIFIC_HELP     \
    "Usage: %s [stdio]" "\n" \
    ""                  "\n" \
    "%s"                "\n" \
    ""                  "\n"

typedef union {
	sg_snake_t snake;
} sg_game_t;

struct sg_game_i *games[] = {
	&SG_SNAKE_GAME_IMPL,
	NULL
};

int main(int _argc, char *_argv[])
{
	sg_game_t         game;
	int               game_inited = 0;
	sg_player_t       players[20];
	int               player_count = 0;
	char const       *program = basename(_argv[0]);
	int               arg = 1;
	int               ret = 1, e;
	struct sg_game_i *selected_game = sg_game_search(games, program);
	char              buffer[5000];
	char              lret;
	
	if (_argc > 1 && (!strcmp(_argv[1], "-h") || !strcmp(_argv[1], "--help"))) {
		if (selected_game) {
			printf(SOCKGAMES_SPECIFIC_HELP,program,selected_game->description);
		} else {
			printf(SOCKGAMES_GENERAL_HELP, program);
		}
		ret = 0;
		goto cleanup;
	}
	
	if(!selected_game) {
		if(_argc == 1 || !strcasecmp(_argv[1], "list")) {
			for (int i=0; games[i]; i++) {
				printf("[%s]\n%s\n",games[i]->name,games[i]->description);
			}
			ret = 0;
			goto cleanup;
		} else {
			selected_game = sg_game_search(games, _argv[1]);
			if(selected_game == NULL) {
				fprintf(stderr,"%s: error: Selected game `%s` not found.\n", program, _argv[1]);
				ret = 1;
				goto cleanup;
			}
			arg++;
		}
	}
	
	if (!strncasecmp(selected_game->name, "snake", 5)) {
		sg_player_init(&players[player_count]);
		sg_player_set_stdio(&players[player_count]);
		player_count++;
	} else {
		goto cleanup;
	}
	
 restart:
	e = selected_game->init(&game);
	if(e<0/*err*/) {
		fprintf(stderr,"%s: error: %s\n", program,"");
		ret = 1;
		goto cleanup;
	}
	game_inited = 1;
	
	
	e = sg_game_start(&game, selected_game, NULL, buffer, sizeof(buffer));
	if(e<0/*err*/) {
		fprintf(stderr,"%s: error: %s\n",program,"");
		ret = 1;
		goto cleanup;
	}
	lret = sg_game_loop(&game, selected_game, players, player_count);
	
	if(lret=='e') {
		fprintf(stderr,"%s: error: %s\n",program,"");
		ret = 1;
		goto cleanup;
	} else if(lret=='r'){
		goto restart;
	}
	
 cleanup:
	if(game_inited) {
		selected_game->clean(&game);
	}
	for(int i=0;i<player_count;i++) {
		sg_player_clean(&players[i]);
	}
	return ret;
}
