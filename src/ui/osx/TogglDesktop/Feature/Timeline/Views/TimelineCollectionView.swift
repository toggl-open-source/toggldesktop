//
//  TimelineCollectionView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 9/9/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineCollectionViewDelegate: class {

    func timelineShouldCreateEmptyEntry(with startTime: TimeInterval)
}

final class TimelineCollectionView: NSCollectionView {

    // MARK: Variables

    weak var timelineDelegate: TimelineCollectionViewDelegate?

    // MARK: View

//    override func mouseDown(with event: NSEvent) {
//        super.mouseDown(with: event)
//        if #available(OSX 10.13, *) {
//            handleMouseClick(with: event)
//        }
//    }
}

// MARK: Private

extension TimelineCollectionView {

    fileprivate func handleMouseClick(with event: NSEvent) {
        guard let flowLayout = collectionViewLayout as? TimelineFlowLayout else { return }

        // Convert to CollectionView coordinator
        let clickedPoint = convert(event.locationInWindow, from: nil)

        // Single click and on empty space
        guard event.clickCount == 1,
            indexPathForItem(at: clickedPoint) == nil else { return }

        // Skip if the click is in Time Label and Activity section
        guard flowLayout.isInTimeEntrySection(at: clickedPoint) else { return }

        // Get timestamp from click point, depend on zoom level and position
        let timestamp = flowLayout.convertTimestamp(from: clickedPoint)
        timelineDelegate?.timelineShouldCreateEmptyEntry(with: timestamp)
    }
}
