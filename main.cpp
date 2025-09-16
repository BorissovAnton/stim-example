#include <iostream>
#include "stim.h"

int main (int argc, char *argv[]) {
  stim::Circuit c;
  c.safe_append_u("H", {0}, {}, "");
  std::cout << c << std::endl;
  return 0;
}
