#ifndef CGITHERMOSTAT_H
#define CGITHERMOSTAT_H

#include "httpd.h"

int cgiThermostat(HttpdConnData *connData);
void tplThermostat(HttpdConnData *connData, char *token, void **arg);

#endif
