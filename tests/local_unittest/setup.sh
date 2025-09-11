time g++ -Wall -Wextra -Werror -O3 -std=c++17 -pthread -c \
    -I"googletest/googletest/include" -I"googletest/googletest" \
    -I"googletest/googlemock/include" -I"googletest/googlemock" \
    googletest/googletest/src/gtest-all.cc \
    googletest/googletest/src/gtest_main.cc \
    googletest/googlemock/src/gmock-all.cc \
    googletest/googlemock/src/gmock_main.cc

# move all of the object files just created to a "bin" dir
mkdir -p bin
mv -t bin gtest-all.o gtest_main.o gmock-all.o gmock_main.o

# Use the `ar` "archive" utility to create the *.a static library archive files
# from the 4 object files above
time ar -rv bin/libgtest.a bin/gtest-all.o
time ar -rv bin/libgtest_main.a bin/gtest_main.o
time ar -rv bin/libgmock.a bin/gmock-all.o
time ar -rv bin/libgmock_main.a bin/gmock_main.o