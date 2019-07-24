

ABOUT IO
========

Io is a small, prototype-based programming language. This package includes it and, depending on the release, some addons (for networking, graphics, sound, etc). See the docs in IoVM/_docs or visit http://iolanguage.com/ for more info.

BUILDING
========

To build (the first time) type:

  make configure

and then:

  make

If you're on MSWindows, building under cygwin is known to work.

WHAT GETS BUILT
===============

Description of the folders:

IoVM - the Io VM 
  builds: 
   io - command line executable
   libIoVM.a - embeddable library

IoServer - non-GUI addons (networking, encryption, etc.)
  builds: 
   ioServer - command line executable (a superset of io)
   libIoServer.a - embeddable library
   libpcre.a - regular expression library

IoDesktop - GUI addons (graphics, sound, events, etc.)
  builds:
   ioDesktop - command line executable (a superset of ioServer)
   libIoDesktop.a - embeddable library
   libOpenAL.a
   libpng.a
   libjpg.a
   libtiff.a
   libz.a
   libfreetype.a 

If you have problems, bug reports or suggestions, please email me. I only have an OSX/Linux boxes to test on, so I especially appreciate any feedback about issues on other platforms.

Cheers,
Steve Dekorte
steve at dekorte dot com
