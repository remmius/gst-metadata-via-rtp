# Gstreamer plugins to send gst-meta-data as RTP-Header

This project contains two GStreamer-plugins (meta2rtp and metahandle) to enable the transport of meta-data over the network using RTP. If the metadata are not added to the RTP-header, they are not included in the RTP-package and are therefore lost on sending the data. 
Therefore the plugin (meta2rtp) transforms specific meta-data attached to a buffer to the RTP-header and vice versa. The second plugin (metahandle) supports three modes: on the sender side it writes a predefined set of boundings-boxes into the specific metadata (modus=writer) to demonstrate the setup with videotestsrc or convert existing GstVideoRegionOfInterestMeta into the specific meta (modus=converter). On the reciever side it can be used to display the incoming bounding-boxes (modus=reader). 
The specific metadata is currently a bounding-box based on gstvideometa with some modifications to be passed through "rtph264depay" and other plugins.
This meta-data transportation could be helpful e.g. in case the reciever is displaying the annotate video and performing some video-analysis/CV and therefore benefits from recieving unmodified videodata. Main benefit of this approach is that the meta-data and video-data do not need to be resyncronised again on the reciever side.
So far this has only been tested with the state pipelines below using h264-data, but should not be limited to it. Modifications of the pads might be neccessary.

## Usage
Either use the docker file to run the example pipelines, or install the neccessary files (see docker-file for list of pacakges) and use the meson-build process of the GStreamer template repository.

### With plain Ubuntu 20.04
Checkout this repo and go to its base-directory. Install relevant packages (gstreamer,opencv), see dockerfile for details.
    
Configure and build all examples (application and plugins) as such:

    meson builddir
    ninja -C builddir
    
Now start the sender and reciever pipeline. For demonstration-purposes the metahandle can be used to generate static bounding boxes to avoid the need of specfic video-data.
#### Run the sender pipeline with:

##### Use artifical meta-data with videotestsrc

    GST_PLUGIN_PATH=./builddir/gst-plugin/ GST_DEBUG=3 gst-launch-1.0 -v videotestsrc ! 'video/x-raw, width=(int)240, height=(int)240, framerate=(fraction)30/1' ! videoconvert ! metahandle modus=writer ! videoconvert  ! x264enc key-int-max=15 tune=zerolatency ! rtph264pay mtu=1300 ! meta2rtp modus=meta2rtp ! udpsink host=127.0.0.1 port=5555

    With this pipeline you should see the 3 static bounding boxes on the reciever-side video with a name-tag.
    
##### Use camera-input and facedetect-plugin to write metadata

    GST_PLUGIN_PATH=./builddir/gst-plugin/ GST_DEBUG=3 gst-launch-1.0 -v autovideosrc ! decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB,width=384, height=240 ! facedetect  ! metahandle modus=converter ! videoconvert  ! x264enc key-int-max=15 tune=zerolatency ! rtph264pay mtu=1300 ! meta2rtp modus=meta2rtp ! udpsink host=127.0.0.1 port=5555
    
    In forseen usecase, you would set "facedetect display=false" to not draw the bounding box already before sending it of course.
    Note: The metahandle plugin is in best case directly after the facedetect-plugin. Main reason is that some plugins remove the video-metadata from the buffer. The specific metadata pass at least the x264enc and rtph264pay plugins.

##### Run the reciever pipeline:

    GST_PLUGIN_PATH=./builddir/gst-plugin/ GST_DEBUG=3 gst-launch-1.0 -v udpsrc port=5555 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264" !  meta2rtp modus=rtp2meta ! rtph264depay  ! avdec_h264 ! videoconvert ! tee name=t ! queue  ! metahandle modus=reader ! videoconvert  ! autovideosink t. ! videoconvert  ! autovideosink
    This displays 2 windows. One with the bounding-box and one without the bounding-boxes.
This was tested on Ubunutu 20.04 with GStreamer 1.16.2 and OpenCV 4.2.0

