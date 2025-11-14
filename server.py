#!/usr/bin/env python3
import re
import ssl
import socket
from time import time
from irc.client import SimpleIRCClient, ServerConnectionError
from irc.connection import Factory
from irc.connection import Factory

from fbot import *

class fbotServer(SimpleIRCClient):
	def __init__(self, server, name, auto_join_list):
		def ssl_wrapper(sock):
			context = ssl.create_default_context()
			return context.wrap_socket(sock, server_hostname="chud.cyou")

		self.connection_time = 0
		self.auto_join_list  = auto_join_list

		super().__init__()
		super().connect(
			server,
			6697,
			name,
			connect_factory = Factory(wrapper=ssl_wrapper)
		)

	def run_event_handler(self, event_name: str, data: {}):
		gen = handle_event(event_name, data)
		for response in gen:
			if response is None: break
			if response != "" and "channel" in data:
				self.connection.privmsg(data["channel"], response)

	def on_welcome(self, connection, event):
		self.connection_time = time()
		for chan in self.auto_join_list: connection.join(chan)
		print(f"-- Connected to {connection.server_address} --")

	# === Common event handlers ===

	def on_join(self, connection, event):
		records = {
			"username": event.source.nick,
			"channel": event.target,
		}
		self.run_event_handler("join", records)

	def on_pubmsg(self, connection, event):
		records = {
			"username": event.source.nick,
			"channel": event.target,
			"message": event.arguments[0],
		}
		self.run_event_handler("chan_msg", records)

	def on_privmsg(self, connection, event):
		records = {
			"username": event.source.nick,
			"channel": event.source.nick,
			"message": event.arguments[0],
		}
		self.run_event_handler("priv_msg", records)

	def on_part(self, connection, event):
		records = {
			"username": event.source.nick,
			"channel": event.target,
		}
		self.run_event_handler("part", records)

	def on_quit(self, connection, event):
		records = {
			"message": event.arguments[0] if event.arguments else "",
		}
		self.run_event_handler("quit", records)

def main() -> None:
	client = fbotServer("chud.cyou", "real_AM", ["#fbot"])

	#add_fbot("parrot.pl", ['chan_msg'])
	#add_fbot("hate.pl", ['chan_msg'])
	add_fbot("hw.c", ['join'])

	while client.connection.is_connected():
		client.reactor.process_once(timeout=0.2)

if __name__ == "__main__":
	main()
