/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_bbb.h"
#include "ti_am335x_adc_buffer.h"

/* If you declare any globals in php_bbb.h uncomment this:
*/
ZEND_DECLARE_MODULE_GLOBALS(bbb)

/* True global resources - no need for thread safety here */
static int le_bbb;

/* {{{ bbb_functions[]
 *
 * Every user visible function must have an entry in bbb_functions[].
 */
const zend_function_entry bbb_functions[] = {
	PHP_FE(confirm_bbb_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(setup_adc,		NULL)
	PHP_FE(adc_read_value,		NULL)
	PHP_FE(adc_read_raw,		NULL)
	PHP_FE(adc_cleanup,		NULL)
	PHP_FE(adc_buffer_open,		NULL)
	PHP_FE(adc_buffer_close,	NULL)
	PHP_FE(adc_buffer_read,		NULL)
	PHP_FE(i2c_open,		NULL)
	PHP_FE(i2c_close,		NULL)
	PHP_FE(i2c_write_quick,		NULL)
	PHP_FE(i2c_read_byte,		NULL)
	PHP_FE(i2c_write_byte,		NULL)
	PHP_FE(i2c_read_byte_data,	NULL)
	PHP_FE(i2c_write_byte_data,	NULL)
	PHP_FE(i2c_read_word_data,	NULL)
	PHP_FE(i2c_write_word_data,	NULL)
	PHP_FE(i2c_get_last_error,	NULL)
	PHP_FE(lcd_begin,			NULL)
	PHP_FE(lcd_print,			NULL)
	PHP_FE(gpio_setup,		NULL)
	PHP_FE(gpio_output,		NULL)
	PHP_FE(gpio_input,		NULL)
	PHP_FE(gpio_get_mode,		NULL)
	PHP_FE(gpio_cleanup,		NULL)
	//PHP_FE(adc_read_raw,	NULL)
	PHP_FE_END	/* Must be the last line in bbb_functions[] */
};
/* }}} */

/* {{{ bbb_module_entry
 */
zend_module_entry bbb_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"bbb",
	bbb_functions,
	PHP_MINIT(bbb),
	PHP_MSHUTDOWN(bbb),
	PHP_RINIT(bbb),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(bbb),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(bbb),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_BBB_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_BBB
ZEND_GET_MODULE(bbb)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("bbb.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_bbb_globals, bbb_globals)
    STD_PHP_INI_ENTRY("bbb.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_bbb_globals, bbb_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_bbb_init_globals
 */
/* Uncomment this function if you have INI entries
*/
static void php_bbb_init_globals(zend_bbb_globals *bbb_globals)
{
	bbb_globals->is_adc_initialized = 0;
	bbb_globals->is_pwm_initialized = 0;
	bbb_globals->setup_error = 0;
	bbb_globals->module_setup = 0;
	int i;
	for (i=0; i<120; i++) {
		bbb_globals->gpio_direction[i] = -1;
	}
	bbb_globals->lcd = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(bbb)
{
	ZEND_INIT_MODULE_GLOBALS(bbb, php_bbb_init_globals,
NULL);
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(bbb)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(bbb)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(bbb)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(bbb)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "bbb support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_bbb_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_bbb_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "bbb", arg);
	RETURN_STRINGL(strg, len, 0);
}

PHP_FUNCTION(setup_adc)
{
	BBB_G(adc) = ADC_setup();

	RETURN_TRUE;
}

PHP_FUNCTION(adc_read_value)
{
	unsigned int ain;
	double value;
	int success, arg_len;
	long channel;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &channel) == FAILURE) {
		RETURN_NULL();
	}

	// check setup was called prior
	if (!BBB_G(adc)->adc_initialized)
	{
		RETURN_STRING("ADC has not been initialized.  You must call setup_adc() before calling read.", 1);
	}    

	if (channel < 0 || channel > 7) {
		RETURN_STRING("Invalid AIN key or name.", 1);
	}

	if (ADC_read_value(BBB_G(adc), (unsigned int) channel, &value) == NULL) {
		RETURN_STRING("Error while reading AIN port. Invalid or locked AIN file.", 1);
	}

	//scale modifier
	value = value / 4096.0;

	RETURN_DOUBLE((double) value);
}

PHP_FUNCTION(adc_read_raw)
{
	unsigned int ain;
	double value;
	int success, arg_len;
	long channel;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &channel) == FAILURE) {
		RETURN_NULL();
	}

	// check setup was called prior
	if (!BBB_G(adc)->adc_initialized)
	{
		RETURN_STRING("ADC has not been initialized.  You must call setup_adc() before calling read.", 1);
	}    

	if (channel < 0 || channel > 6) {
		RETURN_STRING("Invalid AIN key or name.", 1);
	}

	if (ADC_read_value(BBB_G(adc), (unsigned int) channel, &value) == NULL) {
		RETURN_STRING("Error while reading AIN port. Invalid or locked AIN file.", 1);
	}

	RETURN_DOUBLE((double) value);
}

