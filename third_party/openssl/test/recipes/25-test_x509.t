#! /usr/bin/env perl
# Copyright 2015-2020 The OpenSSL Project Authors. All Rights Reserved.
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

setup("test_x509");

plan tests => 12;

require_ok(srctop_file('test','recipes','tconversion.pl'));

my $pem = srctop_file("test/certs", "cyrillic.pem");
my $out_msb = "out-cyrillic.msb";
my $out_utf8 = "out-cyrillic.utf8";
my $msb = srctop_file("test/certs", "cyrillic.msb");
my $utf = srctop_file("test/certs", "cyrillic.utf8");

ok(run(app(["openssl", "x509", "-text", "-in", $pem, "-out", $out_msb,
            "-nameopt", "esc_msb"])));
is(cmp_text($out_msb, srctop_file("test/certs", "cyrillic.msb")),
   0, 'Comparing esc_msb output');
ok(run(app(["openssl", "x509", "-text", "-in", $pem, "-out", $out_utf8,
            "-nameopt", "utf8"])));
is(cmp_text($out_utf8, srctop_file("test/certs", "cyrillic.utf8")),
   0, 'Comparing utf8 output');

 SKIP: {
    skip "DES disabled", 1 if disabled("des");

    my $p12 = srctop_file("test", "shibboleth.pfx");
    my $p12pass = "σύνθημα γνώρισμα";
    my $out_pem = "out.pem";
    ok(run(app(["openssl", "x509", "-text", "-in", $p12, "-out", $out_pem,
                "-passin", "pass:$p12pass"])));
    unlink $out_pem;
}

SKIP: {
    skip "EC disabled", 1 if disabled("ec");

    # producing and checking self-issued (but not self-signed) cert
    my @path = qw(test certs);
    my $subj = "/CN=CA"; # using same DN as in issuer of ee-cert.pem
    my $extfile = srctop_file("test", "v3_ca_exts.cnf");
    my $pkey = srctop_file(@path, "ca-key.pem"); #  issuer private key
    my $pubkey = "ca-pubkey.pem"; # the corresponding issuer public key
    # use any (different) key for signing our self-issued cert:
    my $signkey = srctop_file(@path, "ee-ecdsa-key.pem");
    my $selfout = "self-issued.out";
    my $testcert = srctop_file(@path, "ee-cert.pem");
    ok(run(app(["openssl", "pkey", "-in", $pkey, "-pubout", "-out", $pubkey]))
       &&
       run(app(["openssl", "x509", "-new", "-force_pubkey", $pubkey,
                "-subj", $subj, "-extfile", $extfile,
                "-signkey", $signkey, "-out", $selfout]))
       &&
       run(app(["openssl", "verify", "-no_check_time",
                "-trusted", $selfout, "-partial_chain", $testcert])));
    unlink $pubkey;
    unlink $selfout;
}

subtest 'x509 -- x.509 v1 certificate' => sub {
    tconversion("x509", srctop_file("test","testx509.pem"));
};
subtest 'x509 -- first x.509 v3 certificate' => sub {
    tconversion("x509", srctop_file("test","v3-cert1.pem"));
};
subtest 'x509 -- second x.509 v3 certificate' => sub {
    tconversion("x509", srctop_file("test","v3-cert2.pem"));
};

subtest 'x509 -- pathlen' => sub {
    ok(run(test(["v3ext", srctop_file("test/certs", "pathlen.pem")])));
};

subtest 'x500 -- subjectAltName' => sub {
     my $fp = srctop_file("test/certs", "fake-gp.pem");
     my $out = "ext.out";
     ok(run(app(["openssl", "x509", "-text", "-in", $fp, "-out", $out])));
     ok(has_doctor_id($out));
     unlink $out;
};

sub has_doctor_id { 
    $_ = shift @_;
    open(DATA,$_) or return 0;
    $_= join('',<DATA>); 
    close(DATA);
    return m/2.16.528.1.1003.1.3.5.5.2-1-0000006666-Z-12345678-01.015-12345678/;
}
