//
//  ResponderObservable.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 30.10.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

protocol ResponderObservable: AnyObject {
    var observations: (
        becomeResponder: [UUID: () -> Void],
        resignResponder: [UUID: () -> Void]
    ) { get set }

    @discardableResult
    func observeBecomeFirstResponder<T: AnyObject>(_ observer: T, using closure: @escaping () -> Void) -> ObservationToken

    @discardableResult
    func observeResignFirstResponder<T: AnyObject>(_ observer: T, using closure: @escaping () -> Void) -> ObservationToken
}

extension ResponderObservable {
    @discardableResult
    func observeBecomeFirstResponder<T: AnyObject>(_ observer: T, using closure: @escaping () -> Void) -> ObservationToken {
        let id = UUID()

        observations.becomeResponder[id] = { [weak self, weak observer] in
            guard observer != nil else {
                self?.observations.becomeResponder.removeValue(forKey: id)
                return
            }
            closure()
        }

        return ObservationToken { [weak self] in
            self?.observations.becomeResponder.removeValue(forKey: id)
        }
    }

    @discardableResult
    func observeResignFirstResponder<T: AnyObject>(_ observer: T, using closure: @escaping () -> Void) -> ObservationToken {
        let id = UUID()

        observations.resignResponder[id] = { [weak self, weak observer] in
            guard observer != nil else {
                self?.observations.resignResponder.removeValue(forKey: id)
                return
            }
            closure()
        }

        return ObservationToken { [weak self] in
            self?.observations.resignResponder.removeValue(forKey: id)
        }
    }
}
