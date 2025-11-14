#!/usr/bin/env python3
from os import environ, pathsep
import fric

environ["PATH"] += pathsep + "./arsenal/"

fric.add_arm("parrot.pl", ['chan_msg'])
#fric.add_arm("hate.pl", ['chan_msg'])
#fric.add_arm("hw.out", ['join'])

fric.Fric("chud.cyou", "Fric", ["#fric"])
