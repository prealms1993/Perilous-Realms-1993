This is the source code for Perilous Realms MUD circa 1993-1994.

It includes all the world information and is a complete working
version from that era.   It does not include any players except
immortals and test characters.  This compiles on Linux, NetBsd,
and Windows (with cygwin) as a 32-bit app but will run great on
a 64-bit system.

Brief History
Back in 1992-1994 I played PR extensively and over time worked
up to being an 'implementer' - meaning that I added features, 
updates and otherwise worked on code.  At the time PR ran on a
NEXT server and in preparation for moving to a new server I ported 
this to Intel architecture for both 386BSD and Linux.  The code 
here is based on that port.

Getting Started
If you pulled this from github you're going to need to build 
the application and rebuild the binary files.  Follow the 
"Compiling" section below.  Once compiled just run dmserver
from the main directory (the one above lib/).
Once running, telnet (or mud client) into the port and create
a character.  Once you're in the world look at the maps and
in the first town square there's an information building that
has enough information to really get going!

Usage
First off, this is probably unsafe to run directly on the 
modern internet.  What was safe and secure 20+ years ago is 
probably not true anymore.

To start the server:
./dmserver [port]
port default is 4000
Useful options:
-a Accept Any Password - bypasses password check on login
-w WizLock  (only allow immortals)
-b DebugLevel (1,2, 11, 1001, 1234 are useful levels)
-d Data files directory (default is ./lib)

Configuration files
lib/maxplayers    Maximum number of players

Compiling
Make sure that only the 32-bit version of gcc is used.  Pointers
are cast as integers in the code and it will not function when 
compiled as a 64-bit app.

Steps:
cd src
make clean
make depend
make proto
make fcns
make tran
make
cd ..
./GenObjs.sh
src/dmserver 4000

Thats it. You should be able to run dmserver from the main
directory.  (ie: 'src/dmserver 4000').

Regenerating the World
This app directly saves/loads most of its files in a binary
format.  This means that all the "object/mob/shops/worlds"
require converting to a binary format for loading.  This requires
a separate conversion program called 'tran' which is located
in 'src'.  That must first be made before converting the
data files ("make tran").

Once that is made there is a script "GenObjs.sh" that will
run the 'tran' program and generate the files.  Note that
the GenObjs.sh script is written for cygwin and may
require minor edits for linux or netbsd.  

Changes from original source code
Beyond the required modifications to port to Linux/cygwin,
there are only a few changes:
* New characters get 100k gold and 100k experience.  (go to 
  your guild to level up)
* Parameter to bypass password check (when passwords are lost)
* Fixed a couple of minor bugs

Included characters (mostly immortals):  
All passwords are set to '123456'.
	abbie
	bolt
	cedar
	countess
	crazy
	deth
	eaglelim
	goon
	goth
	madsen
	major
	olorin
	sajax
	superman
	testy
	thunk
	walkerboh




