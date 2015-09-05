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
#include "c_adc.h"

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
	PHP_FE(setup_adc,	NULL)
	PHP_FE(adc_read_value,	NULL)
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
	adc_setup();

	RETURN_TRUE;
}

PHP_FUNCTION(adc_read_value)
{
	unsigned int ain;
	float value;
	int success, arg_len;
	char *channel = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &channel, &arg_len) == FAILURE) {
		return;
	}

	// check setup was called prior
	if (!adc_initialized)
	{
	    //PyErr_SetString(PyExc_RuntimeError, "You must setup() ADC prior to calling read.");
	    return;
	}    

	if (!get_adc_ain(channel, &ain)) {
	    //PyErr_SetString(PyExc_ValueError, "Invalid AIN key or name.");
	    return;    
	}

	success = read_value(ain, &value);

	if (success == -1) {
	    //PyErr_SetFromErrnoWithFilename(PyExc_IOError, "Error while reading AIN port. Invalid or locked AIN file.");
	    return NULL;
	}

	//scale modifier
	value = value / 1800.0;

	RETURN_DOUBLE((double) value);
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
