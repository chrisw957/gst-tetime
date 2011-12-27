/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2011 chris <<user@hostname.org>>
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
 * SECTION:element-tetime
 *
 * FIXME:Describe tetime here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! tetime ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gsttetime.h"

GST_DEBUG_CATEGORY_STATIC (gst_tetime_debug);
#define GST_CAT_DEFAULT gst_tetime_debug

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
  PROP_LATE
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

GST_BOILERPLATE (Gsttetime, gst_tetime, GstElement,
    GST_TYPE_ELEMENT);

static void gst_tetime_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_tetime_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_tetime_set_caps (GstPad * pad, GstCaps * caps);
static GstFlowReturn gst_tetime_chain (GstPad * pad, GstBuffer * buf);

/* GObject vmethod implementations */

static void
gst_tetime_base_init (gpointer gclass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "tetime",
    "FIXME:Generic",
    "FIXME:Generic Template Element",
    "chris <<user@hostname.org>>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the tetime's class */
static void
gst_tetime_class_init (GsttetimeClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_tetime_set_property;
  gobject_class->get_property = gst_tetime_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_LATE,
      g_param_spec_uint64 ("late", "Late",
          "Number of late frames", 0, G_MAXUINT64, 0,
          G_PARAM_READWRITE));

}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_tetime_init (Gsttetime * filter,
    GsttetimeClass * gclass)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_setcaps_function (filter->sinkpad,
                                GST_DEBUG_FUNCPTR(gst_tetime_set_caps));
  gst_pad_set_getcaps_function (filter->sinkpad,
                                GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));
  gst_pad_set_chain_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_tetime_chain));

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  gst_pad_set_getcaps_function (filter->srcpad,
                                GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));

  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
  filter->silent = FALSE;
}

