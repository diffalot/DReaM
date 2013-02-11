OBJS=rights_jni.o rights_xface.o
TARGET=librightsxface.so
LDFLAGS=-shared -L${JAVA_HOME}/jre/lib/i386 -L${JAVA_HOME}/jre/lib/i386/server -lpthread -ljava
CFLAGS=-g -D_DEBUG -I${JAVA_HOME}/include/linux -I${JAVA_HOME}/include

INSTALL_PATH=~/_BUILD

$(TARGET):$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ 
	
$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $<

install: 
	install -D librightsxface.so ${INSTALL_PATH}/lib/librightsxface.so
	install -D rights_xface.h    ${INSTALL_PATH}/include/rights_xface/rights_xface.h
    
clean:
	$(RM) -f *.o *.so $(TARGET)
