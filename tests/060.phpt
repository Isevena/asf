--TEST--
Check for Asf_Logger
--SKIPIF--
<?php if (!extension_loaded("asf")) print "skip"; ?>
--INI--
asf.use_namespace=0
--FILE--
<?php
error_reporting(0);

echo '-----error type-----', PHP_EOL;
var_dump(Asf_Logger::init(''));
var_dump(Asf_Logger::init(111));
var_dump(Asf_Logger::init('./222'));
var_dump(Asf_Logger::init('////'));
var_dump(Asf_Logger::init('/tmp/abcdefg/'));

echo '-----getInstance-----', PHP_EOL;
var_dump(is_object(Asf_Logger::init('/tmp/asf.logger.0.txt')));
var_dump(is_object(Asf_Logger::init('/tmp/asf.logger.0.txt')));
var_dump(is_object(Asf_Logger::init('/tmp/asf.logger.1.txt')));

echo '-----write log-----', PHP_EOL;
var_dump(Asf_Logger::init('/tmp/asf.logger.1.txt')->emergency('hello world'));
var_dump(Asf_Logger::init('/tmp/asf.logger.1.txt')->alert('hello world'));
var_dump(Asf_Logger::init('/tmp/asf.logger.1.txt')->critical('hello world'));
var_dump(Asf_Logger::init('/tmp/asf.logger.1.txt')->error('hello world'));
var_dump(Asf_Logger::init('/tmp/asf.logger.1.txt')->warning('hello world'));
var_dump(Asf_Logger::init('/tmp/asf.logger.1.txt')->notice('hello world'));
var_dump(Asf_Logger::init('/tmp/asf.logger.1.txt')->info('hello world'));
var_dump(Asf_Logger::init('/tmp/asf.logger.1.txt')->debug('hello world'));

echo '-----is_array-----', PHP_EOL;
var_dump(is_array(Asf_Logger::getHandles()));
?>
--EXPECTF--
-----error type-----
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
-----getInstance-----
bool(true)
bool(true)
bool(true)
-----write log-----
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
-----is_array-----
bool(true)

