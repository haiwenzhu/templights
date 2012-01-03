/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:bug <bugwhen@gmail.com>                                       |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_TEMPLIGHTS_H
#define PHP_TEMPLIGHTS_H

extern zend_module_entry templights_module_entry;
#define phpext_templights_ptr &templights_module_entry

#ifdef PHP_WIN32
#	define PHP_TEMPLIGHTS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_TEMPLIGHTS_API __attribute__ ((visibility("default")))
#else
#	define PHP_TEMPLIGHTS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(templights);
PHP_MSHUTDOWN_FUNCTION(templights);
PHP_RINIT_FUNCTION(templights);
PHP_RSHUTDOWN_FUNCTION(templights);
PHP_MINFO_FUNCTION(templights);

PHP_FUNCTION(tpl_path);
PHP_FUNCTION(tpl_render);

ZEND_BEGIN_MODULE_GLOBALS(templights)
	char *templates_path;
ZEND_END_MODULE_GLOBALS(templights)

#ifdef ZTS
#define TEMPLIGHTS_G(v) TSRMG(templights_globals_id, zend_templights_globals *, v)
#else
#define TEMPLIGHTS_G(v) (templights_globals.v)
#endif

#endif	/* PHP_TEMPLIGHTS_H */
