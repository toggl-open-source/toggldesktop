//
//  TimelineDatasource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineDatasourceDelegate: class {

    func shouldPresentTimeEntryEditor(in view: NSView, timeEntry: TimeEntryViewItem)
    func shouldPresentTimeEntryHover(in view: NSView, timeEntry: TimelineTimeEntry)
    func shouldPresentActivityHover(in view: NSView, activity: TimelineActivity)
    func startNewTimeEntry(at started: TimeInterval, ended: TimeInterval)
}

final class TimelineDatasource: NSObject {

    fileprivate struct Constants {
        static let TimeLabelCellID = NSUserInterfaceItemIdentifier("TimelineTimeLabelCell")
        static let TimeLabelCellXIB = NSNib.Name("TimelineTimeLabelCell")
        static let TimeEntryCellID = NSUserInterfaceItemIdentifier("TimelineTimeEntryCell")
        static let TimeEntryCellXIB = NSNib.Name("TimelineTimeEntryCell")
        static let EmptyTimeEntryCellID = NSUserInterfaceItemIdentifier("TimelineEmptyTimeEntryCell")
        static let EmptyTimeEntryCellXIB = NSNib.Name("TimelineEmptyTimeEntryCell")
        static let ActivityCellID = NSUserInterfaceItemIdentifier("TimelineActivityCell")
        static let ActivityCellXIB = NSNib.Name("TimelineActivityCell")
        static let DividerViewID = NSUserInterfaceItemIdentifier("DividerViewID")
        static let BackgroundViewID = NSUserInterfaceItemIdentifier("BackgroundViewID")
    }

    enum ZoomLevel: Int {
        case x1 = 0 // normal
        case x2
        case x3
        case x4

        // The span represent how long between current TimeLabel -> Next label
        var span: TimeInterval {
            switch self {
            case .x4:
                return 7200.0 // Each 2 hours
            case .x1,
                .x2,
                .x3:
                return 3600 // Each 1 hour
            }
        }

        var nextLevel: ZoomLevel? {
            return ZoomLevel(rawValue: self.rawValue + 1)
        }

        var previousLevel: ZoomLevel? {
            return ZoomLevel(rawValue: self.rawValue - 1)
        }

        // If the zoom is x4, two TE could so close
        // This minimum gap prevents it happends
        var minimumGap: CGFloat {
            return 2.0
        }
    }

    // MARK: Variables

    weak var delegate: TimelineDatasourceDelegate?
    private unowned let collectionView: NSCollectionView
    private let flow: TimelineFlowLayout
    private(set) var timeline: TimelineData?
    private var zoomLevel: ZoomLevel = .x1
    
    // MARK: Init

    init(_ collectionView: NSCollectionView) {
        self.collectionView = collectionView
        self.flow = TimelineFlowLayout()
        super.init()
        flow.flowDelegate = self
        collectionView.delegate = self
        collectionView.dataSource = self
        collectionView.collectionViewLayout = flow
        collectionView.register(NSNib(nibNamed: Constants.TimeLabelCellXIB, bundle: nil), forItemWithIdentifier: Constants.TimeLabelCellID)
        collectionView.register(NSNib(nibNamed: Constants.TimeEntryCellXIB, bundle: nil), forItemWithIdentifier: Constants.TimeEntryCellID)
        collectionView.register(NSNib(nibNamed: Constants.ActivityCellXIB, bundle: nil), forItemWithIdentifier: Constants.ActivityCellID)
        collectionView.register(NSNib(nibNamed: Constants.EmptyTimeEntryCellXIB, bundle: nil), forItemWithIdentifier: Constants.EmptyTimeEntryCellID)
        collectionView.register(TimelineDividerView.self, forSupplementaryViewOfKind: NSCollectionView.elementKindSectionFooter, withIdentifier: Constants.DividerViewID)
        collectionView.register(TimelineBackgroundView.self, forSupplementaryViewOfKind: NSCollectionView.elementKindSectionHeader, withIdentifier: Constants.BackgroundViewID)
    }

    func render(_ timeline: TimelineData) {
        self.timeline = timeline
        flow.currentDate = Date(timeIntervalSince1970: timeline.start)
        collectionView.reloadData()
    }

    func update(_ zoomLevel: ZoomLevel) {
        self.zoomLevel = zoomLevel
        timeline?.render(with: zoomLevel)
        flow.apply(zoomLevel)
        collectionView.reloadData()
        scrollToVisibleItem()
    }

    func scrollToVisibleItem() {
        guard let timeline = timeline else { return }

        // Skip if both are empty
        if timeline.timeEntries.isEmpty && timeline.activities.isEmpty {
            return
        }

        // Get the section should be presented
        var section: TimelineData.Section?
        if !timeline.timeEntries.isEmpty {
            section = .timeEntry
        } else if !timeline.activities.isEmpty {
            section = .activity
        }

        // Scroll to visible item
        if let section = section {
            collectionView.scrollToItems(at: Set<IndexPath>(arrayLiteral: IndexPath(item: 0, section: section.rawValue)),
                                         scrollPosition: [.centeredHorizontally, .centeredVertically])
        }
    }
}

extension TimelineDatasource: NSCollectionViewDataSource, NSCollectionViewDelegateFlowLayout, NSCollectionViewDelegate {

    func numberOfSections(in collectionView: NSCollectionView) -> Int {
        guard let timeline = timeline else { return 0 }
        return timeline.numberOfSections
    }

