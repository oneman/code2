#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define KEY_MOD_LSHIFT 0x02

const char ascii_to_hid_key_map[95][2] = {
    {0, KEY_SPACE}, {KEY_MOD_LSHIFT, KEY_1}, {KEY_MOD_LSHIFT, KEY_APOSTROPHE},
    {KEY_MOD_LSHIFT, KEY_3}, {KEY_MOD_LSHIFT, KEY_4}, {KEY_MOD_LSHIFT, KEY_5},
    {KEY_MOD_LSHIFT, KEY_7}, {0, KEY_APOSTROPHE}, {KEY_MOD_LSHIFT, KEY_9},
    {KEY_MOD_LSHIFT, KEY_0}, {KEY_MOD_LSHIFT, KEY_8},
    {KEY_MOD_LSHIFT, KEY_EQUAL},
    {0, KEY_COMMA}, {0, KEY_MINUS}, {0, KEY_DOT}, {0, KEY_SLASH}, {0, KEY_0},
    {0, KEY_1}, {0, KEY_2}, {0, KEY_3}, {0, KEY_4}, {0, KEY_5}, {0, KEY_6},
    {0, KEY_7}, {0, KEY_8}, {0, KEY_9}, {KEY_MOD_LSHIFT, KEY_SEMICOLON},
    {0, KEY_SEMICOLON}, {KEY_MOD_LSHIFT, KEY_COMMA}, {0, KEY_EQUAL},
    {KEY_MOD_LSHIFT, KEY_DOT}, {KEY_MOD_LSHIFT, KEY_SLASH},
    {KEY_MOD_LSHIFT, KEY_2},
    {KEY_MOD_LSHIFT, KEY_A}, {KEY_MOD_LSHIFT, KEY_B}, {KEY_MOD_LSHIFT, KEY_C},
    {KEY_MOD_LSHIFT, KEY_D}, {KEY_MOD_LSHIFT, KEY_E}, {KEY_MOD_LSHIFT, KEY_F},
    {KEY_MOD_LSHIFT, KEY_G}, {KEY_MOD_LSHIFT, KEY_H}, {KEY_MOD_LSHIFT, KEY_I},
    {KEY_MOD_LSHIFT, KEY_J}, {KEY_MOD_LSHIFT, KEY_K}, {KEY_MOD_LSHIFT, KEY_L},
    {KEY_MOD_LSHIFT, KEY_M}, {KEY_MOD_LSHIFT, KEY_N}, {KEY_MOD_LSHIFT, KEY_O},
    {KEY_MOD_LSHIFT, KEY_P}, {KEY_MOD_LSHIFT, KEY_Q}, {KEY_MOD_LSHIFT, KEY_R},
    {KEY_MOD_LSHIFT, KEY_S}, {KEY_MOD_LSHIFT, KEY_T}, {KEY_MOD_LSHIFT, KEY_U},
    {KEY_MOD_LSHIFT, KEY_V}, {KEY_MOD_LSHIFT, KEY_W}, {KEY_MOD_LSHIFT, KEY_X},
    {KEY_MOD_LSHIFT, KEY_Y}, {KEY_MOD_LSHIFT, KEY_Z}, {0, KEY_LEFTBRACE},
    {0, KEY_BACKSLASH}, {0, KEY_RIGHTBRACE}, {KEY_MOD_LSHIFT, KEY_6},
    {KEY_MOD_LSHIFT, KEY_MINUS}, {0, KEY_GRAVE}, {0, KEY_A}, {0, KEY_B},
    {0, KEY_C}, {0, KEY_D}, {0, KEY_E}, {0, KEY_F}, {0, KEY_G}, {0, KEY_H},
    {0, KEY_I}, {0, KEY_J}, {0, KEY_K}, {0, KEY_L}, {0, KEY_M}, {0, KEY_N},
    {0, KEY_O}, {0, KEY_P}, {0, KEY_Q}, {0, KEY_R}, {0, KEY_S}, {0, KEY_T},
    {0, KEY_U}, {0, KEY_V}, {0, KEY_W}, {0, KEY_X}, {0, KEY_Y}, {0, KEY_Z},
    {KEY_MOD_LSHIFT, KEY_LEFTBRACE}, {KEY_MOD_LSHIFT, KEY_BACKSLASH},
    {KEY_MOD_LSHIFT, KEY_RIGHTBRACE}, {KEY_MOD_LSHIFT, KEY_GRAVE},
};