PHP_FUNCTION(adc_cleanup)
{
	ADC_cleanup(BBB_G(adc));

	RETURN_TRUE;
}

PHP_FUNCTION(adc_buffer_open)
{
	long length, channels;
	ti_adc_buffer *buff = NULL;
	int ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &length, &channels) == FAILURE) {
		RETURN_NULL();
	}

	if (BBB_G(buffer) == NULL) {
		BBB_G(buffer) = ti_adc_buffer_init((int) length, (int) channels);
		buff = BBB_G(buffer);
		if (buff == NULL) {
			RETURN_STRING("Error initializing buffer.", 1);
		}
		ret = ti_adc_buffer_open(buff);
		if (ret < 0) {
			RETURN_STRING("Error opening buffer.", 1);
		}
	}

	RETURN_TRUE;
}

PHP_FUNCTION(adc_buffer_close)
{
	ti_adc_buffer *buff = BBB_G(buffer);

	if (buff != NULL) {
		if (ti_adc_buffer_close(buff) != 0) {
			RETURN_STRING("Error closing buffer.", 1);
		}
	}

	RETURN_TRUE;
}

PHP_FUNCTION(adc_buffer_read)
{
	ti_adc_buffer *buff = BBB_G(buffer);
	int ret, i;
	zval *array;

	if (buff == NULL) {
		RETURN_STRING("Buffer is not open!", 1);
	}

	ret = ti_adc_buffer_read(buff);
	if (ret < 0) {
		RETURN_STRING("Error reading from buffer.", 1);
	}

	ALLOC_INIT_ZVAL(array);
	array_init(array);
	zval **channel = (zval **) emalloc(sizeof(zval *) * buff->num_channels);
	for (i = 0; i < buff->num_channels; i++) {
		ALLOC_INIT_ZVAL(channel[i]);
		array_init(channel[i]);
		add_next_index_zval(array, channel[i]);
	}

	while (ti_adc_buffer_next(buff) != -1) {
		for (i = 0; i < buff->num_channels; i++) {
			add_next_index_long(channel[i], (long) ti_adc_buffer_get_current(buff, i));
		}
	}

	efree(channel);
	/* Let's for now assume not calling dtor will not create a memory leak...
	 * Though I think I understand how that works... I think it probably stands for
	 * "decrement total object references". The only reason I can see that it might
	 * be necessary to call dtor on a freshly created zval is if ALLOC_INIT_ZVAL()
	 * increments the reference count when it shouldn't? That seems silly but might
	 * be the case.
	for (i = 0; i < buff->num_channels; i++) {
		zval_dtor(channel[i]);
	}
	*/

	RETURN_ZVAL(array, 0, 1);
}

#define SMBus_ASSERT_OPEN() do { \
	if (BBB_G(smbus) == NULL) { \
		RETURN_NULL(); \
	} \
} while(0)

