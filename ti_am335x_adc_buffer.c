#include <ti_am335x_adc_buffer.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <linux/types.h>
#include <string.h>
#include <poll.h>
#include <endian.h>
#include <getopt.h>
#include <inttypes.h>
#include "iio_utils.h"


#define DEV_CHAN_EN_FORMAT "in_voltage%d_en"
#define BUFFER_ENABLE 1
#define BUFFER_DISABLE 0
#define CHANNEL_ENABLE 1
#define CHANNEL_DISABLE 0

int ti_adc_get_buf_dir_name(ti_adc_buffer *buffer, int dev_num) {

	int ret = 0;

	ret = asprintf(&buffer->buf_dir_name, "%siio:device%d/buffer", iio_dir, dev_num);
	if (ret < 0) {
		printf("Failed to allocate memory for constructing buffer directory name.\n");
		return -ENOMEM;
	}

	return 0;
}

int ti_adc_buffer_enable(ti_adc_buffer *buffer) {

	int ret = 0;

	ret = write_sysfs_int("enable", buffer->buf_dir_name, BUFFER_ENABLE);
	if (ret < 0) {
		printf("Could write to 'buffer/enable' sysfs file.\n");
		return -1;
	}

	return 0;
}

int ti_adc_buffer_disable(ti_adc_buffer *buffer) {

	int ret = 0;

	ret = write_sysfs_int("enable", buffer->buf_dir_name, BUFFER_DISABLE);
	if (ret < 0) {
		printf("Could write to 'buffer/enable' sysfs file.\n");
		return -1;
	}

	return 0;
}

int ti_adc_buffer_set_length(ti_adc_buffer *buffer, int length) {

	int ret = 0;

	ret = write_sysfs_int("length", buffer->buf_dir_name, length);
	if (ret < 0) {
		printf("Could not write to 'buffer/length' sysfs file.\n");
		return -errno;
	}

	return 0;
}

int ti_adc_get_scan_elements_dir(ti_adc_buffer *buffer, int dev_num) {

	int ret = 0;

	ret = asprintf(&buffer->scan_el_dir, "%siio:device%d/scan_elements", iio_dir, dev_num);
	if (ret < 0) {
		printf("Failed to allocate memory for constructing scan_elements dir name.\n");
		return -ENOMEM;
	}

	return 0;
}

int ti_adc_buffer_channel_enable(ti_adc_buffer *buffer, int channel) {

	int ret = 0;
	char *chan;

	ret = asprintf(&chan, DEV_CHAN_EN_FORMAT, channel);
	if (ret < 0) {
		printf("Failed to allocate memory for constructing channel _en file name.\n");
		return -ENOMEM;
	}

	ret = write_sysfs_int(chan, buffer->scan_el_dir, CHANNEL_ENABLE);
	if (ret < 0) {
		printf("Could not write to %s/%s sysfs file.\n", buffer->scan_el_dir, chan);
		return -errno;
	}
	free(chan);

	buffer->num_channels++;

	return 0;
}

int ti_adc_buffer_channel_disable(ti_adc_buffer *buffer, int channel) {

	int ret = 0;
	char *chan;

	ret = asprintf(&chan, DEV_CHAN_EN_FORMAT, channel);
	if (ret < 0) {
		printf("Failed to allocate memory for constructing channel _en file name.\n");
		return -ENOMEM;
	}

	ret = write_sysfs_int(chan, buffer->scan_el_dir, CHANNEL_DISABLE);
	if (ret < 0) {
		printf("Could not write to %s/%s sysfs file.\n", buffer->scan_el_dir, chan);
		return -errno;
	}
	free(chan);

	buffer->num_channels--;

	return 0;
}

/**
 * length: buffer length
 * channels: bitmasked value: which channels to open
 */
