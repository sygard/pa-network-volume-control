/***
 *	This file is part of pa-network-volume-control.
 *
 *  Copyright 2010 - 2010 Tor Martin Slåen <tormsl@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <pulse/pulseaudio.h>
#include <pulse/mainloop.h>
#include <pulse/ext-stream-restore.h>

#include "mixer.h"
#include "volumecontrols.h"



static pa_context *context = NULL;
static pa_mainloop *m = NULL;
static pa_mainloop_api *api = NULL;
static pa_io_event* user_event = NULL;

#ifdef DEBUG
static void print_sink_info(const pa_sink_info *i);
static void print_server_info(const pa_server_info *i);
#endif
static void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
static void server_info_callback(pa_context *c, const pa_server_info *i, void* userdata);

static char *default_sink_name = NULL;

static void adjust_volume_up_callback(pa_context *c, int success, void *userdata) {
	assert(c);
	if( !success ) {
		fprintf(stderr, "Could not adjust volume on default sink.");
	}
}

static void adjust_volume_up_handler_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
	/* if end of list */
	if( eol )
		return;

	assert(c);
	assert(i);

	pa_cvolume *new_volume = pa_cvolume_inc( (pa_cvolume*)&i->volume, VOL_INC_DEC_STEP);

	pa_operation_unref(pa_context_set_sink_volume_by_name(context, default_sink_name, new_volume,
				adjust_volume_up_callback, NULL));

	/* TODO: free i? */
}

static void adjust_volume_up() {
	pa_context_get_sink_info_by_name(context, default_sink_name, adjust_volume_up_handler_callback, NULL);
}

static void adjust_volume_dn_callback(pa_context *c, int success, void *userdata) {
	assert(c);
	if( !success ) {
		fprintf(stderr, "Could not adjust volume on default sink.");
	}
}

static void adjust_volume_dn_handler_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
	/* if end of list */
	if( eol )
		return;

	assert(c);
	assert(i);

	pa_cvolume *new_volume = pa_cvolume_dec( (pa_cvolume*) &i->volume, VOL_INC_DEC_STEP);

	pa_operation_unref(pa_context_set_sink_volume_by_name(context, default_sink_name, new_volume,
				adjust_volume_dn_callback, NULL));

	/* TODO: free i? */
}

static void adjust_volume_down() {
	pa_operation_unref(pa_context_get_sink_info_by_name(context, default_sink_name,
				adjust_volume_dn_handler_callback, NULL));
}

static void mute_callback(pa_context *c, int success, void *userdata) {
	assert(c);
}

static void mute() {
	pa_operation_unref(pa_context_set_sink_mute_by_index(context, 0, 1, mute_callback, NULL));
}

static void nomute_callback(pa_context *c, int success, void *userdata) {
	assert(c);
}

static void nomute() {
	pa_operation_unref(pa_context_set_sink_mute_by_index(context, 0, 0, nomute_callback, NULL));
}

static void quit(int ret) {
	assert(api);
	pa_mainloop_quit(m, ret);
	api->quit(api, ret);
}

static void exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata) {
#ifdef DEBUG
	fprintf(stderr, "\nGot exit signal callback.\n");
#endif

	quit(0);
}

int mixer_control(volumetype_t type) {

	switch(type) {
		case VOL_UP:
			adjust_volume_up();
			break;
		case VOL_DN:
			adjust_volume_down();
			break;
		case MUTE:
			mute();
			break;
		case NOMUTE:
			nomute();
			break;
		default:
			/* return error */
			return -1;
	}

	return 0;
}


void mixer_run_mainloop() {
	assert(user_event);
	int ret = 1;
	if( pa_mainloop_run(m, &ret) < 0) {
		fprintf(stderr, "Could not run mainloop.\n");
		return;
	}
}

static void user_callback(pa_mainloop_api *a, pa_io_event *e, int fd, pa_io_event_flags_t f, void *userdata) {
	assert(a = api);
	assert(e);
	assert(e == user_event);
	
	void (*callback)(void) = userdata;

	callback();

	/* TODO: free? */
}

void mixer_subscribe_fd(int fd, void *callback) {
	static int first = 0;
	if( first++) {
		/* TODO: build support for several user-defined callbacks */
		fprintf(stderr, "ERROR: Can only handle one user defined callbak. TODO needed.\n");
		assert(0);
	}
	user_event = api->io_new(api, fd, PA_IO_EVENT_INPUT, user_callback, callback);
}

static void server_info_callback(pa_context *c, const pa_server_info *i, void* userdata) {
	assert(c);
	assert(i);

#ifdef DEBUG
	print_server_info(i);
#endif
	/* Need to store the default sink name in order to be able
	 * to query the PA server. */
	default_sink_name = strdup(i->default_sink_name);

	/* TODO: Need to free i? */
}

