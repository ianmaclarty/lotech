/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#ifdef LTIOS

int ltIOSMain(int argc, char *argv[]);

void ltIOSInit();
void ltIOSSetViewController(UIViewController *view_c);
void ltIOSTeardown();

void ltIOSResize(int width, int height);

void ltIOSRender();

void ltIOSGarbageCollect();

void ltIOSSaveState();

void ltIOSTouchesBegan(NSSet *touches);
void ltIOSTouchesMoved(NSSet *touches);
void ltIOSTouchesEnded(NSSet *touches);
void ltIOSTouchesCancelled(NSSet *touches);

UIViewController *ltIOSGetViewController();

void ltIOSResignActive();
void ltIOSBecomeActive();

void ltIOSSampleAccelerometer(LTdouble *x, LTdouble *y, LTdouble *z);

#endif
