/*******************************************************************************
 * pxgsettings - A helper binary to query gsettings
 * Copyright (C) 2006 Nathaniel McCallum <nathaniel@natemccallum.com>
 * Copyright (C) 2011 Dominique Leuenberger <dominique@leuenberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 ******************************************************************************/

#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include <stdexcept>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

using namespace std;

static GMainLoop* loop = NULL;

static int print_value(GVariant *value, const char *suffix) {

	if (!value) return 0;
	if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING)) {
		return printf("%s%s", g_variant_get_string(value, NULL), suffix);
	}
	else if(g_variant_is_of_type(value, G_VARIANT_TYPE_INT32)) {
		return printf("%d%s", g_variant_get_int32(value), suffix);
	}
	else if(g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN)) {
		gboolean result;
		result = g_variant_get_boolean(value);
		return printf("%s%s", result ? "true" : "false",  suffix);
	}
	else if(g_variant_is_of_type(value, G_VARIANT_TYPE_ARRAY)) {
		int count;
		const gchar** items;
		items = g_variant_get_strv(value, NULL);
		for (count=0; items[count]; count++) {
			printf("%s%s", count < 1 ? "" : ",",  items[count]);
		}
		printf("%s", suffix);
		return count;
	}
	else {
		throw exception();
	}

	return 0;
}

static void on_value_change(GSettings *settings, const gchar *key, gpointer user_data) {
	printf("%s/%s\t", (gchar *)user_data, key);
	print_value(g_settings_get_value(settings, key), "\n");
}

static void on_sig(int /*signal*/) {
	g_main_loop_quit(loop);
}

static gboolean err(GIOChannel* /*source*/, GIOCondition /*condition*/, gpointer /*data*/) {
	g_main_loop_quit(loop);
	return false;
}

static gboolean in(GIOChannel *source, GIOCondition condition, gpointer data) {
	gchar *key, *val;
	GIOStatus st = g_io_channel_read_line(source, &key, NULL, NULL, NULL);

	// Remove the trailing '\n'
	for (int i=0 ; key && key[i] ; i++)
		if (key[i] == '\n')
			key[i] = '\0';

	// If we were successful
	if (key && st == G_IO_STATUS_NORMAL) {
		if (!g_strrstr(key, "\t"))
			goto exit;

		val = g_strrstr(key, "\t") + 1;
		*(val-1) = '\0';

		g_free(key);
		return true;
	}
	else if (key && st == G_IO_STATUS_AGAIN) {
		g_free(key);
		return in(source, condition, data);
	}

exit:
	g_free(key);
	return err(source, condition, data);
}

int main(int argc, char **argv) {
	if (argc < 2) return 1;

	// Register sighup handler
	if (signal(SIGHUP, on_sig) == SIG_ERR || signal(SIGPIPE, on_sig) == SIG_ERR || signal(SIGABRT, on_sig) == SIG_ERR) {
		fprintf(stderr, "Unable to trap signals!");
		return 2;
	}

	// Switch stdout to line buffering
	if (setvbuf(stdout, NULL, _IOLBF, 0)) {
		fprintf(stderr, "Unable to switch stdout to line buffering!");
		return 3;
	}

	// Switch stdin to line buffering
	if (setvbuf(stdin, NULL, _IOLBF, 0)) {
		fprintf(stderr, "Unable to switch stdin to line buffering!");
		return 4;
	}

	// Init
	g_type_init();

	// Get the main loop
	loop = g_main_loop_new(NULL, false);

	// Setup our GIO Channels
	GIOChannel* inchan  = g_io_channel_unix_new(fileno(stdin));
	GIOChannel* outchan = g_io_channel_unix_new(fileno(stdout));
	g_io_add_watch(inchan,  G_IO_IN,  in, NULL);
	g_io_add_watch(inchan,  G_IO_PRI, in, NULL);
	g_io_add_watch(inchan,  G_IO_ERR, err, NULL);
	g_io_add_watch(inchan,  G_IO_HUP, err, NULL);
	g_io_add_watch(outchan, G_IO_ERR, err, NULL);
	g_io_add_watch(outchan, G_IO_HUP, err, NULL);

	// Get GConf client
	GSettings* client;

	for (int i=1; i<argc; i++) {
		client = g_settings_new(argv[i]);
		gchar** keys = g_settings_list_keys(client);
		for (int j=0; keys[j]; on_value_change(client, keys[j++],argv[i] ));
		g_signal_connect(client, "changed::", (GCallback) on_value_change, argv[i]);
	}


	g_main_loop_run(loop);

	// Cleanup
	while (G_IS_OBJECT(client)) {
		g_object_unref(client);
	}
	g_io_channel_shutdown(inchan,  FALSE, NULL);
	g_io_channel_shutdown(outchan, FALSE, NULL);
	g_io_channel_unref(inchan);
	g_io_channel_unref(outchan);
	g_main_loop_unref(loop);
}
