//
//  String+Optional.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/13/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Foundation

extension Optional where Wrapped == String {

	var safeUnwrapped: String {
		switch self {
		case .none:
			return ""
		case .some(let value):
			return value
		}
	}
}
