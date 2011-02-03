#ifdef IOS
#ifndef LTIOS_H
#define LTIOS_H

#import <UIKit/UIKit.h>

void ltIOSInit(const char *file);
void ltIOSTeardown();

void ltIOSRender();

void ltIOSTouchesBegan(NSSet *touches);
void ltIOSTouchesMoved(NSSet *touches);
void ltIOSTouchesEnded(NSSet *touches);
void ltIOSTouchesCancelled(NSSet *touches);

#endif
#endif
