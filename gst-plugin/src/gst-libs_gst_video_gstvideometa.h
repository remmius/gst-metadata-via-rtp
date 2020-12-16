/* GStreamer
 * Copyright (C) <2011> Wim Taymans <wim.taymans@gmail.com>
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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
 
 /*This is purly based on the code of https://github.com/GStreamer/gst-plugins-base/blob/master/gst-libs/gst/video/gstvideometa.h
 * Only real change is the tags definition, which is reduced to one video tag in order to pass the metadata in the rtp264depay-element. 
 * The remaining changes are name related
 * Added gint type_id to metadata to efficently transport the bounding box name as gint - however only for predefined names
 */

#ifndef __GST_MYVIDEO_META_H__
#define __GST_MYVIDEO_META_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <string.h>

G_BEGIN_DECLS

#define GST_CAPS_FEATURE_META_GST_MYVIDEO_META "meta:GstMyVideoMeta"

enum {dummy,face,box0,box1,box2};

const char * typeid_to_string(int type){
   switch(type) {
      case face: return "face";
      case box0: return "box0";
      case box1: return "box1";
      case box2: return "box2";
      default:   return "dummy";
   }
}
int string_to_typeid(const char* str){
    if(!strcmp(str,"face")) return face;
    else if (!strcmp (str,"box0")) return box1;
    else if (!strcmp (str,"box1")) return box2;
    else if (!strcmp (str,"box2")) return box2;
    else return dummy;
}
/**
 * GstMyVideoMeta:
 * @meta: parent #GstMeta 
 * @id: identifier of this particular ROI
 * @parent_id: identifier of its parent ROI, used f.i. for ROI hierarchisation.
 * @x: x component of upper-left corner
 * @y: y component of upper-left corner
 * @w: bounding box width
 * @h: bounding box height

Removed:
 * @roi_type: GQuark describing the semantic of the Roi (f.i. a face, a pedestrian) 
 *      ->does not work within different processes
 * @params: list of #GstStructure containing element-specific params for downstream, see gst_video_region_of_interest_meta_add_params(). (Since: 1.14)
 *      ->could be send on an addtional data-id header..but currently dont see a usecase here - add if neccessary
 *
 * Extra buffer metadata describing an image region of interest
 */
typedef struct {
  GstMeta meta;

  //GQuark roi_type;
  gint type_id;
  
  gint id;
  gint parent_id;

  guint x;
  guint y;
  guint w;
  guint h;

  //GList *params;
} GstMyMeta;

GST_VIDEO_API
GType              gst_myvideo_meta_api_get_type (void);
#define GST_MYVIDEO_META_API_TYPE (gst_myvideo_meta_api_get_type())
GST_VIDEO_API
const GstMetaInfo *gst_myvideo_meta_get_info (void);
#define GST_MYVIDEO_META_INFO (gst_myvideo_meta_get_info())

#define gst_buffer_get_myvideo_meta(b) \
        ((GstMyMeta*)gst_buffer_get_meta((b),GST_MYVIDEO_META_API_TYPE))

GST_VIDEO_API
GstMyMeta *gst_buffer_add_myvideo_meta_full (GstBuffer * buffer,gint id,gint parent_id,gint type_id,guint x,guint y,guint w, guint h);

//GST_VIDEO_API
//GstMyMeta *gst_buffer_get_myvideo_meta_id (GstBuffer   * buffer,gint id);

//GST_VIDEO_API
//GstMyMeta *gst_buffer_add_myvideo_meta    (GstBuffer   * buffer,gint type_id,guint x,guint y,guint w, guint h);

/*
GST_VIDEO_API
void gst_myvideo_meta_add_param (GstMyMeta * meta,GstStructure * s);

GST_VIDEO_API
GstStructure *gst_myvideo_meta_get_param (GstMyMeta * meta,const gchar * name);
*/
G_END_DECLS

#endif /* __GST_MYVIDEO_META_H__ */
