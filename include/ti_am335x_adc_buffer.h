/**
 * Helper functions for accessing buffered IO 
 * on the TI AM335x ADC IIO device.
 */
#ifndef _TI_AM335X_ADC_BUFFER_H
#define _TI_AM335X_ADC_BUFFER_H
#include <iio_utils.h>
#define ADC_DEVICE_NAME "TI-am335x-adc"
#define CHAN_0	1
#define CHAN_1	2
#define CHAN_2	4
#define CHAN_3	8
#define CHAN_4	16
#define CHAN_5	32
#define CHAN_6	64
#define CHAN_7	128
#define IS_CHAN_0_ENABLED(c) ( (CHAN_0 & c) > 0 )
#define IS_CHAN_1_ENABLED(c) ( (CHAN_1 & c) > 0 )
#define IS_CHAN_2_ENABLED(c) ( (CHAN_2 & c) > 0 )
#define IS_CHAN_3_ENABLED(c) ( (CHAN_3 & c) > 0 )
#define IS_CHAN_4_ENABLED(c) ( (CHAN_4 & c) > 0 )
#define IS_CHAN_5_ENABLED(c) ( (CHAN_5 & c) > 0 )
#define IS_CHAN_6_ENABLED(c) ( (CHAN_6 & c) > 0 )
#define IS_CHAN_7_ENABLED(c) ( (CHAN_7 & c) > 0 )

/**
 * struct ti_adc_buffer: information about a buffer.
 * @scan_size: The scan size in bytes.
 * @num_channels: The number of active channels.
 * @scan_index: An index into the buffer data.
 * @read_size: The size last returned by read()ing the buffer device.
 * @buf_len: The length of the buffer.
 * @dev_num: The number of the buffer's underlying IIO device.
 * @data_len: The number of entries read into @data.
 * @channels_en: The bitmask of enabled channels.
 * @dev_name: The name of the IIO device driver.
 * @data: Points to where to store data read from buffer.
 * @buf_dir_name: The FQN of the IIO device's buffer directory.
 * @scan_el_dir: The FQN of the IIO device's scan_elements directory.
 * @dev: The FQN of the device special file.
 * @dev_dir_name: The FQN of the IIO device's sysfs directory.
 * @channels: Channel information needed by iio_utils.
 */
typedef struct ti_adc_buffer {
	int scan_size;
	int num_channels;
	int scan_index;
	int read_size;
	int buf_len;
	int dev_num;
	int dev_fd;
	int data_len;
	int channels_en;
	char *dev_name;
	char *data;
	char *buf_dir_name;
	char *scan_el_dir;
	char *dev;
	char *dev_dir_name;
	struct iio_channel_info *channels;
} ti_adc_buffer;

int ti_adc_buffer_enable(ti_adc_buffer *buffer);

int ti_adc_buffer_disable(ti_adc_buffer *buffer);

int ti_adc_buffer_set_length(ti_adc_buffer *buffer, int length);

int ti_adc_buffer_channel_enable(ti_adc_buffer *buffer, int channel);

int ti_adc_buffer_channel_disable(ti_adc_buffer *buffer, int channel);

/**
 * length: buffer length
 * channels: bitmasked value: which channels to open
 */
ti_adc_buffer *ti_adc_buffer_init(int length, int channels); //pub

int ti_adc_buffer_open(ti_adc_buffer *buffer); //pub

int ti_adc_buffer_close(ti_adc_buffer *buffer); //pub

int ti_adc_buffer_read(ti_adc_buffer *buffer); //pub

int ti_adc_buffer_next(ti_adc_buffer *buffer); //pub

uint16_t ti_adc_buffer_get_current(ti_adc_buffer *buffer, int channel); //pub

uint16_t ti_adc_buffer_get(ti_adc_buffer *buffer, int index, int channel); //pub


int ti_adc_size_from_channelarray(struct iio_channel_info *channels, int num_channels);

uint16_t ti_adc_conv_scan_item(int input, struct iio_channel_info *info);

#endif /* _TI_AM335X_ADC_BUFFER_H */
