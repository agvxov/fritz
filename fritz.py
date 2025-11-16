# XXX: TODO polling event

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

# Controls the poll period. In seconds.
poll_tick = 1

# --- ------- ---
# --- Private ---
# --- ------- ---

arms = []
event_queues = {
	'poll'	   : [],
	'join'	   : [],
	'part'	   : [],
	'quit'	   : [],
	'priv_msg' : [],
	'chan_msg' : [],
}

# FCGI process wrapper
class Fritz_arm:
	def __init__(self, name : str) -> None:
		self.name	   = name
		self.sock_path = name.replace('.', '-') + ".sock"
		self.client    = FastCGIClient(f"unix://{self.sock_path}")
		pid = fork()
		if pid == 0:
			ctypes.CDLL("libc.so.6").prctl(1, signal.SIGTERM) # NOTE non-portable
		else:
			execvp(self.name, f"{name}".split(' '))
	def request(self, params={}, stdin=b''):
		return self.client.request(params, stdin)

def sigint_handler(signum, frame):
	def unlink_sockets():
		for a in arms: unlink(a.sock_path)
	unlink_sockets()
	exit(0)

def handle_event(event : str, data : {}):
	message = ''

	if "MESSAGE" in data:
		message = data["MESSAGE"].encode()
		del data["MESSAGE"] # remove from passed env

	for b in event_queues[event]:
		response = b.client.request(data, message)
		r = response.decode(errors='ignore').strip()
		print("<< " + r)
		yield r

	yield None


# --- ------ ---
# --- Public ---
# --- ------ ---

def add_arm(name, events):
	arm = Fritz_arm(name)
	arms.append(arm)
	for event in events:
		event_queues[event].append(arm)

class Fritz(SimpleIRCClient):
	def __init__(self, server, nick, auto_join_list, port=6697, is_ssl=True):
		def ssl_wrapper(sock):
			context = ssl.create_default_context()
			return context.wrap_socket(sock, server_hostname="chud.cyou")

		self.connection_time = 0
		self.nick			 = nick
		self.auto_join_list  = auto_join_list
		self.joined			 = []

		if port < 6690:
			is_ssl = False

		factory = Factory(wrapper=ssl_wrapper) if is_ssl else irc.connection.Factory()

		super().__init__()
		super().connect(
			server,
			port,
			nick,
			connect_factory = factory
		)

		signal.signal(signal.SIGINT, sigint_handler)

		self.run()

	def run(self):
		next_poll = 0

		while self.connection.is_connected():
			self.reactor.process_once(timeout=.2)

			now = time()
			if now >= next_poll:
				self.run_event_handler("poll", {})
				next_poll = now + poll_tick

	def run_event_handler(self, event_name: str, data: {}):
		if event_name != "poll":
			print(f">> {event_name}: {{{data}}}")

		data["JOINED"] = ":".join(self.joined)
		data["EVENT"]  = event_name

		for response in handle_event(event_name, data):
			if response is None: break
			if response != "":
				try:
					lines	 = response.splitlines()
					metadata = lines[0].strip()
					target	 = metadata
					body	 = lines[1:]
				except:
					print("!! Invalid response.")
					continue
				for l in body:
					self.connection.privmsg(target, l)

	def on_welcome(self, connection, event):
		self.connection_time = time()
		for chan in self.auto_join_list: connection.join(chan)
		print(f"-- Fritz online @ {connection.server_address}")

	# === Common event handlers ===

	def on_join(self, connection, event):
		if event.source.nick == self.nick:
			self.joined.append(event.target)
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
