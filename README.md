pam_synology - A PAM authentication module which uses the Synology REST API for authentication
==============================================================
[![Build Status](https://travis-ci.org/rikvdh/pam_synology.svg?branch=master)](https://travis-ci.org/rikvdh/pam_synology)

Introduction
------------
This is a PAM module which allows the user to authenticate users against a Synology user-account
 using the PAM subsystem in *nix based systems.

Status
------
It works, very crude and all SSL verification is disabled by default. If you want to try it, read ahead.
Report any bugs, it can help me.

Prerequisites
-------------
To try this module, you need the following stuff:
 - A *nix (or similar) system, in which PAM facility is set up and working
   either system-wide or in a chroot jail.
 - A Synology NAS which HTTP(S) interface accessible from the server having the pam_synology module

Installation instructions
-------------------------

An example of the configuration file:
```
auth sufficient pam_synology.so url=https://mysynologyhostname:5001
account sufficient pam_synology.so
```

---------------------------------------------------------------

Available options
-----------------
The module options are listed below with default in ()s:

url

    Base URL for your Synology NAS where the web-interface is hosted

Links
-----
- Synology
  http://www.synology.com/

- PAM
  http://pam.sourceforge.net/
