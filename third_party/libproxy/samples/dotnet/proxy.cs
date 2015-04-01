/*******************************************************************************
 * libproxy - A library for proxy configuration
 * Copyright (C) 2009 Dominique Leuenberger <dominique@leuenberger.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 ******************************************************************************/

using System;
using libproxy;

public class proxy {
    public static int Main (string[] args)
    {
        if (args.Length > 0) {
            ProxyFactory pf = new ProxyFactory();
            string[] Proxies = pf.GetProxies(args[0]);
            Console.WriteLine(Proxies[0]);
            pf = null;
            return 0;
        } else {
            Console.WriteLine("Please start the program with one parameter");
            Console.WriteLine("Example: proxy.exe http://libproxy.googlecode.com/");
            return 1;
        }
    }
}
