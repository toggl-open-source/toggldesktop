#! /usr/bin/env perl
# Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html


use strict;
use warnings;

use OpenSSL::Test;

setup("test_provider");

plan tests => 9;

 SKIP: {
     skip "No default provider?", 6
         unless ok(run(app([qw(openssl provider default)])),
                   "try running 'openssl provider default'");

     my $prev = 2;              # The amount of lines from -v
     my @checks = qw( -v -vv -vvv );
     my %op = ( -v => '==',
                -vv => '>',
                -vvv => '>' );
     my $i = 0;

     foreach (@checks) {
         my @cmd = ('openssl', 'provider', $_, 'default');
         my @lines = ( map { (my $x = $_) =~ s|\R$||; $x }
                       run(app([@cmd]), capture => 1) );

         my $curr = scalar @lines;
         my $cmp = "$curr $op{$_} $prev";

         ok(eval $cmp,
            "'openssl provider $_ default' line count $op{$_} $prev");
         ok($lines[0] eq '[ default ]',
            "'openssl provider -v default' first line is '[ default ]'");

         $prev = $curr;
     }
}

 SKIP: {
     skip "No null provider?", 1
         unless ok(run(app([qw(openssl provider null)])),
                   "try running 'openssl provider null'");

     my @cmd = ('openssl', 'provider', '-vvv', 'null');
     my @lines = ( map { (my $x = $_) =~ s|\R$||; $x }
                   run(app([@cmd]), capture => 1) );

     my $curr = scalar @lines;
     my $cmp = "$curr == 1";
     ok(eval $cmp,
        "'openssl provider $_ default' line count == 1");
}

