#include <c_types.h>
#include <osapi.h>
#include "user_cgi.h"
#include "user_configstore.h"
#include "json/jsonparse.h"
#include "json/jsontree.h"


//Some helper functions to do json<->http interaction

//This is so much not thread-safe... hope we never have to do this in a multi-threaded environment.
//Also: this limits the json responses we can send out to whatever amount of buffer httpdSend has internally.
static HttpdConnData *jsonPutcharConn;
int ICACHE_FLASH_ATTR jsonHttpdPutchar(int c) {
	char sc[2]={c, 0};
	httpdSend(jsonPutcharConn, sc, 1);
	return 0;
}

//Send a jsontree plus its values out of a http socket
void ICACHE_FLASH_ATTR jsonSend(HttpdConnData *connData, struct jsontree_value *tree) {
	struct jsontree_context json;
	json.values[0]=(struct jsontree_value *)tree;
	jsontree_reset(&json);
//	find_json_path(&json, path);
	json.path=json.depth;
	json.putchar=jsonHttpdPutchar;
	jsonPutcharConn=connData;
	while (jsontree_print_next(&json) && json.path<=json.depth);
}

//Parse a POSTed json tree
void ICACHE_FLASH_ATTR jsonParse(struct jsontree_context *json, char *msg, int len) {
	struct jsontree_value *v;
	struct jsontree_callback *c;
	struct jsontree_callback *c_bak = NULL;

	while ((v=jsontree_find_next(json, JSON_TYPE_CALLBACK)) != NULL) {
		c=(struct jsontree_callback *)v;
		if (c==c_bak) {
			continue;
		}
		c_bak=c;
		if (c->set!=NULL) {
			struct jsonparse_state js;
			jsonparse_setup(&js, msg, len);
			c->set(json, &js);
		}
	}
}


//Getter for JSON config tree
static int ICACHE_FLASH_ATTR treeConfigGet(struct jsontree_context *js_ctx) {
	const char *path=jsontree_path_name(js_ctx, js_ctx->depth-1);
	jsontree_write_string(js_ctx, path);
	os_printf("%s\n", path);
	return 0;
}

//Setter for JSON config tree
static int ICACHE_FLASH_ATTR treeConfigSet(struct jsontree_context *js_ctx,  struct jsonparse_state *parser) {
	return 0;
}

//GETter for a color
static int ICACHE_FLASH_ATTR treeColorGet(struct jsontree_context *js_ctx) {
	const char *path=jsontree_path_name(js_ctx, js_ctx->depth-1);
	int idx=js_ctx->index[js_ctx->depth-2];
	if (os_strcmp(path, "red")==0) {
		jsontree_write_int(js_ctx, myConfig.bval[idx].r);
	} else if (os_strcmp(path, "green")==0) {
		jsontree_write_int(js_ctx, myConfig.bval[idx].g);
	} else if (os_strcmp(path, "blue")==0) {
		jsontree_write_int(js_ctx, myConfig.bval[idx].b);
	} else if (os_strcmp(path, "coldwhite")==0) {
		jsontree_write_int(js_ctx, myConfig.bval[idx].cw);
	} else if (os_strcmp(path, "warmwhite")==0) {
		jsontree_write_int(js_ctx, myConfig.bval[idx].ww);
	}
	return 0;
}

//SETter for an array of color defs
static int ICACHE_FLASH_ATTR treeColorSet(struct jsontree_context *js_ctx,  struct jsonparse_state *parser) {
	int type;
	int ix=-1;
	int i;
	char buffer[100];
	while ((type=jsonparse_next(parser))!=0) {
		if (type==JSON_TYPE_ARRAY) {
			ix=-1;
		} else if (type==JSON_TYPE_OBJECT) {
			ix++;
		} else if (type==JSON_TYPE_PAIR_NAME) {
			jsonparse_copy_value(parser, buffer, sizeof(buffer));
			if (jsonparse_strcmp_value(parser, "red")==0) {
				jsonparse_next(parser); jsonparse_next(parser);
				myConfig.bval[ix].r=jsonparse_get_value_as_int(parser);
			} else if (jsonparse_strcmp_value(parser, "green")==0) {
				jsonparse_next(parser); jsonparse_next(parser);
				myConfig.bval[ix].g=jsonparse_get_value_as_int(parser);
			} else if (jsonparse_strcmp_value(parser, "blue")==0) {
				jsonparse_next(parser); jsonparse_next(parser);
				myConfig.bval[ix].b=jsonparse_get_value_as_int(parser);
			} else if (jsonparse_strcmp_value(parser, "coldwhite")==0) {
				jsonparse_next(parser); jsonparse_next(parser);
				myConfig.bval[ix].cw=jsonparse_get_value_as_int(parser);
			} else if (jsonparse_strcmp_value(parser, "warmwhite")==0) {
				jsonparse_next(parser); jsonparse_next(parser);
				myConfig.bval[ix].ww=jsonparse_get_value_as_int(parser);
			}
		}
	}
	return 0;
}


//Configuration JSON struct
static struct jsontree_callback cfgCallback=JSONTREE_CALLBACK(treeConfigGet, treeConfigSet);
static struct jsontree_callback colorCallback=JSONTREE_CALLBACK(treeColorGet, treeColorSet);

JSONTREE_OBJECT(colJsonobj, 
	JSONTREE_PAIR("red", &colorCallback),
	JSONTREE_PAIR("green", &colorCallback),
	JSONTREE_PAIR("blue", &colorCallback),
	JSONTREE_PAIR("coldwhite", &colorCallback),
	JSONTREE_PAIR("warmwhite", &colorCallback));

JSONTREE_ARRAY(buttonCfgJsonobj,
	JSONTREE_PAIR_ARRAY(&colJsonobj),
	JSONTREE_PAIR_ARRAY(&colJsonobj),
	JSONTREE_PAIR_ARRAY(&colJsonobj),
	JSONTREE_PAIR_ARRAY(&colJsonobj),
	JSONTREE_PAIR_ARRAY(&colJsonobj));

JSONTREE_OBJECT(configJsonobj,
	JSONTREE_PAIR("buttoncfg", &buttonCfgJsonobj));




//CGI fn to get the config JSON struct
int ICACHE_FLASH_ATTR cgiGetConfig(HttpdConnData *connData) {
	int *pos=(int *)&connData->cgiData;
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	jsonSend(connData, (struct jsontree_value *)&configJsonobj);
	
	return HTTPD_CGI_DONE;
}


//CGI that sets the JSON config struct
int ICACHE_FLASH_ATTR cgiSetConfig(HttpdConnData *connData) {
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	if(connData->post->len) {
		struct jsontree_context js;
		os_printf("%s",connData->post->buff);
		jsontree_setup(&js, (struct jsontree_value *)&configJsonobj, jsonHttpdPutchar);
		jsonParse(&js, connData->post->buff, connData->post->len);
		configSave();
		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", "text/json");
		httpdEndHeaders(connData);
		httpdSend(connData, "{\"status\": \"ok\"}", -1);
	} else {
		httpdSend(connData, "HTTP/1.0 500 Internal Server Error\r\nServer: esp8266-httpd/0.3\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: 24\r\n\r\nPOST error.\r\n", -1);
	}
	
	return HTTPD_CGI_DONE;
}

