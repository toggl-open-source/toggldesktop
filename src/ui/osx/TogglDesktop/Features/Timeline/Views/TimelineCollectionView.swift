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
    func timelineDidClick(with startTime: TimeInterval)
}

/// Main View of the Timline
/// Responsible for handling the Click Action on the cell
/// We don't use didSelection on CollectionView because we couldn't select twice on the selected view
final class TimelineCollectionView: NSCollectionView {

    // MARK: Variables

    weak var timelineDelegate: TimelineCollectionViewDelegate?

    private var isDragCreatingEntry = false
    private var isClickCreatingEntry = false
    private var mouseDragStartTime: TimeInterval = 0
    private var mouseDragEndTime: TimeInterval = 0
    private var clickLocation = NSPoint.zero

    private struct Constants {
        static let minDeltaY = CGFloat(3.0)
    }

    private var dragEntryStartTime: TimeInterval {
        min(mouseDragStartTime, mouseDragEndTime)
    }

    private var dragEntryEndTime: TimeInterval {
        max(mouseDragStartTime, mouseDragEndTime)
    }

    // MARK: Mouse Events

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)

        guard isClickOrDragToCreateEvent(event) else { return }
        guard let dragStart = timestamp(from: event) else { return }
        mouseDragStartTime = dragStart
        clickLocation = event.locationInWindow
        isClickCreatingEntry = true
    }

    override func mouseDragged(with event: NSEvent) {
        super.mouseDragged(with: event)

        guard isDragCreatingEntry || isClickCreatingEntry else { return }
        guard let timestamp = timestamp(from: event) else { return }
        mouseDragEndTime = timestamp
        if isClickCreatingEntry && isDrag(event) {
            // dragging detected
            isClickCreatingEntry = false
            isDragCreatingEntry = true
            timelineDelegate?.timelineDidStartDragging(withStartTime: dragEntryStartTime, endTime: dragEntryEndTime)
        } else if isDragCreatingEntry {
            timelineDelegate?.timelineDidUpdateDragging(withStartTime: dragEntryStartTime,
                                                        endTime: dragEntryEndTime)
        }
    }

    override func mouseUp(with event: NSEvent) {
        super.mouseUp(with: event)

        if isClickCreatingEntry {
            timelineDelegate?.timelineDidClick(with: mouseDragStartTime)
            isClickCreatingEntry = false
            return
        }
        guard isDragCreatingEntry else { return }
        guard let timestamp = timestamp(from: event) else { return }
        mouseDragEndTime = timestamp
        isDragCreatingEntry = false

        timelineDelegate?.timelineDidEndDragging(withStartTime: dragEntryStartTime,
                                                 endTime: dragEntryEndTime)
    }

    // MARK: - Private

    private func isClickOrDragToCreateEvent(_ event: NSEvent) -> Bool {
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

    private func isDrag(_ event: NSEvent) -> Bool {
        return abs(clickLocation.y - event.locationInWindow.y) > Constants.minDeltaY
    }
}
