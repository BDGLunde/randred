/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2020 System Administrator <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
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
 * SECTION:element-randred
 *
 * FIXME:Describe randred here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! randred ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <time.h>
#include <stdlib.h>
#include <gst/gst.h>
#include <gst/video/video.h>

#include "gstrandred.h"

GST_DEBUG_CATEGORY_STATIC (randred_debug);
#define GST_CAT_DEFAULT randred_debug


/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

/* GstRandRed properties */
enum
{
  PROP_0,
  PROP_RED_CHANNEL
};  

#define DEFAULT_PROP_RED_CHANNEL 127

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ AYUV, "
            "ARGB, BGRA, ABGR, RGBA, Y444, "
            "xRGB, RGBx, xBGR, BGRx, RGB, BGR, Y42B, NV12, "
            "NV21, YUY2, UYVY, YVYU, I420, YV12, IYUV, Y41B }"))
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{ AYUV, "
            "ARGB, BGRA, ABGR, RGBA, Y444, "
            "xRGB, RGBx, xBGR, BGRx, RGB, BGR, Y42B, NV12, "
            "NV21, YUY2, UYVY, YVYU, I420, YV12, IYUV, Y41B }"))
    );

/* GObject handler declarations */
static void gst_randred_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gst_randred_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

/* GstBaseTransform handler declarations */
//static void gst_randred_before_transform (GstBaseTransform *transform, GstBuffer *buf);

/* GstVideoFilter handler declarations */
static gboolean gst_randred_set_info (GstVideoFilter *vfilter, GstCaps *incaps,
    GstVideoInfo *in_info, GstCaps *outcaps, GstVideoInfo *out_info);
static GstFlowReturn gst_randred_transform_frame_ip (GstVideoFilter *vfilter,
    GstVideoFrame *frame);


G_DEFINE_TYPE (GstRandRed, gst_randred, GST_TYPE_VIDEO_FILTER);
#define gst_randred_parent_class parent_class


/* GObject vmethod implementations */

/* initialize the randred's class */
static void
gst_randred_class_init (GstRandRedClass *g_class)
{
  /* Get different aspects/references to g_class */
  GObjectClass *gobject_class = (GObjectClass *)g_class;
  GstElementClass *gstelement_class = (GstElementClass *)g_class;
  GstBaseTransformClass *transform_class = (GstBaseTransformClass *)g_class;
  GstVideoFilterClass *videofilter_class = (GstVideoFilterClass *)g_class;

  GST_DEBUG_CATEGORY_INIT(randred_debug, "randred", GST_DEBUG_BG_YELLOW, "Log for randred");

  gobject_class->set_property = gst_randred_set_property;
  gobject_class->get_property = gst_randred_get_property;

  g_object_class_install_property(gobject_class, PROP_RED_CHANNEL,
      g_param_spec_int("red_channel", "Red", "red_channel",
          0, 254, 127,
          G_PARAM_READWRITE));

  gst_element_class_set_static_metadata(gstelement_class,
    "RandRed",
    "FIXME:Generic",
    "Randomly sets the red-channel of frame pixels to a random value",
    "Edward Lunde <<elunde01@gmail.com>>");

  gst_element_class_add_pad_template(gstelement_class, gst_static_pad_template_get(&src_factory));
  gst_element_class_add_pad_template (gstelement_class, gst_static_pad_template_get(&sink_factory));

  //transform_class->before_transform = GST_DEBUG_FUNCPTR(gst_my_filter_before_transform);
  transform_class->transform_ip_on_passthrough = FALSE;

  videofilter_class->set_info = GST_DEBUG_FUNCPTR(gst_randred_set_info);
  videofilter_class->transform_frame_ip = GST_DEBUG_FUNCPTR(gst_randred_transform_frame_ip);
}

/* initialize the new video-filter element
 */
static void
gst_randred_init (GstRandRed *filter)
{
  filter->red_channel = DEFAULT_PROP_RED_CHANNEL;
}

