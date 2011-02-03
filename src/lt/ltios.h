#ifdef IOS
#ifndef LTIOS_H
#define LTIOS_H

#import <UIKit/UIKit.h>

void ltIOSInit(const char *file);
void ltIOSTeardown();

void ltIOSRender();

void ltIOSTouchesBegin(NSSet *touches);
void ltLuaTouchesMove(NSSet *touches);
void ltLuaTouchesEnd(NSSet *touches);

#endif
#endif
