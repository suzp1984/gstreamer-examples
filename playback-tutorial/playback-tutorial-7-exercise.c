#include <gst/gst.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

int tutorial_main(int argc, char *argv[]) {
  GstElement *pipeline, *bin, *videoeffect, *convert, *sink;
  GstPad *pad, *ghost_pad;
  GstBus *bus;
  GstMessage *msg;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Build the pipeline */
  pipeline = gst_parse_launch ("playbin uri=https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm", NULL);

  /* Create the elements inside the sink bin */
  videoeffect = gst_element_factory_make ("vertigotv", "effectv");
  convert = gst_element_factory_make ("videoconvert", "convert");
  sink = gst_element_factory_make ("autovideosink", "video_sink");
  if (!videoeffect || !convert || !sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Create the sink bin, add the elements and link them */
  bin = gst_bin_new ("video_sink_bin");
  gst_bin_add_many (GST_BIN (bin), videoeffect, convert, sink, NULL);
  gst_element_link_many (videoeffect, convert, sink, NULL);
  pad = gst_element_get_static_pad (videoeffect, "sink");
  ghost_pad = gst_ghost_pad_new ("sink", pad);
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (bin, ghost_pad);
  gst_object_unref (pad);

  /* Configure the videoeffect */
  g_object_set (G_OBJECT (videoeffect), "speed", (gfloat)50.0, NULL);
  g_object_set (G_OBJECT (videoeffect), "zoom-speed", (gfloat)1.1, NULL);

  /* Set playbin's video sink to be our sink bin */
  g_object_set (GST_OBJECT (pipeline), "video-sink", bin, NULL);

  /* Start playing */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);
  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

  /* Free resources */
  if (msg != NULL)
    gst_message_unref (msg);
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}

int
main (int argc, char *argv[])
{
#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
    return gst_macos_main ((GstMainFunc) tutorial_main, argc, argv, NULL);
#else
    return tutorial_main (argc, argv);
#endif
}