#define SMBus_ASSERT_CLOSED() do { \
	if (BBB_G(smbus) != NULL) { \
		RETURN_NULL(); \
	} \
} while(0)

#define SMBus_RETURN_ERROR() do { \
	RETURN_STRING(SMBus_get_last_error(BBB_G(smbus)), 1); \
} while(0)

PHP_FUNCTION(i2c_open)
{
	long bus_num;
	SMBus *_smbus;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &bus_num) == FAILURE) {
		RETURN_NULL();
	}

	SMBus_ASSERT_CLOSED();

	_smbus = SMBus_new();
	BBB_G(smbus) = _smbus;
	if (SMBus_open(_smbus, (int) bus_num) == NULL) {
		SMBus_RETURN_ERROR();
	}

	RETURN_TRUE;
}


PHP_FUNCTION(i2c_close)
{
	SMBus_ASSERT_OPEN();

	if (SMBus_dealloc(BBB_G(smbus)) != NULL) {
		SMBus_RETURN_ERROR();
	}

	BBB_G(smbus) = NULL;

	RETURN_TRUE;
}

PHP_FUNCTION(i2c_write_quick)
{

	SMBus_ASSERT_OPEN();
	long addr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &addr) == FAILURE) {
		RETURN_NULL();
	}

	if (SMBus_write_quick(BBB_G(smbus), (int) addr) == NULL) {
		SMBus_RETURN_ERROR();
	}

	RETURN_TRUE;
}

PHP_FUNCTION(i2c_read_byte)
{
	SMBus_ASSERT_OPEN();
	long addr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &addr) == FAILURE) {
		RETURN_NULL();
	}

	if (SMBus_read_byte(BBB_G(smbus), (int) addr) == NULL) {
		SMBus_RETURN_ERROR();
	}

	RETURN_LONG(BBB_G(smbus)->last_result);
}

PHP_FUNCTION(i2c_write_byte)
{
	SMBus_ASSERT_OPEN();
	long addr, value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &addr, &value) == FAILURE) {
		RETURN_NULL();
	}

	if (SMBus_write_byte(BBB_G(smbus), (int) addr, (int) value) == NULL) {
		SMBus_RETURN_ERROR();
	}

	RETURN_TRUE;
}

PHP_FUNCTION(i2c_read_byte_data)
{
	SMBus_ASSERT_OPEN();
	long addr, cmd;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &addr, &cmd) == FAILURE) {
		RETURN_NULL();
	}

	if (SMBus_read_byte_data(BBB_G(smbus), (int) addr, (int) cmd) == NULL) {
		SMBus_RETURN_ERROR();
	}

	RETURN_LONG(BBB_G(smbus)->last_result);
}

PHP_FUNCTION(i2c_write_byte_data)
{
	SMBus_ASSERT_OPEN();
	long addr, cmd, value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &addr, &cmd, &value) == FAILURE) {
		RETURN_NULL();
	}

	if (SMBus_write_byte_data(BBB_G(smbus), (int) addr, (int) cmd, (int) value) == NULL) {
		SMBus_RETURN_ERROR();
	}

	RETURN_TRUE;
}

PHP_FUNCTION(i2c_read_word_data)
{
	SMBus_ASSERT_OPEN();
	long addr, cmd;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &addr, &cmd) == FAILURE) {
		RETURN_NULL();
	}

	if (SMBus_read_word_data(BBB_G(smbus), (int) addr, (int) cmd) == NULL) {
		SMBus_RETURN_ERROR();
	}

	RETURN_LONG(BBB_G(smbus)->last_result);
}

PHP_FUNCTION(i2c_write_word_data)
{
	SMBus_ASSERT_OPEN();
	long addr, cmd, value;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &addr, &cmd, &value) == FAILURE) {
		RETURN_NULL();
	}

	if (SMBus_write_word_data(BBB_G(smbus), (int) addr, (int) cmd, (int) value) == NULL) {
		SMBus_RETURN_ERROR();
	}

	RETURN_TRUE;
}

