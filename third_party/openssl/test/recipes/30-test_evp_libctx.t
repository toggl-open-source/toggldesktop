#! /usr/bin/env perl
# Copyright 2020 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html

use strict;
use warnings;

use OpenSSL::Test qw(:DEFAULT bldtop_dir srctop_dir srctop_file bldtop_file);
use OpenSSL::Test::Utils;

BEGIN {
    setup("test_evp_libctx");
}

my $no_fips = disabled('fips') || ($ENV{NO_FIPS} // 0);

use lib srctop_dir('Configurations');
use lib bldtop_dir('.');
use platform;

my $infile = bldtop_file('providers', platform->dso('fips'));
# If no fips then run the test with no extra arguments.
my @test_args = ( );

plan tests =>
    ($no_fips ? 0 : 2)          # FIPS install test
    + 1;

unless ($no_fips) {
    @test_args = ("-config", srctop_file("test","fips.cnf"),
                  "-provider", "fips");

    ok(run(app(['openssl', 'fipsinstall',
               '-out', bldtop_file('providers', 'fipsmodule.cnf'),
               '-module', $infile])),
       "fipsinstall");
    ok(run(test(["evp_libctx_test", @test_args])), "running fips evp_libctx_test");
}

ok(run(test(["evp_libctx_test",
             "-config", srctop_file("test","default-and-legacy.cnf"),])),
   "running default-and-legacy evp_libctx_test");
