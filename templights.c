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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_templights.h"
#include "pcre/php_pcre.h"
#include <ctype.h>
#include "ext/standard/php_smart_str.h"

#define VERSION "0.1"

static int le_templights;
ZEND_DECLARE_MODULE_GLOBALS(templights);

/*static function*/
static char* tpl_read(char *filename)
{
	char *contents, *real_filename;
	int len;
	php_stream *stream;

	int path_len = strlen(TEMPLIGHTS_G(templates_path));
	if (path_len != 0)
	{
		int filename_len = strlen(filename);
		int real_filename_len = path_len + filename_len + 1;
		if (*(TEMPLIGHTS_G(templates_path)+path_len-1) == '/')
		{
			real_filename = (char*)emalloc(real_filename_len);
			strncpy(real_filename, TEMPLIGHTS_G(templates_path), path_len);
			strncpy(real_filename+path_len, filename, filename_len);
			real_filename[real_filename_len-1] = '\0';
		}
		else
		{
			real_filename_len += 1;
			real_filename = (char*)emalloc(real_filename_len);
			strncpy(real_filename, TEMPLIGHTS_G(templates_path), path_len);
			real_filename[path_len] = '/';
			strncpy(real_filename+path_len+1, filename, filename_len);
			real_filename[real_filename_len-1] = '\0';
		}
	}
	else
	{
		real_filename = filename;
	}
	
	stream = php_stream_open_wrapper(real_filename, "rb", REPORT_ERRORS, NULL);
	if (!stream)
	{
		return NULL;
	}

	if ((len = php_stream_copy_to_mem(stream, &contents, PHP_STREAM_COPY_ALL, 0)) > 0)
	{
		return contents;
	}
	else
	{
		return NULL;
	}
}

#define TOK_REGEX "/(\\{%)(.+)\\{|\\}(%\\})|\\{%%[\\S\\s]*?%%\\}|(\\{%)(.+)(%\\})/"
static zval* tpl_token(char *contents)
{
	pcre_cache_entry *pce;
	long limit_val = -1;
	long flags = 1|2;
	zval *ret;
	
	MAKE_STD_ZVAL(ret);

	if ((pce = pcre_get_compiled_regex_cache(TOK_REGEX, strlen(TOK_REGEX) TSRMLS_CC)) == NULL)
	{
		return ret;
	}

	php_pcre_split_impl(pce, contents, strlen(contents), ret, limit_val, flags TSRMLS_CC);
	
	//return ret;
	zval *tmp; MAKE_STD_ZVAL(tmp); ZVAL_ZVAL(tmp, ret, 1, 0); efree(ret); return(tmp);
}

static zval** array_shift(zval *zvl)
{
	zval **entry;
	ulong index;
	char *key = NULL;
	uint key_len = 0;

	if (zend_hash_num_elements(Z_ARRVAL_P(zvl)) == 0)
	{
		return NULL;
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(zvl));
	zend_hash_get_current_data(Z_ARRVAL_P(zvl), (void**)&entry);

	zend_hash_get_current_key_ex(Z_ARRVAL_P(zvl), &key, &key_len, &index, 0, NULL);
	zend_hash_del_key_or_index(Z_ARRVAL_P(zvl), key, key_len, index, (key) ? HASH_DEL_KEY : HASH_DEL_INDEX);

	unsigned int k = 0;
	int should_rehash = 0;
	Bucket *p = Z_ARRVAL_P(zvl)->pListHead;
	while (p != NULL) {
		if (p->nKeyLength == 0)
		{
			if (p->h != k)
			{
				p->h = k++;
				should_rehash = 1;
			}
			else {
				k++;
			}
		}
		p = p->pListNext;
	}
	Z_ARRVAL_P(zvl)->nNextFreeElement = k;
	if (should_rehash)
	{
		zend_hash_rehash(Z_ARRVAL_P(zvl));
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(zvl));

	return entry;
	efree(*entry);	
}

static zval* tpl_parse(zval* tokens, zval *nodelist)
{
	zval **entry;
	char *token;
	//zval *nodelist;
	if (nodelist == NULL)
	{
		MAKE_STD_ZVAL(nodelist);
		array_init(nodelist);
	}
	
	while((entry = array_shift(tokens)) != NULL)
	{
		if (strncmp(Z_STRVAL_PP(entry), "%}", Z_STRLEN_PP(entry)) == 0) break;
		if (strncmp(Z_STRVAL_PP(entry), "{%", Z_STRLEN_PP(entry)) == 0)
		{
			entry = array_shift(tokens);
			//char *name = trim(Z_STRVAL_PP(entry));
			char *name = php_trim(Z_STRVAL_PP(entry), Z_STRLEN_PP(entry), NULL, 0, NULL, 3); 
			char c = name[0];
			if (isupper(c))
			{
				add_assoc_zval(nodelist, name, tpl_parse(tokens, NULL));
			}
			else
			{
				zval *parents = tpl_parse(tpl_token(tpl_read(name)), NULL);
				//zval *parents;MAKE_STD_ZVAL(parents);ZVAL_ZVAL(parents, temp, 1, 0);
				nodelist = tpl_parse(tokens, parents);
			}
		}
		else {
			char *contents = php_trim(Z_STRVAL_PP(entry), Z_STRLEN_PP(entry), NULL, 0, NULL, 3); 
			if (contents != "")
			{
				add_next_index_string(nodelist, contents, 1); 
			}
		}
	}

	//efree_PP(entry);
	return nodelist;
}

