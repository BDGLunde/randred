Build dependencies:
    - Meson: https://mesonbuild.com/
        - Why? Because it's what the official Gstreamer repos are currently using and it appears to work fine.
    - Gstreamer: https://gstreamer.freedesktop.org/documentation/installing/on-linux.html?gi-language=c

Build plugins like so:
    Set pwd to: gitroot/experimental/gst
    meson builddir
    ninja -C builddir
    ninja -C builddir install

How to test randred plugin:

gst-launch-1.0 -v videotestsrc ! video/x-raw,format=RGB ! randred ! autovideosink

RGB format is specified in videotestsrc capability filter because that's the only class of formats randred
supports.

Or if you want to test sending frames over network:

server: gst-launch-1.0 -v videotestsrc ! video/x-raw,format=RGB ! randred ! videoconvert ! x264enc ! tcpserversink host=0.0.0.0 port=5000 sync-method=2 recover-policy=keyframe

client: gst-launch-1.0 tcpclientsrc host=0.0.0.0 port=5000 ! decodebin ! autovideosink
