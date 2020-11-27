plugin_c_args = ['-DHAVE_CONFIG_H']

cdata = configuration_data()
cdata.set_quoted('PACKAGE_VERSION', gst_version)
cdata.set_quoted('PACKAGE', 'gst-template-plugin')
cdata.set_quoted('GST_LICENSE', 'LGPL')
cdata.set_quoted('GST_API_VERSION', api_version)
cdata.set_quoted('GST_PACKAGE_NAME', 'GStreamer template Plug-ins')
cdata.set_quoted('GST_PACKAGE_ORIGIN', 'https://gstreamer.freedesktop.org')
configure_file(output : 'config.h', configuration : cdata)

inc = include_directories('src')
libmeta=shared_library('mymeta',
        'src/gst-libs_gst_video_gstvideometa.c',
        include_directories: inc,
        dependencies : [gst_dep,gst_video_dep],)
libmeta_dep=declare_dependency(include_directories: inc, link_with : libmeta)

# Plugin gstmeta2rtp
plugin_sources = [
  'src/gstmeta2rtp.c'
  ]

gstmeta2rtp = library('gstmeta2rtp',
  plugin_sources,
  c_args: plugin_c_args,
  dependencies : [gst_dep,gst_rtp_dep,gst_video_dep,libmeta_dep],
  install : true,
  install_dir : plugins_install_dir,
)

# Plugin metahandle
plugin_sources = [
  'src/gstmetahandle.cpp'
  ]
#https://github.com/cfoch/gst-plugins-cheese/blob/master/gst/face/gstcheesefacedetect.cpp
opencv_dep = dependency('opencv', version : '>= 3.0.0', required : false)
if not opencv_dep.found()
  opencv_dep = dependency('opencv4', version : '>= 4.0.0', required : false)
endif

gstmetahandle = library('gstmetahandle',
  plugin_sources,
  cpp_args : plugin_c_args,
  link_args : ['-lgstopencv-1.0'],
  dependencies : [gst_dep,gst_rtp_dep,gst_video_dep,libmeta_dep,opencv_dep],
  install : true,
  install_dir : plugins_install_dir,
)
