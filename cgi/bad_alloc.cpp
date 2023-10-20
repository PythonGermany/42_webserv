// before main

bool randAllocFail;

void *operator new(size_t size) throw(std::bad_alloc) {
  if (randAllocFail && rand() % 100 == 0) {
    std::cerr << BRIGHT_YELLOW << "malloc(): ENOMEM" << RESET << std::endl;
    throw std::bad_alloc();
  }
  void *p = malloc(size);
  if (!p) throw std::bad_alloc();
  return p;
};

void operator delete(void *p) throw() { free(p); }

void handler(int) {
  randAllocFail = !randAllocFail;
  std::cerr << "throw std::bad_alloc() randomdly: " << randAllocFail
            << std::endl;
}

// beginning of main
{
  randAllocFail = false;
  unsigned int seed = 1697821214;
  std::ofstream debug("seed.txt");
  debug << "seed: " << seed << std::endl;
  srand(seed);
  signal(SIGQUIT, handler);
}

// position where it should start to fail
randAllocFail = true