RAYLIB=/home/pi/raylib
RAYGUI=/home/pi/raygui
RAYLIB_LIBS= -lm \
			 -lpthread \
			 -lbrcmGLESv2 \
			 -lbrcmEGL \
			 -lvcos \
			 -lvchiq_arm \
			 -lbcm_host \
             -latomic \

CC      = gcc
CFLAGS += -Wall -Wextra -ggdb 
CFLAGS := -I/opt/vc/include -I$(RAYLIB)/src -I$(RAYGUI)/src $(CFLAGS)
LDLIBS := $(LDLIBS) $(RAYLIB_LIBS) -L/opt/vc/lib
OBJS = main.o 

output_video_dir: CFLAGS += -DOUTPUT_VID_DIR=\"OutputVideos\"
main: $(OBJS) $(RAYLIB)/src/libraylib.a $(output_video_dir)
	$(CC) -o $@ $(OBJS) $(RAYLIB)/src/libraylib.a $(LDLIBS)
clean:
	rm *.o
