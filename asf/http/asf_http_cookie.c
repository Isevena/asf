/*
  +----------------------------------------------------------------------+
  | API Services Framework                                               |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Jiapeng Fan <fanjiapeng@126.com>                             |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "Zend/zend_interfaces.h" /* zend_call_method* */

#include "php_asf.h"
#include "kernel/asf_namespace.h"
#include "asf_exception.h"
#include "kernel/asf_func.h"

#include "http/asf_http_request.h"
#include "http/asf_http_cookieinterface.h"
#include "http/asf_http_cookie.h"

zend_class_entry *asf_http_cookie_ce;

zend_string *asf_http_cookie_string_extend(zval *cookie, zend_string *key) /* {{{ */
{
    zval *prefix = zend_read_property(Z_OBJCE_P(cookie), cookie, ZEND_STRL(ASF_COOKIE_PRONAME_PREFIX), 1, NULL);

    if (!Z_ISNULL_P(prefix)) {
        zend_string *new_key = zend_string_extend(Z_STR_P(prefix), ZSTR_LEN(key) + Z_STRLEN_P(prefix), 0);
        memcpy(ZSTR_VAL(new_key) + Z_STRLEN_P(prefix), ZSTR_VAL(key), ZSTR_LEN(key));
        ZSTR_VAL(new_key)[ZSTR_LEN(new_key)] = '\0';
        
        return new_key;
    }

    return zend_string_copy(key);
}
/* }}} */

/* {{{ proto object Asf_Http_Cookie::__construct(void)
*/
PHP_METHOD(asf_http_cookie, __construct)
{
    zval *configs = NULL, *pzval = NULL, zconfig;
    HashTable *st = NULL, *sz = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|a", &configs) == FAILURE) {
        return;
    }

    if (!configs) {
        return;
    }

    st = Z_ARRVAL_P(configs);

    if (zend_hash_num_elements(st) < 1) {
        return;
    }

    array_init(&zconfig);
    sz = Z_ARRVAL(zconfig);

    if ((pzval = zend_hash_str_find(st, ASF_COOKIE_EXPIRE, ASF_COOKIE_EXPIRE_LEN)) && Z_TYPE_P(pzval) == IS_LONG) {
        add_assoc_long_ex(&zconfig, ASF_COOKIE_EXPIRE, ASF_COOKIE_EXPIRE_LEN, Z_LVAL_P(pzval));
    }

    if ((pzval = zend_hash_str_find(st, ASF_COOKIE_PATH, ASF_COOKIE_PATH_LEN)) && Z_TYPE_P(pzval) == IS_STRING) {
        add_assoc_stringl_ex(&zconfig, ASF_COOKIE_PATH, ASF_COOKIE_PATH_LEN, Z_STRVAL_P(pzval), Z_STRLEN_P(pzval));
    }

    if ((pzval = zend_hash_str_find(st, ASF_COOKIE_DOMAIN, ASF_COOKIE_DOMAIN_LEN)) && Z_TYPE_P(pzval) == IS_STRING) {
        add_assoc_stringl_ex(&zconfig, ASF_COOKIE_DOMAIN, ASF_COOKIE_DOMAIN_LEN, Z_STRVAL_P(pzval), Z_STRLEN_P(pzval));
    }
    
    if ((pzval = zend_hash_str_find(st, ASF_COOKIE_SECURE, ASF_COOKIE_SECURE_LEN)) && Z_TYPE_P(pzval) == IS_LONG) {
        add_assoc_long_ex(&zconfig, ASF_COOKIE_SECURE, ASF_COOKIE_SECURE_LEN, Z_LVAL_P(pzval));
    }
    
    if ((pzval = zend_hash_str_find(st, ASF_COOKIE_HTTPONLY, ASF_COOKIE_HTTPONLY_LEN))) {
        add_assoc_long_ex(&zconfig, ASF_COOKIE_HTTPONLY, ASF_COOKIE_HTTPONLY_LEN, Z_LVAL_P(pzval));
    }
    
    zend_update_property(asf_http_cookie_ce, getThis(), ZEND_STRL(ASF_COOKIE_PRONAME_CONFIG), &zconfig);
    zval_ptr_dtor(&zconfig);
}
/* }}} */