void emit(int fd, int type, int code, int val)
{
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   /* timestamp values below are ignored */
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(fd, &ie, sizeof(ie));
}

int pushy(void)
{
   struct uinput_setup usetup;

   int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);


   /*
    * The ioctls below will enable the device that is about to be
    * created, to pass key events, in this case the space key.
    */
   ioctl(fd, UI_SET_EVBIT, EV_KEY);
   ioctl(fd, UI_SET_KEYBIT, KEY_SPACE);

   memset(&usetup, 0, sizeof(usetup));
   usetup.id.bustype = BUS_USB;
   usetup.id.vendor = 0x1234; /* sample vendor */
   usetup.id.product = 0x5678; /* sample product */
   strcpy(usetup.name, "Example device");

   ioctl(fd, UI_DEV_SETUP, &usetup);
   ioctl(fd, UI_DEV_CREATE);

   /*
    * On UI_DEV_CREATE the kernel will create the device node for this
    * device. We are inserting a pause here so that userspace has time
    * to detect, initialize the new device, and can start listening to
    * the event, otherwise it will not notice the event we are about
    * to send. This pause is only needed in our example code!
    */
   sleep(1);

   /* Key press, report the event, send key release, and report again */
   emit(fd, EV_KEY, KEY_SPACE, 1);
   emit(fd, EV_SYN, SYN_REPORT, 0);
   emit(fd, EV_KEY, KEY_SPACE, 0);
   emit(fd, EV_SYN, SYN_REPORT, 0);

   /*
    * Give userspace some time to read the events before we destroy the
    * device with UI_DEV_DESTROY.
    */
   sleep(1);

   ioctl(fd, UI_DEV_DESTROY);
   close(fd);

   return 0;
}

int pointy(void) {
   struct uinput_setup usetup;
   int i = 50;

   int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

   /* enable mouse button left and relative events */
   ioctl(fd, UI_SET_EVBIT, EV_KEY);
   ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);

   ioctl(fd, UI_SET_EVBIT, EV_REL);
   ioctl(fd, UI_SET_RELBIT, REL_X);
   ioctl(fd, UI_SET_RELBIT, REL_Y);

   memset(&usetup, 0, sizeof(usetup));
   usetup.id.bustype = BUS_USB;
   usetup.id.vendor = 0x1234; /* sample vendor */
   usetup.id.product = 0x5678; /* sample product */
   strcpy(usetup.name, "Example device");

   ioctl(fd, UI_DEV_SETUP, &usetup);
   ioctl(fd, UI_DEV_CREATE);

   /*
    * On UI_DEV_CREATE the kernel will create the device node for this
    * device. We are inserting a pause here so that userspace has time
    * to detect, initialize the new device, and can start listening to
    * the event, otherwise it will not notice the event we are about
    * to send. This pause is only needed in our example code!
    */
   sleep(1);

   /* Move the mouse diagonally, 5 units per axis */
   while (i--) {
      emit(fd, EV_REL, REL_X, 5);
      emit(fd, EV_REL, REL_Y, 5);
      emit(fd, EV_SYN, SYN_REPORT, 0);
      usleep(15000);
   }

   /*
    * Give userspace some time to read the events before we destroy the
    * device with UI_DEV_DESTROY.
    */
   sleep(1);

   ioctl(fd, UI_DEV_DESTROY);
   close(fd);

   return 0;
}

void key() {
  pointy();
  pushy();
}
