#!/usr/bin/env python3
from sys import path
path.insert(0, "../../")

import fritz

from os import environ, pathsep
environ["PATH"] += pathsep + "./arsenal/hibot/"

fritz.add_arm("hibot", ['priv_msg', 'chan_msg'])

fritz.Fritz("chud.cyou", "hibot", ["#hibot"])
