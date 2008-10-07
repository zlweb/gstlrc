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
 * SECTION:element-lrcsink
 *
 * <refsect2>
 * <para>
 * Sinkes an .lrc file into raw or compressed audio and/or video streams.
 * </para>
 * <para>
 * This element supports both push and pull-based scheduling, depending on the
 * capabilities of the upstream elements.
 * </para>
 * <title>Example launch line</title>
 * <para>
 * <programlisting>
 * gst-launch filesrc location=test.lrc ! lrcsink
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
#include "gstlrcsink.h"

GST_DEBUG_CATEGORY_STATIC (lrcsink_debug);
#define GST_CAT_DEFAULT lrcsink_debug

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("text/lrc")
    );

static void gst_lrc_sink_base_init (GstLrcSinkClass * klass);
static void gst_lrc_sink_class_init (GstLrcSinkClass * klass);
static void gst_lrc_sink_init (GstLrcSink * lrc, GstLrcSinkClass * gclass);
static void gst_lrc_sink_finalize (GObject * object);
static GstFlowReturn gst_lrc_sink_chain (GstPad * pad, GstBuffer * buf);

static GstStateChangeReturn gst_lrc_sink_change_state (GstElement * element,
    GstStateChange transition);

static GstElementClass *parent_class = NULL;

/* GObject methods */

GType
gst_lrc_sink_get_type (void)
{
  static GType lrc_sink_type = 0;

  if (!lrc_sink_type) {
    static const GTypeInfo lrc_sink_info = {
      sizeof (GstLrcSinkClass),
      (GBaseInitFunc) gst_lrc_sink_base_init,
      NULL,
      (GClassInitFunc) gst_lrc_sink_class_init,
      NULL,
      NULL,
      sizeof (GstLrcSink),
      0,
      (GInstanceInitFunc) gst_lrc_sink_init,
    };

    lrc_sink_type =
        g_type_register_static (GST_TYPE_ELEMENT,
        "GstLrcSink", &lrc_sink_info, 0);
  }

  return lrc_sink_type;
}

static void
gst_lrc_sink_base_init (GstLrcSinkClass * klass)
{
  static const GstElementDetails gst_lrc_sink_details =
      GST_ELEMENT_DETAILS ("lrc sink",
      "Sink/LRCSink",
      "sink lyrics",
      "Zhao Liang <zlweb@163.com>");
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_set_details (element_class, &gst_lrc_sink_details);
}

static void
gst_lrc_sink_class_init (GstLrcSinkClass * klass)
{
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
  GObjectClass *gobject_class = (GObjectClass *) klass;

  GST_DEBUG_CATEGORY_INIT (lrcsink_debug, "lrcsink",
      0, "Sink for lrc files");

  parent_class = g_type_class_peek_parent (klass);

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sinktemplate));
  
  gobject_class->finalize = gst_lrc_sink_finalize;
  gstelement_class->change_state =
      GST_DEBUG_FUNCPTR (gst_lrc_sink_change_state);
}

static void
gst_lrc_sink_init (GstLrcSink * lrc, GstLrcSinkClass * gclass)
{
  GstElementClass *element_klass = GST_ELEMENT_CLASS (gclass);
  GstPadTemplate *tmpl;
  
  lrc->sinkpad = gst_pad_new_from_static_template (&sinktemplate, "sink");
  gst_element_add_pad (GST_ELEMENT (lrc), lrc->sinkpad);

}

static void
gst_lrc_sink_finalize (GObject * object)
{
  GstLrcSink *lrc = GST_LRC_SINK (object);

  GST_DEBUG ("lrc: finalize");

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GstFlowReturn
gst_lrc_sink_chain (GstPad * pad, GstBuffer * buf)
{
  GstFlowReturn res;
  GstLrcSink *lrc = GST_LRC_SINK (GST_PAD_PARENT (pad));

  GST_DEBUG ("Store %d bytes ", GST_BUFFER_SIZE (buf));

  gst_buffer_unref (buf);
  return res;
}

static GstStateChangeReturn
gst_lrc_sink_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstLrcSink *lrc = GST_LRC_SINK (element);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    goto done;

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:

      break;
    default:
      break;
  }

done:
  return ret;
}

