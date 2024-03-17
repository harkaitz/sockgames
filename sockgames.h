#include <unistd.h>
#include <stdbool.h>
#include "aux/pbuffer.h"

typedef int err_t;
typedef struct sg_table_s  sg_table_t;
typedef struct sg_snake_s  sg_snake_t;
typedef struct sg_clock_s sg_clock_t;
typedef struct sg_points_s sg_points_t;
typedef struct sg_coor_s sg_coor_t;
typedef struct sg_player_s sg_player_t;
typedef struct sg_table_map_s sg_table_map_t;

#define SG_TABLE_MAX_X       100
#define SG_TABLE_MAX_Y       100
#define SG_TABLE_MAX_HEADERS 20
#define SG_TABLE_MAX_STORAGE 5000
#ifndef SG_SNAKE_MAX_PLAYERS
#  define SG_SNAKE_MAX_PLAYERS 1
#endif
#define SG_COOR_LEFT()  sg_coor(-1, 0)
#define SG_COOR_RIGHT() sg_coor( 1, 0)
#define SG_COOR_UP()    sg_coor( 0,-1)
#define SG_COOR_DOWN()  sg_coor( 0, 1)
#define SG_COOR_EQUAL(C1,C2) (((C1).x == (C2).x) && ((C1).y == (C2).x))
#ifndef perr
#  define perr(A,...) A
#endif

struct sg_table_s {
	unsigned char	 world[SG_TABLE_MAX_Y][SG_TABLE_MAX_X];
	int		 x_len,y_len;
	char const	*h[SG_TABLE_MAX_HEADERS];
	int		 h_count;
	char const	*b[SG_TABLE_MAX_HEADERS];
	int	 	 b_count;
};

struct sg_clock_s {
	unsigned long	last_update;
	unsigned long	period;
	unsigned long	min_period;
	int		ticks;
};

struct sg_points_s {
	long	number;
};

struct sg_coor_s {
	int	x,y;
};

struct sg_table_map_s {
	unsigned char	 fr;
	char const	*to;
};

struct sg_player_s {
	int	active;
	int	human;
	long	scount;
	int	fd[2];
	struct	pbuffer_s io[2];
	char	o_data[40000];
	char	i_data[512];
};

typedef enum sg_move_arrow_e {
	SG_MOVE_ARROW_NOTHING	= 0x00,
	SG_MOVE_ARROW_LEFT	= 0x01,
	SG_MOVE_ARROW_RIGHT	= 0x02,
	SG_MOVE_ARROW_UP	= 0x03,
	SG_MOVE_ARROW_DOWN	= 0x04
} sg_move_arrow_t;

struct sg_snake_s {
	struct	sg_table_s      table;
	struct	sg_clock_s      clock;
	struct	sg_points_s     points [SG_SNAKE_MAX_PLAYERS];
	enum	sg_move_arrow_e moves  [SG_SNAKE_MAX_PLAYERS];
	struct	sg_coor_s       head   [SG_SNAKE_MAX_PLAYERS];
	bool	has_head [SG_SNAKE_MAX_PLAYERS];
};

struct sg_game_i {
	char const *name;
	char const *description;
	err_t	 (*init)     (void *);
	void	 (*clean)    (void *);
	int	 (*print)    (void *, char *, size_t);
	err_t	 (*refresh)  (void *,unsigned long,sg_player_t [],int);
	err_t	 (*load)     (void *,char []);
	bool	 (*finished) (void *);
	char	 const *world;
	unsigned long us_rate;
};

err_t	sg_clock_init(sg_clock_t *, unsigned long, unsigned long);
void	sg_clock_clean(sg_clock_t *);
err_t	sg_clock_set_2x(sg_clock_t *);
void	sg_clock_update(sg_clock_t *,unsigned long);
bool	sg_clock_get_tick(sg_clock_t *);
void	sg_clock_refresh(sg_clock_t *, pbuffer_t *,int);

void	sg_points_init(sg_points_t [],int);
void	sg_points_clean(sg_points_t [],int);
void	sg_points_read_table(sg_points_t [], int, sg_table_t *);
void	sg_points_refresh(sg_points_t [], int, pbuffer_t *, int);

void	sg_table_init(sg_table_t *);
void    sg_table_clean(sg_table_t *);
int	sg_table_print(sg_table_t *, char *, size_t);
err_t	sg_table_read_b(sg_table_t *, char []);
err_t	sg_table_get_number (sg_table_t *,char const [],long *);
err_t	sg_table_get_string (sg_table_t *,char const [],char const **);

sg_coor_t	sg_coor(int, int);
sg_coor_t	sg_coor_near(sg_coor_t, sg_coor_t);
bool		sg_coor_is_valid(sg_coor_t, const sg_table_t *);
sg_coor_t	sg_coor_random(const sg_table_t *);

bool		sg_table_search(const sg_table_t *, unsigned char, sg_coor_t *);
unsigned char	sg_table_get(const sg_table_t *, sg_coor_t, unsigned char);
void		sg_table_set(sg_table_t *, sg_coor_t, unsigned char);
err_t		sg_table_refresh(sg_table_t *, pbuffer_t *, int, int, sg_table_map_t []);

void	sg_player_init(sg_player_t *);
void	sg_player_set_stdio(sg_player_t *);
void	sg_player_clean(sg_player_t *);
void	sg_player_close(sg_player_t *);
char	sg_player_character(int);
bool	sg_player_number(char, int *);


err_t  sg_game_start    (void *, struct sg_game_i *, char const [], char [], size_t);
char   sg_game_loop     (void *, struct sg_game_i *, sg_player_t [], int);
bool   sg_game_finished (void *, struct sg_game_i *);
struct sg_game_i *sg_game_search(struct sg_game_i **, char const []);

err_t	sg_snake_init(sg_snake_t *);
void	sg_snake_clean(sg_snake_t *);
int	sg_snake_print(sg_snake_t *, char *, size_t);

extern struct sg_game_i SG_SNAKE_GAME_IMPL;

void sg_move_reset(pbuffer_t *);
bool sg_move_get_arrow(pbuffer_t *,sg_move_arrow_t *);
