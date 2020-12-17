FROM ubuntu:18.04
ENV TZ=Europe/Kiev
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && apt-get install gcc g++ ninja-build pkg-config meson yasm unzip wget git cmake build-essential -y 

#Gstreamer1.0
RUN apt-get install -y libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev
#Open-cv source: https://github.com/alyssaq/reconstruction/blob/master/Dockerfile
RUN apt-get install -y \
  libswscale-dev \
  libtbb2 \
  libtbb-dev \
  libjpeg-dev \
  libpng-dev \
  libtiff-dev \
  #libjasper-dev \
  libavformat-dev \
  libpq-dev \
  libeigen3-dev \
  libopencv-dev \
  && apt-get clean && rm -rf /var/lib/apt/list/*
  
#Comipling yourself is not needed for this usecase - installing opencv from repo is sufficient
#RUN cv_version=3.4.1 \
#  && cd / \
#  && wget https://github.com/opencv/opencv/archive/"$cv_version".zip -O opencv.zip \
#  && unzip opencv.zip \
#  && wget https://github.com/opencv/opencv_contrib/archive/"$cv_version".zip -O opencv_contrib.zip \
#  && unzip opencv_contrib \
#  && mkdir /opencv-"$cv_version"/build \
#  && cd /opencv-"$cv_version"/build \
#  && cmake -G Ninja \
#    -DSFM_DEPS_OK=TRUE \
#    -DOPENCV_EXTRA_MODULES_PATH=/opencv_contrib-"$cv_version"/modules \
#    -DBUILD_opencv_legacy=OFF \
#    -DBUILD_TIFF=ON \
#   -DENABLE_AVX=ON \
#    -DWITH_OPENGL=ON \
#    -DWITH_OPENCL=ON \
#    -DWITH_IPP=ON \
#    -DWITH_TBB=ON \
#    -DWITH_EIGEN=ON \
#    -DWITH_VTK=ON \
#    -DWITH_V4L=ON \
#    -DBUILD_EXAMPLES=OFF \
#    -DINSTALL_C_EXAMPLES=OFF \
#    -DINSTALL_PYTHON_EXAMPLES=OFF \
#    -DBUILD_TESTS=OFF \
#    -DBUILD_PERF_TESTS=OFF \
#    -DCMAKE_BUILD_TYPE=RELEASE \
#    .. \
#  && ninja -j4 install \
#  && rm /opencv.zip \
#  && rm /opencv_contrib.zip
#RUN ldconfig -v

#WORKDIR /data
COPY . .

#build plugins
RUN meson builddir && ninja -C builddir/ 
ENV GST_PLUGIN_PATH ./builddir/gst-plugin/
#docker build --tag metadata .
#SENDER:
#docker run -it --rm --net=host  metadata gst-launch-1.0 -v videotestsrc ! 'video/x-raw, width=(int)240, height=(int)240, framerate=(fraction)30/1' ! videoconvert ! metahandle modus=writer ! videoconvert  ! x264enc key-int-max=15 ! rtph264pay mtu=1300 ! meta2rtp modus=meta2rtp ! udpsink host=$TARGET_IP port=5555
#RECIEVER:
#docker run -it -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=unix$DISPLAY --rm --net=host  metadata gst-launch-1.0 -v udpsrc port=5555 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264" !  meta2rtp modus=rtp2meta ! rtph264depay  ! avdec_h264 ! videoconvert  ! metahandle modus=reader ! videoconvert  ! autovideosink

