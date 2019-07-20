#! /bin/sh -e
cd "src/libbp"
clang -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -c -fPIC blastpit.c -o blastpit.o
clang -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -c -fPIC linkedlist.c -o linkedlist.o
clang -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -c -fPIC message.c -o message.o
clang -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -c -fPIC mqtt.c -o mqtt.o
clang -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -c -fPIC new.c -o new.o
clang -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -c -fPIC parser.c -o parser.o
clang blastpit.o linkedlist.o message.o mqtt.o new.o parser.o -shared -Wl,-soname=libbp.so.1 -o libbp.so.1
clang -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -I. -I../../sub/Unity/src -I../../sub/Unity/extras/fixture/src -c ../../sub/Unity/extras/fixture/src/unity_fixture.c -o unity_fixture.o
clang -DUNITY_OUTPUT_COLOR -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -I. -I../../sub/Unity/src -I../../sub/Unity/extras/fixture/src -c ../../sub/Unity/src/unity.c -o unity.o
clang -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -I. -I../../sub/Unity/src -I../../sub/Unity/extras/fixture/src -c t_blastpit.c -o t_blastpit.o
clang -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -I. -I../../sub/Unity/src -I../../sub/Unity/extras/fixture/src -c t_linkedlist.c -o t_linkedlist.o
clang -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -I. -I../../sub/Unity/src -I../../sub/Unity/extras/fixture/src -c t_message.c -o t_message.o
clang -Wall -Wpedantic -Wextra -Werror -O0 -ggdb3 -I. -I../../sub/Unity/src -I../../sub/Unity/extras/fixture/src -c t_mqtt.c -o t_mqtt.o
clang unity.o unity_fixture.o t_blastpit.o -o t_blastpit_x -lpaho-mqtt3a -lm -Wl,-rpath=. -Wl,-rpath=src/libbp libbp.so.1
clang unity.o unity_fixture.o t_linkedlist.o -o t_linkedlist_x -lpaho-mqtt3a -lm -Wl,-rpath=. -Wl,-rpath=src/libbp libbp.so.1
clang unity.o unity_fixture.o t_message.o -o t_message_x -lpaho-mqtt3a -lm -Wl,-rpath=. -Wl,-rpath=src/libbp libbp.so.1
clang unity.o unity_fixture.o t_mqtt.o -o t_mqtt_x -lpaho-mqtt3a -lm -Wl,-rpath=. -Wl,-rpath=src/libbp libbp.so.1
swig -python blastpy.i
clang -c -fPIC blastpy_wrap.c `python2.7-config --includes`
clang -shared blastpit.o mqtt.o message.o linkedlist.o new.o blastpy_wrap.o -o _blastpy.so -lpaho-mqtt3a
