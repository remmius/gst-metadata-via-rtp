project('gst-template', 'c','cpp', version : '1.17.0.1', license : 'LGPL')

plugins_install_dir = join_paths(get_option('libdir'), 'gstreamer-1.0')

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

gst_version = meson.project_version()

api_version = '1.0'

gst_dep = dependency('gstreamer-1.0',
    fallback : ['gstreamer', 'gst_dep'])
    
gst_rtp_dep = dependency('gstreamer-rtp-1.0')

gst_video_dep = dependency('gstreamer-video-1.0')

#gst_base_dep = dependency('gstreamer-base-1.0',fallback : ['gstreamer', 'gst_base_dep'])

subdir('gst-app')
subdir('gst-plugin')
