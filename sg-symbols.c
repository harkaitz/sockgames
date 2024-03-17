#include "sockgames.h"
#include <string.h>

static const char player_chars[] = "123456789ABCDEFGHYJKLMNOPQRSTUVXYZ";

char
sg_player_character(int _num)
{
	int player_num = (_num<(sizeof(player_chars)-1))?_num:(sizeof(player_chars)-1);
	return player_chars[player_num];
}

bool
sg_player_number(char _chr, int *_num)
{
	const char *fnd = strchr(player_chars, _chr);
	if(fnd) {
		*_num = fnd-player_chars;
		return 1;
	} else {
		return 0;
	}
}