#ifdef DEBUG
static void print_server_info(const pa_server_info *i) {
	fprintf(stderr, "\nPulseAudio Server Info\n");
	fprintf(stderr, "  user_name           : %s\n", i->user_name);
	fprintf(stderr, "  host_name           : %s\n", i->host_name);
	fprintf(stderr, "  server_version      : %s\n", i->server_version);
	fprintf(stderr, "  server_name         : %s\n", i->server_name);
	fprintf(stderr, "  default_sink_name   : %s\n", i->default_sink_name);
	fprintf(stderr, "  default_source_name : %s\n", i->default_source_name);
}
#endif

static void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
	if( eol )
		return;
	
	assert(c);
	assert(i);

#ifdef DEBUG
	print_sink_info(i);
#endif

	/* TODO: Need to free i? */
}

#ifdef DEBUG
static void print_sink_info(const pa_sink_info *i) {
	fprintf(stderr, "\nSink info (%d) `%s'\n", i->index, i->name);
	fprintf(stderr, "  description        : %s\n", i->description);
	fprintf(stderr, "  channels (%d)\n", i->volume.channels);
	uint8_t n;
	for(n = 0; n < i->volume.channels; n++) {
		fprintf(stderr, "    volume %d         : %d\n", n, i->volume.values[n] );
	}
	fprintf(stderr, "  muted              : %s\n", i->mute? "true":"false");
	fprintf(stderr, "  driver             : %s\n", i->driver);
	fprintf(stderr, "  n_volume_steps     : %d\n", i->n_volume_steps);
}
#endif

static void context_state_callback(pa_context *c, void *userdata) {
	assert(c);

	switch(pa_context_get_state(c)) {
		case PA_CONTEXT_UNCONNECTED:
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;
		case PA_CONTEXT_READY:
			/* Initiate request to get server info */
			pa_operation_unref(pa_context_get_server_info(context,
						server_info_callback, NULL));

			/* Initiate request to get sink info */
			pa_operation_unref(pa_context_get_sink_info_list(context,
						sink_info_callback, NULL));
			break;
		case PA_CONTEXT_FAILED:
			fprintf(stderr, "Failed to connecto to the PA server..\n");
			quit(0);
			break;
		case PA_CONTEXT_TERMINATED:
#ifdef DEBUG
			fprintf(stderr, "Terminated connection to the PA server.\n");
#endif
			quit(0);
			break;
		default:
			break;
	}

}

int mixer_init() {
	int r;
	/* Create a new mainloop.
	 * This loop will be the main program loop and runs until
	 * program is terminated. If there is a need to listen to other
	 * socket- or filedescriptors, one need to add it to the mainloops
	 * list of watched sockets. This is done by calling mixer_subscribe_fd
	 * with the appropriate parameters. */
	m = pa_mainloop_new();
	assert(m);

	/* Fetch the api of the mainloop */
	api = pa_mainloop_get_api(m);
	assert(api);

	/* Initiate the signal handler of the api */
	r = pa_signal_init(api);
	assert(r == 0);

	/* Subscribe to SIGINT and SIGTERM signals. The function
	 * exit_signal_callback is called when the signals are
	 * caught. */
	pa_signal_new(SIGINT, exit_signal_callback, NULL);
	pa_signal_new(SIGTERM, exit_signal_callback, NULL);

	/* Make a properties struct to tell pulseaudio who we are */
	pa_proplist *proplist = pa_proplist_new();
	pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME,		APPLICATION_NAME);
	pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID,			APPLICATION_ID);
	pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME,	"audio-card");
	pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION,		APPLICATION_VERSION);

	/* Create a new context. This structure is used throughout the application
	 * to communicate with the pulseaudio server */
	//context = pa_context_new_with_proplist(api, NULL, proplist);
	context = pa_context_new(api, APPLICATION_NAME);
	assert(context);

	/* Free resources used by proplist */
	pa_proplist_free(proplist);

	/* Request state callbacks */
	pa_context_set_state_callback(context,context_state_callback, NULL);

	/* Finally, connect to the pulseaudio server */
	if( pa_context_connect(context, NULL, (pa_context_flags_t)0, NULL) < 0 ) {
		fprintf(stderr, "Could not connect to PA context\n.");
		return -1;
	}

	return 0;
}
/** Cleanup resouces used by the mixer. */
void mixer_cleanup() {
	if( context ) {
		pa_context_disconnect(context);
		pa_context_unref(context);
	}

	if( user_event ) {
		assert(api);
		api->io_free(user_event);
	}

	if( m ) {
		pa_signal_done();
		pa_mainloop_free(m);
	}

	if( default_sink_name ) {
		free( default_sink_name );
	}
}
