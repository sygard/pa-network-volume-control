#ifndef MIXER_H
#define MIXER_H

#define VOL_INC_DEC_STEP 500

enum volumetype {
	VOL_UP,
	VOL_DN,
	MUTE,
	NOMUTE,
};

typedef enum volumetype volumetype_t;


int mixer_control(volumetype_t type);
int mixer_init();
void mixer_cleanup();
void mixer_run_mainloop();
void mixer_subscribe_fd(int fd, void *callback);


#endif
