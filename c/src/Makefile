# 6Harmonics Inc
# by Qige: qige@6harmonics.com, qige.zhao@6wilink.com
# 2016.07.13 Preview fwbj
# 2017.02.01 v11-50

APP=gws_abb

all: $(APP)

$(APP): *.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -I. -Iiwinfo -o $@ $^ -L. -liwinfo

clean:
	rm -f $(APP)

