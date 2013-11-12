APPNAME=Lotech Client
APPPACKAGE=com.ianmaclarty.lotech
APPVERSIONCODE=1
APPVERSIONNAME=1.0
APPORIENTATION=landscape
APPICON=default_icon.png
APPKEYSTORE=XXX
APPKEYALIAS=XXX

-include Make.params

APKPREFIX=$(shell echo $(APPNAME) | tr ' ' '_')
APKFILE=$(APKPREFIX)-$(APPVERSIONNAME).apk

default: install

AndroidManifest.xml: AndroidManifest.xml.in
	cat $< \
		| sed "s/APPNAME/$(APPNAME)/g" \
		| sed "s/APPPACKAGE/$(APPPACKAGE)/g" \
		| sed "s/APPORIENTATION/$(APPORIENTATION)/g" \
		| sed "s/APPVERSIONNAME/$(APPVERSIONNAME)/g" \
		| sed "s/APPVERSIONCODE/$(APPVERSIONCODE)/g" \
		> $@

icons.date:
	./install_icon $(APPICON)
	touch $@

.PHONY: install
install: AndroidManifest.xml icons.date
	cd jni && $(MAKE) clean all
	ant clean debug install

.PHONY: release
release: AndroidManifest.xml icons.date
	rm -f bin/LotechClient-release-unsigned.apk
	rm -f $(APKFILE)
	cd jni && $(MAKE) clean all
	ant clean release
	jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 \
		-keystore $(APPKEYSTORE) bin/LotechClient-release-unsigned.apk $(APPKEYALIAS)
	zipalign -v 4 bin/LotechClient-release-unsigned.apk $(APKFILE)
	jarsigner -verify $(APKFILE)

.PHONY: debug
debug: AndroidManifest.xml icons.date
	cd jni && $(MAKE) clean all
	ant clean debug

.PHONY: clean
clean:
	ant clean
	cd jni && $(MAKE) clean
	rm -f AndroidManifest.xml
	rm -f icons.date
	rm -f `find res/ -name "icon.png"`
