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

/**
 * SECTION:element-lrcdemux
 *
 * <refsect2>
 * <para>
 * Demuxes an .lrc file into raw or compressed audio and/or video streams.
 * </para>
 * <para>
 * This element supports both push and pull-based scheduling, depending on the
 * capabilities of the upstream elements.
 * </para>
 * <title>Example launch line</title>
 * <para>
 * <programlisting>
 * gst-launch filesrc location=test.lrc ! lrcdemux name=demux  demux.audio_00 ! decodebin ! audioconvert ! audioresample ! autoaudiosink   demux.video_00 ! queue ! decodebin ! ffmpegcolorspace ! videoscale ! autovideosink
 * </programlisting>
 * Play (parse and decode) an .lrc file and try to output it to
 * an automatically detected soundcard and videosink. If the lrc file contains
 * compressed audio or video data, this will only work if you have the
 * right decoder elements/plugins installed.
 * </para>
 * </refsect2>
 *
 * Last reviewed on 2006-12-29 (0.10.6)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "gstlrcdemux.h"
#include "gstlrcsink.h"

static gboolean
plugin_init (GstPlugin * plugin)
{
  gst_element_register (plugin, "lrcdemux",
      GST_RANK_PRIMARY, GST_TYPE_LRC_DEMUX);
  
  gst_element_register (plugin, "lrcsink",
      GST_RANK_PRIMARY, GST_TYPE_LRC_SINK);

  return TRUE;
}

#if 0
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "lrc",
    "lrc song code", plugin_init, VERSION, GST_LICENSE,
    GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "lrc",
    "lrc song code",
    plugin_init, VERSION, "Proprietary", "GStreamer", "www.motorola.com")

