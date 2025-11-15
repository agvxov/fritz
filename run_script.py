#!/usr/bin/env python3
import fritz

# Fritz looks for handlers in $PATH
from os import environ, pathsep
environ["PATH"] += pathsep + "./arsenal/"

# Push handler processes, referred to as "arm"s.
# Fritz manages the life time of these processes.
# The name of the program is used to deduce the named socket to communicate through.
# The choice of whether you would like to run multiple Fritz instances with one arm
#  or a monolith, is up to you.
fritz.add_arm("parrot.pl", ['chan_msg']) # Perl script repeating messages
fritz.add_arm("hate.pl", ['chan_msg'])   # Perl script adding !hate command
fritz.add_arm("hw.out", ['join'])		 # C hello world program

# Construct and run bot.
# Inverts control.
# Do not call multiple times!
fritz.Fritz(
	"chud.cyou",	# server address
	"Fritz",		# bot username
	["#fritz"]		# channels to join
)

# NOTE: To implement your own arm, read `fritz.py`!
