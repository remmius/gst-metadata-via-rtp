/* 
 * GStreamer
 * Copyright (C) 2006 Stefan Kost <ensonic@users.sf.net>
 * Copyright (C) 2020 Niels De Graef <niels.degraef@gmail.com>
 * Copyright (C) 2020 klaus <<user@hostname.org>>
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
 
#ifndef __GST_META2RTP_H__
#define __GST_META2RTP_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

#include <gst/rtp/gstrtpbuffer.h>
#include <gst/video/gstvideometa.h>
#include "gst-libs_gst_video_gstvideometa.h"

G_BEGIN_DECLS


#define GST_TYPE_META2RTP \
  (gst_meta2rtp_get_type())

//G_DECLARE_FINAL_TYPE (Gstmeta2rtp, gst_meta2rtp,GST, PLUGIN_TEMPLATE, GstBaseTransform)
    
/* #defines don't like whitespacey bits */
#define GST_META2RTP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_META2RTP,Gstmeta2rtp))
  
#define GST_META2RTP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_META2RTP,Gstmeta2rtpClass))
  
#define GST_IS_META2RTP(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_META2RTP))
  
#define GST_IS_META2RTP_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_META2RTP))

typedef struct _Gstmeta2rtp Gstmeta2rtp;
typedef struct _Gstmeta2rtpClass Gstmeta2rtpClass;


struct _Gstmeta2rtp {
  GstBaseTransform element;
  
  GstPad *sinkpad, *srcpad;

  gboolean silent;
  gboolean modus;
  guint data_id;
  
  guint cur_frame;  
};

struct _Gstmeta2rtpClass 
{
  GstElementClass parent_class;
};

GType gst_meta2rtp_get_type (void);

G_END_DECLS

#endif /* __GST_META2RTP_H__ */
