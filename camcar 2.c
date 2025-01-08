
// camcar.c: Updates for Initio Car Project

#include <stdlib.h>
#include <initio.h>
#include <curses.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <pthread.h>
#include <assert.h>
#include "detect_blob.h"

// Constants for behavior
#define DIST_MIN 60
#define DIST_MAX 100
#define MAX_ALIGN_TIME 3000000  // 3 seconds in microseconds

pthread_mutex_t count_mutex; // Mutex to protect thread communication

// Data structure for shared thread communication
struct thread_dat {
    TBlobSearch blob;  // Blob object from the camera
    int blobnr;        // Blob number (tracks new images)
    int bExit;         // Termination flag for the thread
};

void camcar(int argc, char *argv[], struct thread_dat *ptdat) {
    int ch = 0;
    int blobnr = 0;
    TBlobSearch blob;

    while (ch != 'q') {
        int obstacle_L, obstacle_R, obstacle;
        int blobSufficient;
        int carBlobAligned;
        int distance;
        enum { distok, tooclose, toofar } distanceState;
        static unsigned long align_start_time = 0;

        mvprintw(1, 1, "%s: Press 'q' to end program", argv[0]);
        mvprintw(10, 1, "Status: blob(size=%d, halign=%f, blobnr=%u)", blob.size, blob.halign, ptdat->blobnr);

        obstacle_L = (initio_IrLeft() != 0);
        obstacle_R = (initio_IrRight() != 0);
        obstacle = obstacle_L || obstacle_R;

        if (obstacle) {
            mvprintw(3, 1, "State OA: Obstacle Detected! Stopping.");
            initio_DriveForward(0);
        } else {
            pthread_mutex_lock(&count_mutex);
            blob = ptdat->blob;
            pthread_mutex_unlock(&count_mutex);

            blobSufficient = (blob.size > 20);

            if (!blobSufficient) {
                mvprintw(3, 1, "State SB: Searching for blob.");
                if (blobnr < ptdat->blobnr) {
                    blobnr = ptdat->blobnr;
                }
            } else {
                carBlobAligned = (blob.halign >= -0.15 && blob.halign <= 0.15);

                if (!carBlobAligned) {
                    mvprintw(3, 1, "State AB: Aligning to blob.");
                    if (blobnr < ptdat->blobnr) {
                        if (blob.halign < 0) {
                            initio_SpinRight(50);
                        } else {
                            initio_SpinLeft(50);
                        }
                        align_start_time = micros();
                        blobnr = ptdat->blobnr;
                    }
                } else {
                    if (micros() - align_start_time > MAX_ALIGN_TIME) {
                        initio_DriveForward(0);
                        align_start_time = 0;
                    }

                    distance = initio_UsGetDistance();
                    if (distance < DIST_MIN) {
                        distanceState = tooclose;
                    } else if (distance > DIST_MAX) {
                        distanceState = toofar;
                    } else {
                        distanceState = distok;
                    }

                    switch (distanceState) {
                        case toofar:
                            mvprintw(3, 1, "State FB: Moving forward.");
                            initio_DriveForward(50);
                            break;
                        case tooclose:
                            mvprintw(3, 1, "State RB: Moving backward.");
                            initio_DriveBackward(50);
                            break;
                        case distok:
                            mvprintw(3, 1, "State KD: Maintaining distance.");
                            initio_DriveForward(0);
                            break;
                    }
                }
            }
        }

        ch = getch();
        if (ch != ERR) mvprintw(2, 1, "Key code: '%c' (%d)", ch, ch);
        refresh();
    }
}

void *worker(void *p_thread_dat) {
    struct thread_dat *ptdat = (struct thread_dat *) p_thread_dat;
    const char blobColor[3] = {255, 0, 0};
    TBlobSearch blob;

    while (!ptdat->bExit) {
        blob = cameraSearchBlob(blobColor);
        pthread_mutex_lock(&count_mutex);
        ptdat->blob = blob;
        ptdat->blobnr++;
        pthread_mutex_unlock(&count_mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    WINDOW *mainwin = initscr();
    noecho();
    cbreak();
    nodelay(mainwin, TRUE);
    keypad(mainwin, TRUE);

    initio_Init();

    pthread_t cam_thread;
    pthread_attr_t pt_attr;
    struct thread_dat tdat = {0};

    pthread_attr_init(&pt_attr);
    pthread_mutex_init(&count_mutex, NULL);
    pthread_create(&cam_thread, &pt_attr, worker, &tdat);

    camcar(argc, argv, &tdat);

    tdat.bExit = 1;
    pthread_join(cam_thread, NULL);
    pthread_attr_destroy(&pt_attr);
    pthread_mutex_destroy(&count_mutex);

    initio_Cleanup();
    endwin();

    return EXIT_SUCCESS;
}
