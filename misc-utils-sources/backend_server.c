#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <err.h>
#include <event.h>
#include <evhttp.h>

#include <string.h>
#include <stdio.h>

char * escape(char*str) {
    
    char *escStr;
    int i, count_final, count = strlen(str);
    
    count_final = count + 1;
    
    for(i=0; i<count; i++) {
    	if (str[i] == '\'' || str[i] == '\n') {
    			count_final++;
    	}
    } 

    escStr = (char *) malloc(count_final * sizeof(char));

    count_final = 0;
	
    for(i=0; i<count; i++) {
    	if (str[i] == '\'' || str[i] == '\n') {
    			escStr[count_final] = '\\';
    			count_final++;
    	}
    	if (str[i] == '\n') {
    		escStr[count_final] = 'n';
    	} else {
    		escStr[count_final] = str[i];
    	}
    	count_final++;
    }
    escStr[count_final] = '\0';

    return escStr;
}

int exec(char* cmd, char * ret, int max) {
    FILE* pipe = popen(cmd, "r");
    int ret_val = 0;
    if (!pipe) ret_val = 0;
    //char buffer[128];
    //while((!feof(pipe)) && (counter < max)) {
    	//if(fgets(buffer, 128, pipe) != NULL) {
    		//strncpy(&ret[counter], buffer, 128);
    	//}
    	//counter += 128;
    //}
    int max_remaining = max;
    while (!feof(pipe)) {
    	max_remaining = max - strlen(ret) - 1;
    	if (fgets(&ret[strlen(ret)], max_remaining, pipe) == NULL) {
    		break;
	    }	
    }
    
    pclose(pipe);
    return ret_val;
}

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
	char * cmd;
	int valid_cmd = 1;


	cmd = (char *) malloc(1024 * sizeof(char));

	buf = evbuffer_new();
	if (buf == NULL)
		err(1, "failed to create response buffer");
	evhttp_add_header(evhttp_request_get_output_headers(req),
		    "Content-Type", "text/javascript");
	//evbuffer_add_printf(buf, "Requested: %s id=%d parm=%s", evhttp_request_uri(req), _get_int(req, "id", -1), _get_chars(req, "parm", "NULL"));	

	if (strncmp(_get_chars(req, "cmd", ""),"set_brightness", 50) == 0) {
		sprintf(cmd, "/bin/bash -c 'echo %d > /sys/devices/platform/stmp3xxx-bl/backlight/stmp3xxx-bl/brightness'", _get_int(req, "parm", 50));
	} else if (strncmp(_get_chars(req, "cmd", ""),"shell", 50) == 0) {
		sprintf(cmd, "/bin/bash -c '%s'", _get_chars(req, "parm", ""));
	} else if (strncmp(_get_chars(req, "cmd", ""),"exec", 50) == 0) {
		sprintf(cmd, "%s", _get_chars(req, "parm", ""));
	} else {
		valid_cmd = 0;
	}


	if (valid_cmd) {
		char * ret;
		char * ret_escaped;
		ret = (char *) malloc(4096 *sizeof(char));
		exec(cmd, ret, 4096 *sizeof(char));
		//fprintf(stderr, "%s\n", ret);
		ret_escaped = escape(ret);
		free(ret);
		evbuffer_add_printf(buf, "{\"valid\":%d,\"result\":\"%s\"}", valid_cmd, ret_escaped);
		free(ret_escaped);
	} else {
		evbuffer_add_printf(buf, "{\"valid\":%d,\"result\":\"\"}", valid_cmd);
	}

	evhttp_send_reply(req, HTTP_OK, "OK", buf);

	free(cmd);
}

int main(int argc, char **argv)
{
	struct evhttp *httpd;
	event_init();
	httpd = evhttp_start(argv[argc-2], atoi(argv[argc-1]));
	evhttp_set_cb(httpd, "/request_sys", sys_handler, NULL); 
/* Set a callback for all other requests. */
	evhttp_set_gencb(httpd, notfound_hander, NULL);
event_dispatch();    /* Not reached in this code as it is now. */
	evhttp_free(httpd);    
	return 0;
}