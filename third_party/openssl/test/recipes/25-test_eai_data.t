#! /usr/bin/env perl
# Copyright 2019 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html


use strict;
use warnings;

use File::Spec;
use OpenSSL::Test::Utils;
use OpenSSL::Test qw/:DEFAULT srctop_file/;

setup("test_eai_data");

#./util/wrap.pl apps/openssl verify -nameopt utf8 -no_check_time -CAfile test/recipes/25-test_eai_data/ascii_chain.pem test/recipes/25-test_eai_data/ascii_leaf.pem
#./util/wrap.pl apps/openssl verify -nameopt utf8 -no_check_time -CAfile test/recipes/25-test_eai_data/utf8_chain.pem test/recipes/25-test_eai_data/utf8_leaf.pem
#./util/wrap.pl apps/openssl verify -nameopt utf8 -no_check_time -CAfile test/recipes/25-test_eai_data/utf8_chain.pem test/recipes/25-test_eai_data/ascii_leaf.pem
#./util/wrap.pl apps/openssl verify -nameopt utf8 -no_check_time -CAfile test/recipes/25-test_eai_data/ascii_chain.pem test/recipes/25-test_eai_data/utf8_leaf.pem

plan tests => 11;

require_ok(srctop_file('test','recipes','tconversion.pl'));
my $folder = "test/recipes/25-test_eai_data";

my $ascii_pem = srctop_file($folder, "ascii_leaf.pem");
my $utf8_pem  = srctop_file($folder, "utf8_leaf.pem");

my $ascii_chain_pem = srctop_file($folder, "ascii_chain.pem");
my $utf8_chain_pem  = srctop_file($folder, "utf8_chain.pem");

my $out = "san.tmp";

ok(run(app(["openssl", "x509", "-ext", "subjectAltName", "-in", $ascii_pem, "-noout", "-out", $out])));
is(cmp_text($out, srctop_file($folder, "san.ascii")), 0, 'Comparing othername for ASCII domain');

ok(run(app(["openssl", "x509", "-ext", "subjectAltName", "-in", $utf8_pem, "-noout", "-out", $out])));
is(cmp_text($out, srctop_file($folder, "san.utf8")), 0, 'Comparing othername for IDN domain');

unlink $out;

ok(run(app(["openssl", "verify", "-nameopt", "utf8", "-no_check_time", "-verify_email", "学生\@elementary.school.example.com", "-CAfile", $ascii_chain_pem, $ascii_pem])));
ok(run(app(["openssl", "verify", "-nameopt", "utf8", "-no_check_time", "-verify_email", "医生\@大学.example.com", "-CAfile", $utf8_chain_pem, $utf8_pem])));

ok(run(app(["openssl", "verify", "-nameopt", "utf8", "-no_check_time", "-CAfile", $ascii_chain_pem, $ascii_pem])));
ok(run(app(["openssl", "verify", "-nameopt", "utf8", "-no_check_time", "-CAfile", $utf8_chain_pem, $utf8_pem])));

ok(!run(app(["openssl", "verify", "-nameopt", "utf8", "-no_check_time", "-CAfile", $ascii_chain_pem, $utf8_pem])));
ok(!run(app(["openssl", "verify", "-nameopt", "utf8", "-no_check_time", "-CAfile", $utf8_chain_pem,  $ascii_pem])));

