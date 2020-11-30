/* GStreamer
 * Copyright (C) <2011> Wim Taymans <wim.taymans@gmail.com>
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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
 
/*This is purly based on the code of https://github.com/GStreamer/gst-plugins-base/blob/master/gst-libs/gst/video/gstvideometa.c
 * Only real change is the tags definition, which is reduced to one video tag in order to pass the metadata in the rtp264depay-element. 
 * The remaining changes are name related.
 * Added gint type_id to metadata to efficently transport the bounding box name as gint - however only for predefined names
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gst-libs_gst_video_gstvideometa.h"

#ifndef GST_DISABLE_GST_DEBUG
#define GST_CAT_DEFAULT ensure_debug_category()
static GstDebugCategory *
ensure_debug_category (void)
{
  static gsize cat_gonce = 0;

  if (g_once_init_enter (&cat_gonce)) {
    gsize cat_done;

    cat_done = (gsize) _gst_debug_category_new ("myvideometa", 0, "myvideometa");

    g_once_init_leave (&cat_gonce, cat_done);
  }

  return (GstDebugCategory *) cat_gonce;
}
#else
#define ensure_debug_category() /* NOOP */
#endif /* GST_DISABLE_GST_DEBUG */


/* Region of Interest Meta implementation *******************************************/

GType
gst_myvideo_meta_api_get_type (void)
{
  static volatile GType type;
  static const gchar *tags[] =
  { GST_META_TAG_VIDEO_STR,NULL
  };
  /* If more than 1 tag is used, meta data does not pass rtph264depay 
     { GST_META_TAG_VIDEO_STR, GST_META_TAG_MEMORY_STR,
    GST_META_TAG_VIDEO_COLORSPACE_STR,
    GST_META_TAG_VIDEO_SIZE_STR, NULL
  };
  */

  if (g_once_init_enter (&type)) {
    GType _type =
        gst_meta_api_type_register ("GstMyVideoMetaAPI", tags);
    GST_INFO ("registering");
    g_once_init_leave (&type, _type);
  }
  return type;
}


static gboolean
gst_myvideo_meta_transform (GstBuffer * dest, GstMeta * meta,
    GstBuffer * buffer, GQuark type, gpointer data)
{
  GstMyMeta *dmeta, *smeta;

  if (GST_META_TRANSFORM_IS_COPY (type)) {
    smeta = (GstMyMeta *) meta;

    GST_DEBUG ("copy region of interest metadata");
    dmeta = gst_buffer_add_myvideo_meta_full (dest,smeta->id,smeta->parent_id,smeta->type_id,smeta->x,smeta->y,smeta->w,smeta->h);
        //gst_buffer_add_myvideo_meta_id (dest,
        //smeta->roi_type, smeta->x, smeta->y, smeta->w, smeta->h);
        
    if (!dmeta)
      return FALSE;

    //dmeta->id = smeta->id;
    //dmeta->parent_id = smeta->parent_id;
    //dmeta->type_id=smeta->type_id;
    //dmeta->params = g_list_copy_deep (smeta->params,
    //    (GCopyFunc) gst_structure_copy, NULL);
  } else if (GST_VIDEO_META_TRANSFORM_IS_SCALE (type)) {
    GstVideoMetaTransform *trans = data;
    gint ow, oh, nw, nh;
    ow = GST_VIDEO_INFO_WIDTH (trans->in_info);
    nw = GST_VIDEO_INFO_WIDTH (trans->out_info);
    oh = GST_VIDEO_INFO_HEIGHT (trans->in_info);
    nh = GST_VIDEO_INFO_HEIGHT (trans->out_info);
    GST_DEBUG ("scaling region of interest metadata %dx%d -> %dx%d", ow, oh, nw,
        nh);

    smeta = (GstMyMeta *) meta;
    dmeta =
        gst_buffer_add_myvideo_meta_full (dest,
        smeta->id,smeta->parent_id,smeta->type_id, (smeta->x * nw) / ow, (smeta->y * nh) / oh,
        (smeta->w * nw) / ow, (smeta->h * nh) / oh);
    if (!dmeta)
      return FALSE;

    //dmeta->id = smeta->id;
    //dmeta->parent_id = smeta->parent_id;
    //dmeta->type_id=smeta->type_id;
    GST_DEBUG ("region of interest (id:%d, parent id:%d) offset %dx%d -> %dx%d",
        smeta->id, smeta->parent_id, smeta->x, smeta->y, dmeta->x, dmeta->y);
    GST_DEBUG ("region of interest size   %dx%d -> %dx%d", smeta->w, smeta->h,
        dmeta->w, dmeta->h);
  } else {
    /* return FALSE, if transform type is not supported */
    return FALSE;
  }
  return TRUE;
}

