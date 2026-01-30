#!/bin/python3
import time
from irc.client import SimpleIRCClient, ServerConnectionError
from irc.connection import Factory

class MinBot(SimpleIRCClient):
	def __init__(self, server, nick, auto_join_list, port):
		self.nick			= nick
		self.auto_join_list = auto_join_list
		self.message_queue  = []

		super().__init__()
		try:
			super().connect(
				server,
				port,
				nick
			)
		except Exception as e:
			print(f"!! Failed to establish connection: '{e}'.")
			exit(1)

		self.run()

	def run(self):
		while self.connection.is_connected():
			self.reactor.process_once(timeout=.2)

	def on_welcome(self, connection, event):
		for chan in self.auto_join_list:
			connection.join(chan)

MinBot(
	"chud.cyou",
	"MinBot",
	["#fritz"],
	6667
)
