# 6Harmonics Inc.
# by Qige: qige@6harmonics.com, qige.zhao@6wilink.com, 2016.11.14

APP=gws-tpc

all: $(APP)-38 $(APP)-33

$(APP)-38: *.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -I. -o $@ $^ -L. -D HW_GWS5K -D USE_AGC -D USE_8WATT

$(APP)-33: *.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -I. -o $@ $^ -L. -D HW_GWS5K -D USE_AGC

clean:
	rm -f $(APP)

