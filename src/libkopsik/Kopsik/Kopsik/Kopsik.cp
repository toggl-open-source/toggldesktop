/*
 *  Kopsik.cp
 *  Kopsik
 *
 *  Created by Tambet Masik on 9/23/13.
 *  Copyright (c) 2013 Tambet Masik. All rights reserved.
 *
 */

#include <iostream>
#include "Kopsik.h"
#include "KopsikPriv.h"

void Kopsik::HelloWorld(const char * s)
{
	 KopsikPriv *theObj = new KopsikPriv;
	 theObj->HelloWorldPriv(s);
	 delete theObj;
};

void KopsikPriv::HelloWorldPriv(const char * s) 
{
	std::cout << s << std::endl;
};

