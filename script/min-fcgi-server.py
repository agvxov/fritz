#!/usr/bin/python3
from sys import exit
from time import sleep
from fcgi_client import FastCGIClient

SOCK_PATH = "hw-c.sock"

client   = FastCGIClient(f"unix://{SOCK_PATH}")
response = client.request({}, "")
print(response)
