#!/usr/bin/env python3
import fritz

from os import environ, pathsep
environ["PATH"] += pathsep + "./arsenal/"

fritz.add_arm("hw.out", ['join'])
fritz.add_arm("exit.out", ['join'])

fritz.Fritz(
	"127.0.0.1",	# server address
	"Fritz",		# bot username
	["#fritz"],		# channels to join
	port=6667,
	is_ssl=False
)
