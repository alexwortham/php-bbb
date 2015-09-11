/*
 * smbusmodule.c - Python bindings for Linux SMBus access through i2c-dev
 * Copyright (C) 2005-2007 Mark M. Hoffman <mhoffman@lightlink.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//#include "structmember.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

/*
** These are required to build this module against Linux older than 2.6.23.
*/
#ifndef I2C_SMBUS_I2C_BLOCK_BROKEN
#undef I2C_SMBUS_I2C_BLOCK_DATA
#define I2C_SMBUS_I2C_BLOCK_BROKEN	6
#define I2C_SMBUS_I2C_BLOCK_DATA	8
#endif

#define MAX_ERR_LEN	256
#define SMBus_SUCCESS(self) do { \
	self->last_error[0] = '\0'; \
	return self; \
} while(0)

typedef struct _SMBus_block_node {
	struct _SMBus_block_node *next;
	struct _SMBus_block_node *prev;
	long value;
} SMBus_block_node;

typedef struct {
	int fd;		/* open file descriptor: /dev/i2c-?, or -1 */
	int addr;	/* current client SMBus address */
	int pec;	/* !0 => Packet Error Codes enabled */
	long last_result;
	SMBus_block_node *block_list;
	SMBus_block_node *block_data;
	int block_list_size;
	char last_error[MAX_ERR_LEN];
} SMBus;

static void
SMBus_add_block_data(SMBus *self, long data) {
	SMBus_block_node *new_node = malloc(sizeof(SMBus_block_node));
	new_node->value = data;
	new_node->next = NULL;
	new_node->prev = NULL;
	
	if (self->block_list == NULL) {
		self->block_list = new_node;
		self->block_list_size = 1;
	} else {
		if (self->block_list->prev == NULL) {
			self->block_list->prev = new_node;
			self->block_list->next = new_node;
			new_node->next = self->block_list;
			new_node->prev = self->block_list;
		} else {
			SMBus_block_node *tail = self->block_list->prev;
			self->block_list->prev = new_node;
			tail->next = new_node;
			new_node->next = self->block_list;
			new_node->prev = tail;
		}
		self->block_list_size++;
	}

	return;
}

static long
SMBus_read_next_block(SMBus *self) {
	if (self->block_list == NULL) {
		snprintf(self->last_error, MAX_ERR_LEN, "EMPTY");
		return (long) -1;
	}

	if (self->block_data == NULL) {
		self->block_data = self->block_list;
		return self->block_data->value;
	}

	self->block_data = self->block_data->next;
	if (self->block_data == NULL) {
		snprintf(self->last_error, MAX_ERR_LEN, "EOL");
		return (long) -1;
	}

	return self->block_data->value;
}

static void
SMBus_close_block_data(SMBus *self) {
	if (self->block_list == NULL) return;

	if (self->block_list->prev != NULL) {
		self->block_list->prev->next = NULL;
	}

	SMBus_block_node *curr = self->block_list;
	SMBus_block_node *next = curr->next;
	while (next != NULL) {
		free(curr);
		curr = next;
		next = next->next;
	}
	free(curr);

	self->block_list = NULL;
	self->block_data = NULL;
	self->block_list_size = 0;

	return;
}

static SMBus *
SMBus_new()
{
	SMBus *self = malloc(sizeof(SMBus));

	self->fd = -1;
	self->addr = -1;
	self->pec = 0;

	return (SMBus *)self;
}

static SMBus *
SMBus_close(SMBus *self)
{
	if ((self->fd != -1) && (close(self->fd) == -1)) {
		snprintf(self->last_error, MAX_ERR_LEN, "Could not close i2c device.");
		return NULL;
	}

	self->fd = -1;
	self->addr = -1;
	self->pec = 0;

	SMBus_SUCCESS(self);
}

/*
 * Unlike all other functions, a NULL return value indicates success.
*/
static SMBus *
SMBus_dealloc(SMBus *self)
{
	if (SMBus_close(self) == NULL) {
		return self;
	}

	free(self);

	return NULL;
}

#define MAXPATH 16

static SMBus *
SMBus_open(SMBus *self, int bus)
{
	char path[MAXPATH];

	if (snprintf(path, MAXPATH, "/dev/i2c-%d\n", bus) >= MAXPATH) {
		snprintf(self->last_error, MAX_ERR_LEN, "Bus number is invalid.");
		return NULL;
	}

	if ((self->fd = open(path, O_RDWR, 0)) == -1) {
		snprintf(self->last_error, MAX_ERR_LEN, "Could not open i2c device %s\n", path);
		return NULL;
	}

	SMBus_SUCCESS(self);
}

