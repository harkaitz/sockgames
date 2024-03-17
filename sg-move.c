#include "sockgames.h"
#include "aux/pterm.h"

void
sg_move_reset (pbuffer_t *buffer)
{
	buffer->pos = 0;
}

bool
sg_move_get_arrow(pbuffer_t *buffer, sg_move_arrow_t *arrow)
{
	sg_move_arrow_t ret = SG_MOVE_ARROW_NOTHING;
	for (int i=0;i<(buffer->pos-2);i++) {
		if (buffer->str[i]  !=PTERM_ESCAPE_CHAR) {
			continue;
		}
		if (buffer->str[i+1]!='[') {
			continue;
		}
		switch (buffer->str[i+2]) {
		case 'A': ret = SG_MOVE_ARROW_UP;    break;
		case 'B': ret = SG_MOVE_ARROW_DOWN;  break;
		case 'C': ret = SG_MOVE_ARROW_RIGHT; break;
		case 'D': ret = SG_MOVE_ARROW_LEFT;  break;
		}
	}
	if (ret!=SG_MOVE_ARROW_NOTHING) {
		*arrow = ret;
	}
	return (ret==SG_MOVE_ARROW_NOTHING)?0:1;
}
