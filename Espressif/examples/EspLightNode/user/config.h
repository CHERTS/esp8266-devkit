/*
 * config.h
 *
 *  Created on: Nov 19, 2014
 *      Author: frans-willem
 */

#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_
#include <c_types.h>

struct ConfigRunner {
	void (*beginmodule)(struct ConfigRunner *runner, const char *name, const char *description);
	void (*endmodule)(struct ConfigRunner *runner);
	void (*booloption)(struct ConfigRunner *runner, const char *name, const char *description, uint8_t *ptrvalue, uint8_t defvalue);
	void (*stringoption)(struct ConfigRunner *runner, const char *name, const char *description, char *ptrvalue, uint16_t len, const char *defvalue);
	void (*intoption)(struct ConfigRunner *runner, const char *name, const char *description, void *ptrvalue, uint8_t size, uint32_t minvalue, uint32_t maxvalue, uint32_t defvalue);
/*	void (*beginselectoption)(struct ConfigRunner *, const char *name, const char *description, uint8_t *ptrvalue);
	void (*selectoption)(struct ConfigRunner *, const char *name, const char *description, uint8_t value, uint8_t isdef);
	void (*endselectoption)(struct ConfigRunner *);*/
};
struct HttpdConnectionSlot;
void config_load();
void config_html(struct HttpdConnectionSlot *slot);

#define DEFINE_CONFIG(module) void module ## _runconfig(struct ConfigRunner *_configrunner);

#define BEGIN_CONFIG(module, description) void module ## _runconfig(struct ConfigRunner *_configrunner) { if (_configrunner->beginmodule) _configrunner->beginmodule(_configrunner, description , description);
#define END_CONFIG() if (_configrunner->endmodule) (_configrunner)->endmodule(_configrunner); }

#define CONFIG_SUB(name) module ## _runconfig(_configrunner)
#define CONFIG_BOOLEAN(name, description, address, defvalue) if (_configrunner->booloption) _configrunner->booloption(_configrunner, name, description, address, defvalue)
#define CONFIG_STRING(name, description, address, len, defvalue) if (_configrunner->stringoption) _configrunner->stringoption(_configrunner, name, description, address, len, defvalue)
#define CONFIG_INT(name, description, address, min, max, defvalue) if (_configrunner->intoption) _configrunner->intoption(_configrunner, name, description, address, sizeof(*(address)), min, max, defvalue)

#endif /* CONFIG_CONFIG_H_ */
