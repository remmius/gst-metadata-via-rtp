/*
 * GStreamer
 * Copyright (C) 2006 Stefan Kost <ensonic@users.sf.net>
 * Copyright (C) 2020 Klaus Hammer <<user@hostname.org>>
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
 
//works
//GST_PLUGIN_PATH=./builddir/gst-plugin/ GST_DEBUG=3 gst-launch-1.0 -v videotestsrc ! 'video/x-raw, width=(int)240, height=(int)240, framerate=(fraction)30/1' ! videoconvert ! metahandle modus=writer ! videoconvert  ! x264enc key-int-max=15 ! rtph264pay ! meta2rtp modus=meta2rtp ! udpsink host=127.0.0.1 port=5555
//GST_PLUGIN_PATH=./builddir/gst-plugin/ GST_DEBUG=3 gst-launch-1.0 -v udpsrc port=5555 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264" !  meta2rtp modus=rtp2meta ! rtph264depay  ! avdec_h264 ! videoconvert  ! metahandle modus=reader ! videoconvert  ! autovideosink

/**
 * SECTION:element-metahandle
 *
 * FIXME:Describe metahandle here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! metahandle ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/base.h>
#include <gst/controller/controller.h>

#include "gstmetahandle.h"

guint frame_count=0;

GST_DEBUG_CATEGORY_STATIC (gst_metahandle_debug);
#define GST_CAT_DEFAULT gst_metahandle_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT,
  PROP_MODUS,
};

//Plugin-Property Modus: PROP_METAHANDLE
enum
{
  MODUS_READER,
  MODUS_WRITER
};

static const GEnumValue modus_types[] = {
  {MODUS_READER, "Read out metadata from buffer and draw it", "reader"},
  {MODUS_WRITER, "Write dummy metadata to buffer as metadata", "writer"},
  {0, NULL, NULL},
};

#define GST_METAHANDLE_MODUS (gst_metahandle_modus_get_type())
static GType
gst_metahandle_modus_get_type (void)
{
  static GType modus_type = 0;

  if (!modus_type) {
    modus_type =
        g_enum_register_static ("GstMetaHandleModus", modus_types);
  }
  return modus_type;
}
//Plugin-Property Modus: PROP_METAHANDLE -END


/* the capabilities of the inputs and outputs.
 *
 * FIXME:describe the real formats here.
 */
static GstStaticPadTemplate sink_template =
GST_STATIC_PAD_TEMPLATE (
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("RGBx"))
);

static GstStaticPadTemplate src_template =
GST_STATIC_PAD_TEMPLATE (
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("RGBx"))
);

#define gst_metahandle_parent_class parent_class
//G_DEFINE_TYPE (Gstmetahandle, gst_metahandle, GST_TYPE_BASE_TRANSFORM);//doe snot work
G_DEFINE_TYPE (Gstmetahandle, gst_metahandle, GST_TYPE_ELEMENT);
//G_DEFINE_TYPE (Gstmetahandle, gst_metahandle, GST_TYPE_OPENCV_VIDEO_FILTER);

static void gst_metahandle_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_metahandle_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_metahandle_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_metahandle_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);
//static GstFlowReturn gst_metahandle_transform_ip (GstOpencvVideoFilter * filter, GstBuffer * buf, cv::Mat img);
//static GstFlowReturn gst_metahandle_transform_ip (GstBaseTransform * base,GstBuffer * outbuf);

/* GObject vmethod implementations */