static char* tpl_tostring(zval *zvl)
{
	zval **entry;
	smart_str result = {0};
	while ((entry = array_shift(zvl)))
	{
		if (Z_TYPE_PP(entry) == IS_ARRAY)
		{
			zval *temp;
			MAKE_STD_ZVAL(temp);
			ZVAL_ZVAL(temp, *entry, 1, 0);
			char *tstr = tpl_tostring(temp);
			smart_str_appendl(&result, tstr, strlen(tstr));
			efree(temp);
		}
		else
		{
			smart_str_appendl(&result, Z_STRVAL_PP(entry), Z_STRLEN_PP(entry));
			//strcat(ret, Z_STRVAL_PP(entry));
		}
	}
	smart_str_0(&result);
	char *ret = emalloc(result.len+1);
	strncpy(ret, result.c, result.len);
	ret[result.len] = '\0';
	smart_str_free(&result);
	return ret;
}

const zend_function_entry templights_functions[] = {
	PHP_FE(tpl_path, NULL)
	PHP_FE(tpl_render, NULL)
	{NULL, NULL, NULL}
};

zend_module_entry templights_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"templights",
	templights_functions,
	PHP_MINIT(templights),
	PHP_MSHUTDOWN(templights),
	PHP_RINIT(templights),
	PHP_RSHUTDOWN(templights),
	PHP_MINFO(templights),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", 
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_TEMPLIGHTS
ZEND_GET_MODULE(templights)
#endif

static void php_templights_globals_ctor(zend_templights_globals *templights_globals TSRMLS_DC)
{
	templights_globals->templates_path = (char*)malloc(100);
}

static void php_templights_globals_dtor(zend_templights_globals *templights_globals TSRMLS_DC)
{
	free(templights_globals->templates_path);
}

PHP_MINIT_FUNCTION(templights)
{
#ifdef ZTS
	ts_allocate_id(&templights_global_id, sizeof(zend_templights_globals), (ts_allocate_ctor)php_templights_globals_ctor, (ts_allocate_dtor)php_templights_globals_dtor);
#else
	php_templights_globals_ctor(&templights_globals TSRMLS_CC);
#endif
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(templights)
{
#ifndef ZTS
	php_templights_globals_dtor(&templights_globals TSRMLS_CC);
#endif
	return SUCCESS;
}

PHP_RINIT_FUNCTION(templights)
{
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(templights)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(templights)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "templights support", "enabled");
	php_info_print_table_row(2, "Version", VERSION);
	php_info_print_table_end();
}

PHP_FUNCTION(tpl_path)
{
	char *path;
	int path_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE)
	{
		RETURN_FALSE;
	}

	if (path_len > 100)
	{
		php_error_docref(NULL, E_ERROR, "the path's length is to long");
		RETURN_FALSE;
	}

	strcpy(TEMPLIGHTS_G(templates_path), path);
	RETURN_TRUE;
}

PHP_FUNCTION(tpl_render)
{
	char *contents, *code, *filename;
	zval *temp, *ret, *input = NULL;
	int code_len, filename_len;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|a", &filename, &filename_len, &input) == FAILURE)
	{
		RETURN_FALSE;
	}

	MAKE_STD_ZVAL(ret);
	char *content = tpl_read(filename);
	if (!content)
	{
		RETURN_FALSE;
	}
	zval *item = tpl_parse(tpl_token(content), NULL);
	ZVAL_ZVAL(ret, item, 1, 0);
	char *str = tpl_tostring(ret);
	efree(ret);

	code_len = strlen("?>") + strlen(str) + 1;
	code = (char*)emalloc(code_len);
	if (!code)
	{
		RETURN_FALSE;
	}
	strncpy(code, "?>", strlen("?>"));
	strncpy(code+strlen("?>"), str, strlen(str));
	code[code_len-1] = '\0';

	/*extract the input, class the function extract*/
	if (input)
	{
		zend_fcall_info fci;
		zend_fcall_info_cache fcic = empty_fcall_info_cache;

		zval *func; 
		zval *ret_ptr = NULL;
		MAKE_STD_ZVAL(func);
		ZVAL_STRING(func, "extract", 1);
		zval **param[1];
		param[0] = &input;

		if(call_user_function_ex(EG(function_table), NULL, func, &ret_ptr, 1, param, 0, EG(active_symbol_table)) == FAILURE)
		{
			efree(func);
			RETURN_FALSE;
		}
		efree(func);
	}

	/*eval the script string*/
	zend_eval_string_ex(code, NULL, "My line", 1);
	RETURN_TRUE;
	efree(code);
}
//EOF;