static void
gst_tetime_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gsttetime *filter = GST_TETIME (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_tetime_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gsttetime *filter = GST_TETIME (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    case PROP_LATE:
      g_value_set_uint64 (value, filter->late);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles the link with other elements */
static gboolean
gst_tetime_set_caps (GstPad * pad, GstCaps * caps)
{
  Gsttetime *filter;
  GstPad *otherpad;

  filter = GST_TETIME (gst_pad_get_parent (pad));
  otherpad = (pad == filter->srcpad) ? filter->sinkpad : filter->srcpad;
  gst_object_unref (filter);

  return gst_pad_set_caps (otherpad, caps);
}

  char atarifont[13][16] = 
	{
	{0x00, 0x00, 0x3c, 0x3c, 0x66, 0x66, 0x6e, 0x6e, 0x76, 0x76, 0x66, 0x66, 0x3c, 0x3c, 0x00, 0x00},
	{0x00, 0x00, 0x18, 0x18, 0x38, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x7e, 0x00, 0x00},
	{0x00, 0x00, 0x3c, 0x3c, 0x66, 0x66, 0x0c, 0x0c, 0x18, 0x18, 0x30, 0x30, 0x7e, 0x7e, 0x00, 0x00},
        {0x00, 0x00, 0x7e, 0x7e, 0x0c, 0x0c, 0x18, 0x18, 0x0c, 0x0c, 0x66, 0x66, 0x3c, 0x3c, 0x00, 0x00},
        {0x00, 0x00, 0x0c, 0x0c, 0x1c, 0x1c, 0x3c, 0x3c, 0x6c, 0x6c, 0x7e, 0x7e, 0x0c, 0x0c, 0x00, 0x00},
        {0x00, 0x00, 0x7e, 0x7e, 0x60, 0x60, 0x7c, 0x7c, 0x06, 0x06, 0x66, 0x66, 0x3c, 0x3c, 0x00, 0x00},
        {0x00, 0x00, 0x3c, 0x3c, 0x60, 0x60, 0x7c, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x3c, 0x00, 0x00},
        {0x00, 0x00, 0x7e, 0x7e, 0x06, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00},
        {0x00, 0x00, 0x3c, 0x3c, 0x66, 0x66, 0x3c, 0x3c, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x3c, 0x00, 0x00},
        {0x00, 0x00, 0x3c, 0x3c, 0x66, 0x66, 0x3e, 0x3e, 0x06, 0x06, 0x0c, 0x0c, 0x38, 0x38, 0x00, 0x00},
        {0x00, 0x00, 0x06, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x30, 0x30, 0x60, 0x60, 0x40, 0x40, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00}, 
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	};

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_tetime_chain (GstPad * pad, GstBuffer * buf)
{
  Gsttetime *filter;
  GstCaps *caps = gst_buffer_get_caps(buf);
  guint16 *data = (guint16 *)GST_BUFFER_DATA(buf);
  GstStructure *structure = gst_caps_get_structure(caps, 0);
  GstClockTime in_ts, in_dur;
  GstBuffer *newbuf;

  int ch, row, column, offset;
  time_t rawtime;
  struct tm * timeinfo;
  char i, buffer[40];

  char m_width[] = "width";
  char m_height[] = "height"; 

  const GValue *width_gval = gst_structure_get_value(structure, m_width);
  const GValue *height_gval = gst_structure_get_value(structure, m_height);

  int width = g_value_get_int (width_gval);
  int height = g_value_get_int(height_gval); 

  filter = GST_TETIME (GST_OBJECT_PARENT (pad));

  in_ts = GST_BUFFER_TIMESTAMP (buf);
  in_dur = GST_BUFFER_DURATION (buf);
  //g_print("in_ts %" GST_TIME_FORMAT "\n", GST_TIME_ARGS(in_ts));
  //g_print("in_dur %" GST_TIME_FORMAT "\n\n", GST_TIME_ARGS(in_dur));

  if (in_ts > (filter->next_ts + (0.5)*(in_dur)))
        {
        // The incoming buffer is late by more than 1.5 times,
        // so we must have missed a frame.  To compensate, repeate
        // this one twice
        //g_print("#\n");
        filter->late++;
        //newbuf = gst_buffer_copy(buf);
        //GST_BUFFER_TIMESTAMP(newbuf) = filter->next_ts;
        //gst_pad_push(filter->srcpad, newbuf);
        }

  filter->next_ts = in_ts + in_dur;

  rawtime = time(NULL);
  timeinfo = localtime(&rawtime);
  strftime(buffer,80,"%m/%d/%y %H:%M:%S", timeinfo);
  //g_print("time %s\n",buffer);
  //g_print("buffer len %d\n",strlen(buffer));

/*
  if (filter->silent == FALSE)
    g_print ("I'm plugged, therefore I'm in.\n");
*/
  for(ch=0; ch<strlen(buffer); ch++) // ch = character
        {
        // convert character
        i = buffer[ch];
	if ((i >= 48) && (i <=57))
	   {
	   i = i - 48;
           }
        else if (i == 47)
           i = 10;
        else if (i == 58)
           i = 11;
        else
           i = 12;

        for(row=0; row<16; row++)
                {
                for(column=0; column<8; column=column+1)
                        {
                        // white
                        offset = (width * (16+row)) + ((ch+1)*16);
                        if ((atarifont[i][row] << column) & 0x80)
                                {
                                data[offset + (column*2)] = 0xeb80;
                                data[offset + (column*2) + 1] = 0xeb80;
                                }
                         }

                }
        for(row=0; row<16; row++)
                {
                for(column=0; column<8; column=column+1)
                        {
                        // black
                        offset = (width * (17+row)) + ((ch+1)*16);
                        if ((atarifont[i][row] << column) & 0x80)
                                {
                                data[offset + (column*2) +2 ] = 0x1080;
                                data[offset + (column*2) + 3] = 0x1080;
                                }
                         }

                }

        }

  /* just push out the incoming buffer without touching it */
  return gst_pad_push (filter->srcpad, buf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
tetime_init (GstPlugin * tetime)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template tetime' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_tetime_debug, "tetime",
      0, "Template tetime");

  return gst_element_register (tetime, "tetime", GST_RANK_NONE,
      GST_TYPE_TETIME);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirsttetime"
#endif

/* gstreamer looks for this structure to register tetimes
 *
 * exchange the string 'Template tetime' with your tetime description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "tetime",
    "Template tetime",
    tetime_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
