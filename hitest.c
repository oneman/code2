#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char **argv) {
  int fd;
  int i, res, desc_size = 0;
  char buf[256];
  struct hidraw_report_descriptor rpt_desc;
  struct hidraw_devinfo info;
  char device[16];
  int device_num = 0;
  for (;;) {
    memset(device, 0, 16);
    snprintf(device, 16, "/dev/hidraw%d", device_num++);
    memset(&rpt_desc, 0x0, sizeof(rpt_desc));
    memset(&info, 0x0, sizeof(info));
    memset(buf, 0x0, sizeof(buf));
    if ((fd = open(device, O_RDWR)) < 0) { break; }
    res = ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
	if (res < 0)
		perror("HIDIOCGRDESCSIZE");
	else
		printf("Report Descriptor Size: %d\n", desc_size);
	/* Get Report Descriptor */
	rpt_desc.size = desc_size;
	res = ioctl(fd, HIDIOCGRDESC, &rpt_desc);
	if (res < 0) {
		perror("HIDIOCGRDESC");
	} else {
		printf("Report Descriptor:\n");
		for (i = 0; i < rpt_desc.size; i++)
			printf("%hhx ", rpt_desc.value[i]);
		puts("\n");
	}
	/* Get Raw Name */
	res = ioctl(fd, HIDIOCGRAWNAME(256), buf);
	if (res < 0)
		perror("HIDIOCGRAWNAME");
	else
		printf("Raw Name: %s\n", buf);
    if (rpt_desc.value[3] == 2) { printf("its a mouse\n\n"); }
    if (rpt_desc.value[3] == 7) { printf("its a keyboard\n\n"); }
  }
}
