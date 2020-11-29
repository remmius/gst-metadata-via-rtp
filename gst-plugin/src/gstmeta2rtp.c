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

//Testpipelines:
//GST_PLUGIN_PATH=~/gstreamer/gst-metadata-via-rtp/builddir/gst-plugin/ GST_DEBUG=3 gst-launch-1.0 -v videotestsrc ! 'video/x-raw, width=(int)240, height=(int)240, framerate=(fraction)30/1' ! videoconvert ! x264enc ! rtph264pay ! meta2rtp meta2rtp=true ! meta2rtp meta2rtp=false ! rtph264depay ! avdec_h264 ! autovideosink

/**
 * SECTION:element-meta2rtp
 *
 * FIXME:Describe meta2rtp here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! meta2rtp ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/base.h>
#include <gst/controller/controller.h>

#include "gstmeta2rtp.h"

GST_DEBUG_CATEGORY_STATIC (gst_meta2rtp_debug);
#define GST_CAT_DEFAULT gst_meta2rtp_debug

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
  PROP_META2RTP
};

/* the capabilities of the inputs and outputs.
 *
 * FIXME:describe the real formats here.
 */
static GstStaticPadTemplate sink_template =
GST_STATIC_PAD_TEMPLATE (
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"video\", "
        "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
        "clock-rate = (int) 90000, " "encoding-name = (string) \"H264\"")
);

static GstStaticPadTemplate src_template =
GST_STATIC_PAD_TEMPLATE (
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"video\", "
        "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
        "clock-rate = (int) 90000, " "encoding-name = (string) \"H264\"")
);

#define gst_meta2rtp_parent_class parent_class
G_DEFINE_TYPE (Gstmeta2rtp, gst_meta2rtp, GST_TYPE_ELEMENT);
//G_DEFINE_TYPE (Gstmeta2rtp, gst_meta2rtp, GST_TYPE_BASE_TRANSFORM);

static void gst_meta2rtp_set_property (GObject * object, guint prop_id,const GValue * value, GParamSpec * pspec);
static void gst_meta2rtp_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);

//static GstFlowReturn gst_meta2rtp_transform_ip (GstBaseTransform * base,GstBuffer * outbuf);
static gboolean gst_meta2rtp_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_meta2rtp_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);


/* GObject vmethod implementations */

