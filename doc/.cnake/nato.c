char otan(char *buf, u64 sz) {
  if (buf == NULL) exit(1);
  if (sz < 4) exit(1);
  if (sz > 8) exit(1);
  if (sz == 4) {
    if (memcmp("echo", buf, 4) == 0) return 'e';
    if (memcmp("golf", buf, 4) == 0) return 'g';
    if (memcmp("kilo", buf, 4) == 0) return 'k';
    if (memcmp("lima", buf, 4) == 0) return 'l';
    if (memcmp("mike", buf, 4) == 0) return 'm';
    if (memcmp("papa", buf, 4) == 0) return 'p';
    if (memcmp("xray", buf, 4) == 0) return 'x';
    if (memcmp("zulu", buf, 4) == 0) return 'z';
  }
  if (sz == 5) {
    if (memcmp("alpha", buf, 5) == 0) return 'a';
    if (memcmp("bravo", buf, 5) == 0) return 'b';
    if (memcmp("delta", buf, 5) == 0) return 'd';
    if (memcmp("hotel", buf, 5) == 0) return 'h';
    if (memcmp("india", buf, 5) == 0) return 'i';
    if (memcmp("oscar", buf, 5) == 0) return 'o';
    if (memcmp("romeo", buf, 5) == 0) return 'r';
    if (memcmp("tango", buf, 5) == 0) return 't';
  }
  if (sz == 6) {
    if (memcmp("quebec", buf, 6) == 0) return 'q';
    if (memcmp("sierra", buf, 6) == 0) return 's';
    if (memcmp("victor", buf, 6) == 0) return 'v';
    if (memcmp("yankee", buf, 6) == 0) return 'y';
  }
  if (sz == 7) {
    if (memcmp("juliett", buf, 6) == 0) return 'j';
    if (memcmp("charlie", buf, 7) == 0) return 'c';
    if (memcmp("foxtrot", buf, 7) == 0) return 'f';
    if (memcmp("uniform", buf, 7) == 0) return 'u';
    if (memcmp("whiskey", buf, 7) == 0) return 'w';
  }
  if (sz == 8) {
    if (memcmp("november", buf, 8) == 0) return 'n';
  }
  exit(1);
  return '0';
}

u8 word_digit(char *buf, u64 sz) {
  if (buf == NULL) exit(1);
  if (sz < 3) exit(1);
  if (sz > 5) exit(1);
  if (sz == 3) {
    if (memcmp("one", buf, 3) == 0) return 1;
    if (memcmp("two", buf, 3) == 0) return 2;
    if (memcmp("six", buf, 3) == 0) return 6;
  }
  if (sz == 4) {
    if (memcmp("zero", buf, 4) == 0) return 0;
    if (memcmp("four", buf, 4) == 0) return 4;
    if (memcmp("five", buf, 4) == 0) return 5;
    if (memcmp("nine", buf, 4) == 0) return 9;
  }
  if (sz == 5) {
    if (memcmp("three", buf, 5) == 0) return 3;
    if (memcmp("seven", buf, 5) == 0) return 7;
    if (memcmp("eight", buf, 5) == 0) return 8;
  }
  exit(1);
}

char *digit_word(u8 d) {
  if (d > 9) exit(1);
  if (d == 0) { return("zero"); }
  if (d == 1) { return("one"); }
  if (d == 2) { return("two"); }
  if (d == 3) { return("three"); }
  if (d == 4) { return("four"); }
  if (d == 5) { return("five"); }
  if (d == 6) { return("six"); }
  if (d == 7) { return("seven"); }
  if (d == 8) { return("eight"); }
  if (d == 9) { return("nine"); }
  exit(1);
}

char *nato(char letter) {
  if ((letter == 'a') || (letter == 'A')) return "alpha";
  if ((letter == 'b') || (letter == 'B')) return "bravo";
  if ((letter == 'c') || (letter == 'C')) return "charlie";
  if ((letter == 'd') || (letter == 'D')) return "delta";
  if ((letter == 'e') || (letter == 'E')) return "echo";
  if ((letter == 'f') || (letter == 'F')) return "foxtrot";
  if ((letter == 'g') || (letter == 'G')) return "golf";
  if ((letter == 'h') || (letter == 'H')) return "hotel";
  if ((letter == 'i') || (letter == 'I')) return "india";
  if ((letter == 'j') || (letter == 'J')) return "juliett";
  if ((letter == 'k') || (letter == 'K')) return "kilo";
  if ((letter == 'l') || (letter == 'L')) return "lima";
  if ((letter == 'm') || (letter == 'M')) return "mike";
  if ((letter == 'n') || (letter == 'N')) return "november";
  if ((letter == 'o') || (letter == 'O')) return "oscar";
  if ((letter == 'p') || (letter == 'P')) return "papa";
  if ((letter == 'q') || (letter == 'Q')) return "quebec";
  if ((letter == 'r') || (letter == 'R')) return "romeo";
  if ((letter == 's') || (letter == 'S')) return "sierra";
  if ((letter == 't') || (letter == 'T')) return "tango";
  if ((letter == 'u') || (letter == 'U')) return "uniform";
  if ((letter == 'v') || (letter == 'V')) return "victor";
  if ((letter == 'w') || (letter == 'W')) return "whiskey";
  if ((letter == 'x') || (letter == 'X')) return "xray";
  if ((letter == 'y') || (letter == 'Y')) return "yankee";
  if ((letter == 'z') || (letter == 'Z')) return "zulu";
  return "";
}