static gboolean
gst_myvideo_meta_init (GstMeta * meta, gpointer params,
    GstBuffer * buffer)
{
  GstMyMeta *emeta = (GstMyMeta *) meta;
  //emeta->roi_type = 0;
  emeta->type_id=0;
  emeta->id = 0;
  emeta->parent_id = 0;
  emeta->x = emeta->y = emeta->w = emeta->h = 0;
  //emeta->params = NULL;

  return TRUE;
}

static void
gst_myvideo_meta_free (GstMeta * meta, GstBuffer * buffer)
{
  //GstMyMeta *emeta = (GstMyMeta *) meta;
 //g_list_free_full (emeta->params, (GDestroyNotify) gst_structure_free);
}

const GstMetaInfo *
gst_myvideo_meta_get_info (void)
{
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter ((GstMetaInfo **) & meta_info)) {
    const GstMetaInfo *mi =
        gst_meta_register (GST_MYVIDEO_META_API_TYPE,
        "GstMyVideoMeta",
        sizeof (GstMyMeta),
        gst_myvideo_meta_init,
        gst_myvideo_meta_free,
        gst_myvideo_meta_transform);
    g_once_init_leave ((GstMetaInfo **) & meta_info, (GstMetaInfo *) mi);
  }
  return meta_info;
}

/**
 * gst_buffer_get_video_region_of_interest_meta_id:
 * @buffer: a #GstBuffer
 * @id: a metadata id
 *
 * Find the #GstMyVideoMeta on @buffer with the given @id.
 *
 * Buffers can contain multiple #GstMyVideoMeta metadata items if
 * multiple regions of interests are marked on a frame.
 *
 * Returns: (transfer none): the #GstMyVideoMeta with @id or %NULL when there is
 * no such metadata on @buffer.
 */
GstMyMeta *
gst_buffer_get_myvideo_meta_id (GstBuffer * buffer, gint id)
{
  gpointer state = NULL;
  GstMeta *meta;
  const GstMetaInfo *info = GST_MYVIDEO_META_INFO;

  while ((meta = gst_buffer_iterate_meta (buffer, &state))) {
    if (meta->info->api == info->api) {
      GstMyMeta *vmeta =
          (GstMyMeta *) meta;
      if (vmeta->id == id)
        return vmeta;
    }
  }
  return NULL;
}

/**
 * gst_buffer_add_myvideo_meta_full:
 * @buffer: a #GstBuffer
 * @id: Id of the meta-data
 * @parent_id used for framecount
 * @type_id Type of the region of interest (e.g. 0,1,)
 * @x: X position
 * @y: Y position
 * @w: width
 * @h: height
 *
 * Attaches #GstMyVideoMeta metadata to @buffer with the given
 * parameters.
 *
 * Returns: (transfer none): the #GstMyVideoMeta on @buffer.
 */