/* initialize the meta2rtp's class */
static void
gst_meta2rtp_class_init (Gstmeta2rtpClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_meta2rtp_set_property;
  gobject_class->get_property = gst_meta2rtp_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
    g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));
  g_object_class_install_property (gobject_class, PROP_META2RTP,
      g_param_spec_boolean ("meta2rtp", "Meta2RTP", "True: Convert Meta to RTP-Header; False: Convert RTP-Header to Meta",
          FALSE, G_PARAM_READWRITE));
  gst_element_class_set_details_simple (gstelement_class,
    "meta2rtp",
    "Generic/Filter",
    "FIXME:Generic Template Filter",
    "Klaus Hammer <<user@hostname.org>>");

  gst_element_class_add_pad_template (gstelement_class,gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (gstelement_class,gst_static_pad_template_get (&sink_template));

  //GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =GST_DEBUG_FUNCPTR (gst_meta2rtp_transform_ip);

  /* debug category for filtering log messages
   *
   * FIXME:exchange the string 'Template meta2rtp' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_meta2rtp_debug, "meta2rtp", 0, "Template meta2rtp");
}

static gboolean
gst_meta2rtp_sink_event (GstPad * pad, GstObject * parent, GstEvent * event)
{
  Gstmeta2rtp *filter;
  gboolean ret;

  filter = GST_META2RTP (parent);

  GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps * caps;

      gst_event_parse_caps (event, &caps);
      /* do something with the caps */

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

/* initialize the new element
 * initialize instance structure
 */
static void
gst_meta2rtp_init (Gstmeta2rtp *filter)
{
  filter->silent = FALSE;
  filter->meta2rtp=TRUE;
  filter->cur_frame=0;
  
  filter->sinkpad = gst_pad_new_from_static_template (&sink_template, "sink");
  gst_pad_set_event_function (filter->sinkpad,GST_DEBUG_FUNCPTR(gst_meta2rtp_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,GST_DEBUG_FUNCPTR(gst_meta2rtp_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template (&src_template, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
  
}

static void
gst_meta2rtp_set_property (GObject * object, guint prop_id,const GValue * value, GParamSpec * pspec)
{
  Gstmeta2rtp *filter = GST_META2RTP (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    case PROP_META2RTP:
      filter->meta2rtp = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_meta2rtp_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstmeta2rtp *filter = GST_META2RTP (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    case PROP_META2RTP:
      g_value_set_boolean (value, filter->meta2rtp);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstBaseTransform vmethod implementations */
guint ID_DATA=5;

/* this function does the actual processing
 */
 
 /*
static GstFlowReturn
gst_meta2rtp_transform_ip (GstBaseTransform * base, GstBuffer * buf)
{
  Gstmeta2rtp *filter = GST_META2RTP (base);

  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (buf)))
    gst_object_sync_values (GST_OBJECT (filter), GST_BUFFER_TIMESTAMP (buf));
   ////when is this actually called?
  return GST_FLOW_OK;
}
*/
static GstFlowReturn
gst_meta2rtp_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
  Gstmeta2rtp *filter;

  filter = GST_META2RTP (parent);
  //make buffer writable if not already
 
  if(buf){
      
      if(!gst_buffer_is_writable(buf)){
        buf=gst_buffer_make_writable(buf);
      }      
      if (filter->meta2rtp == TRUE ){
           
            //write data to rtp-header
            //read out data from meta-data from buffer            
            const GstMetaInfo *gstmetainfo_videoroi=gst_myvideo_meta_get_info(); 
            guint number_data_sets=gst_buffer_get_n_meta(buf,gstmetainfo_videoroi->api);     
            gpointer state=NULL;
            
            GstMyMeta write_data[number_data_sets];
            GstMyMeta * video_meta_write;
            for(int k=0;k<number_data_sets;k++){
                video_meta_write = gst_buffer_iterate_meta_filtered(buf,&state,gstmetainfo_videoroi->api);
                //better would be to read out all meta-memory as a block to avoid copying of metadata
                write_data[k]=*video_meta_write;
                //g_print("meta2rtp-writer: %d\n", write_data[k].parent_id);
            }
            //prepare buffer
            GstRTPBuffer rtpbuf;
            memset (&rtpbuf, 0, sizeof(GstRTPBuffer));
            gst_rtp_buffer_map (buf,(GstMapFlags)GST_MAP_WRITE, &rtpbuf);
            
            //write data to buffer    
            guint data_size=sizeof(write_data);
            gpointer gp_data= write_data;
            gst_rtp_buffer_add_extension_twobytes_header(&rtpbuf,0,ID_DATA,gp_data,data_size);
                    
            //free buffer
            gst_rtp_buffer_unmap(&rtpbuf);
            
        }        
      else{          
        //Read data
        //Prepare buffer
        GstRTPBuffer rtpbuf_read;
        memset (&rtpbuf_read, 0, sizeof(GstRTPBuffer));
        gst_rtp_buffer_map (buf,(GstMapFlags)GST_MAP_WRITE, &rtpbuf_read);
        
        //Read buffer
        guint8  appbit;
        gpointer gp_data;
        guint data_size;
        //g_print ("rtp2meta 0 \n");
        
        if(gst_rtp_buffer_get_extension_twobytes_header(&rtpbuf_read,&appbit,ID_DATA,0,&gp_data,&data_size)){
            guint numb_data_sets=data_size/sizeof(GstMyMeta);
            GstMyMeta *video_meta_rec;
            GstMyMeta *video_meta_write;
            //handle data
            //g_print("rtp2meta datasets %d \n",numb_data_sets);
            
            if(numb_data_sets>0){                
                video_meta_rec=&((GstMyMeta *)gp_data)[0];
                g_print("rtp2meta-frame: %d\n", video_meta_rec->parent_id);    
                if(video_meta_rec->parent_id != filter->cur_frame){//RTP splits frame in several package - only need meta-data once: but order seems to be messed up even streamed locally
                    for(guint n=0;n<numb_data_sets;n++){
                        
                        video_meta_rec=&((GstMyMeta *)gp_data)[n];
                        //Store result in buffer-meta data
                        video_meta_write=gst_buffer_add_myvideo_meta_id(buf,video_meta_rec->roi_type,video_meta_rec->x,video_meta_rec->y,video_meta_rec->w,video_meta_rec->h);
                        video_meta_write->id=video_meta_rec->id;
                        video_meta_write->parent_id=video_meta_rec->parent_id;
                        //TODO copy glist params
                        //video_meta_write=gst_buffer_add_meta(buf,gst_myvideo_meta_get_info(),NULL);                       
                        //*video_meta_write=*video_meta_rec;//write meta-data into buffer-memory //this does not work due to the GList *params field in the struct...
                        //gst_myvideo_meta_add_param(video_meta_write,gst_structure_new_empty("roi/metahandle"));
                        
                    }
                    //update cur_frame
                    filter->cur_frame=video_meta_rec->parent_id;
                    //g_print(gst_structure_to_string (gst_myvideo_meta_get_param(video_meta_write,"roi/drawbox")));
                }
                //else{g_print("meta2rtp-reader-duplicated meta");//maybe do: gst_buffer_remove_meta?}
                
            }
             
        }
        else{g_print("no data");}
        
        //free rtp-buffer
        gst_rtp_buffer_unmap (&rtpbuf_read);
        }
  }
  //g_print ("rtp2meta beforeend \n");
  /* just push out the incoming buffer without touching it */
  //GstFlowReturn test=gst_pad_push (filter->srcpad, buf);
  return gst_pad_push (filter->srcpad, buf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
meta2rtp_init (GstPlugin * meta2rtp)
{
  return gst_element_register (meta2rtp, "meta2rtp", GST_RANK_NONE,
      GST_TYPE_META2RTP);
}

/* gstreamer looks for this structure to register meta2rtps
 *
 * FIXME:exchange the string 'Template meta2rtp' with you meta2rtp description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    meta2rtp,
    "Template meta2rtp",
    meta2rtp_init,
    PACKAGE_VERSION,
    GST_LICENSE,
    GST_PACKAGE_NAME,
    GST_PACKAGE_ORIGIN
)
