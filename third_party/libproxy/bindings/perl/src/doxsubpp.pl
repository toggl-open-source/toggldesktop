use strict;
use warnings;
use ExtUtils::MakeMaker;

my $perl = $ARGV[0];
my $in   = $ARGV[1];
my $out  = $ARGV[2];

my $mm = ExtUtils::MakeMaker->new( {
    NAME => 'Libproxy',
    NEEDS_LINKING => 1,
} );

my $perl_include_path = $mm->{PERL_INC};
my @xsubinfo = split "\n", $mm->tool_xsubpp();

my $xsubppdir = (map{ my $foo = $_; $foo =~ s/XSUBPPDIR = //; $foo } grep{ m/^XSUBPPDIR =/ } @xsubinfo)[0];
my $xsubpp = "$xsubppdir/xsubpp";

my $xsubppargs = (map{ my $foo = $_; $foo =~ s/XSUBPPARGS = //; $foo } grep{ m/^XSUBPPARGS =/ } @xsubinfo)[0];

my $cmd = "$perl $xsubpp $xsubppargs -typemap typemap $in > $out";
system $cmd;
