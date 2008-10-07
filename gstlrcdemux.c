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

GST_DEBUG_CATEGORY_STATIC (lrcdemux_debug);
#define GST_CAT_DEFAULT lrcdemux_debug

static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("text/lrc")
    );

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY
    );

static void gst_lrc_demux_base_init (GstLrcDemuxClass * klass);
static void gst_lrc_demux_class_init (GstLrcDemuxClass * klass);
static void gst_lrc_demux_init (GstLrcDemux * lrc, GstLrcDemuxClass * gclass);
static void gst_lrc_demux_finalize (GObject * object);


static void gst_lrc_demux_loop (GstPad * pad);
static gboolean gst_lrc_demux_sink_activate (GstPad * sinkpad);
static gboolean gst_lrc_demux_sink_activate_pull (GstPad * sinkpad,
    gboolean active);
static GstFlowReturn gst_lrc_demux_chain (GstPad * pad, GstBuffer * buf);

static GstStateChangeReturn gst_lrc_demux_change_state (GstElement * element,
    GstStateChange transition);

static GstElementClass *parent_class = NULL;

/* GObject methods */

GType
gst_lrc_demux_get_type (void)
{
  static GType lrc_demux_type = 0;

  if (!lrc_demux_type) {
    static const GTypeInfo lrc_demux_info = {
      sizeof (GstLrcDemuxClass),
      (GBaseInitFunc) gst_lrc_demux_base_init,
      NULL,
      (GClassInitFunc) gst_lrc_demux_class_init,
      NULL,
      NULL,
      sizeof (GstLrcDemux),
      0,
      (GInstanceInitFunc) gst_lrc_demux_init,
    };

    lrc_demux_type =
        g_type_register_static (GST_TYPE_ELEMENT,
        "GstLrcDemux", &lrc_demux_info, 0);
  }

  return lrc_demux_type;
}

static void
gst_lrc_demux_base_init (GstLrcDemuxClass * klass)
{
  static const GstElementDetails gst_lrc_demux_details =
      GST_ELEMENT_DETAILS ("lrc demuxer",
      "Codec/Demuxer",
      "Demultiplex a lrc file",
      "Zhao Liang <zlweb@163.com>");
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_set_details (element_class, &gst_lrc_demux_details);
}

static void
gst_lrc_demux_class_init (GstLrcDemuxClass * klass)
{
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
  GObjectClass *gobject_class = (GObjectClass *) klass;

  GST_DEBUG_CATEGORY_INIT (lrcdemux_debug, "lrcdemux",
      0, "Demuxer for lrc files");

  parent_class = g_type_class_peek_parent (klass);

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&srctemplate));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sinktemplate));
  
  gobject_class->finalize = gst_lrc_demux_finalize;
  gstelement_class->change_state =
      GST_DEBUG_FUNCPTR (gst_lrc_demux_change_state);
}

static void
gst_lrc_demux_init (GstLrcDemux * lrc, GstLrcDemuxClass * gclass)
{
  GstElementClass *element_klass = GST_ELEMENT_CLASS (gclass);
  GstPadTemplate *tmpl;
  
  lrc->sinkpad = gst_pad_new_from_static_template (&sinktemplate, "sink");

  gst_pad_set_activate_function (lrc->sinkpad,
      GST_DEBUG_FUNCPTR (gst_lrc_demux_sink_activate));
  gst_pad_set_activatepull_function (lrc->sinkpad,
      GST_DEBUG_FUNCPTR (gst_lrc_demux_sink_activate_pull));
  gst_pad_set_chain_function (lrc->sinkpad,
      GST_DEBUG_FUNCPTR (gst_lrc_demux_chain));
  gst_element_add_pad (GST_ELEMENT (lrc), lrc->sinkpad);

  lrc->srcpad = gst_pad_new_from_static_template (&srctemplate, "src");
  gst_element_add_pad (GST_ELEMENT (lrc), lrc->srcpad);

  lrc->lyricList = NULL;
  lrc->lyrics = NULL;
  lrc->album = NULL;
  lrc->artist = NULL;
  lrc->creator = NULL;
  lrc->title = NULL;
  lrc->offset = 0;
  lrc->parsed = FALSE;
}

