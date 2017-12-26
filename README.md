# DNS-Name-Resolution-Engine

This is a multi-threaded application that resolves domain names to IP addresses, similar to the operation performed each time you access a browser.
It was created as an assignment for CSCI 3753: Operating Systems at the University of Colorado Boulder.
The repository contains:

util.c and util.h - two files which contain the DNS lookup functionality.

a /names directory, which contains the names being resolved.

multilookup.c, which is the core of the application, and creates the resolver/ requester threads that service the name resolution.

a MAKEFILE, to build and clean the application.
