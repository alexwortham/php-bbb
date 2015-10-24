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
#include <ti_am335x_adc_buffer.h>

typedef struct ADC {
	int adc_initialized;
	int module_setup;
	int setup_error;
	char **ain_files;
} ADC;

#define CHAN_SIZE 50
#define NUM_CHANNELS 8
#define PTR_SIZE (sizeof(char *))
const char *adc_dir_format = "%siio:device%d/in_voltage%d_raw";

ADC *
ADC_new(void) {
	ADC *adc = malloc(sizeof(ADC));
	adc->adc_initialized = 0;
	adc->module_setup = 0;
	adc->setup_error = 0;
    	int dev_num = find_type_by_name(ADC_DEVICE_NAME, "iio:device");
	adc->ain_files = malloc( NUM_CHANNELS * PTR_SIZE );
	int i;
	for (i = 0; i < NUM_CHANNELS; i++) {
    		asprintf(&adc->ain_files[i], adc_dir_format, iio_dir, dev_num, i);
	}

	return adc;
}

ADC *
ADC_initialize(ADC *self)
{
    char *test_path;
    FILE *fh;
    int dev_num;
    if (self->adc_initialized) {
        return self;
    }

    /* Test opening the sysfs file for channel 0 */
    fh = fopen(self->ain_files[0], "r");

    if (!fh) {
        return NULL; 
    }
    fclose(fh);

    (self->adc_initialized) = 1;
    return self;
}

ADC *
ADC_setup() {
	ADC *adc = ADC_new();
	return ADC_initialize(adc);
}

ADC *
ADC_read_value(ADC *self, unsigned int ain, double *value)
{
    FILE * fh;
    int err, try_count=0;
    int read_successful = 0;
    int raw_val;
    
    // Workaround to AIN bug where reading from more than one AIN would cause access failures
    while (!read_successful && try_count < 3)
    {
        fh = fopen(self->ain_files[ain], "r");

        // Likely a bad path to the ocp device driver 
        if (!fh) {
		printf("Could not open %s\n", self->ain_files[ain]);
            return NULL;
        }

        fseek(fh, 0, SEEK_SET);
        err = fscanf(fh, "%d", &raw_val);

        if (err != EOF) read_successful = 1;
        fclose(fh);

	*value = (double) raw_val;

        try_count++;
    }

    if (read_successful) return self;

    // Fall through and fail
    return NULL;
}

ADC *
ADC_cleanup(ADC *self)
{
	int i;
	for (i = 0; i < NUM_CHANNELS; i++) {
    		free(&self->ain_files[i]);
	}
	free(self->ain_files);
	free(self);

	return NULL;
}

