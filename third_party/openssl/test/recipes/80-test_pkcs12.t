#! /usr/bin/env perl
# Copyright 2016-2018 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html

use strict;
use warnings;

use OpenSSL::Test qw/:DEFAULT srctop_file/;
use OpenSSL::Test::Utils;

use Encode;

setup("test_pkcs12");

plan skip_all => "The PKCS12 command line utility is not supported by this OpenSSL build"
    if disabled("des");

my $pass = "σύνθημα γνώρισμα";

my $savedcp;
if (eval { require Win32::API; 1; }) {
    # Trouble is that Win32 perl uses CreateProcessA, which
    # makes it problematic to pass non-ASCII arguments, from perl[!]
    # that is. This is because CreateProcessA is just a wrapper for
    # CreateProcessW and will call MultiByteToWideChar and use
    # system default locale. Since we attempt Greek pass-phrase
    # conversion can be done only with Greek locale.

    Win32::API->Import("kernel32","UINT GetSystemDefaultLCID()");
    if (GetSystemDefaultLCID() != 0x408) {
        plan skip_all => "Non-Greek system locale";
    } else {
        # Ensure correct code page so that VERBOSE output is right.
        Win32::API->Import("kernel32","UINT GetConsoleOutputCP()");
        Win32::API->Import("kernel32","BOOL SetConsoleOutputCP(UINT cp)");
        $savedcp = GetConsoleOutputCP();
        SetConsoleOutputCP(1253);
        $pass = Encode::encode("cp1253",Encode::decode("utf-8",$pass));
    }
} elsif ($^O eq "MSWin32") {
    plan skip_all => "Win32::API unavailable";
} else {
    # Running MinGW tests transparently under Wine apparently requires
    # UTF-8 locale...

    foreach(`locale -a`) {
        s/\R$//;
        if ($_ =~ m/^C\.UTF\-?8/i) {
            $ENV{LC_ALL} = $_;
            last;
        }
    }
}
$ENV{OPENSSL_WIN32_UTF8}=1;

plan tests => 5;

# Test different PKCS#12 formats
ok(run(test(["pkcs12_format_test"])), "test pkcs12 formats");

# just see that we can read shibboleth.pfx protected with $pass
ok(run(app(["openssl", "pkcs12", "-noout",
            "-password", "pass:$pass",
            "-in", srctop_file("test", "shibboleth.pfx")])),
   "test_pkcs12");

my @path = qw(test certs);
my $tmpfile = "tmp.p12";

# Test the -chain option with -untrusted
ok(run(app(["openssl", "pkcs12", "-export", "-chain",
            "-CAfile",  srctop_file(@path,  "sroot-cert.pem"),
            "-untrusted", srctop_file(@path, "ca-cert.pem"),
            "-in", srctop_file(@path, "ee-cert.pem"),
            "-nokeys", "-passout", "pass:", "-out", $tmpfile])),
   "test_pkcs12_chain_untrusted");

# Test the -passcerts option
ok(run(app(["openssl", "pkcs12", "-export",
            "-in", srctop_file(@path, "ee-cert.pem"),
            "-certfile", srctop_file(@path, "v3-certs-TDES.p12"),
            "-passcerts", "pass:v3-certs",
            "-nokeys", "-passout", "pass:v3-certs", "-descert",
            "-out", $tmpfile])),
   "test_pkcs12_passcert");
unlink $tmpfile;

# Test reading legacy PKCS#12 file
ok(run(app(["openssl", "pkcs12", "-export",
            "-in", srctop_file(@path, "v3-certs-RC2.p12"),
            "-passin", "pass:v3-certs",
            "-provider", "default", "-provider", "legacy",
            "-nokeys", "-passout", "pass:v3-certs", "-descert",
            "-out", $tmpfile])),
   "test_pkcs12_passcert");
unlink $tmpfile;

SetConsoleOutputCP($savedcp) if (defined($savedcp));
