/* GStreamer
 * Copyright (C) <2008> Zhao Liang <zlweb@163.com>
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

#ifndef __GST_LRC_DEMUX_H__
#define __GST_LRC_DEMUX_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_LRC_DEMUX \
  (gst_lrc_demux_get_type ())
#define GST_LRC_DEMUX(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_LRC_DEMUX, GstLrcDemux))
#define GST_LRC_DEMUX_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_LRC_DEMUX, GstLrcDemuxClass))
#define GST_IS_LRC_DEMUX(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_LRC_DEMUX))
#define GST_IS_LRC_DEMUX_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_LRC_DEMUX))

#define LRC_BLOCK_SIZE 50

typedef struct _GstLrcDemux {
  GstElement     parent;

  /* pads */
  GstPad        *sinkpad;
  GstPad        *srcpad;

  /* properity*/
  gchar* lyrics;
  gchar* title;
  gchar* artist;
  gchar* album;
  gchar* creator;
  gint offset;
  
  /* private data */
  GList *lyricList;
  gboolean parsed;
} GstLrcDemux;

typedef struct _GstLrcDemuxClass {
  GstElementClass parent_class;
} GstLrcDemuxClass;

GType           gst_lrc_demux_get_type (void);

G_END_DECLS

#endif /* __GST_LRC_DEMUX_H__ */

