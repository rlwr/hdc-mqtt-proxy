AM_CFLAGS = -Wall
LDADD = -lwraclient -lrt -lpthread -lpaho-mqtt3as 

EXTRA_DIST = LICENSE aclocal.m4 ChangeLog autogen.sh

bin_PROGRAMS = hdc_mqtt_proxy
hdc_mqtt_proxy_SOURCES = agent.c agent.h main.c mqtt.c mqtt.h

