int samebytes(u8 *aa, u8 *bb, size_t len) {
  if ((memcmp(aa, bb, len)) == 0) return 1;
  return 0;
}

size_t byte_scan(u8 *dat, size_t sz) {
  size_t n;
  size_t z;
  for (n = 0; n < sz;) {
    u8 b = dat[n];
    z = text_len(dat + n, sz - n);
    if (z > 3) {
      if (n > 0) { printf("\n"); }
      text_scan(dat + n, z);
      if (n + z < sz) { printf("\n"); }
      n += z;
    } else {
      printf("%02X", b);
      n++;
    }
  }
  return sz;
}

size_t data_scan(u8 *dat, size_t sz) {
  if (sz > 1000 * 1000) {
    printf("%lu.%lum\n",  sz/1000000, (sz % 1000000) / 1000);
  } else {
    if (sz < 1000) printf("%lubytes\n", sz);
    if (sz >= 1000) printf("%luk\n",  sz/1000);
  }
  return byte_scan(dat, sz);
}
