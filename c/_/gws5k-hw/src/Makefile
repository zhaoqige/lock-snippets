# 6Harmonics Inc.
# by Qige: qige@6harmonics.com, qige.zhao@6wilink.com, 2016.07.20

APP=gws-hw

all: $(APP)

$(APP): *.c
	#$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -I. -o $@ $^ -L. -DDEBUG_APP
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -I. -o $@ $^ -L.

clean:
	rm -f $(APP)