/* {{{ proto bool Asf_Http_Cookie::prefix([string $prefix = ''])
*/
PHP_METHOD(asf_http_cookie, prefix)
{
    zend_string *key = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|S", &key) == FAILURE) {
        return;
    }

    zend_update_property_str(asf_http_cookie_ce, getThis(), ZEND_STRL(ASF_COOKIE_PRONAME_PREFIX), key);

    RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool Asf_Http_Cookie::set(string $name [, mixed $value = '' [, int expire = 0]])
*/
PHP_METHOD(asf_http_cookie, set)
{
    zend_string *key = NULL, *value = NULL;
    zval *self = NULL;
    zend_long lexpire = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|Sl", &key, &value, &lexpire) == FAILURE) {
        return;
    }

    self = getThis();
    zval *path = NULL, *domain = NULL, *expire = NULL, *secure = NULL, *httponly = NULL;
    zend_ulong ctime = time(NULL);
    
    zval *pzval = zend_read_property(asf_http_cookie_ce, self, ZEND_STRL(ASF_COOKIE_PRONAME_CONFIG), 1, NULL);
    if (!Z_ISNULL_P(pzval)) {
        HashTable *ht = Z_ARRVAL_P(pzval);
        
        path      = zend_hash_str_find(ht, ASF_COOKIE_PATH, ASF_COOKIE_PATH_LEN);
        domain    = zend_hash_str_find(ht, ASF_COOKIE_DOMAIN, ASF_COOKIE_DOMAIN_LEN);
        expire    = zend_hash_str_find(ht, ASF_COOKIE_EXPIRE, ASF_COOKIE_EXPIRE_LEN);
        secure    = zend_hash_str_find(ht, ASF_COOKIE_SECURE, ASF_COOKIE_SECURE_LEN);
        httponly  = zend_hash_str_find(ht, ASF_COOKIE_HTTPONLY, ASF_COOKIE_HTTPONLY_LEN);
    }

    zend_string *new_key = asf_http_cookie_string_extend(getThis(), key);

    int ret = php_setcookie(new_key, value,
            lexpire ? (lexpire + ctime) : (expire ? (Z_LVAL_P(expire) + ctime) : 0), 
            (path ? Z_STR_P(path) : NULL),
            (domain ? Z_STR_P(domain) : NULL),
            (secure ? Z_LVAL_P(secure) : 0),
            1,
            (httponly ? Z_LVAL_P(httponly) : 0));

    zend_string_release(new_key);

    if (ret == SUCCESS) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto bool Asf_Http_Cookie::forever(string $name, string $value)
*/
PHP_METHOD(asf_http_cookie, forever)
{
    zend_string *key = NULL, *value = NULL;
    zval *self = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS", &key, &value) == FAILURE) {
        return;
    }

    zval zmn, args[3];
    self = getThis();

    ZVAL_STRINGL(&zmn, "set", 3); 
    ZVAL_STR_COPY(&args[0], key);
    ZVAL_STR_COPY(&args[1], value);
    ZVAL_LONG(&args[2], 315360000);
    
    call_user_function_ex(&Z_OBJCE_P(self)->function_table, self, &zmn, return_value, 3, args, 1, NULL);
    zend_string_release(Z_STR(zmn)); 
    zval_ptr_dtor(&args[0]);
    zval_ptr_dtor(&args[1]);
}
/* }}} */

/* {{{ proto bool Asf_Http_Cookie::has(string $name)
*/
PHP_METHOD(asf_http_cookie, has)
{
    zend_string *key = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &key) == FAILURE) {
        return;
    }

    zval *cookie = asf_http_req_pg_find(TRACK_VARS_COOKIE);
    HashTable *ht = Z_ARRVAL_P(cookie);

    if (zend_hash_num_elements(ht) < 1) {
        RETURN_FALSE;
    }

    zend_string *new_key = asf_http_cookie_string_extend(getThis(), key);

    RETVAL_BOOL(zend_symtable_exists(ht, new_key));
    zend_string_release(new_key);
}
/* }}} */

