#define yeap return 1;
#define nope return 0;
#define otherwise

int is_leap_year(u64 year) {
  if ((year % 400) == 0) yeap
  if ((year % 100) == 0) nope
  if ((year % 4) == 0) yeap
  otherwise nope
}
