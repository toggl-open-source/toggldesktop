/*******************************************************************************
 * libproxy - A library for proxy configuration
 * Copyright (C) 2006 Nathaniel McCallum <nathaniel@natemccallum.com>
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

namespace libproxy {
	using System;
	using System.Runtime.InteropServices;

	public class ProxyFactory {
		private HandleRef self;

		[DllImport ("proxy")]
		private static extern
			IntPtr px_proxy_factory_new();
		
		[DllImport ("proxy")]
		private static extern 
			IntPtr px_proxy_factory_get_proxies(HandleRef self, string url);
		
		[DllImport ("proxy")]
		private static extern
			void px_proxy_factory_free(HandleRef self);

		public ProxyFactory()
		{
			this.self = new HandleRef(this, px_proxy_factory_new());
		}
		
		public string[] GetProxies(string url)
		{
			int count = 0;
			
			// Get the results
			// TODO: If we call both this function and px_proxy_factory_free()
			// this crashes, figure out why...
			IntPtr array = px_proxy_factory_get_proxies(this.self, url);

			// Count the number of returned strings
			while (Marshal.ReadIntPtr(array, count * IntPtr.Size) != IntPtr.Zero) count++;
			
			// Allocate a correctly sized array
			string[] proxies = new string[count];
			
			// Fill the response array
			for (int i=0 ; i < count ; i++)
			{
				IntPtr p = Marshal.ReadIntPtr(array, i * IntPtr.Size);
				proxies[i] = Marshal.PtrToStringAnsi(p);
				Marshal.FreeCoTaskMem(p); // Free the string
			}
			
			// Free the array itself
			Marshal.FreeCoTaskMem(array);
			
			return proxies;
		}
		
		~ProxyFactory()
		{
			// TODO: See note above...
			px_proxy_factory_free(this.self);
		}
	}
}
