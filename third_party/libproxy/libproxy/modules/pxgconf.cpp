#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include <stdexcept>

#include <glib.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

using namespace std;

static GMainLoop* loop = NULL;

static int print_value(const GConfValue *value, const char *suffix) {
	int count = 0;
	GSList* cursor = NULL;

	if (!value) return 0;

	switch (value->type) {
	case GCONF_VALUE_STRING:
		return printf("%s%s", gconf_value_get_string(value), suffix);
	case GCONF_VALUE_INT:
		return printf("%d%s", gconf_value_get_int(value), suffix);
	case GCONF_VALUE_FLOAT:
		return printf("%f%s", gconf_value_get_float(value), suffix);
	case GCONF_VALUE_BOOL:
		if (gconf_value_get_bool(value))
			return printf("true%s", suffix);
		return printf("false%s", suffix);
	case GCONF_VALUE_LIST:
		cursor = gconf_value_get_list(value);
		if (g_slist_length (cursor) == 0)
			count += printf("%s", suffix);
		else for ( ; cursor ; cursor = g_slist_next(cursor))
			count += print_value((const GConfValue *) cursor->data, cursor->next ? "," : suffix);
		return count;
	case GCONF_VALUE_PAIR:
		return  print_value(gconf_value_get_car(value), ",") +
			print_value(gconf_value_get_cdr(value), suffix);
	default:
		throw exception();
	}


	return 0;
}

static void on_value_change(GConfClient* /*client*/, guint /*cnxn_id*/, GConfEntry* entry, void* /*user_data*/) {
	printf("%s\t", gconf_entry_get_key(entry));
	print_value(gconf_entry_get_value(entry), "\n");
}

static void on_sig(int /*signal*/) {
	g_main_loop_quit(loop);
}

static gboolean err(GIOChannel* /*source*/, GIOCondition /*condition*/, gpointer /*data*/) {
	g_main_loop_quit(loop);
	return false;
}

static gboolean set_key(const char *key, const char *val) {
	gboolean error = false;
	GConfClient *client = gconf_client_get_default();
	GConfValue  *value  = gconf_client_get(client, key, NULL);
	GConfValueType type = value ? value->type : GCONF_VALUE_STRING;
	gconf_value_free(value);

	switch (type) {
		case GCONF_VALUE_STRING:
			error = !gconf_client_set_string(client, key, val, NULL);
			break;
		case GCONF_VALUE_INT:
			int ival;
			error = sscanf(val, "%d", &ival) != 1;
			error = error || !gconf_client_set_int(client, key, ival, NULL);
			break;
		case GCONF_VALUE_FLOAT:
			float fval;
			error = sscanf(val, "%f", &fval) != 1;
			error = error || !gconf_client_set_float(client, key, fval, NULL);
			break;
		case GCONF_VALUE_BOOL:
			error = !gconf_client_set_float(client, key, !g_strcmp0(val, "true"), NULL);
			break;
		case GCONF_VALUE_LIST:
		case GCONF_VALUE_PAIR:
		default:
			g_critical("Invalid value type!");
			error = true;
	}

	g_object_unref(client);
	return !error;
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

		if (!set_key(key, val))
			goto exit;

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
	GConfClient* client = gconf_client_get_default();

	// Add server notifications for all keys
	for (int i=1 ; i < argc ; i++) {
		GConfValue *value;
		gconf_client_add_dir(client, argv[i], GCONF_CLIENT_PRELOAD_NONE, NULL);
		gconf_client_notify_add(client, argv[i], on_value_change, NULL, NULL, NULL);
		value = gconf_client_get(client, argv[i], NULL);
		if (value) {
			gconf_value_free (value);
			gconf_client_notify(client, argv[i]);
		} else {
			printf("%s\n", argv[i]);
		}
	}

	g_main_loop_run(loop);

	// Cleanup
	g_object_unref(client);
	g_io_channel_shutdown(inchan,  FALSE, NULL);
	g_io_channel_shutdown(outchan, FALSE, NULL);
	g_io_channel_unref(inchan);
	g_io_channel_unref(outchan);
	g_main_loop_unref(loop);
}
