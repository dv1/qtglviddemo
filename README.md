A Qt 5 demo application for rendering videos on 3D objects integrated in QtQuick 2.
===================================================================================


Overview
--------

The qtglviddemo demo application shows how to render video frames on 3D objects,
which in turn are integrated in a QML and QtQuick 2 based user interface.

The video frames are produced by the GStreamer GstPlayer library. They are uploaded
into OpenGL textures, which are then used on 3D meshes. These meshes are rendered
in [QQuickFramebufferObject QtQuick 2 items](https://doc.qt.io/qt-5/qquickframebufferobject.html),
and the items are composed by a PathView on screen. The 3D objects can be rotated
with the mouse or with touch events. UI controls allow for adjusting several parameters
such as opacity, scale, mesh type, etc. and for adding/removing objects. The video
frames can come from local video files, network streams, or Video4Linux2 based
video capture devices.

Subtitles can be shown on screen. The subtitles can come either from the playing media
itself, or from a FIFO if one is enabled in the configuration file.

There is also special support built in for Vivante GPUs, specifically their direct
texture extensions. This makes it possible to render high resolution videos smoothly
on i.MX6 machines for example.


License
-------

This demo application is licensed under the [GNU General Public License, version 3](https://www.gnu.org/licenses/gpl-3.0.html).


Dependencies
------------

* Qt 5.7 or newer
* GStreamer 1.10 or newer
* libudev


Building and installing
-----------------------

qtglviddemo uses qmake for building. qmake can do out-of-tree builds, and it is
generally recommended to do so.

First, create a build directory and go into it:

    mkdir build
    cd build

Next, run qmake:

    qmake ..

It is possible to enable additional features via qmake by appending to the CONFIG
variable, like this:

    qmake .. CONFIG+="[features]" PREFIX="[install prefix]"

Where `[features]` is a whitespace separated list of feature names and
`[install prefix` is path that will be used as prefix when installing files.
PREFIX is optional; if not specified, it will default to `/usr/local`.

The following features are available:

* vivante : Build the additional Vivante GPU support.
* useImxV4L2 : When adding video streams from capture devices, use `imxv4l2://`
  URLs instead of `v4l2://` ones to make use of gstreamer-imx' imxv4l2videosrc
  element.

Example on i.MX6 with [gstreamer-imx](https://github.com/Freescale/gstreamer-imx) installed:

    qmake .. CONFIG+="vivante useImxV4L2"

Then, run:

    make

Parallel make is supported by qmake generate Makefiles.

The build can then be installed by running:

    make install

This will copy the built binary to PREFIX/bin. So, if for example the prefix
was configured in the qmake call to be `/opt/local`, then the binary will be
copied to `/opt/local/bin`.


Running the demo application
----------------------------

The application's command line arguments can be shown by running `qtglviddemo -h`:

    Usage: ./qtglviddemo [options]
    Qt5 OpenGL video demo

    Options:
      -h, --help                         Displays this help.
      -v, --version                      Displays version information.
      -w, --write-config-at-end          Write configuration when program is ended
      -c, --config-file <config-file>    Configuration file to use
      -s, --splashscreen <splashscreen>  Filename of splashscreen to use

The splashscreen must be in a format supported by Qt. JPEG and PNG are a good pick.

Simply running qtglviddemo without any switches will run the application with
a default configuration.

Several Unix signals are caught for facilitating a graceful shutdown. These are
SIGINT, SIGTERM, SIGQUIT. This makes it possible to for example end the program
by pressing Ctrl+C on the console without an abrupt stop.


Configuration
-------------

qtglviddemo accepts configuration files in JSON format. At the top level,
the following fields are supported:

* fifoPath: This configures the path to the FIFO mentioned in the introduction
  If set to a valid not yet existing absolute filename, the user can then pipe
  data into this FIFO, and it will show up on screen if the 3D objects' subtitle
  source is set to "FIFO". If fifoPath refers to an existing file, no FIFO will
  be active, so make sure the path is valid and the filename is not already in use.

* deviceNodeNameMap: Some devices, such as `mxc_v4l2` based i.MX6 video capture
  devices, do not have any model name. Since these are usually fixed-mounted,
  they will always have the same Linux device node. This makes it possible to
  define a name for them, which will be shown in the UI, making it easier for
  the user to know what device this is. That's the purpose of deviceNodeNameMap:
  it is an array of JSON objects, each object containing a "name" and a "node"
  value. Devices with nodes listed in this field will use the configured name
  instead of the `ID_MODEL` V4L2 string value.

* items: The items/objects shown on screen.

The items are configured through the user interface. The other two fields are
configured manually.

Here is an example of a configuration with 1 item, a FIFO path, and a device
node name map that sets the name of `/dev/video0` to be "Built-in camera":

    {
        "items": [
            {
                "cropRectangle": [
                    0,
                    0,
                    100,
                    100
                ],
                "meshType": "cube",
                "opacity": 0.7,
                "rotation": [
                    -0.633301317691803,
                    -0.16265153884887695,
                    -0.755376398563385,
                    -0.04336431622505188
                ],
                "scale": 1,
                "subtitleSource": "media",
                "url": "file:///home/root/test123.mkv"
            }
        ],
        "fifoPath": "/tmp/qtglviddemo-fifo",
        "deviceNodeNameMap" : [
            { "name": "Built-in camera", "node": "/dev/video0" }
        ]
    }

Configuration files are specified with the `-c` command line switch. If the
`-w` switch is also present, then the configuration file will be updated
when the demo application exits.


How it works
------------

This is a high level explanation of how the demo application operates.

The essential components are the `VideoObjectItem` QtQuick 2 item and the
`VideoObjectModel` Qt list data model. The latter contains "descriptions",
which describe an object, its parameters (mesh type, URL, opacity etc.).
The former can render such an object.

They are linked together in the QML UI through a [PathView](http://doc.qt.io/qt-5/qml-qtquick-pathview.html).
Using the MVC pattern, the list of objects is implemented as the list of
descriptions in the VideoObjectModel. There is one such model instantiated
in the application, and made globally accessible in the QML script.
The PathView's model property is set to this data model, meaning that
PathView uses the descriptions as the data to render. PathView uses
VideoObjectItem as a delegate; in QtQuick 2, delegates specify *how*
an item in a data model is rendered. So, PathView creates VideoObjectItem
instances for each description in the VideoObjectModel. If descriptions
are added or removed, then VideoObjectItem instances are automatically
created or destroyed.

The video playback is part of VideoObjectItem. It is automatically started
once the VideoObjectItem is created. The playback is done by the
GStreamerPlayer C++ class, which hands out decoded video frames through
the [GStreamer appsink element](https://gstreamer.freedesktop.org/data/doc/gstreamer/head/gst-plugins-base-libs/html/gst-plugins-base-libs-appsink.html).
The video frames are then fed into a "video material", which consists of
a texture (which gets the video frame pixels uploaded into) and a set
of parameters.

Video capture devices are discovered by using libudev. Any devices that
are hotplugged are also detected.


Limitations
-----------

* qtglviddemo does not support media without a video track.
* Adding too many objects can worsen behavior and performance of the
  demo, depending on the platform. For example, trying to play multiple
  1080p videos on an i.MX6 machine may not work properly, because this
  exceeds the bandwidth limitations of such machines. Also, since objects
  are rendered into FBOs, the GPU allocates memory for each FBO. With
  too many objects, the GPU may run out of memory (depending on how it
  is configured).
* Only simple subtitles with [Pango markup](https://developer.gnome.org/pango/stable/PangoMarkupFormat.html)
  are supported. More advances formats, such as TTML or WebVTT, would require
  substantially more complex code, which is beyond the scope of this demo.
