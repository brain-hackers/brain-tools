TARGET=usbstorage.exe
CPP=clarm
CPPFLAGS=/nologo /W3 /O2 /EHsc /QRarch4T /QRinterwork-return \
	/D "ARM" /D "_ARM_" /D "ARMV4I" /D UNDER_CE=400 /D _WIN32_WCE=400 \
	/D "UNICODE" /D "_UNICODE" \
	/I../kncecomm-0.17
LDFLAGS=/NOLOGO /SUBSYSTEM:WINDOWSCE
LIBS=../kncecomm-0.17/kncecomm-0.17.lib
OBJS=\
	usbstorage.obj \
	connwindow.obj
RESOURCE=usbstorage.res

all : $(TARGET)

$(TARGET) : $(OBJS) $(RESOURCE)
	link $(LDFLAGS) /OUT:$@ $(OBJS) $(RESOURCE) $(LIBS)

clean :
	-del $(TARGET) $(OBJS)
