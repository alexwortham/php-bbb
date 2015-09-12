/*
Copyright (c) 2013 Adafruit
Author: Justin Cooper

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
	int adc_initialized;
	int module_setup;
	int setup_error;
	char adc_prefix_dir[40];
	char ocp_dir[25];
	char ctrl_dir[35];
} ADC;

int build_path(const char *partial_path, const char *prefix, char *full_path, size_t full_path_len)
{
    DIR *dp;
    struct dirent *ep;

    dp = opendir (partial_path);
    if (dp != NULL) {
        while ((ep = readdir (dp))) {
            // Enforce that the prefix must be the first part of the file
            char* found_string = strstr(ep->d_name, prefix);

            if (found_string != NULL && (ep->d_name - found_string) == 0) {
                snprintf(full_path, full_path_len, "%s/%s", partial_path, ep->d_name);
                (void) closedir (dp);
                return 1;
            }
        }
        (void) closedir (dp);
    } else {
        return 0;
    }

    return 0;
}

ADC *
load_device_tree(ADC *self, const char *name)
{
    FILE *file = NULL;
    char slots[40];
    char line[256];

    build_path("/sys/devices", "bone_capemgr", self->ctrl_dir, sizeof(self->ctrl_dir));
    snprintf(slots, sizeof(slots), "%s/slots", self->ctrl_dir);

    file = fopen(slots, "r+");
    if (!file) {
        //PyErr_SetFromErrnoWithFilename(PyExc_IOError, slots);
        return NULL;
    }

    while (fgets(line, sizeof(line), file)) {
        //the device is already loaded, return 1
        if (strstr(line, name)) {
            fclose(file);
            return self;
        }
    }

    //if the device isn't already loaded, load it, and return
    fprintf(file, name);
    fclose(file);

    //0.2 second delay
    nanosleep((struct timespec[]){{0, 200000000}}, NULL);

    return self;
}

ADC *
unload_device_tree(ADC *self, const char *name)
{
    FILE *file = NULL;
    char slots[40];
    char line[256];
    char *slot_line;

    build_path("/sys/devices", "bone_capemgr", self->ctrl_dir, sizeof(self->ctrl_dir));
    snprintf(slots, sizeof(slots), "%s/slots", self->ctrl_dir);

    file = fopen(slots, "r+");
    if (!file) {
        //PyErr_SetFromErrnoWithFilename(PyExc_IOError, slots);
        return NULL;
    }

    while (fgets(line, sizeof(line), file)) {
        //the device is loaded, let's unload it
        if (strstr(line, name)) {
            slot_line = strtok(line, ":");
            //remove leading spaces
            while(*slot_line == ' ')
                slot_line++;

            fprintf(file, "-%s", slot_line);
            fclose(file);
            return self;
        }
    }

    //not loaded, close file
    fclose(file);

    return self;
}

ADC *
ADC_new(void) {
	ADC *adc = malloc(sizeof(ADC));
	adc->adc_initialized = 0;
	adc->module_setup = 0;
	adc->setup_error = 0;

	return adc;
}

ADC *
ADC_initialize(ADC *self)
{
    char test_path[40];
    FILE *fh;
    if (self->adc_initialized) {
        return self;
    }

    if (load_device_tree(self, "cape-bone-iio")) {
        build_path("/sys/devices", "ocp.", (self->ocp_dir), sizeof((self->ocp_dir)));
        build_path((self->ocp_dir), "helper.", (self->adc_prefix_dir), sizeof((self->adc_prefix_dir)));
        strncat((self->adc_prefix_dir), "/AIN", sizeof((self->adc_prefix_dir)));

        // Test that the directory has an AIN entry (found correct devicetree)
        snprintf(test_path, sizeof(test_path), "%s%d", (self->adc_prefix_dir), 0);
        
        fh = fopen(test_path, "r");

        if (!fh) {
            return NULL; 
        }
        fclose(fh);

        (self->adc_initialized) = 1;
        return self;
    }

    return NULL;
}

ADC *
ADC_setup() {
	ADC *adc = ADC_new();
	return ADC_initialize(adc);
}

ADC *
ADC_read_value(ADC *self, unsigned int ain, float *value)
{
    FILE * fh;
    char ain_path[40];
    int err, try_count=0;
    int read_successful;
    snprintf(ain_path, sizeof(ain_path), "%s%d", (self->adc_prefix_dir), ain);
    
    read_successful = 0;

    // Workaround to AIN bug where reading from more than one AIN would cause access failures
    while (!read_successful && try_count < 3)
    {
        fh = fopen(ain_path, "r");

        // Likely a bad path to the ocp device driver 
        if (!fh) {
            return NULL;
        }

        fseek(fh, 0, SEEK_SET);
        err = fscanf(fh, "%f", value);

        if (err != EOF) read_successful = 1;
        fclose(fh);

        try_count++;
    }

    if (read_successful) return self;

    // Fall through and fail
    return NULL;
}

ADC *
ADC_cleanup(ADC *self)
{
	unload_device_tree(self, "cape-bone-iio");
	free(self);

	return NULL;
}

