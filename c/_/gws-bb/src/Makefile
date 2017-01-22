# 6Harmonics Inc
# by Qige: qige@6harmonics.com, qige.zhao@6wilink.com, 2016.07.13

APP=gws-bb

all: $(APP)

$(APP): *.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -I. -Iiwinfo -o $@ $^ -L. -liwinfo

clean:
	rm -f $(APP)