/* {{{ proto mixed Asf_Http_Cookie::get(string $name)
*/
PHP_METHOD(asf_http_cookie, get)
{
    zend_string *key = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &key) == FAILURE) {
        return;
    }

    if (UNEXPECTED(asf_func_isempty(ZSTR_VAL(key)))) {
        asf_trigger_error(ASF_ERR_COOKIE_PUBLIC, "Parameter 'name' cannot be empty");
        return;
    }

    /* Need to get in real time ? */
    zval *cookie = asf_http_req_pg_find(TRACK_VARS_COOKIE);
    HashTable *ht = Z_ARRVAL_P(cookie);

    if (zend_hash_num_elements(ht) < 1) {
        RETURN_NULL();
    }

    zend_string *new_key = asf_http_cookie_string_extend(getThis(), key);

    zval *ret = zend_symtable_find(ht, new_key);
    zend_string_release(new_key);

    if (ret) {
        RETURN_ZVAL(ret, 1, 0);
    }
    
    RETURN_NULL();
}
/* }}} */

/* {{{ proto bool Asf_Http_Cookie::del(string $name [, string $...])
*/
PHP_METHOD(asf_http_cookie, del)
{
    zval *args = NULL;
    int argc = 0;
    uint i = 0;

    ZEND_PARSE_PARAMETERS_START(1, -1)
        Z_PARAM_VARIADIC('+', args, argc)
    ZEND_PARSE_PARAMETERS_END();

    zval *cookie = asf_http_req_pg_find(TRACK_VARS_COOKIE);
    if (zend_hash_num_elements(Z_ARRVAL_P(cookie)) < 1) {
        RETURN_FALSE;
    }

    zval zmn, *self = getThis(), retval, new_args[1];
    
    ZVAL_STRINGL(&zmn, "set", 3);

    for (i = 0; i < argc; i++) {
        ZVAL_COPY_VALUE(&new_args[0], &args[i]);
        call_user_function_ex(&Z_OBJCE_P(self)->function_table, self, &zmn, &retval, 1, new_args, 1, NULL);
    }

    ZVAL_COPY_VALUE(return_value, &retval);

    zend_string_release(Z_STR(zmn)); 
}
/* }}} */

/* {{{ proto bool Asf_Http_Cookie::clear()
*/
PHP_METHOD(asf_http_cookie, clear)
{
    zval *cookie = asf_http_req_pg_find(TRACK_VARS_COOKIE);
    HashTable *ht = Z_ARRVAL_P(cookie);

    size_t argc = zend_hash_num_elements(ht);
    if (argc < 1) {
        RETURN_FALSE;
    }
    
    zval args[argc], zmn;
    zend_string *key = NULL;
    uint i = 0;
    zval *self = getThis();
    zend_long idx = 0;

    ZEND_HASH_FOREACH_KEY(ht, idx, key) {
        if (key) {
            ZVAL_STR(&args[i++], key);
        } else {
            ZVAL_LONG(&args[i++], idx);
        }
    } ZEND_HASH_FOREACH_END();

    argc = i;

    ZVAL_STRINGL(&zmn, "del", 3); 
    call_user_function_ex(&Z_OBJCE_P(self)->function_table, self, &zmn, return_value, argc, args, 1, NULL);
    zend_string_release(Z_STR(zmn)); 
}
/* }}} */

/* {{{ asf_http_cookie_methods[]
*/
zend_function_entry asf_http_cookie_methods[] = {
    PHP_ME(asf_http_cookie, __construct,    asf_http_cookieinterface_init_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(asf_http_cookie, prefix,         asf_http_cookieinterface_prefix_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(asf_http_cookie, set,            asf_http_cookieinterface_set_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(asf_http_cookie, forever,        asf_http_cookieinterface_set_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(asf_http_cookie, has,            asf_http_cookieinterface_has_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(asf_http_cookie, get,            asf_http_cookieinterface_get_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(asf_http_cookie, del,            asf_http_cookieinterface_del_arginfo, ZEND_ACC_PUBLIC)
    PHP_ME(asf_http_cookie, clear,          NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
/* }}} */

ASF_INIT_CLASS(http_cookie) /* {{{ */
{
    ASF_REGISTER_CLASS_PARENT(asf_http_cookie, Asf_Http_Cookie, Asf\\HTTP\\Cookie, ZEND_ACC_FINAL);

    zend_declare_property_null(asf_http_cookie_ce, ZEND_STRL(ASF_COOKIE_PRONAME_CONFIG), ZEND_ACC_PROTECTED);
    zend_declare_property_null(asf_http_cookie_ce, ZEND_STRL(ASF_COOKIE_PRONAME_PREFIX), ZEND_ACC_PROTECTED);

    ASF_INIT(http_cookieinterface);

    zend_class_implements(asf_http_cookie_ce, 1, asf_http_cookieinterface_ce);

    return SUCCESS;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
