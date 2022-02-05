/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

#include "dx_utilities.h"
#include "location_from_ip.h"
#include "parson.h"
#include "utils.h"
#include <applibs/log.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static struct location_info locationInfo;
static const char *geoIfyURL = "https://get.geojs.io/v1/ip/geo.json";

static char* get_value_by_key(JSON_Object *rootObject, const char *key)
{
    char *value = NULL;
    if (json_object_has_value_of_type(rootObject, key, JSONString)) {
        const char *json_value = json_object_get_string(rootObject, key);
        if (json_value != NULL) {
            size_t len = strlen(json_value);
            value = (char *)malloc(len + 1);
            if (value != NULL) {
                memset(value, 0x00, len + 1);
                strncpy(value, json_value, len);
            }
        }
    }
    return value;
}

struct location_info *GetLocationData(void)
{
    if (locationInfo.updated){
        return &locationInfo;
    }

    JSON_Value *rootProperties = NULL;

    memset(&locationInfo, 0x00, sizeof(locationInfo));

    char *data = getHttpData(geoIfyURL);

    if (data != NULL) {

        rootProperties = json_parse_string(data);
        if (rootProperties == NULL) {
            goto cleanup;
        }

        JSON_Object *rootObject = json_value_get_object(rootProperties);
        if (rootObject == NULL) {
            goto cleanup;
        }

        if (!json_object_has_value_of_type(rootObject, "country_code", JSONString) || 
			!json_object_has_value_of_type(rootObject, "latitude", JSONString) ||
            !json_object_has_value_of_type(rootObject, "longitude", JSONString)) {
            goto cleanup;
        }

        // const char *countryCode = json_object_get_string(rootObject, "country_code");
        const char *latitude = json_object_get_string(rootObject, "latitude");
        const char *longitude = json_object_get_string(rootObject, "longitude");

        locationInfo.lat = strtod(latitude, NULL);
        locationInfo.lng = strtod(longitude, NULL);

        locationInfo.countryCode = get_value_by_key(rootObject, "country_code");
        locationInfo.country = get_value_by_key(rootObject, "country");
        locationInfo.region = get_value_by_key(rootObject, "region");
        locationInfo.city = get_value_by_key(rootObject, "city");        
        
        locationInfo.updated = true;
    }

cleanup:

    if (rootProperties != NULL) {
        json_value_free(rootProperties);
    }

    if (data != NULL) {
        free(data);
        data = NULL;
    }

    return &locationInfo;
}