PHP_FUNCTION(i2c_get_last_error)
{
	SMBus_ASSERT_OPEN();

	RETURN_STRING(SMBus_get_last_error(BBB_G(smbus)), 1);
}

PHP_FUNCTION(lcd_begin)
{
	SMBus_ASSERT_OPEN();
	long addr;

	LiquidCrystal_I2C *lcd = BBB_G(lcd);

	if (lcd != NULL) {
		RETURN_TRUE;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &addr) == FAILURE) {
		RETURN_NULL();
	}

	lcd = new LiquidCrystal_I2C((uint8_t) addr, 16, 2, BBB_G(smbus));

	lcd->begin(16, 2);
	lcd->backlight();

	BBB_G(lcd) = lcd;

	RETURN_TRUE;
}

PHP_FUNCTION(lcd_print)
{
	LiquidCrystal_I2C *lcd = BBB_G(lcd);
	char *str;
	int str_len;

	if (lcd == NULL) {
		RETURN_NULL();
	}

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
		RETURN_NULL();
    }

	lcd->print(str, str_len);

	RETURN_TRUE;
}

PHP_FUNCTION(gpio_setup)
{
	unsigned int gpio;
	char *channel;
	int len;
	int direction;
	int pud = PUD_OFF;
	int initial = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|ll", &channel, &len, &direction, &pud, &initial) == FAILURE) {
		RETURN_NULL();
	}

	if (direction != INPUT && direction != OUTPUT)
	{
		RETURN_STRING("An invalid direction was passed to setup()", 1);
	}

	if (direction == OUTPUT)
	   pud = PUD_OFF;

	if (pud != PUD_OFF && pud != PUD_DOWN && pud != PUD_UP)
	{
		RETURN_STRING("Invalid value for pull_up_down - should be either PUD_OFF, PUD_UP or PUD_DOWN", 1);
	}

	if (get_gpio_number(channel, &gpio))
	    RETURN_NULL();

	gpio_export(gpio);
	gpio_set_direction(gpio, direction);
	if (direction == OUTPUT) {
	    gpio_set_value(gpio, initial);
	} else {
	    gpio_set_value(gpio, pud);
	}

	BBB_G(gpio_direction)[gpio] = direction;

	RETURN_TRUE;
}

PHP_FUNCTION(gpio_output)
{
	unsigned int gpio;
	int value, len;
	char *channel;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &channel, &len, &value) == FAILURE) {
		RETURN_NULL();
	}

	if (get_gpio_number(channel, &gpio))
		RETURN_NULL();      

	if (BBB_G(gpio_direction)[gpio] != OUTPUT)
	{
		RETURN_STRING("The GPIO channel has not been setup() as an OUTPUT", 1);
	}

	gpio_set_value(gpio, value);

	RETURN_TRUE;
}

PHP_FUNCTION(gpio_input)
{
	unsigned int gpio;
	char *channel;
	unsigned int value;
	int len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &channel, &len) == FAILURE) {
		RETURN_NULL();
	}

	if (get_gpio_number(channel, &gpio))
		RETURN_NULL();

	// check channel is set up as an input or output
	if (BBB_G(gpio_direction)[gpio] != INPUT && BBB_G(gpio_direction)[gpio] != OUTPUT)
	{
		RETURN_STRING("You must setup() the GPIO channel first", 1);
	}

	gpio_get_value(gpio, &value);

	RETURN_LONG((long) value);
}

PHP_FUNCTION(gpio_get_mode)
{
	unsigned int gpio;
	unsigned int value;
	char *channel;
	int len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &channel, &len) == FAILURE) {
		RETURN_NULL();
	}

	if (get_gpio_number(channel, &gpio))
		RETURN_NULL();

	gpio_get_direction(gpio, &value);

	RETURN_LONG((long) value);
}

PHP_FUNCTION(gpio_cleanup)
{

	exports_cleanup();

	RETURN_TRUE;
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
