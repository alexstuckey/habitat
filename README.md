System Garden: Habitat
=====================

Welcome to Habitat: extensible collector, monitor and viewer for system 
and applications. Also the main gateway into the System Garden project.

This files covers changes, known problems, installation, how to run,
how to connect to [System Garden](http://www.systemgarden.com/) (not supported in Alpha releases)
and documentation. See the file LICENSE for Habitat's licensing terms.

Version 2.0 Alpha 1
-------------------

The GUI has been rewritten for Gtk late 2.x / early 3.x and major updates 
to the interface have been made to make it more usable and logical. 
The GUI is now called `MyHabitat`.


Changes
-------

- Since the 1.x series, ringstore files have changed their driver name 
  from 'rs:' to 'grs:' (they are based on GBDM) to allow for other
  storage methods to be evaluated as ringstores
- The local host ringstore is now 'checkpointed' hourly, used as a hook
  to housekeep and compact files to save storage (its become a route 
  method so it can be reused).
- Alternative standard job tables are now provided to meet specific needs:
  two for now.
- Configuration available in machine readable format from clockwork
- `Edit->Collector…` now reads job file from clockwork's config not ours
- Some old irritating memory leaks squashed
- Implemented text visualisation
- Killed all 'normal' memory leaks & tided up in myhabitat
- Killed some 'abnormal' memory leaks too!
- Updated ring and visualisation icons - smarted and bigger: 32px high!
- Added the 'Other' button menu
- Added a Macintosh 'names' probe
- Added local: and localmeta: route drivers to abstract local data access
- Added format detection of actual data read
- Added format input and default detection from `File->Open` menu
- Timeless tables now understood and will grey out time slider
- File route as a table termination bugs fixed (rt_file_tread())
- Colours added to message logs
- Labels added above the curve and instance lists in the chart view
- RPMs fixed
- Clockwork restart process simplified to make more reliable
- Refactored SIGCHILD signal handler make safe
- Added autorefresh code to the data display, defaulting to 30 seconds
- Autorefresh time added to preferences window & hooked up to config
  as myhab.update
- Loaded config values into preferences on initialisation
- Made safe from single sample data
- Display local view 5 seconds from startup
- Renamed its to habrs, manual pages too
- Disabled debug comments in rcache
- Adding a host or file will always expand the parent node in choice tree
- Generated HTML manual pages given a style similar to the website


Known Issues
------------
- Connection to System Garden repositories not in alpha
- Horizontal timeline scale does not adapt under all conditions
- Day & time scale adaptions lose date reference
- At detailed resolutions graphing can corrupt
- Print and print preview does not work
- Replication menu option not working
- Resizing the chart can leave curve lists too large
- Drawing and undrawing a curve corrupts the lines
- Tooltips fail when scrolling right in a table view of data
- Does not create .deb
- Viewing some text data remotely causes a coredump of myhabitat
- Clockwork network security and reliability — switch to [libmicrohttpd](http://www.gnu.org/software/libmicrohttpd/)
- Auto update resets any zoomed state, as v1, check if zoomed and don't update
- Starting from no database does not start displaying properly & generates
  lots of error messages
- Test RPMs, check file names and location wisdom
- Create a .deb
- Signal handlers that need to be made signal safe :-
        `runq_sigdispatch()`


Notes
-----
- Debian source dependency packages (needed for compilation):-
      `pkg-config` `libgtk2.0-dev` `libgdbm-dev` `libcurl4-openssl-dev`
      libreadine-dev libgtkdatabox-dev man2html rpm
- Debian binary dependency packages (needed for installation):-
      `libgdbm` `libcurl4-openssl` `libreadline` `libgtk2.0-0` `libgtkdatabox`
- RedHat/Fedora source dependency packages (needed for compilation):-
      `gtk2-devel` `gdbm-devel` `curl-devel` `readline-devel` `gtkdatabox-devel`
      man2html rpm
- RedHat/Fedora binary dependency packages (needed for installation):-
      `readline` `gdbm` `curl` `gtk2` `gtkdatabox`


Installation
------------

Either install the system specific package for you operating system
(such as RPM file for Red Hat compatibles or Mac .dmg image) or extract 
the binary .tar file into your home directory. Habitat does not need 
root permissions unless you need to monitor protected files.

The Habitat distribution is currently around 3-4 MBytes in Linux
and the data created will grow to around 5 MBytes using the default settings
probes. Checkpoint jobs must be retained to ensure that the GRS data store
is kept to this level of space.


How to Start
------------

Just run the command 'myhabitat' inside the directory `$HAB/bin`, which 
will start the Gtk GUI and invite you to start the collection agent.
To start the collection daemon only (see below), just run `clockwork` from `$HAB/bin`.

In the current release, the default data collection rate is every 60 
seconds and will store this information to disk, in a Habitat 'ringstore' 
format which keeps the impact to system load to a minimum. 
Unfortunately, this means that you will need to wait for two to three 
minutes for meaningful data to appear.

See the manual page `clockwork(8)` and the Habitat Administration Manual 
for more information about the collection agent.


The GUI
-------

MyHabitat has a split view with data sources on the left and the selected
data on the right, similar to an 'explorer' type interface. 
The local machine is at the top of the choices, with file, network 
and repository sources following as they get added to the list.
Each source is typically a server, service or grouping of servers and 
services.

When a source is selected, a default data view is shown to the right, with
buttons along the top to change the data seen and how it is visualised.
The buttons below help navigate and alter the appearance of the tool.

The slider at the bottom shows how much data has been loaded and can
be displayed; move it to the left to load and display more data. Zooming
into the chart does not affect this control.

See the manual page `myhabitat(1)` for more information.


Standard Data
-------------

Habitat can collect and store practically any time series tabular data. 
Out of the box, the following are collected and buttons for each are 
shown in the top viewing row in MyHabitat.

<table>
  <tr>
    <th>Name</th><th>Description</th>
  </tr>
  <tr>
    <td>CPU</td><td>System and processor statistics</td>
  </tr>
  <tr>
    <td>Storage</td><td>Capacity and performance of storage</td>
  </tr>
  <tr>
    <td>Network</td><td>Networking statistics </td>
  </tr>
  <tr>
    <td>Processes</td><td>Process table over time (potentially filtered)</td>
  </tr>
  <tr>
    <td>Uptime</td><td>How long the machine has been running</td>
  </tr>
  <tr>
    <td>Events</td><td>Local detection of patterns found or data thresholds crossed</td>
  </tr>
  <tr>
    <td>Other</td><td>Menu of all the other time series data. In habitat jargon, these are also called 'rings' (as they are ring buffers)</td>
  </tr>
</table>

Each ring holds many attributes, such as processor utilisation (%cpu or %work)
and these can be seen in a table or selected in a chart.

See the manual pages `habget(1)`, `habput(1)` for command line extraction 
and import of data. Also the **Habitat User Manual** for more information.


Running Just The Collector
--------------------------

Habitat consists of a back end for data gathering called 'clockwork'
and a set of front ends for display and analysis of results.
MyHabitat, the front end GUI, will ask to start a clockwork back end 
if does not find it running on its host. There are options to stop 
asking and also to leave it running.

To collect data on several machines without the GUI, just run 
'clockwork' on each, which as a daemon will background itself and 
by default, log its own errors and warnings into the data store 
for later (see WHERE DOES EVERYTHING GO? below).

If there is a problem launching clockwork, starting

    $HAB/bin/clockwork -d

will cause diagnostic messages to be sent to stderr. If the failure is
not obvious, send this output to support@systemgarden.com. If there 
are still problems, an exhaustive set of debug messages can be obtained
with

    $HAB/bin/clockwork -D

This places clockwork into developer debug mode.

To stop debug output, stop the clockwork daemon with `killclock(8)`
(see the manual page).


Where Does Everything Go?
-------------------------

By convention (and default), almost everything is written to a single file

    $HAV/var/<yourhost>.grs

where \<host\> is the name of your computer, obatined by typing 
'hostname' at a command prompt. This file can also act as a source of 
data and configuration. Depending on configuration, the file name
may also include the domainname.

Inside this file are rings of data, such as a log and uptime data
which are a time series of tables, time stamped and sequenced to 
make ordered and unique. The rings have descriptions and a 
defined maximum length (including infinite) which can be customised.
Data is removed when it exceeds the retained limit.
The command line utility `irs` or MyHabitat can be used to look at 
this data.

Job directives are held in a file (see $VAR/lib/*.job) and several
pre-fabricated tables have been constructed for different collection 
requirements. This can be configured by MyHabitat or `-J` or `-j` flags
using clockwork from the command line.

All the parts of habitat are configured in the same way, using 
`$HAB/etc/habitat.conf` and `~/.habrc`. It can also be augmented from 
more central locations: see the manual page `habconf(5)`.

Following normal operating system conventions, the data file will have the 
permissions and ownership of the creating user.


Performance Gathering Probes
----------------------------

A probe is a small piece of code inside or called by the back end 
clockwork. Its job is to sample data, process it and potentially repackage
for data display. The output is a single table per invocation (called a 
Fat Headed Array) and an error or log stream.

Probes can also act as filters to a queue of data in order to perform 
mathematics or other manipulation, such as averaging. In this way, a
job configuration can describe a pipe network with stateful data stored.

Clockwork uses probes and other methods to sample data and build 
the network. See `habprobe(1)`, `habmeth(1)` and `clockwork(8)`.


To Exit The Front End
---------------------

Type `^X` or click on the menu `File->Exit` to leave the application.


Stopping Clockwork Normally And What To Expect
----------------------------------------------

Clockwork is a daemon, detached from the command line.
As such, it is designed to stay runnning indefinitely, permanently 
recording your system's activities. There are no problems leaving it
to run as it always trys to curtail the amount to disk space used by
removing the oldest high frequency data, leaving lower frequency samples.
It even restarts itself daily to prevent memory or resource leaks 
in probes.

To stop clockwork running and cease collection of samples, use `killclock`
on the command line. From the MyHabitat GUI, click on the 'Collection' 
button at the window's footer or `Data->Collection…` from the menu, 
then choose 'Stop Collection' from the pop-up window. 
This will signal a shutdown or reset a situation if there has been a crash.

To restart, just run `clockwork' from the command line or click 'Collection'
at the foot of the MyHabitat window, followed by 'Start Collection' in 
the popup.


Reading Data From System Garden
-------------------------------

Data can be read from System Garden's central data repository in addition
local and peer systems. Once set up, one can then browse host data without
burdening the host machine unnecessarily.

To set this up, you need an account with System Garden
connected to your organisation (see [http://www.systemgarden.com](http://www.systemgarden.com)).
Click on 'Repository' at the foot of the MyHabitat window or select 
`Edit->System Garden` from the menu. Click 'Read from System Garden' and
type in your account details (username, password and organisation).

An additional set of items will appear in the choice window called 
'REPOSITORY'. Under this will be all the machines attached to your 
organisation that are contributing data, optionally organised by group.


Sending Data To System Garden
-----------------------------

To send data to System Garden's repository, click 'Repository' from 
the foot of MyHabitst's window and tick 'Send to System Garden'.
The first time this is done, a specific upload account is created 
for security. By default, this will send a subset of collected data
to System Garden every 24 hours and cause your machine to show up
as a repository in the choice section.

The sending frequency can be adapted to suit and any machine added will
be seen from participating MyHabitat clients.

On the command line, use `habrep(8)` to force a replication to the
repository. See `habrep(8)`.


Other Commands
--------------
Other commands exists to complement the GUI and data collector.

<table>
  <tr>
    <th>Name</th><th>Description</th>
  </tr>
  <tr>
    <td>killclock</td><td>stop the clockwork daemon</td>
  </tr>
  <tr>
    <td>statclock</td><td>print information about clockwork</td>
  </tr>
  <tr>
    <td>irs</td><td>command line interface to ringstore storage & admin</td>
  </tr>
  <tr>
    <td>habge</td><td>get data from a route</td>
  </tr>
  <tr>
    <td>habput</td><td>send data to a route</td>
  </tr>
  <tr>
    <td>habedit</td><td>edit a route or ringstore (useful for configuring the jobs in clockwork)</td>
  </tr>
  <tr>
    <td>habmeth</td><td>run a clockwork method manually</td>
  </tr>
  <tr>
    <td>habprobe</td><td>run one of the built-in data gathering probe manually</td>
  </tr>
  <tr>
    <td>habrep</td><td>synchronise data with the repository</td>
  </tr>
</table>

Documentation & Next Steps
--------------------------
To learn more about habitat, look at the System Garden web site 
[http://www.systemgarden.com/habitat](http://www.systemgarden.com/habitat). It gives information about the product 
and company and is the place to go to for new downloads and documentation. 

By clicking on *documentation* under the *habitat* heading, you
get overviews, presentations, white papers and manuals. As more is
written, it will be placed there, so check back periodically.

License
-------

System Garden has released Habitat under the Free Software Foundation's 
Gnu Public License version 3 (GPLv3).   Please look at the file LICENSE in the 
top directory for a full description of the GPL and to understand 
the legal position.  Do not use this software unless you understand 
and agree to the license.

Nigel Stuckey

System Garden Ltd.

October 2011

[welcome@systemgarden.com](mailto:welcome@systemgarden.com)
