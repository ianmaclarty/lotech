all: libcurl.a

C_FILES:=	\
	src/base64.c  \
	src/bundles.c  \
	src/conncache.c  \
	src/connect.c  \
	src/curl_addrinfo.c  \
	src/curl_rand.c  \
	src/easy.c  \
	src/escape.c  \
	src/fileinfo.c  \
	src/formdata.c  \
	src/getenv.c  \
	src/getinfo.c  \
	src/hash.c  \
	src/hostip4.c  \
	src/hostip6.c  \
	src/hostip.c  \
	src/hostsyn.c  \
	src/http_chunks.c  \
	src/http.c  \
	src/if2ip.c  \
	src/inet_ntop.c \
	src/inet_pton.c \
	src/llist.c  \
	src/mprintf.c  \
	src/multi.c  \
	src/netrc.c  \
	src/nonblock.c  \
	src/parsedate.c  \
	src/pipeline.c  \
	src/progress.c  \
	src/rawstr.c  \
	src/select.c  \
	src/sendf.c  \
	src/share.c  \
	src/slist.c  \
	src/speedcheck.c  \
	src/splay.c  \
	src/sslgen.c  \
	src/strequal.c  \
	src/strerror.c  \
	src/strtok.c  \
	src/timeval.c  \
	src/transfer.c  \
	src/url.c  \
	src/warnless.c  \
	src/wildcard.c

O_FILES:=$(patsubst %.c,%.o,$(C_FILES))

ifndef LTCC
LTCC=gcc
endif

GCC_FLAGS=-Iinclude -I. $(LTCFLAGS) $(TARGET_FLAGS) -DCURL_STATICLIB
GCC=$(CROSS)$(LTCC)
AR=$(CROSS)ar rcus

$(O_FILES): %.o: %.c
	@echo CC $@
	@$(GCC) $(GCC_FLAGS) -c $< -o $@

libcurl.a: $(O_FILES)
	@echo AR $@
	@$(AR) $@ $^

.PHONY: clean
clean:
	rm -f src/*.o *.a
