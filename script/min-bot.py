#!/bin/python3
from irc.client import SimpleIRCClient, ServerConnectionError
from irc.connection import Factory

class MinBot(SimpleIRCClient):
	def __init__(self, server, nick, auto_join_list, port, is_ssl):
		def ssl_wrapper(sock):
			context = ssl.create_default_context()
			return context.wrap_socket(sock, server_hostname=server)

		self.nick			= nick
		self.auto_join_list = auto_join_list

		factory = Factory(wrapper=ssl_wrapper) if is_ssl else Factory()

		super().__init__()
		try:
			super().connect(
				server,
				port,
				nick,
				connect_factory = factory
			)
		except Exception as e:
			print(f"!! Failed to establish connection: '{e}'.")
			exit(1)

		self.run()

	def run(self):
		while self.connection.is_connected():
			self.reactor.process_once(timeout=.2)

	def on_welcome(self, connection, event):
		for chan in self.auto_join_list: connection.join(chan)

MinBot(
	"chud.cyou",
	"MiBot",
	["#fritz"],
	6667,
	False
)
