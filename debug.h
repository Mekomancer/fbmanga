#ifdef NDEBUG
#define dprf(...) 
#else
#define dprf(...) std::print(__VA_ARGS__)
#endif
