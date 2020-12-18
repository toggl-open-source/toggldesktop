#! /usr/bin/env perl
# Copyright 2015-2020 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html


use strict;
use warnings;

use OpenSSL::Test::Utils;
use OpenSSL::Test qw/:DEFAULT srctop_file/;

setup("test_req");

plan tests => 16;

require_ok(srctop_file('test','recipes','tconversion.pl'));

# What type of key to generate?
my @req_new;
if (disabled("rsa")) {
    @req_new = ("-newkey", "dsa:".srctop_file("apps", "dsa512.pem"));
} else {
    @req_new = ("-new");
    note("There should be a 2 sequences of .'s and some +'s.");
    note("There should not be more that at most 80 per line");
}

# TODO(3.0) This should be removed as soon as missing support is added
# Identified problems:
# - SM2 lacks provider-native keymgmt and decoder
# - ED25519, ED448, X25519 and X448 signature implementations do not
#   respond to the "algorithm-id" parameter request.
my @tmp_loader_hack = qw(-engine loader_attic)
    unless disabled('dynamic-engine') || disabled('deprecated-3.0');

# Check for duplicate -addext parameters, and one "working" case.
my @addext_args = ( "openssl", "req", "-new", "-out", "testreq.pem",
    "-config", srctop_file("test", "test.cnf"), @req_new );
my $val = "subjectAltName=DNS:example.com";
my $val2 = " " . $val;
my $val3 = $val;
$val3 =~ s/=/    =/;
ok( run(app([@addext_args, "-addext", $val])));
ok(!run(app([@addext_args, "-addext", $val, "-addext", $val])));
ok(!run(app([@addext_args, "-addext", $val, "-addext", $val2])));
ok(!run(app([@addext_args, "-addext", $val, "-addext", $val3])));
ok(!run(app([@addext_args, "-addext", $val2, "-addext", $val3])));

subtest "generating alt certificate requests with RSA" => sub {
    plan tests => 3;

    SKIP: {
        skip "RSA is not supported by this OpenSSL build", 2
            if disabled("rsa");

        ok(run(app(["openssl", "req",
                    "-config", srctop_file("test", "test.cnf"),
                    "-section", "altreq",
                    "-new", "-out", "testreq-rsa.pem", "-utf8",
                    "-key", srctop_file("test", "testrsa.pem")])),
           "Generating request");

        ok(run(app(["openssl", "req",
                    "-config", srctop_file("test", "test.cnf"),
                    "-verify", "-in", "testreq-rsa.pem", "-noout"])),
           "Verifying signature on request");

        ok(run(app(["openssl", "req",
                    "-config", srctop_file("test", "test.cnf"),
                    "-section", "altreq",
                    "-verify", "-in", "testreq-rsa.pem", "-noout"])),
           "Verifying signature on request");
    }
};


subtest "generating certificate requests with RSA" => sub {
    plan tests => 2;

    SKIP: {
        skip "RSA is not supported by this OpenSSL build", 2
            if disabled("rsa");

        ok(run(app(["openssl", "req",
                    "-config", srctop_file("test", "test.cnf"),
                    "-new", "-out", "testreq-rsa.pem", "-utf8",
                    "-key", srctop_file("test", "testrsa.pem")])),
           "Generating request");

        ok(run(app(["openssl", "req",
                    "-config", srctop_file("test", "test.cnf"),
                    "-verify", "-in", "testreq-rsa.pem", "-noout"])),
           "Verifying signature on request");
    }
};

subtest "generating certificate requests with DSA" => sub {
    plan tests => 2;

    SKIP: {
        skip "DSA is not supported by this OpenSSL build", 2
            if disabled("dsa");

        ok(run(app(["openssl", "req",
                    "-config", srctop_file("test", "test.cnf"),
                    "-new", "-out", "testreq-dsa.pem", "-utf8",
                    "-key", srctop_file("test", "testdsa.pem")])),
           "Generating request");

        ok(run(app(["openssl", "req",
                    "-config", srctop_file("test", "test.cnf"),
                    "-verify", "-in", "testreq-dsa.pem", "-noout"])),
           "Verifying signature on request");
    }
};

subtest "generating certificate requests with ECDSA" => sub {
    plan tests => 2;

    SKIP: {
        skip "ECDSA is not supported by this OpenSSL build", 2
            if disabled("ec");

        ok(run(app(["openssl", "req",
                    "-config", srctop_file("test", "test.cnf"),
                    "-new", "-out", "testreq-ec.pem", "-utf8",
                    "-key", srctop_file("test", "testec-p256.pem")])),
           "Generating request");

        ok(run(app(["openssl", "req",
                    "-config", srctop_file("test", "test.cnf"),
                    "-verify", "-in", "testreq-ec.pem", "-noout"])),
           "Verifying signature on request");
    }
};