static int
SMBus_init(SMBus *self, int bus)
{
	if (bus >= 0) {
		SMBus_open(self, bus);
	}

	return 0;
}

/*
 * private helper function, 0 => success, !0 => error
 */
static int
SMBus_set_addr(SMBus *self, int addr)
{
	int ret = 0;

	if (self->addr != addr) {
		ret = ioctl(self->fd, I2C_SLAVE, addr);
		self->addr = addr;
	}

	return ret;
}

#define SMBus_SET_ADDR(self, addr) do { \
	if (SMBus_set_addr(self, addr)) { \
		printf("Could not set addr to %d\n", addr); \
		return NULL; \
	} \
} while(0)

static SMBus *
SMBus_write_quick(SMBus *self, int addr)
{
	__s32 result;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_write_quick(self->fd, I2C_SMBUS_WRITE))) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting write_quick to addr %d.\n", addr);
		return NULL;
	}

	SMBus_SUCCESS(self);
}

static SMBus *
SMBus_read_byte(SMBus *self, int addr)
{
	__s32 result;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_read_byte(self->fd)) == -1) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting read_byte from addr %d.\n", addr);
		return NULL;
	}
	
	self->last_result = (long)result;

	SMBus_SUCCESS(self);
}

static SMBus *
SMBus_write_byte(SMBus *self, int addr, int val)
{
	__s32 result;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_write_byte(self->fd, (__u8)val)) == -1) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting to write_byte %d to addr %d.\n", addr, val);
		return NULL;
	}

	SMBus_SUCCESS(self);
}

static SMBus *
SMBus_read_byte_data(SMBus *self, int addr, int cmd)
{
	__s32 result;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_read_byte_data(self->fd, (__u8)cmd)) == -1) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting read_byte_data from addr %d with cmd %d.\n", addr, cmd);
		return NULL;
	}

	self->last_result = (long)result;

	SMBus_SUCCESS(self);
}

static SMBus *
SMBus_write_byte_data(SMBus *self, int addr, int cmd, int val)
{
	__s32 result;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_write_byte_data(self->fd,
				(__u8)cmd, (__u8)val)) == -1) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting to write_byte_data to addr %d with cmd %d and val %d.\n", addr, cmd, val);
		return NULL;
	}

	SMBus_SUCCESS(self);
}

static SMBus *
SMBus_read_word_data(SMBus *self, int addr, int cmd)
{
	__s32 result;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_read_word_data(self->fd, (__u8)cmd)) == -1) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting to read_word_data from addr %d with cmd %d.\n", addr, cmd);
		return NULL;
	}

	self->last_result = (long)result;

	SMBus_SUCCESS(self);
}

static SMBus *
SMBus_write_word_data(SMBus *self, int addr, int cmd, int val)
{
	__s32 result;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_write_word_data(self->fd,
				(__u8)cmd, (__u16)val)) == -1) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting to write_word_data to addr %d with cmd %d and val %d.\n", addr, cmd, val);
		return NULL;
	}

	SMBus_SUCCESS(self);
}

static SMBus *
SMBus_process_call(SMBus *self, int addr, int cmd, int val)
{
	__s32 result;

	SMBus_SET_ADDR(self, addr);

	if ((result = i2c_smbus_process_call(self->fd,
				(__u8)cmd, (__u16)val)) == -1) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting process_call to addr %d with cmd %d and val %d.\n", addr, cmd, val);
		return NULL;
	}

	SMBus_SUCCESS(self);
}

/*
 * private helper function; returns a new list of integers
 */
static void
SMBus_buf_to_list(SMBus *self, __u8 const *buf, int len)
{
	int ii;
	/*
	 * I'm assuming that if you are calling this you have done whatever
	 * you needed to do with the previous block data and no longer need it.
	 */
	SMBus_close_block_data(self);

	for (ii = 0; ii < len; ii++) {
		SMBus_add_block_data(self, (long)buf[ii]);
	}
	return;
}

