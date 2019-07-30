//
//  CalendarCollectionView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 7/23/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol CalendarCollectionViewDelegate: class {

    func calendarCollectionViewDidClicked(at indexPath: IndexPath)
    func calendarCollectionViewDidPress(_ key: Key)
}

final class CalendarCollectionView: NSCollectionView {

    // MARK: Variables

    weak var calendarDelegate: CalendarCollectionViewDelegate?

    // MARK: Overriden

    override func mouseUp(with event: NSEvent) {
        super.mouseUp(with: event)

        if #available(OSX 10.12, *) {
            handleClickAction(with: event)
        }
    }

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)

        if #available(OSX 10.12, *) {} else {
            handleClickAction(with: event)
        }
    }

    override func keyDown(with event: NSEvent) {
        super.keyDown(with: event)

        guard let key = Key(rawValue: Int(event.keyCode)) else { return }
        calendarDelegate?.calendarCollectionViewDidPress(key)
    }

    // MARK: Private

    private func handleClickAction(with event: NSEvent) {
        let clickedPoint = convert(event.locationInWindow, from: nil)
        guard event.clickCount == 1,
            let indexPath = indexPathForItem(at: clickedPoint) else { return }
        calendarDelegate?.calendarCollectionViewDidClicked(at: indexPath)
    }
}
