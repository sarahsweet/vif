clang-3.3 -O3 -I"/usr/lib/llvm-3.3/include" -I"." refgen.cpp -std=c++1y \
    -lstdc++ -lclang-3.3 -lCCfits -lcfitsio -lwcs -llapack -o bin/refgen