static SMBus *
SMBus_read_block_data(SMBus *self, int addr, int cmd)
{
	union i2c_smbus_data data;

	SMBus_SET_ADDR(self, addr);

	/* save a bit of code by calling the access function directly */
	if (i2c_smbus_access(self->fd, I2C_SMBUS_READ, (__u8)cmd,
				I2C_SMBUS_BLOCK_DATA, &data)) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting to read_block_data from addr %d with cmd %d.\n", addr, cmd);
		return NULL;
	}

	/* first byte of the block contains (remaining) data length */
	SMBus_buf_to_list(self, &data.block[1], data.block[0]);

	SMBus_SUCCESS(self);
}

/*
 * private helper function: convert an integer list to union i2c_smbus_data
 */
static int
SMBus_list_to_data(SMBus *self, union i2c_smbus_data *data)
{
	int ii, len;
	len = self->block_list_size;
	if (len <= 0) {
		snprintf(self->last_error, MAX_ERR_LEN, "List must have at least 1 integer.\n");
		return 0; /* fail */
	}

	if (len > 32) {
		snprintf(self->last_error, MAX_ERR_LEN, "List must not be more than 32 integers.\n");
		return 0; /* fail */
	}

	/* first byte is the length */
	data->block[0] = (__u8)len;

	
	for (ii = 0; ii < len; ii++) {
		long val = SMBus_read_next_block(self);
		data->block[ii+1] = (__u8)val;
	}

	return 1; /* success */
}

static SMBus *
SMBus_write_block_data(SMBus *self, int addr, int cmd)
{
	union i2c_smbus_data data;

	SMBus_SET_ADDR(self, addr);

	/* save a bit of code by calling the access function directly */
	if (i2c_smbus_access(self->fd, I2C_SMBUS_WRITE, (__u8)cmd,
				I2C_SMBUS_BLOCK_DATA, &data)) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting to write_block_data from addr %d with cmd %d.\n", addr, cmd);
		return NULL;
	}

	SMBus_SUCCESS(self);
}

static SMBus *
SMBus_block_process_call(SMBus *self, int addr, int cmd)
{
	union i2c_smbus_data data;

	SMBus_SET_ADDR(self, addr);

	/* save a bit of code by calling the access function directly */
	if (i2c_smbus_access(self->fd, I2C_SMBUS_WRITE, (__u8)cmd,
				I2C_SMBUS_BLOCK_PROC_CALL, &data)) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting to block_process_call from addr %d with cmd %d.\n", addr, cmd);
		return NULL;
	}

	/* first byte of the block contains (remaining) data length */
	SMBus_buf_to_list(self, &data.block[1], data.block[0]);

	SMBus_SUCCESS(self);
}

static SMBus *
SMBus_read_i2c_block_data(SMBus *self, int addr, int cmd, int len)
{
	union i2c_smbus_data data;

	SMBus_SET_ADDR(self, addr);

	data.block[0] = len;
	/* save a bit of code by calling the access function directly */
	if (i2c_smbus_access(self->fd, I2C_SMBUS_READ, (__u8)cmd,
				len == 32 ? I2C_SMBUS_I2C_BLOCK_BROKEN:
				I2C_SMBUS_I2C_BLOCK_DATA, &data)) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting to read_i2c_block_data from addr %d with cmd %d and len.\n", addr, cmd, len);
		return NULL;
	}

	/* first byte of the block contains (remaining) data length */
	SMBus_buf_to_list(self, &data.block[1], data.block[0]);

	SMBus_SUCCESS(self);
}

static SMBus *
SMBus_write_i2c_block_data(SMBus *self, int addr, int cmd)
{
	union i2c_smbus_data data;

	SMBus_SET_ADDR(self, addr);

	/* save a bit of code by calling the access function directly */
	if (i2c_smbus_access(self->fd, I2C_SMBUS_WRITE, (__u8)cmd,
				I2C_SMBUS_I2C_BLOCK_BROKEN, &data)) {
		snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting to write_i2c_block_data to addr %d with cmd %d.\n", addr, cmd);
		return NULL;
	}

	SMBus_SUCCESS(self);
}

static int
SMBus_get_pec(SMBus *self)
{
	return self->pec;
}

static int
SMBus_set_pec(SMBus *self, int pec)
{
	if (self->pec != pec) {
		if (ioctl(self->fd, I2C_PEC, pec)) {
			snprintf(self->last_error, MAX_ERR_LEN, "An IO error occured while attempting to set_pec.");
			return -1;
		}
		self->pec = pec;
	}

	return 0;
}
