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

#ifndef PHP_BBB_H
#define PHP_BBB_H

extern zend_module_entry bbb_module_entry;
#define phpext_bbb_ptr &bbb_module_entry

#define PHP_BBB_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_BBB_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_BBB_API __attribute__ ((visibility("default")))
#else
#	define PHP_BBB_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(bbb);
PHP_MSHUTDOWN_FUNCTION(bbb);
PHP_RINIT_FUNCTION(bbb);
PHP_RSHUTDOWN_FUNCTION(bbb);
PHP_MINFO_FUNCTION(bbb);

PHP_FUNCTION(confirm_bbb_compiled);	/* For testing, remove later. */
PHP_FUNCTION(setup_adc);
PHP_FUNCTION(adc_read_value);
//PHP_FUNCTION(adc_read_raw);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/

ZEND_BEGIN_MODULE_GLOBALS(bbb)
	int	is_adc_initialized;
	int	is_pwm_initialized;
	char	adc_prefix_dir[40];
ZEND_END_MODULE_GLOBALS(bbb)

/* In every utility function you add that needs to use variables 
   in php_bbb_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as BBB_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define BBB_G(v) TSRMG(bbb_globals_id, zend_bbb_globals *, v)
#else
#define BBB_G(v) (bbb_globals.v)
#endif

#endif	/* PHP_BBB_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
