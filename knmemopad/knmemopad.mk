TARGET=AppMain.exe
CPP=clarm
CPPFLAGS=/nologo /W3 /O2 /EHsc /QRarch4T /QRinterwork-return \
	/D "ARM" /D "_ARM_" /D "ARMV4I" /D UNDER_CE=400 \
	/D "UNICODE" /D "_UNICODE" \
	/I../knceutil-0.12 /I../kncedlg-0.10
LDFLAGS=/NOLOGO /SUBSYSTEM:WINDOWSCE
LIBS=commctrl.lib ../knceutil-0.12/knceutil-0.12.lib \
    ../kncedlg-0.10/kncedlg-0.10.lib
OBJS=\
	knmemopad.obj
RESOURCE=knmemopad.res

all : $(TARGET)

$(TARGET) : $(OBJS) $(RESOURCE)
	link $(LDFLAGS) /OUT:$@ $(OBJS) $(RESOURCE) $(LIBS)

clean :
	-del $(TARGET) $(OBJS)