#### 

### With dockerfile
Checkout this repo and go to its base-directory and build the image with e.g. 

    docker build --tag metadata .
    
Start a container for the sender side with:

    export TARGET_IP=192.168.0.18
    docker run -it --rm --net=host  metadata gst-launch-1.0 -v videotestsrc ! 'video/x-raw, width=(int)240, height=(int)240, framerate=(fraction)30/1' ! videoconvert ! metahandle modus=writer ! videoconvert  ! x264enc key-int-max=15 tune=zerolatency ! rtph264pay mtu=1300 ! meta2rtp modus=meta2rtp ! udpsink host=$TARGET_IP port=5555

Start a container for the reciever side with:

    xhost +local:root #this is not really save- check for better ways e.g. http://wiki.ros.org/docker/Tutorials/GUI
    docker run -it -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=unix$DISPLAY --rm --net=host  metadata gst-launch-1.0 -v udpsrc port=5555 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264" !  meta2rtp modus=rtp2meta ! rtph264depay  ! avdec_h264 ! videoconvert  ! metahandle modus=reader ! videoconvert  ! autovideosink
    xhost -local:root

# GStreamer template repository

This git module contains template code for possible GStreamer projects.

* gst-app :
  basic meson-based layout for writing a GStreamer-based application.

* gst-plugin :
  basic meson-based layout and basic filter code for writing a GStreamer plug-in.

## License

This code is provided under a MIT license [MIT], which basically means "do
with it as you wish, but don't blame us if it doesn't work". You can use
this code for any project as you wish, under any license as you wish. We
recommend the use of the LGPL [LGPL] license for applications and plugins,
given the minefield of patents the multimedia is nowadays. See our website
for details [Licensing].

## Usage

Configure and build all examples (application and plugins) as such:

    meson builddir
    ninja -C builddir

See <https://mesonbuild.com/Quick-guide.html> on how to install the Meson
build system and ninja.

Modify `gst-plugin/meson.build` to add or remove source files to build or
add additional dependencies or compiler flags or change the name of the
plugin file to be installed.

Modify `meson.build` to check for additional library dependencies
or other features needed by your plugin.

Once the plugin is built you can either install system-wide it with `sudo ninja
-C builddir install` (however, this will by default go into the `/usr/local`
prefix where it won't be picked up by a `GStreamer` installed from packages, so
you would need to set the `GST_PLUGIN_PATH` environment variable to include or
point to `/usr/local/lib/gstreamer-1.0/` for your plugin to be found by a
from-package `GStreamer`).

Alternatively, you will find your plugin binary in `builddir/gst-plugins/src/`
as `libgstplugin.so` or similar (the extension may vary), so you can also set
the `GST_PLUGIN_PATH` environment variable to the `builddir/gst-plugins/src/`
directory (best to specify an absolute path though).

You can also check if it has been built correctly with:

    gst-inspect-1.0 builddir/gst-plugins/src/libgstplugin.so

## Auto-generating your own plugin

You will find a helper script in `gst-plugins/tools/make_element` to generate
the source/header files for a new plugin.

To create sources for `myfilter` based on the `gsttransform` template run:

``` shell
cd src;
../tools/make_element myfilter gsttransform
```

This will create `gstmyfilter.c` and `gstmyfilter.h`. Open them in an editor and
start editing. There are several occurances of the string `template`, update
those with real values. The plugin will be called `myfilter` and it will have
one element called `myfilter` too. Also look for `FIXME:` markers that point you
to places where you need to edit the code.

You can then add your sources files to `gst-plugins/meson.build` and re-run
ninja to have your plugin built.


[MIT]: http://www.opensource.org/licenses/mit-license.php or COPYING.MIT
[LGPL]: http://www.opensource.org/licenses/lgpl-license.php or COPYING.LIB
[Licensing]: https://gstreamer.freedesktop.org/documentation/application-development/appendix/licensing.html