static void
gst_lrc_demux_finalize (GObject * object)
{
  GstLrcDemux *lrc = GST_LRC_DEMUX (object);

  GST_DEBUG ("lrc: finalize");

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* parse string, if valid, store data*/
static gboolean
gst_lrc_parse_line(GstLrcDemux *lrc, const gchar* line)
{
  guint min, sec, hsec;
  gchar lyric[100];
  GstBuffer *buf;
  
  GST_DEBUG("line str: %s", line);
  if ( line[0] == '[' )
  {
    GstClockTime timestamp;
    if (strncmp(line, "[ti:", 4) == 0) {
    }
    else if (strncmp(line, "[ar:", 4) == 0) {
    }
    else if (strncmp(line, "[al:", 4) == 0) {
    }
    else if (strncmp(line, "[by:", 4) == 0) {
    }
    else if (strncmp(line, "[re:", 4) == 0) {
    }
    else if (strncmp(line, "[ve:", 4) == 0) {

    }
    else if (strncmp(line, "[offset:", 8) == 0) {

    }
    else {
      sscanf(line, "[%d:%d.%d]%s", &min, &sec, &hsec, lyric);
      GST_DEBUG("zhaoliang %d:%d.%d", min, sec, hsec);
      buf = gst_buffer_new_and_alloc(strlen(lyric)+1);
      memset(GST_BUFFER_DATA(buf), 0, strlen(lyric)+1);
      strcpy(GST_BUFFER_DATA(buf), lyric);
      timestamp = (min * 60 + sec )* GST_SECOND + hsec * 10 * GST_MSECOND;
      GST_BUFFER_TIMESTAMP(buf) = timestamp;
      lrc->lyricList = g_list_append(lrc->lyricList, buf);
      GST_DEBUG("append one");
    }
  }
  else
    GST_DEBUG("Invalid format, not support");
  
  return FALSE;
}

/* parse data line by line */
static gboolean
gst_lrc_parse_lyrics(GstLrcDemux *lrc)
{
  GstFlowReturn res = GST_FLOW_OK;
  GstBuffer *buf = NULL;
  guint obtained;
  guint64 offset = 0;
  gchar* line;
  gchar* needle = NULL;
  gchar* fragment = NULL;

  while(res == GST_FLOW_OK)  
  {
    res = gst_pad_pull_range (lrc->sinkpad, offset, LRC_BLOCK_SIZE, &buf);
    if ( res == GST_FLOW_OK )
    {
      guint buflen = 0;
      guint  linelen = 0;
      gchar* data;

      obtained = GST_BUFFER_SIZE (buf);
      data = GST_BUFFER_DATA(buf);

      buflen = obtained;
      do
      {
        needle = g_strstr_len(data, buflen, "\n");
        if (needle)
        {
          linelen = (needle - data);
          if (fragment)
          {
            gchar* tmp;

            tmp = g_strndup(data, linelen);
            line = g_strjoin(NULL, fragment, tmp, NULL);
            g_free(fragment);
            g_free(tmp);
            fragment = NULL;
            tmp = NULL;
          }
          else
          {
            line = g_strndup(data, linelen);
          }

          data += linelen+1;
          buflen -= linelen+1;
          //parse line
          gst_lrc_parse_line(lrc, line);
          g_free(line);
          line = NULL;
        }
      }while(needle != NULL);

      //handle fragment
      if (buflen != 0)
      {
        fragment = g_strndup(data, buflen);
      }
      gst_buffer_unref(buf);
    }
    offset += obtained;
    
    //check if we got EOS
    if (res == GST_FLOW_UNEXPECTED)
    {
      if (fragment)
      {
        gst_lrc_parse_line(lrc, fragment);
        g_free(fragment);
        fragment = NULL;
      }
      GST_DEBUG("receive eos");
      break;
    }
  }
  
  if (!lrc->lyricList)
    return FALSE;
}

static void
gst_lrc_demux_loop (GstPad * pad)
{
  gboolean ret;
  GstFlowReturn res;
  GstBuffer *buf;
  
  GstLrcDemux *lrc = GST_LRC_DEMUX (GST_PAD_PARENT (pad));

  if (!lrc->parsed)
  {
    ret = gst_lrc_parse_lyrics(lrc);
    if (!ret)
    {
      //report error
    }
    lrc->parsed = TRUE;
  lrc->lyricList = g_list_first(lrc->lyricList);
  }

  //start push buf from list
  if (lrc->lyricList)
  {
    buf = (GstBuffer *) lrc->lyricList->data;
    if (buf)
      res = gst_pad_push (lrc->srcpad, buf);
    lrc->lyricList = g_list_next(lrc->lyricList);
    GST_DEBUG("push data buf=%x", buf);
    if (!lrc->lyricList)
      gst_pad_push_event (lrc->srcpad, gst_event_new_eos ());
  }
  else
    gst_pad_pause_task (pad); 

  GST_LOG_OBJECT (lrc, "res:%s", gst_flow_get_name (res));
  return;
}

static GstFlowReturn
gst_lrc_demux_chain (GstPad * pad, GstBuffer * buf)
{
  GstFlowReturn res;
  GstLrcDemux *lrc = GST_LRC_DEMUX (GST_PAD_PARENT (pad));

  GST_DEBUG ("Store %d bytes ", GST_BUFFER_SIZE (buf));

  res = gst_pad_push (lrc->srcpad, buf);
  gst_buffer_unref (buf);
  return res;
}

static gboolean
gst_lrc_demux_sink_activate (GstPad * sinkpad)
{
  if (gst_pad_check_pull_range (sinkpad)) {
    GST_DEBUG ("going to pull mode");
    return gst_pad_activate_pull (sinkpad, TRUE);
  } else {
    GST_DEBUG ("going to push (streaming) mode");
    return gst_pad_activate_push (sinkpad, TRUE);
  }
}

static gboolean
gst_lrc_demux_sink_activate_pull (GstPad * sinkpad, gboolean active)
{
  GstLrcDemux *lrc = GST_LRC_DEMUX (GST_OBJECT_PARENT (sinkpad));

  if (active) {
    return gst_pad_start_task (sinkpad, (GstTaskFunction) gst_lrc_demux_loop,
        sinkpad);
  } else {
    return gst_pad_stop_task (sinkpad);
  }
}

static GstStateChangeReturn
gst_lrc_demux_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstLrcDemux *lrc = GST_LRC_DEMUX (element);

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

