from sys import exit
from time import sleep
from fcgi_client import FastCGIClient

fbots = []
event_queues = {
	'join'     : [],
	'part'     : [],
	'quit'     : [],
	'priv_msg' : [],
	'chan_msg' : [],
}

class FBot:
	def __init__(self, name : str) -> None:
		self.name = name
		self.sock_path = name.replace('.', '-') + ".sock"
		self.client = FastCGIClient(f"unix://{self.sock_path}")
	def request(self, params=None, stdin=b''):
		params = params or {}
		return self.client.request(params, stdin)

def add_fbot(name, events):
	bot = FBot(name)
	fbots.append(bot)
	for event in events:
		event_queues[event].append(bot)

def handle_event(event : str, data : {}):
	message = data.get("message", "").encode()
	for b in event_queues[event]:
		response = b.client.request({}, message)
		r = response.decode(errors='ignore').strip()
		yield r
	yield None
