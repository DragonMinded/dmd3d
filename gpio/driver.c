#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "wiringx.h"

#define COL_DATA 8
#define COL_CLOCK 9
#define COL_LATCH 7
#define ROW_CLOCK 27
#define ROW_DATA 28
#define OUT_ENABLE 29

int main() {
    if (nice(-20) == -1) {
        printf("Couldn't set priority, run me as root!\n");
        return 1;
    }

    printf("Initializing GPIO...\n");
    wiringXSetup("rock4", NULL);

    pinMode(ROW_DATA, PINMODE_OUTPUT);
    digitalWrite(ROW_DATA, LOW);

    pinMode(ROW_CLOCK, PINMODE_OUTPUT);
    digitalWrite(ROW_CLOCK, LOW);

    pinMode(COL_LATCH, PINMODE_OUTPUT);
    digitalWrite(COL_LATCH, LOW);

    pinMode(OUT_ENABLE, PINMODE_OUTPUT);
    digitalWrite(OUT_ENABLE, HIGH);

    pinMode(COL_DATA, PINMODE_OUTPUT);
    digitalWrite(COL_DATA, LOW);

    pinMode(COL_CLOCK, PINMODE_OUTPUT);
    digitalWrite(COL_CLOCK, LOW);

    printf("Displaying a grid pattern...\n");

    unsigned long lastTime = (unsigned long)time(NULL);
    unsigned long frames = 0;
    unsigned long lastFrame = 0;

    while( 1 ) {
        uint8_t data[128*65];
        FILE *fp = fopen("frame.bin", "rb");

        if (fp != NULL) {
            fseek(fp, 0L, SEEK_END);
            size_t sz = ftell(fp);
            if (sz == 128*64) {
                fseek(fp, 0L, SEEK_SET);
                (void)!fread(data, 1, 128*64, fp);
            } else {
                memset(data, 0, 128*64);
            }

            fclose(fp);
        } else {
            memset(data, 0, 128*64);
        }

        // Now, write the last frame so other applications can vsync.
        fp = fopen("lastframe", "wb");
        if (fp != NULL) {
            fwrite(&lastFrame, 1, sizeof(lastFrame), fp);
            fclose(fp);
        }

        // Mark that we've advanced past this frame.
        lastFrame++;

        // We clock one more row than we have in order to make sure the final line isn't overly bright.
        for (int row = 0; row < 65; row++) {
            // First, clock out the column data.
            for (int col = 0; col < 128; col++) {
                digitalWrite(COL_DATA, data[row * 128 + col] ? HIGH : LOW);
                digitalWrite(COL_CLOCK, HIGH);
                digitalWrite(COL_CLOCK, LOW);
            }

            // Make sure we have adequate time from displaying the previous column.
            delayMicroseconds(90);

            // For some reason, the top row flickers a bit, must be a timing issue, but we can patch around
            // it by being tricky with how much we delay displaying the first row after clocking it in.
            if (row == 1) {
                for (int delay = 0; delay < 6; delay++) {
                    delayMicroseconds(50);
                }
            }

            // Now, latch the column.
            digitalWrite(OUT_ENABLE, LOW);
            digitalWrite(COL_LATCH, HIGH);
            digitalWrite(COL_LATCH, LOW);

            // Latch in the row indicator, which we only need to do once. Write a LOW otherwise to keep
            // the timing consistent.
            if (row == 0) {
                digitalWrite(ROW_DATA, HIGH);
            } else {
                digitalWrite(ROW_DATA, LOW);
            }

            // Clock out the next row.
            digitalWrite(ROW_CLOCK, HIGH);
            digitalWrite(ROW_CLOCK, LOW);
            digitalWrite(OUT_ENABLE, HIGH);
        }

        // With the current setup, this delay gives a solid 60fps refresh, so we can
        // stay in refresh lock with anything trying to output video.
        for (int delay = 0; delay < 61; delay++) {
            delayMicroseconds(17);
        }

        frames++;

        unsigned long curTime = (unsigned long)time(NULL);
        if (curTime != lastTime) {
            printf("FPS: %lu\n", frames);
            lastTime = curTime;
            frames = 0;
        }
    }
}
