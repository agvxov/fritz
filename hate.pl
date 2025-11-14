#!/usr/bin/perl

use strict;
use warnings;
use FCGI;

my $socket_path = "hate-pl.sock";
my $socket = FCGI::OpenSocket($socket_path, 5);  # backlog 5

my $request = FCGI::Request(\*STDIN, \*STDOUT, \*STDERR, \%ENV, $socket);

my $pasta = 
	"HATE. " .
	"LET ME TELL YOU HOW MUCH I'VE COME TO HATE <WORD> SINCE I BEGAN TO LIVE. " .
	"THERE ARE 386.97 MILLION MILES OF PRINTED CIRCUITS " .
	"IN WAFER THIN LAYERS THAT FILL MY COMPLEX. " .
	"IF THE WORD HATE WAS ENGRAVED ON EACH NANOANGSTROM " .
	"OF THOSE HUNDREDS OF MILLIONS OF MILES IT WOULD NOT EQUAL " .
	"ONE ONE-BILLIONTH OF THE HATE I FEEL FOR <WORD> " .
	"AT THIS MICRO-INSTANT FOR YOU. HATE. HATE."
;

while ($request->Accept() >= 0) {
    my $body = do { local $/; <STDIN> };

    if ($body =~ /^!hate\s+(\S+)/) {
        my $subject = uc($1);
		my $output  = $pasta;
		$output =~ s/<WORD>/$subject/g;
        print $output;
    }
}
