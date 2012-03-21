/* Copyright (C) 2010 Ian MacLarty */
#ifdef LTIOS
#ifndef LTIOS_H
#define LTIOS_H

#import <UIKit/UIKit.h>

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

#endif
#endif
