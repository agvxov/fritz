import ssl
import socket
import ctypes
import signal
from os import fork, execvp, unlink
from sys import exit
from time import time, sleep
from fcgi_client import FastCGIClient
from irc.client import SimpleIRCClient, ServerConnectionError
from irc.connection import Factory

arms = []
event_queues = {
	'join'     : [],
	'part'     : [],
	'quit'     : [],
	'priv_msg' : [],
	'chan_msg' : [],
}

class Fric_arm:
	def __init__(self, name : str) -> None:
		self.name      = name
		self.sock_path = name.replace('.', '-') + ".sock"
		self.client    = FastCGIClient(f"unix://{self.sock_path}")
		pid = fork()
		if pid == 0:
			ctypes.CDLL("libc.so.6").prctl(1, signal.SIGTERM)
		else:
			execvp(self.name, f"{name}".split(' '))
	def request(self, params=None, stdin=b''):
		params = params or {}
		return self.client.request(params, stdin)

def add_arm(name, events):
	arm = Fric_arm(name)
	arms.append(arm)
	for event in events:
		event_queues[event].append(arm)

def unlink_sockets(signum, frame):
	for a in arms:
		unlink(a.sock_path)

def handle_event(event : str, data : {}):
	if "MESSAGE" in data:
		message = data["MESSAGE"].encode()
		del data["MESSAGE"]
	else:
		message = ''

	for b in event_queues[event]:
		response = b.client.request(data, message)
		r = response.decode(errors='ignore').strip()
		print("<< " + r)
		yield r

	yield None

class Fric(SimpleIRCClient):
	def __init__(self, server, name, auto_join_list, port=6697, is_ssl=True):
		def ssl_wrapper(sock):
			context = ssl.create_default_context()
			return context.wrap_socket(sock, server_hostname="chud.cyou")

		self.connection_time = 0
		self.auto_join_list  = auto_join_list

		if port < 6690:
			is_ssl = False

		factory = Factory(wrapper=ssl_wrapper) if is_ssl else irc.connection.Factory()

		super().__init__()
		super().connect(
			server,
			port,
			name,
			connect_factory = factory
		)

		signal.signal(signal.SIGINT, unlink_sockets)

		self.run()

	def run(self):
		while self.connection.is_connected():
			self.reactor.process_once(timeout=.2)

	def run_event_handler(self, event_name: str, data: {}):
		print(f">> {event_name}: {{{data}}}")
		gen = handle_event(event_name, data)
		for response in gen:
			if response is None: break
			if response != "" and "CHANNEL" in data:
				self.connection.privmsg(data["CHANNEL"], response)

	def on_welcome(self, connection, event):
		self.connection_time = time()
		for chan in self.auto_join_list: connection.join(chan)
		print(f"-- Fric online @ {connection.server_address}")

	# === Common event handlers ===

	def on_join(self, connection, event):
		records = {
			"USERNAME" : event.source.nick,
			"CHANNEL"  : event.target,
		}
		self.run_event_handler("join", records)

	def on_pubmsg(self, connection, event):
		records = {
			"USERNAME" : event.source.nick,
			"CHANNEL"  : event.target,
			"MESSAGE"  : event.arguments[0],
		}
		self.run_event_handler("chan_msg", records)

	def on_privmsg(self, connection, event):
		records = {
			"USERNAME" : event.source.nick,
			"CHANNEL"  : event.source.nick,
			"MESSAGE"  : event.arguments[0],
		}
		self.run_event_handler("priv_msg", records)

	def on_part(self, connection, event):
		records = {
			"USERNAME" : event.source.nick,
			"CHANNEL"  : event.target,
		}
		self.run_event_handler("part", records)

	def on_quit(self, connection, event):
		records = {
			"MESSAGE" : event.arguments[0] if event.arguments else "",
		}
		self.run_event_handler("quit", records)
