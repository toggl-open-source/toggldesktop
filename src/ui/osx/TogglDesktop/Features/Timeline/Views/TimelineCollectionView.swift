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
    func timelineDidStartDragging(withStartTime startTime: TimeInterval, endTime: TimeInterval)
    func timelineDidUpdateDragging(withStartTime startTime: TimeInterval, endTime: TimeInterval)
    func timelineDidEndDragging(withStartTime startTime: TimeInterval, endTime: TimeInterval)
}

/// Main View of the Timline
/// Responsible for handling the Click Action on the cell
/// We don't use didSelection on CollectionView because we couldn't select twice on the selected view
final class TimelineCollectionView: NSCollectionView {

    // MARK: Variables

    weak var timelineDelegate: TimelineCollectionViewDelegate?

    private var isDragCreatingEntry = false
    private var mouseDragStartTime: TimeInterval = 0
    private var mouseDragEndTime: TimeInterval = 0

    private var dragEntryStartTime: TimeInterval {
        min(mouseDragStartTime, mouseDragEndTime)
    }

    private var dragEntryEndTime: TimeInterval {
        max(mouseDragStartTime, mouseDragEndTime)
    }

    // MARK: Mouse Events

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)

        guard isDragToCreateEvent(event) else { return }
        guard let dragStart = timestamp(from: event) else { return }
        mouseDragStartTime = dragStart
        mouseDragEndTime = dragStart + 1
        isDragCreatingEntry = true

        timelineDelegate?.timelineDidStartDragging(withStartTime: dragEntryStartTime,
                                                   endTime: dragEntryEndTime)
    }

    override func mouseDragged(with event: NSEvent) {
        super.mouseDragged(with: event)

        guard isDragCreatingEntry else { return }
        guard let timestamp = timestamp(from: event) else { return }
        mouseDragEndTime = timestamp

        timelineDelegate?.timelineDidUpdateDragging(withStartTime: dragEntryStartTime,
                                                    endTime: dragEntryEndTime)
    }

    override func mouseUp(with event: NSEvent) {
        super.mouseUp(with: event)

        guard isDragCreatingEntry else { return }
        guard let timestamp = timestamp(from: event) else { return }
        mouseDragEndTime = timestamp
        isDragCreatingEntry = false

        timelineDelegate?.timelineDidEndDragging(withStartTime: dragEntryStartTime,
                                                 endTime: dragEntryEndTime)
    }

    // MARK: - Private

    private func isDragToCreateEvent(_ event: NSEvent) -> Bool {
        let clickedPoint = convert(event.locationInWindow, from: nil)
        guard event.clickCount == 1 && indexPathForItem(at: clickedPoint) == nil else {
            return false
        }
        guard let layout = collectionViewLayout as? TimelineFlowLayout else { return false }

        return layout.isInTimeEntrySection(at: clickedPoint)
    }

    private func timestamp(from event: NSEvent) -> TimeInterval? {
        guard let layout = collectionViewLayout as? TimelineFlowLayout else { return nil }
        let location = convert(event.locationInWindow, from: nil)
        return layout.convertTimestamp(from: location)
    }
}
