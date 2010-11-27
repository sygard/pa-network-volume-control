#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <assert.h>

#include "volumecontrols.h"
#include "link.h"
#include "mixer.h"

static void print_help(int argc, char **argv);
static void cleanup(int sig);


/** Function called from the link layer when a message is received.
 * This function will parse the message and decive what to do with the
 * content. */
void link_callback(char *remote_host, char *msg, int msglen) {
	/* We should only get one byte at a time. */
	if( msglen != 1 ) {
		return;
	}

	switch(*msg) {
		case '+':
			mixer_control(VOL_UP);
			break;
		case '-':
			mixer_control(VOL_DN);
			break;
		case 'M':
			mixer_control(MUTE);
			break;
		case 'm':
			mixer_control(NOMUTE);
			break;
		default:
			return;
	}
}


int main(int argc, char **argv) {
	uint16_t port;

	if( argc != 2 ) {
		print_help(argc, argv);
		exit(-1);
	}

	port = atoi(argv[1]);

	/** Initialize the mixer. This function will open a connection
	 * to the PulseAudio server running on localhost. */
	if( mixer_init() < 0 )
		exit(-1);

	/** Initialize the link layer with the port specified in the
	 * program argument. */
	if( init_link(port) < 0 ) {
		mixer_cleanup();
		exit(-1);
	}
	
	/* Tell the link layer to call link_callback
	 * when a message is received. */
	link_subscribe_receive(link_callback);
	
	/* Tell the mixer to handle interrupts from the
	 * socket in the link layer. Here we pass the function
	 * link_read exported by the link layer which will
	 * be called back from the mixer. The link layer will
	 * then call the subscribed method above. */
	mixer_subscribe_fd(link_get_fd(), link_read);

	/* Run the mixers (PulseAudios) main loop.
	 * This will run until the program receives a
	 * SIGINT or a SIGTERM and the returns here. */
	mixer_run_mainloop();

	/* Cleanup resources */
	cleanup(0);

	return 0;
}


static void cleanup(int ret) {
	cleanup_link();
	mixer_cleanup();
}

static void print_help(int argc, char **argv) {
	fprintf(stderr, "USAGE: %s <port>\n", argv[0]);

}