    func collectionView(_ collectionView: NSCollectionView, numberOfItemsInSection section: Int) -> Int {
        guard let timeline = timeline else { return 0 }
        return timeline.numberOfItems(in: section)
    }

    func collectionView(_ collectionView: NSCollectionView, itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
        guard let timeline = timeline,
            let section = TimelineData.Section(rawValue: indexPath.section),
            let item = timeline.item(at: indexPath) else { return NSCollectionViewItem() }

        switch section {
        case .timeLabel:
            let cell = collectionView.makeItem(withIdentifier: Constants.TimeLabelCellID, for: indexPath) as! TimelineTimeLabelCell
            let chunk = item as! TimelineTimestamp
            cell.render(chunk)
            return cell
        case .timeEntry:
            switch item {
            case let timeEntry as TimelineTimeEntry:
                let cell = collectionView.makeItem(withIdentifier: Constants.TimeEntryCellID, for: indexPath) as! TimelineTimeEntryCell
                cell.delegate = self
                cell.mouseDelegate = self
                cell.config(for: timeEntry)
                return cell
            case let emptyTimeEntry as TimelineBaseTimeEntry:
                let cell = collectionView.makeItem(withIdentifier: Constants.EmptyTimeEntryCellID, for: indexPath) as! TimelineEmptyTimeEntryCell
                cell.config(for: emptyTimeEntry, at: zoomLevel)
                return cell
            default:
                fatalError("We haven't support yet")
            }

        case .activity:
            let cell = collectionView.makeItem(withIdentifier: Constants.ActivityCellID, for: indexPath) as! TimelineActivityCell
            cell.mouseDelegate = self
            let activity = item as! TimelineActivity
            cell.config(for: activity)
            return cell
        }
    }

    func collectionView(_ collectionView: NSCollectionView, viewForSupplementaryElementOfKind kind: NSCollectionView.SupplementaryElementKind, at indexPath: IndexPath) -> NSView {
        guard let section = TimelineData.Section(rawValue: indexPath.section) else { return NSView() }
        if kind == NSCollectionView.elementKindSectionFooter {
            let view = collectionView.makeSupplementaryView(ofKind: kind,
                                                            withIdentifier: Constants.DividerViewID,
                                                            for: indexPath) as! TimelineDividerView
            view.draw(for: section)
            return view
        } else if kind == NSCollectionView.elementKindSectionHeader {
            let view = collectionView.makeSupplementaryView(ofKind: kind,
                                                            withIdentifier: Constants.BackgroundViewID,
                                                            for: indexPath)
            return view
        }
        return NSView()
    }

    func collectionView(_ collectionView: NSCollectionView, didSelectItemsAt indexPaths: Set<IndexPath>) {
        guard let indexPath = indexPaths.first,
            let cell = collectionView.item(at: indexPath),
            let item = timeline?.item(at: indexPath) else { return }
        switch item {
        case let timeEntry as TimelineTimeEntry:
            delegate?.shouldPresentTimeEntryEditor(in: cell.view, timeEntry: timeEntry.timeEntry)
            collectionView.deselectItems(at: indexPaths)
        case let item as TimelineBaseTimeEntry:
            delegate?.startNewTimeEntry(at: item.start, ended: item.end)
            collectionView.deselectItems(at: indexPaths)
        default:
            break
        }
    }
}

// MARK: TimelineFlowLayoutDelegate

extension TimelineDatasource: TimelineFlowLayoutDelegate {

    func timechunkForItem(at indexPath: IndexPath) -> TimeChunk? {
        return timeline?.timechunkForItem(at: indexPath)
    }

    func isEmptyTimeEntry(at indexPath: IndexPath) -> Bool {
        guard let item = timeline?.item(at: indexPath),
            type(of: item) == TimelineBaseTimeEntry.self else { return false }
        return true
    }

    func columnForItem(at indexPath: IndexPath) -> Int {
        guard let item = timeline?.item(at: indexPath) as? TimelineBaseTimeEntry else { return 0 }
        return item.col
    }

    func shouldDrawDetailBubble(at indexPath: IndexPath) -> Bool {
        guard let item = timeline?.item(at: indexPath) as? TimelineBaseTimeEntry else { return false }
        return item.col == 0 && item.group == 1
    }
}

// MARK: TimelineTimeEntryCellDelegate

extension TimelineDatasource: TimelineTimeEntryCellDelegate {

    func timeEntryCellShouldChangeFirstEntryStopTime(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell) {
        timeline?.changeFirstEntryStopTime(at: entry)
    }

    func timeEntryCellShouldChangeLastEntryStartTime(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell) {
        timeline?.changeLastEntryStartTime(at: entry)
    }
}

// MARK: TimelineBaseCellDelegate

extension TimelineDatasource: TimelineBaseCellDelegate {

    func timelineCellMouseDidExited(_ sender: TimelineBaseCell) {}

    func timelineCellMouseDidEntered(_ sender: TimelineBaseCell) {
        switch sender {
        case let timeEntryCell as TimelineTimeEntryCell:
            guard let timeEntry = timeEntryCell.timeEntry else { return }
            delegate?.shouldPresentTimeEntryHover(in: sender.view, timeEntry: timeEntry)
        case let activityCell as TimelineActivityCell:
            guard let activity = activityCell.activity else { return }
            delegate?.shouldPresentActivityHover(in: sender.view, activity: activity)
        default:
            break
        }
    }
}