GstMyMeta *
gst_buffer_add_myvideo_meta_full (GstBuffer * buffer,gint id,gint parent_id,gint type_id,guint x,guint y,guint w, guint h)
{
  GstMyMeta *meta;

  g_return_val_if_fail (GST_IS_BUFFER (buffer), NULL);

  meta = (GstMyMeta *) gst_buffer_add_meta (buffer,
      GST_MYVIDEO_META_INFO, NULL);
  meta->id = id;
  meta->parent_id = parent_id;
  meta->type_id = type_id;  
  meta->x = x;
  meta->y = y;
  meta->w = w;
  meta->h = h;

  return meta;
}

/**
 * gst_buffer_add_video_region_of_interest_meta:
 * @buffer: a #GstBuffer
 * @roi_type: Type of the region of interest (e.g. "face")
 * @x: X position
 * @y: Y position
 * @w: width
 * @h: height
 *
 * Attaches #GstMyVideoMeta metadata to @buffer with the given
 * parameters.
 *
 * Returns: (transfer none): the #GstMyVideoMeta on @buffer.
 */
/*
GstMyMeta *
gst_buffer_add_myvideo_meta (GstBuffer * buffer,
    const gchar * roi_type, guint x, guint y, guint w, guint h)
{
  return gst_buffer_add_myvideo_meta_id (buffer,
      g_quark_from_string (roi_type), x, y, w, h);
}
*/
/**
 * gst_buffer_add_video_region_of_interest_meta_id:
 * @buffer: a #GstBuffer
 * @roi_type: Type of the region of interest (e.g. "face")
 * @x: X position
 * @y: Y position
 * @w: width
 * @h: height
 *
 * Attaches #GstMyVideoMeta metadata to @buffer with the given
 * parameters.
 *
 * Returns: (transfer none): the #GstMyVideoMeta on @buffer.
 */
/*
GstMyMeta *
gst_buffer_add_myvideo_meta_id (GstBuffer * buffer,
    GQuark roi_type, guint x, guint y, guint w, guint h)
{
  GstMyMeta *meta;

  g_return_val_if_fail (GST_IS_BUFFER (buffer), NULL);

  meta = (GstMyMeta *) gst_buffer_add_meta (buffer,
      GST_MYVIDEO_META_INFO, NULL);
  meta->roi_type = roi_type;
  meta->x = x;
  meta->y = y;
  meta->w = w;
  meta->h = h;

  return meta;
}
*/

/**
 * gst_video_region_of_interest_meta_add_param:
 * @meta: a #GstMyVideoMeta
 * @s: (transfer full): a #GstStructure
 *
 * Attach element-specific parameters to @meta meant to be used by downstream
 * elements which may handle this ROI.
 * The name of @s is used to identify the element these parameters are meant for.
 *
 * This is typically used to tell encoders how they should encode this specific region.
 * For example, a structure named "roi/x264enc" could be used to give the
 * QP offsets this encoder should use when encoding the region described in @meta.
 * Multiple parameters can be defined for the same meta so different encoders
 * can be supported by cross platform applications).
 *
 * Since: 1.14
 */
 /*
void
gst_myvideo_meta_add_param (GstMyMeta *
    meta, GstStructure * s)
{
  g_return_if_fail (meta);
  g_return_if_fail (s);

  meta->params = g_list_append (meta->params, s);
}
*/
/**
 * gst_video_region_of_interest_meta_get_param:
 * @meta: a #GstMyVideoMeta
 * @name: a name.
 *
 * Retrieve the parameter for @meta having @name as structure name,
 * or %NULL if there is none.
 *
 * Returns: (transfer none) (nullable): a #GstStructure
 *
 * Since: 1.14
 * See also: gst_video_region_of_interest_meta_add_param()
 */
 /*
GstStructure *
gst_myvideo_meta_get_param (GstMyMeta *
    meta, const gchar * name)
{
  GList *l;

  g_return_val_if_fail (meta, NULL);
  g_return_val_if_fail (name, NULL);

  for (l = meta->params; l; l = g_list_next (l)) {
    GstStructure *s = l->data;

    if (gst_structure_has_name (s, name))
      return s;
  }

  return NULL;
}
*/