subtest "generating certificate requests with Ed25519" => sub {
    plan tests => 2;

    SKIP: {
        skip "Ed25519 is not supported by this OpenSSL build", 2
            if disabled("ec") || !@tmp_loader_hack;

        ok(run(app(["openssl", "req", @tmp_loader_hack,
                    "-config", srctop_file("test", "test.cnf"),
                    "-new", "-out", "testreq-ed25519.pem", "-utf8",
                    "-key", srctop_file("test", "tested25519.pem")])),
           "Generating request");

        ok(run(app(["openssl", "req", @tmp_loader_hack,
                    "-config", srctop_file("test", "test.cnf"),
                    "-verify", "-in", "testreq-ed25519.pem", "-noout"])),
           "Verifying signature on request");
    }
};

subtest "generating certificate requests with Ed448" => sub {
    plan tests => 2;

    SKIP: {
        skip "Ed448 is not supported by this OpenSSL build", 2
            if disabled("ec") || !@tmp_loader_hack;

        ok(run(app(["openssl", "req", @tmp_loader_hack,
                    "-config", srctop_file("test", "test.cnf"),
                    "-new", "-out", "testreq-ed448.pem", "-utf8",
                    "-key", srctop_file("test", "tested448.pem")])),
           "Generating request");

        ok(run(app(["openssl", "req", @tmp_loader_hack,
                    "-config", srctop_file("test", "test.cnf"),
                    "-verify", "-in", "testreq-ed448.pem", "-noout"])),
           "Verifying signature on request");
    }
};

subtest "generating certificate requests" => sub {
    plan tests => 2;

    ok(run(app(["openssl", "req", "-config", srctop_file("test", "test.cnf"),
                @req_new, "-out", "testreq.pem"])),
       "Generating request");

    ok(run(app(["openssl", "req", "-config", srctop_file("test", "test.cnf"),
                "-verify", "-in", "testreq.pem", "-noout"])),
       "Verifying signature on request");
};

subtest "generating SM2 certificate requests" => sub {
    plan tests => 4;

    SKIP: {
        skip "SM2 is not supported by this OpenSSL build", 4
        if disabled("sm2") || !@tmp_loader_hack;
        ok(run(app(["openssl", "req", @tmp_loader_hack,
                    "-config", srctop_file("test", "test.cnf"),
                    "-new", "-key", srctop_file("test", "certs", "sm2.key"),
                    "-sigopt", "distid:1234567812345678",
                    "-out", "testreq-sm2.pem", "-sm3"])),
           "Generating SM2 certificate request");

        ok(run(app(["openssl", "req", @tmp_loader_hack,
                    "-config", srctop_file("test", "test.cnf"),
                    "-verify", "-in", "testreq-sm2.pem", "-noout",
                    "-vfyopt", "distid:1234567812345678", "-sm3"])),
           "Verifying signature on SM2 certificate request");

        ok(run(app(["openssl", "req", @tmp_loader_hack,
                    "-config", srctop_file("test", "test.cnf"),
                    "-new", "-key", srctop_file("test", "certs", "sm2.key"),
                    "-sigopt", "hexdistid:DEADBEEF",
                    "-out", "testreq-sm2.pem", "-sm3"])),
           "Generating SM2 certificate request with hex id");

        ok(run(app(["openssl", "req", @tmp_loader_hack,
                    "-config", srctop_file("test", "test.cnf"),
                    "-verify", "-in", "testreq-sm2.pem", "-noout",
                    "-vfyopt", "hexdistid:DEADBEEF", "-sm3"])),
           "Verifying signature on SM2 certificate request");
    }
};

my @openssl_args = ("req", "-config", srctop_file("apps", "openssl.cnf"));

run_conversion('req conversions',
               "testreq.pem");
run_conversion('req conversions -- testreq2',
               srctop_file("test", "testreq2.pem"));

sub run_conversion {
    my $title = shift;
    my $reqfile = shift;

    subtest $title => sub {
        run(app(["openssl", @openssl_args,
                 "-in", $reqfile, "-inform", "p",
                 "-noout", "-text"],
                stderr => "req-check.err", stdout => undef));
        open DATA, "req-check.err";
        SKIP: {
            plan skip_all => "skipping req conversion test for $reqfile"
                if grep /Unknown Public Key/, map { s/\R//; } <DATA>;

            tconversion("req", $reqfile, @openssl_args);
        }
        close DATA;
        unlink "req-check.err";

        done_testing();
    };
}
