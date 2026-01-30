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

	def privmsg(self, target : str, message : str):
		n_free_bytes = 112 - len('PRIVMSG  :\r\n') - len(target.encode('utf-8'))
		raw_message = message.encode('utf-8')
		for m in [raw_message[i:i+n_free_bytes] for i in range(0, len(raw_message), n_free_bytes)]:
			self.message_queue.append((target, m.decode('utf-8', errors='ignore')))
			mm = m.decode('utf-8', errors='ignore')
			print(len(f"PRIVMSG {target} :{mm}\r\n".encode()))

	def privmsg_frfr(self):
		if len(self.message_queue) == 0: return
		now = time.monotonic()
		print(now, self.message_queue[0])
		message = self.message_queue.pop(0)
		self.connection.privmsg(*message)

	#def run(self):
	#	while self.connection.is_connected():
	#		self.reactor.process_once(timeout=.2)

	def run(self):
		now = time.monotonic()
		last_message = now - 1
		while self.connection.is_connected():
			self.reactor.process_once(timeout=.2)
			now = time.monotonic()
			if now - last_message > 1:
				last_message = now
				self.privmsg_frfr()

	def on_welcome(self, connection, event):
		for chan in self.auto_join_list:
			connection.join(chan)
			self.privmsg(chan, "ab" * 2024)

MinBot(
	"chud.cyou",
	"MinBot",
	["#fritz"],
	6667
)