ti_adc_buffer *ti_adc_buffer_init(int length, int channels) {

	int dev_num;
	int num_channels;
	int ret;
	struct ti_adc_buffer *buffer;

	/* Find the device requested */
	dev_num = find_type_by_name(ADC_DEVICE_NAME, "iio:device");
	if (dev_num < 0) {
		printf("Failed to find the %s\n", ADC_DEVICE_NAME);
		return NULL;
	}

	printf("Found iio device number: %d\n", dev_num);

	buffer = (struct ti_adc_buffer *) malloc(sizeof(ti_adc_buffer));
	buffer->num_channels = 0;
	buffer->scan_index = 0;
	buffer->read_size = 0;
	buffer->channels_en = channels;
	buffer->dev_num = dev_num;
	buffer->dev_fd = -1;
	buffer->dev_name = ADC_DEVICE_NAME;
	buffer->data = NULL;
	buffer->buf_dir_name = NULL;
	buffer->scan_el_dir = NULL;
	buffer->dev = NULL;
	buffer->dev_dir_name = NULL;

	ret = ti_adc_get_buf_dir_name(buffer, dev_num);
	if (ret < 0) {
		ti_adc_buffer_close(buffer);
		return NULL;
	}
	printf("Found buffer directory name: %s\n", buffer->buf_dir_name);
	ret = ti_adc_get_scan_elements_dir(buffer, dev_num);
	if (ret < 0) {
		ti_adc_buffer_close(buffer);
		return NULL;
	}
	printf("Found scan elements directory: %s\n", buffer->scan_el_dir);
	ret = asprintf(&buffer->dev, "/dev/iio:device%d", dev_num);
	if (ret < 0) {
		ti_adc_buffer_close(buffer);
		return NULL;
	}
	printf("Found iio device file: %s\n", buffer->dev);

	if (length > 0) {
		buffer->buf_len = length;
	} else {
		buffer->buf_len = 1024;//a sensible default
	}
	
	printf("Channel mask is: %d\n", channels);
	/* Check bitmask for channel 0 */
	if (IS_CHAN_0_ENABLED(channels)) {
		printf("0: %d & %d = %d\n", CHAN_0, channels, CHAN_0 & channels);
		ti_adc_buffer_channel_enable(buffer, 0);
	}
	/* Check bitmask for channel 1 */
	if (IS_CHAN_1_ENABLED(channels)) {
		printf("l: %d & %d = %d\n", CHAN_1, channels, CHAN_1 & channels);
		ti_adc_buffer_channel_enable(buffer, 1);
	}
	/* Check bitmask for channel 2 */
	if (IS_CHAN_2_ENABLED(channels))
		ti_adc_buffer_channel_enable(buffer, 2);
	/* Check bitmask for channel 3 */
	if (IS_CHAN_3_ENABLED(channels))
		ti_adc_buffer_channel_enable(buffer, 3);
	/* Check bitmask for channel 4 */
	if (IS_CHAN_4_ENABLED(channels))
		ti_adc_buffer_channel_enable(buffer, 4);
	/* Check bitmask for channel 5 */
	if (IS_CHAN_5_ENABLED(channels))
		ti_adc_buffer_channel_enable(buffer, 5);
	/* Check bitmask for channel 6 */
	if (IS_CHAN_6_ENABLED(channels))
		ti_adc_buffer_channel_enable(buffer, 6);
	/* Check bitmask for channel 7 */
	if (IS_CHAN_7_ENABLED(channels))
		ti_adc_buffer_channel_enable(buffer, 7);

	asprintf(&buffer->dev_dir_name, "%siio:device%d", iio_dir, dev_num);

	/* Sleep 2ms to hopefully avoid race condition when checking enabled channels */
	usleep((unsigned long) 2000);

	ret = build_channel_array(buffer->dev_dir_name, &buffer->channels, &num_channels);
	if (ret) {
		printf("Problem reading scan element information\n");
		ti_adc_buffer_close(buffer);
		return NULL;
	}
	if (num_channels != buffer->num_channels) {
		printf(
			"Not all requested channels were enabled (only %d out of %d are enabled).\n",
			num_channels,
			buffer->num_channels);
		ti_adc_buffer_close(buffer);
		return NULL;
	}
	buffer->scan_size = ti_adc_size_from_channelarray(buffer->channels, buffer->num_channels);
	buffer->data = malloc(buffer->scan_size * buffer->buf_len);
	if (!buffer->data) {
		printf("Could not allocate memory for buffer data.\n");
		ti_adc_buffer_close(buffer);
		return NULL;
	}

	return buffer;
}