static void
gst_randred_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GstRandRed *filter = GST_RANDRED(object);

  switch (prop_id) {
    case PROP_RED_CHANNEL:{
      int val = g_value_get_int(value);

      GST_DEBUG("Changing red from %d to %d", filter->red_channel, val);
      GST_OBJECT_LOCK(filter);
      filter->red_channel = val;
      GST_OBJECT_UNLOCK (filter);
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void
gst_randred_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstRandRed *filter = GST_RANDRED(object);

  switch (prop_id) {
    case PROP_RED_CHANNEL:
      g_value_set_int(value, filter->red_channel);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  } 
}

int
gst_randred_map_epoch_seconds_to_pixel_depth() 
{
  time_t seconds;
  seconds = time(NULL);
  return (int)(seconds % 254);
}

int 
gst_randred_random_pixel_depth(int seed)
{
  time_t seconds;
  srand((unsigned) (time(&seconds)+seed));
  return (int)(rand() % 254);
}

static void
gst_randred_packed_rgb_ip (GstRandRed *filter, GstVideoFrame * frame)
{
  gint i, j, height;
  gint width, stride, row_wrap;
  gint pixel_stride;
  gint offsets[3];
  gint r, g, b;
  gint y, u, v;
  guint8 *data;

  data = GST_VIDEO_FRAME_PLANE_DATA(frame, 0);
  stride = GST_VIDEO_FRAME_PLANE_STRIDE(frame, 0);
  width = GST_VIDEO_FRAME_COMP_WIDTH(frame, 0);
  height = GST_VIDEO_FRAME_COMP_HEIGHT(frame, 0);

  offsets[0] = GST_VIDEO_FRAME_COMP_OFFSET (frame, 0);
  offsets[1] = GST_VIDEO_FRAME_COMP_OFFSET (frame, 1);
  offsets[2] = GST_VIDEO_FRAME_COMP_OFFSET (frame, 2);

  pixel_stride = GST_VIDEO_FRAME_COMP_PSTRIDE (frame, 0);
  row_wrap = stride - pixel_stride * width;

  GST_DEBUG("HELLO THERE");

  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      r = data[offsets[0]];
      g = data[offsets[1]];
      b = data[offsets[2]];
      
      data[offsets[0]] = gst_randred_random_pixel_depth(0);

      data += pixel_stride;
    }
    data += row_wrap;
  }
}

/* This is where we decide which version of our transform code to execute based on frame format 
*  This should execute when caps are received/negotiated(?)
*/
static gboolean
gst_randred_set_info(GstVideoFilter *videofilter, GstCaps *incaps, GstVideoInfo *in_info, GstCaps *outcaps, GstVideoInfo *out_info)
{
  GstRandRed *filter = GST_RANDRED(videofilter);

  GST_DEBUG_OBJECT (filter, "setting caps: in %" GST_PTR_FORMAT " out %" GST_PTR_FORMAT, incaps, outcaps);
  switch (GST_VIDEO_INFO_FORMAT (in_info)) {
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
    case GST_VIDEO_FORMAT_Y41B:
    case GST_VIDEO_FORMAT_Y42B:
    case GST_VIDEO_FORMAT_Y444:
    case GST_VIDEO_FORMAT_NV12:
    case GST_VIDEO_FORMAT_NV21:
        GST_DEBUG("OOH NOOOOOOOOO");
        filter->process = NULL;
        break;
    case GST_VIDEO_FORMAT_YUY2:
    case GST_VIDEO_FORMAT_UYVY:
    case GST_VIDEO_FORMAT_AYUV:
    case GST_VIDEO_FORMAT_YVYU:
      GST_DEBUG("NO YUY2!?!??!");
      filter->process = NULL;
      break;
    case GST_VIDEO_FORMAT_ARGB:
    case GST_VIDEO_FORMAT_ABGR:
    case GST_VIDEO_FORMAT_RGBA:
    case GST_VIDEO_FORMAT_BGRA:
    case GST_VIDEO_FORMAT_xRGB:
    case GST_VIDEO_FORMAT_xBGR:
    case GST_VIDEO_FORMAT_RGBx:
    case GST_VIDEO_FORMAT_BGRx:
    case GST_VIDEO_FORMAT_RGB:
    case GST_VIDEO_FORMAT_BGR:
      filter->process = gst_randred_packed_rgb_ip;
      break;
    default:
      goto invalid_caps;
      break;
  }
  return TRUE;

  /* ERRORS */
invalid_caps:
  {
    GST_ERROR_OBJECT (filter, "Invalid caps: %" GST_PTR_FORMAT, incaps);
    return FALSE;
  }
}

static GstFlowReturn
gst_randred_transform_frame_ip(GstVideoFilter *videofilter, GstVideoFrame *frame)
{
  GstRandRed *filter = GST_RANDRED(videofilter);

  if (!filter->process) 
  {
    GST_ERROR_OBJECT(filter, "Not negotiated yet :(");
    return GST_FLOW_NOT_NEGOTIATED;
  }

  /* i'm not sure why exactly we need this lock yet */
  GST_OBJECT_LOCK(filter);
  filter->process(filter, frame);
  GST_OBJECT_UNLOCK(filter);

  return GST_FLOW_OK;
}


/* PLUGIN/PACKAGE STUFF */

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
plugin_init (GstPlugin *plugin)
{
  return gst_element_register (plugin, "randred", GST_RANK_NONE,
      GST_TYPE_RANDRED);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "randred"
#endif

/* gstreamer looks for this structure to register randred
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    randred,
    "RandRed",
    plugin_init,
    PACKAGE_VERSION,
    GST_LICENSE,
    GST_PACKAGE_NAME,
    GST_PACKAGE_ORIGIN
)