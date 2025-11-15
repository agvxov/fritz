#!/usr/bin/perl

use strict;
use warnings;
use FCGI;

my $socket_path = "parrot-pl.sock";
my $socket = FCGI::OpenSocket($socket_path, 5);  # backlog 5

my $request = FCGI::Request(\*STDIN, \*STDOUT, \*STDERR, \%ENV, $socket);

while ($request->Accept() >= 0) {
    my $body = do { local $/; <STDIN> };
    print $ENV{"CHANNEL"} . "\n";
    print $body;
}
