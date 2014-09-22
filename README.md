mpdtray
=======

a simple mpd tray icon with now playing info using gtk

options
-------
* -h, --host: mpd host (default localhost)
* -p, --port: mpd port (default 6600)
* -t, --timeout: mpd timeout for synchronous operations (default given by libmpdclient)

dependencies
------------
* glib
* gtk+-3.0
* libmpdclient

building & installation
-----------------------
    make
    make install
