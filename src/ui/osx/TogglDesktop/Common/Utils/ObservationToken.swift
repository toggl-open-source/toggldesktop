//
//  ObservationToken.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 30.10.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class ObservationToken {
    private let cancellationClosure: () -> Void

    init(cancellationClosure: @escaping () -> Void) {
        self.cancellationClosure = cancellationClosure
    }

    func cancel() {
        cancellationClosure()
    }
}
