#LibVLC installation
Download VLCPlayer: https://www.videolan.org/vlc/
Download Source Code: https://images.videolan.org/vlc/download-sources.html
Make sure both version matches.

Go to C:\Program Files\VideoLAN\VLC, copy "libvlc.dll", "libvlccore.dll" and "plugins" folder to this folder
Extract .tar.xz then inside .tar, extract "include" folder to this folder

Generate .lib files following the instructions here: https://wiki.videolan.org/GenerateLibFromDll

The result should look something like this:

LibVLC/
├── include/
│   ├── vlc/
│   ├── vlc_access.h
│   └── ...
├── plugins/
│   ├── access/
│   ├── access_output/
│   └── ...
├── .gitignore
├── libvlc.lib
├── libvlc.dll
├── libvlccore.lib
├── libvlccore.dll
├── README.md
