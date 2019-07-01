//
//  TimelineDatasource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineDatasourceDelegate: class {

    func shouldPresentTimeEntryEditor(in view: NSView, timeEntry: TimelineTimeEntry)
    func shouldPresentTimeEntryHover(in view: NSView, timeEntry: TimelineTimeEntry)
    func shouldDismissTimeEntryHover()
}

final class TimelineDatasource: NSObject {

    fileprivate struct Constants {
        static let TimeLabelCellID = NSUserInterfaceItemIdentifier("TimelineTimeLabelCell")
        static let TimeLabelCellXIB = NSNib.Name("TimelineTimeLabelCell")
        static let TimeEntryCellID = NSUserInterfaceItemIdentifier("TimelineTimeEntryCell")
        static let TimeEntryCellXIB = NSNib.Name("TimelineTimeEntryCell")
        static let ActivityCellID = NSUserInterfaceItemIdentifier("TimelineActivityCell")
        static let ActivityCellXIB = NSNib.Name("TimelineActivityCell")
        static let DividerViewID = NSUserInterfaceItemIdentifier("DividerViewID")
    }

    enum ZoomLevel: Int {
        case x1 = 0 // normal
        case x2
        case x3
        case x4

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
    }

    // MARK: Variables

    weak var delegate: TimelineDatasourceDelegate?
    private unowned let collectionView: NSCollectionView
    private let flow: TimelineFlowLayout
    private var timeline: TimelineData?
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
        collectionView.register(TimelineDividerView.self, forSupplementaryViewOfKind: NSCollectionView.elementKindSectionFooter, withIdentifier: Constants.DividerViewID)
    }

    func render(_ timeline: TimelineData) {
        self.timeline = timeline
        collectionView.reloadData()
    }

    func update(_ zoomLevel: ZoomLevel) {
        self.zoomLevel = zoomLevel
        timeline?.render(with: zoomLevel)
        flow.apply(zoomLevel)
        collectionView.reloadData()
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
            let cell = collectionView.makeItem(withIdentifier: Constants.TimeEntryCellID, for: indexPath) as! TimelineTimeEntryCell
            let timeEntry = item as! TimelineTimeEntry
            cell.delegate = self
            cell.config(for: timeEntry)
            return cell
        case .activity:
            let cell = collectionView.makeItem(withIdentifier: Constants.ActivityCellID, for: indexPath) as! TimelineActivityCell
            let activity = item as! TimelineActivity
            cell.config(for: activity)
            return cell
        }
    }

    func collectionView(_ collectionView: NSCollectionView, viewForSupplementaryElementOfKind kind: NSCollectionView.SupplementaryElementKind, at indexPath: IndexPath) -> NSView {
        guard let section = TimelineData.Section(rawValue: indexPath.section) else { return NSView() }
        let view = collectionView.makeSupplementaryView(ofKind: NSCollectionView.elementKindSectionFooter,
                                                        withIdentifier: Constants.DividerViewID, for: indexPath) as! TimelineDividerView
        view.draw(for: section)
        return view
    }

    func collectionView(_ collectionView: NSCollectionView, didSelectItemsAt indexPaths: Set<IndexPath>) {
        guard let indexPath = indexPaths.first,
            let cell = collectionView.item(at: indexPath),
            let item = timeline?.item(at: indexPath) else { return }
        switch item {
        case let timeEntry as TimelineTimeEntry:
            delegate?.shouldPresentTimeEntryEditor(in: cell.view, timeEntry: timeEntry)
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
}

// MARK: TimelineTimeEntryCellDelegate

extension TimelineDatasource: TimelineTimeEntryCellDelegate {

    func timeEntryCellMouseDidEntered(_ sender: TimelineTimeEntryCell) {
        guard let timeEntry = sender.timeEntry else { return }
        delegate?.shouldPresentTimeEntryHover(in: sender.view, timeEntry: timeEntry)
    }

    func timeEntryCellMouseDidExited(_ sender: TimelineTimeEntryCell) {
        delegate?.shouldDismissTimeEntryHover()
    }
}
