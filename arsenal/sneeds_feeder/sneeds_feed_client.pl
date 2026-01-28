#!/usr/bin/perl

use warnings;
use strict;
use feature 'signatures';
use File::Slurp 'slurp', 'write_file';
use XML::Feed;
use Data::Dumper;
use FCGI;

#my $feed_uri = 'http://127.0.0.1:5050/myfeed.atom';
my $feed_uri = 'https://www.youtube.com/feeds/videos.xml?channel_id=UCD6VugMZKRhSyzWEWA9W2fg';

my $socket_path = "sneeds_feed_client-pl.sock";
my $socket      = FCGI::OpenSocket($socket_path, 5);  # backlog 5
my $request     = FCGI::Request(\*STDIN, \*STDOUT, \*STDERR, \%ENV, $socket);

sub ensure_file ($filepath) {
    open my $fh, '>>', $filepath
        or die "Cannot create $filepath: $!"
        unless -e $filepath;
}

sub announce ($entry) {
    print "# Check'em: " . $entry->title . "\n";
}

sub poll_feed {
    my $history_file = 'history.txt';
	ensure_file $history_file;
    my %history = map { $_ => 1 } slurp($history_file, chomp => 1);

    my $feed = XML::Feed->parse(URI->new($feed_uri))
        or die XML::Feed->errstr;

    for my $entry ($feed->entries) {
        if (!exists $history{$entry->id}) {
            announce $entry;
            $history{$entry->id} = 1;
        }
    }

    write_file($history_file, map { "$_\n" } keys %history);
}

while ($request->Accept() >= 0) {
	my $target_channel = (split ':', $ENV{JOINED}, 2)[0];
	print $target_channel . "\n";
	poll_feed;
}

1;
