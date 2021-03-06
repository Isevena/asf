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
#include "php_asf.h"
#include "kernel/asf_namespace.h"
#include "asf_exception.h"

#include "db/asf_db_abstractadapter.h"
#include "db/adapter/asf_db_adapter_sqlite.h"

zend_class_entry *asf_db_adapter_sqlite_ce;

void asf_db_adapter_sqlite_instance(asf_db_t *this_ptr) /* {{{ */
{
    if (Z_ISUNDEF_P(this_ptr)) {
        object_init_ex(this_ptr, asf_db_adapter_sqlite_ce);
    }
}
/* }}} */

ASF_INIT_CLASS(db_adapter_sqlite) /* {{{ */
{
    ASF_REGISTER_CLASS_INTERNAL(asf_db_adapter_sqlite, Asf_Db_Adapter_Sqlite, Asf\\Db\\Adapter\\Sqlite, asf_absadapter_ce, NULL);

    zend_declare_property_string(asf_db_adapter_sqlite_ce, ZEND_STRL("_type"), ASF_DB_TYPE_SQLTILE, ZEND_ACC_PROTECTED);

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
