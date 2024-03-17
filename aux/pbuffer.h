#ifndef PBUFFER_H
#define PBUFFER_H
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

typedef struct pbuffer_s pbuffer_t;
    
struct pbuffer_s {
	int	 pos;
	int	 len;
	char	*str;
};


static inline __attribute__((unused)) char *
pbuffer_vprintf(pbuffer_t *_buf, int _zero, const char *_format, va_list _va)
{
	char *ret = (_buf->str)+(_buf->pos);
	int len = vsnprintf(
	    (_buf->str)+(_buf->pos),
	    (_buf->len)-(_buf->pos),
	    _format,
	    _va
	);
	if((_buf->pos + len + _zero)<(_buf->len)) {
		_buf->pos += len + _zero;
		return ret;
	} else {
		return NULL;
	}
}

static inline __attribute__((unused)) char *
pbuffer_printf(pbuffer_t *_buf, int _zero, char const _format[],...)
{
	va_list va;
	va_start(va, _format);
	char *r = pbuffer_vprintf(_buf, _zero, _format, va);
	va_end(va);
	return r;
}

static inline __attribute__((unused)) pbuffer_t
pbuffer_create(char *_str, size_t _len)
{
	pbuffer_t buf;
	buf.pos = 0;
	buf.len = _len;
	buf.str = _str;
	return buf;
}

static inline __attribute__((unused)) char *
pbuffer_alloc(pbuffer_t *_buf,size_t _len)
{
	int npos = _buf->pos + _len;
	if(npos > _buf->len) {
		return NULL;
	}
	char *ret = _buf->str + _buf->pos;
	_buf->pos += _len;
	return ret;
}

static inline __attribute__((unused)) char *
pbuffer_strdup(pbuffer_t *_buf,const char *_str)
{
	size_t len = strlen(_str)+1;
	char *r = pbuffer_alloc(_buf, len);
	if(r) {
		memcpy(r, _str,len);
	}
	return r;
}

static inline __attribute__((unused)) void
pbuffer_position(pbuffer_t *_buf, char **_out, size_t *_len)
{
	*_out = (_buf->str)+(_buf->pos);
	*_len = (_buf->len)-(_buf->pos);
}

static inline __attribute__((unused)) int
pbuffer_read(pbuffer_t *_buf, int _fd, size_t _num)
{
	size_t len = (_buf->len)-(_buf->pos);
	if (_num<len) {
		len = _num;
	}
	int pos = read(
	    _fd,
	    (_buf->str)+(_buf->pos),
	    len
	);
	if (pos==-1 || pos==0) {
		return pos;
	}
	_buf->pos += pos;
	return _buf->pos;
}

static inline __attribute__((unused)) int
pbuffer_write(pbuffer_t *_buf, int _fd)
{
	int ret = write(_fd, _buf->str, _buf->pos);
	_buf->pos = 0;
	return ret;
}

static inline __attribute__((unused)) int
buffer_overflow2(const char *_b)
{
	return (_b)?1:0;
}

#endif