/* initialize the metahandle's class */
static void
gst_metahandle_class_init (GstmetahandleClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  //GstOpencvVideoFilterClass *gstopencvbasefilter_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  //gstopencvbasefilter_class = (GstOpencvVideoFilterClass *) klass;
  //gstopencvbasefilter_class->cv_trans_ip_func = gst_metahandle_transform_ip;
  
  gobject_class->set_property = gst_metahandle_set_property;
  gobject_class->get_property = gst_metahandle_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
    g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE,(GParamFlags) (G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE)));
  g_object_class_install_property (gobject_class, PROP_MODUS,
      g_param_spec_enum ("modus", "Modus", "Specify modus of plugin: Read and draw meta or write dummy data as metadata to buffer",
        GST_METAHANDLE_MODUS,MODUS_READER,(GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  gst_element_class_set_details_simple (gstelement_class,
    "metahandle",
    "Generic/Filter",
    "Filter to insert gstmymeta to be send downstream  or to extract thm",
    "Klaus Hammer <<https://github.com/remmius>>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_template));

  //GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =GST_DEBUG_FUNCPTR (gst_metahandle_transform_ip);

  /* debug category for fltering log messages
   *
   * FIXME:exchange the string 'Template metahandle' with your description
   */ 
  GST_DEBUG_CATEGORY_INIT (gst_metahandle_debug, "metahandle", 0, "Template metahandle");
}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_metahandle_init (Gstmetahandle *filter)
{
  filter->silent = FALSE;
  filter->modus = MODUS_READER;
  
  filter->sinkpad = gst_pad_new_from_static_template (&sink_template, "sink");
  gst_pad_set_event_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_metahandle_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,GST_DEBUG_FUNCPTR(gst_metahandle_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template (&src_template, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
}

static void
gst_metahandle_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstmetahandle *filter = GST_METAHANDLE (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    case PROP_MODUS:
      filter->modus = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_metahandle_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstmetahandle *filter = GST_METAHANDLE (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    case PROP_MODUS:
      g_value_set_enum (value, filter->modus);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstBaseTransform vmethod implementations */

/* this function handles sink events */
static gboolean
gst_metahandle_sink_event (GstPad * pad, GstObject * parent, GstEvent * event)
{
  Gstmetahandle *filter;
  gboolean ret;

  filter = GST_METAHANDLE (parent);

  GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
    GstCaps * caps;

    gst_event_parse_caps (event, &caps);
      /* do something with the caps */
      //get size
    //GstCaps *caps=gst_pad_get_current_caps (filter->srcpad);
    GstStructure *s = gst_caps_get_structure(caps, 0);
    int width, height;
    gst_structure_get_int (s, "width", &width);
    gst_structure_get_int (s, "height", &height);
    g_print("size, %d %d \n",width,height);
    gst_caps_unref (caps);
    filter->width=width;
    filter->height=height;
      /* and forward */
    ret = gst_pad_event_default (pad, parent, event);
    break;
    }
    default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}

/* this function does the actual processing
 */
static GstFlowReturn
gst_metahandle_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
    Gstmetahandle *filter;

  filter = GST_METAHANDLE (parent);

  if(buf){
        //make buffer writable if not already 
        if(!gst_buffer_is_writable(buf)){
            buf=gst_buffer_make_writable(buf);
        }
        if(filter->modus == MODUS_READER){
            const GstMetaInfo *gstmetainfo_videoroi=gst_myvideo_meta_get_info();
        
            GstMyMeta * video_meta_rec;
            gpointer state=NULL;
            GstMapInfo info;
            gboolean mapped=gst_buffer_map (buf, &info, GST_MAP_WRITE);

            while((video_meta_rec =(GstMyMeta *) gst_buffer_iterate_meta_filtered(buf,&state,gstmetainfo_videoroi->api))){
                if(mapped){
                    // our rectangle...
                    cv::Rect rect(video_meta_rec->x,video_meta_rec->y, video_meta_rec->w, video_meta_rec->h);
                    cv::Mat img(filter->height,filter->width, CV_8UC4, info.data); // change your format accordingly
                    cv::rectangle(img, rect, cv::Scalar(0, 0, 0),1);
                    cv::Size textrect = cv::getTextSize(enum_to_string(video_meta_rec->type_id), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.0, 1, 0);
                    double scalex = (double)video_meta_rec->w / (double)textrect.width;
                    double scaley = (double)video_meta_rec->h / (double)textrect.height;
                    double scale = std::min(std::min(scalex, scaley),1.0);
                    cv::putText(img,enum_to_string(video_meta_rec->type_id),cv::Point(video_meta_rec->x,video_meta_rec->y+video_meta_rec->h), // Coordinates
                            cv::FONT_HERSHEY_COMPLEX_SMALL,scale, cv::Scalar(0,0,0), 1);
                }
            }
            gst_buffer_unmap (buf, &info);            
        }
        
        else{
            //create data to send ->should not be done in this plugin later
            guint number_data_sets_temp=3;
            //GstMyMeta *video_meta;
            for(guint n=0;n<number_data_sets_temp;n++){
                //video_roi_meta=gst_buffer_add_myvideo_meta(buf,"dummy",100*n,10*n+1,10*n+20,40);
                gst_buffer_add_myvideo_meta_full(buf,string_to_enum("dummy")+n,frame_count,1+n,100*n,10*n+1,10*n+20,40);

            }
        }
  }
  else{g_print("metahandle-buffer is empty \n");}
  frame_count=frame_count+1;
  /* just push out the incoming buffer without touching it */
  return gst_pad_push (filter->srcpad, buf);
} 
/*
static GstFlowReturn
gst_metahandle_transform_ip (GstOpencvVideoFilter * base, GstBuffer * buf, cv::Mat cvImg){
    g_print("alive and active \n");
    //cv::Mat resizedImg;
    
    //cv::Point tl, br;
    //tl = cv::Point(face.bounding_box.left(), face.bounding_box.top());
    //br = cv::Point(face.bounding_box.right(), face.bounding_box.bottom());
    //cv::rectangle (cvImg, tl, br, cv::Scalar (0, 255, 0));
    //cvImg.release ();
    return GST_FLOW_OK;
    }
*/
/*
static GstFlowReturn
gst_metahandle_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  Gstmetahandle *filter = GST_METAHANDLE (base);

  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (outbuf)))
    gst_object_sync_values (GST_OBJECT (filter), GST_BUFFER_TIMESTAMP (outbuf));

  if (filter->silent == FALSE)
    g_print ("I'm plugged, therefore I'm in.\n");
  
  // FIXME: do something interesting here.  This simply copies the source
  //to the destination. 

  return GST_FLOW_OK;
}
*/

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
metahandle_init (GstPlugin * metahandle)
{
  return gst_element_register (metahandle, "metahandle", GST_RANK_NONE,
      GST_TYPE_METAHANDLE);
}

/* gstreamer looks for this structure to register metahandles
 *
 * FIXME:exchange the string 'Template metahandle' with you metahandle description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    metahandle,
    "Template metahandle",
    metahandle_init,
    PACKAGE_VERSION,
    GST_LICENSE,
    GST_PACKAGE_NAME,
    GST_PACKAGE_ORIGIN
)
