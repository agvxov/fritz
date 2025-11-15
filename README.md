# Fritz
> FastCGI Relayed Internet chaT \0

Fritz is an IRC bot framework that utilizes FastCGI.

```
            IRC             FCGI
+--------+       +-------+ <----> Event Hander Process
| Server | <---> | Fritz | <----> Event Hander Process
+--------+       +-------+ <----> Event Hander Process
```

With Fritz you can implement bot behaviour in any language
that supports the FCGI protocol.
That is to say, any language that can use sockets
and especially languages that have library support,
such as but not limited to,
Ada, C/C++, Python, Perl, Node.js, Java, Tcl and many Lisp dialects.

Using Fritz, your application does not have to bother with the implementation details of IRC.
Amongst many favourable consequences, this also means that Fritz bots are long-lasting.
For example, as of writing this,
the libircclient C library has been broken for many years now;
it segvs on IPv6 connections and won't compile with up-to-date SSL support.
Now, while Python and its IRC client library
-on which the current Fritz implementation depends on-
may suffer the same fate in the future,
given any tool that supports both IRC and FCGI,
Fritz could be reimplemented under a day and your bots revived without change.

# FCGI
As mentioned, FCGI protocol is utilized for inter-process communication,
however it is used slightly differently than it would be for web development.

Each FCGI program subscribes to one or many events
(instead of corresponding to end points)
and all of them will be called in the order of registration on said event.
Examples of events would be `join`, `chan_msg` or `priv_msg`.

The following environment variables could be defined depending on the event:
+ USERNAME (name of the triggering user)
+ CHANNEL (source chan)
+ MESSAGE (text of the triggering event)

The first line of the response is a metadata header for Fritz,
everything else is a message that should be posted on the server.
Currently the only metadata is the user/channel name that should be targeted with the message,
however the right is reserved to add more, semi-colon separated fields to the header.
If the message body is an empty string, it shall not be posted by Fritz.

# Usage
To run a Fritz bot, you need a run script.

There is no Fritz CLI, however a run script could easily implement one.

The commented example `run_script.py` is provided for your information.

The `arsenal/` directory contains a collection of Fritz compatible bot implementations.