int ti_adc_buffer_open(ti_adc_buffer *buffer) {

	int ret;
	ret = ti_adc_buffer_set_length(buffer, buffer->buf_len);
	if (ret < 0) {
		printf("Could not set buffer length.\n");
		return ret;
	}

	ret = ti_adc_buffer_enable(buffer);
	if (ret < 0) {
		printf("Could not open buffer.\n");
		return ret;
	}

	buffer->dev_fd = open(buffer->dev, O_RDONLY | O_NONBLOCK);
	if (buffer->dev_fd == -1) {
		printf("Could not open buffer device file.\n");
		return -errno;
	}

	return 0;
}

int ti_adc_buffer_close(ti_adc_buffer *buffer) {
	if (buffer->dev_fd > 0) {
		close(buffer->dev_fd);
	}

	ti_adc_buffer_disable(buffer);

	int i, j;
	for (i = 1, j = 0; i <= 128; i *= 2, j++) {
		if ( (i & buffer->channels_en) > 0 ) {
			ti_adc_buffer_channel_disable(buffer, j);
		}
	}

	if (buffer->data)
		free(buffer->data);
	if (buffer->buf_dir_name)
		free(buffer->buf_dir_name);
	if (buffer->scan_el_dir)
		free(buffer->scan_el_dir);
	if (buffer->dev)
		free(buffer->dev);
	if (buffer->dev_dir_name)
		free(buffer->dev_dir_name);
	if (buffer->channels) {
		//disable each channel...
		//free each channel...
	}
	free(buffer);
	buffer = NULL;

	return 0;
}

int ti_adc_buffer_read(ti_adc_buffer *buffer) {

	int read_size = read(buffer->dev_fd,
				buffer->data,
				buffer->buf_len * buffer->scan_size);

	if (read_size >= 0) {
		buffer->read_size = read_size;
		buffer->data_len = buffer->read_size / buffer->scan_size;
		buffer->scan_index = -1;
	}

	return read_size;
}

int ti_adc_buffer_next(ti_adc_buffer *buffer) {

	if (buffer->scan_index + 1 < buffer->data_len) {
		buffer->scan_index++;
		return buffer->scan_index;
	} else {
		return -1;
	}
}

uint16_t ti_adc_buffer_get_current(ti_adc_buffer *buffer, int channel) {
	
	return ti_adc_buffer_get(buffer, buffer->scan_index, channel);
}

uint16_t ti_adc_buffer_get(ti_adc_buffer *buffer, int index, int channel) {

	//WARNING! buffer->channels[channel_num] channel_num might not be the 
	//actual name of the channel. 
	return ti_adc_conv_scan_item(
		*(uint16_t *)(buffer->data + buffer->scan_size * index + buffer->channels[channel].location),
		&buffer->channels[channel]);
}


int ti_adc_size_from_channelarray(struct iio_channel_info *channels, int num_channels) {

	int bytes = 0;
	int i = 0;

	while (i < num_channels) {
		if (bytes % channels[i].bytes == 0)
			channels[i].location = bytes;
		else
			channels[i].location = bytes - bytes%channels[i].bytes
				+ channels[i].bytes;
		bytes = channels[i].location + channels[i].bytes;
		i++;
	}
	return bytes;

}

uint16_t ti_adc_conv_scan_item(int input, struct iio_channel_info *info) {

	/* First swap if incorrect endian */
	if (info->be)
		input = be16toh((uint16_t)input);
	else
		input = le16toh((uint16_t)input);

	/*
	 * Shift before conversion to avoid sign extension
	 * of left aligned data
	 */
	input >>= info->shift;
	uint16_t val = input;

	val &= (1 << info->bits_used) - 1;

	return val;
}
