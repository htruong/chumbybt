#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <err.h>
#include <event.h>
#include <evhttp.h>


void argtoi(struct evkeyvalq *args, char *key, int *val, int def)
{
    char *tmp;

    *val = def;
    tmp = (char *)evhttp_find_header(args, (const char *)key);
    if (tmp) {
        *val = atoi(tmp);
    }
}

int _get_int(struct evhttp_request *req, char *key, int def) {
	struct evkeyvalq    args;
	int ret;
	evhttp_parse_query(req->uri, &args);
	argtoi(&args, key, &ret, def);
	return ret;
} 

char * _get_chars (struct evhttp_request *req, char *key, char * def) {
	struct evkeyvalq args;
	char * ret;
	evhttp_parse_query(req->uri, &args);
	ret = (char *)evhttp_find_header(&args, key);
	if (ret == NULL) {
		return def;
	} 
	return ret;
}

void notfound_hander(struct evhttp_request *req, void *arg) 
{
struct evbuffer *buf;
	buf = evbuffer_new();
	if (buf == NULL)
		err(1, "failed to create response buffer");
	evbuffer_add_printf(buf, "404 Not found");
	evhttp_add_header(evhttp_request_get_output_headers(req),
		    "Content-Type", "text/plain");
	evhttp_send_reply(req, HTTP_NOTFOUND, "Not found", buf);	
}

void sys_handler(struct evhttp_request *req, void *arg)
{
	struct evbuffer *buf;
	char cmd[255] = "";
	int valid_cmd = 1;

	buf = evbuffer_new();
	if (buf == NULL)
		err(1, "failed to create response buffer");
	evhttp_add_header(evhttp_request_get_output_headers(req),
		    "Content-Type", "text/javascript");
	//evbuffer_add_printf(buf, "Requested: %s id=%d parm=%s", evhttp_request_uri(req), _get_int(req, "id", -1), _get_chars(req, "parm", "NULL"));

	if (strncmp(_get_chars(req, "cmd", ""),"set_brightness") == 0) {
		sprintf(cmd, "/bin/bash -c 'echo %d > /sys/devices/platform/stmp3xxx-bl/backlight/stmp3xxx-bl/brightness'", _get_int(req, "val", 0));
		system(cmd);
	} else if (strncmp(_get_chars(req, "cmd", ""),"shutdown") == 0) {
		system("shutdown -P now");
	} else {
		valid_cmd = 0;
	}

	evbuffer_add_printf(buf, "{'result':%d}", valid_cmd);

	evhttp_send_reply(req, HTTP_OK, "OK", buf);
}

int main(int argc, char **argv)
{
	struct evhttp *httpd;
	event_init();
	httpd = evhttp_start("127.0.0.1", 8080);
	evhttp_set_cb(httpd, "/request_sys", sys_handler, NULL); 
/* Set a callback for all other requests. */
	evhttp_set_gencb(httpd, notfound_hander, NULL);
event_dispatch();    /* Not reached in this code as it is now. */
	evhttp_free(httpd);    
	return 0;
}