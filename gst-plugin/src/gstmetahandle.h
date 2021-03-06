/* 
 * GStreamer
 * Copyright (C) 2006 Stefan Kost <ensonic@users.sf.net>
 * Copyright (C) 2020 Niels De Graef <niels.degraef@gmail.com>
 * Copyright (C) 2020 Klaus Hammer <klaushammer52@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 
#ifndef __GST_METAHANDLE_H__
#define __GST_METAHANDLE_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

#include <gst/rtp/gstrtpbuffer.h>
#include <gst/video/gstvideometa.h>
#include "gst-libs_gst_video_gstvideometa.h"


#include "opencv2/opencv.hpp"
#include <opencv2/core/core_c.h>
#if (CV_MAJOR_VERSION >= 3)
#include <opencv2/imgproc/imgproc_c.h>
#endif

#include <gst/opencv/gstopencvutils.h>
#define COLOR_blue  cv::Scalar(255,0,0)
#define COLORS_black  cv::Scalar(0,0,0)
#define COLORS_cyan  cv::Scalar(255,255,0)
#define COLORS_darkred  cv::Scalar(0,0,139)

cv::Scalar typeid_to_cv_color(int type){
    switch(type) {
      case face: return COLOR_blue;
      case box0: return COLORS_black;
      case box1: return COLORS_cyan;
      case box2: return COLORS_darkred;
      default:   return COLORS_black;
   }
}

G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_METAHANDLE \
  (gst_metahandle_get_type())
#define GST_METAHANDLE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_METAHANDLE,Gstmetahandle))
#define GST_METAHANDLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_METAHANDLE,GstmetahandleClass))
#define GST_IS_METAHANDLE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_METAHANDLE))
#define GST_IS_METAHANDLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_METAHANDLE))

typedef struct _Gstmetahandle      Gstmetahandle;
typedef struct _GstmetahandleClass GstmetahandleClass;

struct _Gstmetahandle
{
  //GstOpencvVideoFilter element;
  GstElement element;
  
  GstPad *sinkpad, *srcpad;

  gboolean silent;
  gboolean modus;
  gboolean true_color;
  
  //add "metadata-array"?
  
  guint width;
  guint height;
  guint cv_type;
  guint format;
  
};

struct _GstmetahandleClass 
{
  //GstOpencvVideoFilterClass parent_class;
  GstElementClass parent_class;
};

GType gst_drawbox_get_type (void);

G_END_DECLS

#endif /* __GST_METAHANDLE_H__ */
